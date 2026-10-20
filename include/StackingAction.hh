#ifndef StackingAction_h
#define StackingAction_h 1

#include "G4UserStackingAction.hh"

/**
 * @class StackingAction
 * @brief Detecta la finalización de todas las trazas físicas de un evento
 *        (stackManager->GetNTotalTrack() == 0) e inicia automáticamente
 *        la etapa pre-química y química de Geant4-DNA (G4DNAChemistryManager::Instance()->Run()).
 */
class StackingAction : public G4UserStackingAction
{
public:
  StackingAction() = default;
  ~StackingAction() override = default;

  void NewStage() override;
};

#endif

