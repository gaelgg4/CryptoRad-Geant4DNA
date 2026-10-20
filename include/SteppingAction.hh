#ifndef SteppingAction_h
#define SteppingAction_h 1

#include "G4UserSteppingAction.hh"
#include "globals.hh"

class EventAction;
class DetectorConstruction;
class G4Step;

/**
 * @class SteppingAction
 * @brief Intercepta cada paso de transporte físico en Geant4-DNA y asigna la
 *        energía depositada (Edep) al compartimento celular correspondiente:
 *        - OuterLayer_LV (Cápsula de GXM en C. neoformans o Glucocáliz en humana)
 *        - WallOrCortex_LV (Pared con quitosano/melanina o Complejo Membrana-Corteza)
 *        - Cytoplasm_LV (Citoplasma)
 *        - Nucleus_LV (Núcleo 1C haploide o 2C diploide)
 */
class SteppingAction : public G4UserSteppingAction
{
public:
  SteppingAction(EventAction* eventAction, const DetectorConstruction* detector);
  ~SteppingAction() override = default;

  void UserSteppingAction(const G4Step* step) override;

private:
  EventAction*                fEventAction{nullptr};
  const DetectorConstruction* fDetector{nullptr};
};

#endif

