#include "TimeStepAction.hh"
#include "EventAction.hh"
#include "DetectorConstruction.hh"

#include "G4Scheduler.hh"
#include "G4ITTrackHolder.hh"
#include "G4Track.hh"
#include "G4Molecule.hh"
#include "G4OH.hh"
#include "G4Electron_aq.hh"
#include "G4H2O2.hh"
#include "G4AnalysisManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"

#include <cmath>

TimeStepAction::TimeStepAction(EventAction* eventAction, const DetectorConstruction* detector)
  : G4UserTimeStepAction(),
    fEventAction(eventAction),
    fDetector(detector)
{
  // Ventanas logarítmicas de muestreo (en picosegundos) desde 1 ps hasta 1 us (1e6 ps).
  // Nota: G4EmDNAChemistry_option3 emplea por defecto el integrador IRT (Independent Reaction Times),
  // por lo que no se requiere fijar AddTimeStep manual.
  fTimeCheckpointsPs = {
    1.0, 2.0, 5.0,
    10.0, 20.0, 50.0,
    100.0, 200.0, 500.0,
    1.0e3, 2.0e3, 5.0e3,
    1.0e4, 2.0e4, 5.0e4,
    1.0e5, 2.0e5, 5.0e5,
    1.0e6
  };
}

TimeStepAction::TimeStepAction(const TimeStepAction& other)
  : G4UserTimeStepAction(other),
    fEventAction(other.fEventAction),
    fDetector(other.fDetector),
    fTimeCheckpointsPs(other.fTimeCheckpointsPs),
    fNextCheckpointIdx(0)
{}

void TimeStepAction::StartProcessing()
{
  // Reiniciar el índice de muestreo temporal al comenzar la fase química de cada evento (t = 1 ps)
  fNextCheckpointIdx = 0;
}

void TimeStepAction::UserPostTimeStepAction()
{
  G4double currentTimePs = G4Scheduler::Instance()->GetGlobalTime() / picosecond;

  // En t = 1.0 ps exacto, G4Scheduler aún transfiere los radicales ·OH de la etapa pre-química
  // a GetMainList(); registramos el punto de 1.0 ps inmediatamente después de dicha inserción.
  if (fNextCheckpointIdx == 0 && currentTimePs <= 1.0) {
    return;
  }

  while (fNextCheckpointIdx < fTimeCheckpointsPs.size() &&
         currentTimePs >= fTimeCheckpointsPs[fNextCheckpointIdx]) {
    RecordROSYieldsAtTime(fTimeCheckpointsPs[fNextCheckpointIdx]);
    ++fNextCheckpointIdx;
  }
}

void TimeStepAction::EndProcessing()
{
  // Asegurar registro del último punto temporal en 1 microsegundo (1e6 ps) si no se alcanzó antes
  while (fNextCheckpointIdx < fTimeCheckpointsPs.size()) {
    RecordROSYieldsAtTime(fTimeCheckpointsPs[fNextCheckpointIdx]);
    ++fNextCheckpointIdx;
  }
}

