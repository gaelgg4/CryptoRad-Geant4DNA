#include "EventAction.hh"
#include "RunAction.hh"
#include "DetectorConstruction.hh"

#include "G4Event.hh"
#include "G4AnalysisManager.hh"
#include "G4SystemOfUnits.hh"

EventAction::EventAction(RunAction* runAction, const DetectorConstruction* detector)
  : G4UserEventAction(),
    fRunAction(runAction),
    fDetector(detector)
{}

void EventAction::BeginOfEventAction(const G4Event* event)
{
  fCurrentEventID = event->GetEventID();
  fEdepOuter      = 0.0;
  fEdepWall       = 0.0;
  fEdepCyto       = 0.0;
  fEdepNucleus    = 0.0;
}

void EventAction::EndOfEventAction(const G4Event* event)
{
  // 1. Acumular energía en RunAction para el resumen global de la corrida
  if (fRunAction) {
    fRunAction->AddEventEdep(fEdepOuter, fEdepWall, fEdepCyto, fEdepNucleus);
  }

  if (!fDetector) return;

  // 2. Obtener masas netas de cada capa para calcular Dosis = Edep / Masa en Gray (J/kg)
  G4double massOuter   = fDetector->GetOuterLayerMass();
  G4double massWall    = fDetector->GetWallOrCortexMass();
  G4double massCyto    = fDetector->GetCytoplasmMass();
  G4double massNucleus = fDetector->GetNucleusMass();

  G4double doseOuterGy   = (massOuter > 0.)   ? (fEdepOuter / massOuter) / gray     : 0.0;
  G4double doseWallGy    = (massWall > 0.)    ? (fEdepWall / massWall) / gray       : 0.0;
  G4double doseCytoGy    = (massCyto > 0.)    ? (fEdepCyto / massCyto) / gray       : 0.0;
  G4double doseNucleusGy = (massNucleus > 0.) ? (fEdepNucleus / massNucleus) / gray : 0.0;

  // 3. Registrar fila en Ntuple 0 (LayerMicrodosimetry) del CSV
  auto* analysisManager = G4AnalysisManager::Instance();
  G4int eventID = event->GetEventID();

  analysisManager->FillNtupleIColumn(0, 0, eventID);
  analysisManager->FillNtupleSColumn(0, 1, fDetector->GetCellTypeName());
  analysisManager->FillNtupleDColumn(0, 2, fEdepOuter / eV);
  analysisManager->FillNtupleDColumn(0, 3, fEdepWall / eV);
  analysisManager->FillNtupleDColumn(0, 4, fEdepCyto / eV);
  analysisManager->FillNtupleDColumn(0, 5, fEdepNucleus / eV);
  analysisManager->FillNtupleDColumn(0, 6, doseOuterGy);
  analysisManager->FillNtupleDColumn(0, 7, doseWallGy);
  analysisManager->FillNtupleDColumn(0, 8, doseCytoGy);
  analysisManager->FillNtupleDColumn(0, 9, doseNucleusGy);
  analysisManager->AddNtupleRow(0);
}

