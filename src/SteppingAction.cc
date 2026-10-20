#include "SteppingAction.hh"
#include "EventAction.hh"
#include "DetectorConstruction.hh"

#include "G4Step.hh"
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"
#include "G4StepPoint.hh"

SteppingAction::SteppingAction(EventAction* eventAction, const DetectorConstruction* detector)
  : G4UserSteppingAction(),
    fEventAction(eventAction),
    fDetector(detector)
{}

void SteppingAction::UserSteppingAction(const G4Step* step)
{
  G4double edep = step->GetTotalEnergyDeposit();
  if (edep <= 0.0 || !fEventAction || !fDetector) {
    return;
  }

  const G4StepPoint* prePoint = step->GetPreStepPoint();
  if (!prePoint) return;

  const G4VPhysicalVolume* physVol = prePoint->GetTouchableHandle()->GetVolume();
  if (!physVol) return;

  const G4LogicalVolume* logVol = physVol->GetLogicalVolume();

  // Clasificar la deposición de energía según el volumen lógico atravesado
  if (logVol == fDetector->GetOuterLayerLV()) {
    fEventAction->AddEdepOuterLayer(edep);
  } else if (logVol == fDetector->GetWallOrCortexLV()) {
    fEventAction->AddEdepWallOrCortex(edep);
  } else if (logVol == fDetector->GetCytoplasmLV()) {
    fEventAction->AddEdepCytoplasm(edep);
  } else if (logVol == fDetector->GetNucleusLV()) {
    fEventAction->AddEdepNucleus(edep);
  }
}