void TimeStepAction::RecordROSYieldsAtTime(G4double currentTimePs)
{
  if (!fEventAction || !fDetector) return;

  G4double totalEdepEv = fEventAction->GetTotalCellEdep() / eV;
  if (totalEdepEv <= 0.0) {
    return;
  }

  G4int nOH          = 0;
  G4int nEaq         = 0;
  G4int nH2O2        = 0;
  G4int nOHInNucleus = 0;

  // Acumulador ponderado de supervivencia de ·OH frente a captura antioxidante (scavenging)
  G4double effectiveOHSum = 0.0;
  G4double timeSeconds    = currentTimePs * 1.0e-12;

  // Constantes de captura de primer orden (k_scav = k_bimol * [Scavenger]) para radical ·OH:
  // - Cápsula de GXM (enlaces glucosídicos de manosa/xilosa/ácido glucurónico): ~2.5e7 s^-1
  // - Pared celular con Melanina (grupos indol-quinona y quitosano): ~8.0e8 s^-1 * (fMelanin / 0.10)
  // - Citoplasma basal (glutatión / tioles celulares): ~1.5e7 s^-1
  const G4double kScavCapsule = 2.5e7;
  const G4double kScavWall    = 8.0e8 * (fDetector->GetMelaninMassFraction() / 0.10);
  const G4double kScavBasal   = 1.5e7;

  G4TrackManyList* mainTrackList = G4ITTrackHolder::Instance()->GetMainList();
  if (mainTrackList) {
    for (G4Track* track : *mainTrackList) {
      if (!track) continue;

      const G4VPhysicalVolume* physVol = track->GetVolume();
      const G4LogicalVolume* logVol = physVol ? physVol->GetLogicalVolume() : nullptr;

      // Contabilizar únicamente las especies químicas (ROS) dentro de los compartimentos celulares
      const G4bool isInCell = (logVol == fDetector->GetOuterLayerLV()   ||
                               logVol == fDetector->GetWallOrCortexLV() ||
                               logVol == fDetector->GetCytoplasmLV()    ||
                               logVol == fDetector->GetNucleusLV());
      if (!isInCell) {
        continue;
      }

      const G4ParticleDefinition* partDef = track->GetDefinition();
      if (partDef == G4OH::Definition()) {
        ++nOH;

        if (logVol == fDetector->GetNucleusLV()) {
          ++nOHInNucleus;
        }

        // Evaluar probabilidad de supervivencia de ·OH según el compartimento bioquímico
        G4double kLocal = kScavBasal;
        if (fDetector->GetCellType() == CellType::kCryptococcusStandard) {
          if (logVol == fDetector->GetOuterLayerLV()) {
            kLocal = kScavCapsule;
          } else if (logVol == fDetector->GetWallOrCortexLV()) {
            kLocal = kScavWall;
          }
        }
        effectiveOHSum += std::exp(-kLocal * timeSeconds);

      } else if (partDef == G4Electron_aq::Definition()) {
        ++nEaq;
      } else if (partDef == G4H2O2::Definition()) {
        ++nH2O2;
      }
    }
  }

  // Cálculo de G-values: número de moléculas por cada 100 eV de energía depositada
  G4double norm100eV     = totalEdepEv / 100.0;
  G4double gOH_raw       = static_cast<G4double>(nOH)   / norm100eV;
  G4double gEaq_raw      = static_cast<G4double>(nEaq)  / norm100eV;
  G4double gH2O2_raw     = static_cast<G4double>(nH2O2) / norm100eV;
  G4double gOH_effective = effectiveOHSum / norm100eV;

  // Exportar fila a Ntuple 1 (ROS_Kinetics_GValues) en formato CSV
  auto* analysisManager = G4AnalysisManager::Instance();
  analysisManager->FillNtupleIColumn(1, 0, fEventAction->GetCurrentEventID());
  analysisManager->FillNtupleSColumn(1, 1, fDetector->GetCellTypeName());
  analysisManager->FillNtupleDColumn(1, 2, currentTimePs);
  analysisManager->FillNtupleDColumn(1, 3, totalEdepEv);
  analysisManager->FillNtupleIColumn(1, 4, nOH);
  analysisManager->FillNtupleIColumn(1, 5, nEaq);
  analysisManager->FillNtupleIColumn(1, 6, nH2O2);
  analysisManager->FillNtupleDColumn(1, 7, gOH_raw);
  analysisManager->FillNtupleDColumn(1, 8, gEaq_raw);
  analysisManager->FillNtupleDColumn(1, 9, gH2O2_raw);
  analysisManager->FillNtupleDColumn(1, 10, gOH_effective);
  analysisManager->FillNtupleIColumn(1, 11, nOHInNucleus);
  analysisManager->AddNtupleRow(1);
}

