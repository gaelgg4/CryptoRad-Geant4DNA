#include "StackingAction.hh"

#include "G4StackManager.hh"
#include "G4DNAChemistryManager.hh"

void StackingAction::NewStage()
{
  // Cuando ya no quedan partículas físicas por transportar en la pila del evento,
  // lanzar la disociación pre-química y el transporte browniano/reacciones de ROS (1 ps a 1 us)
  if (stackManager->GetNTotalTrack() == 0) {
    G4DNAChemistryManager::Instance()->Run();
  }
}

