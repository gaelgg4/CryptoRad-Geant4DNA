#ifndef EventAction_h
#define EventAction_h 1

#include "G4UserEventAction.hh"
#include "globals.hh"

class RunAction;
class DetectorConstruction;
class G4Event;

/**
 * @class EventAction
 * @brief Acumula evento por evento la energía física depositada (Edep) en cada uno de
 *        los 4 compartimentos celulares, calcula la microdosis en Gray (Gy) y registra
 *        la fila correspondiente en el Ntuple 0 (LayerMicrodosimetry) del archivo CSV.
 */
class EventAction : public G4UserEventAction
{
public:
  EventAction(RunAction* runAction, const DetectorConstruction* detector);
  ~EventAction() override = default;

  void BeginOfEventAction(const G4Event* event) override;
  void EndOfEventAction(const G4Event* event) override;

  // Métodos llamados por SteppingAction durante la etapa física
  void AddEdepOuterLayer(G4double edep)   { fEdepOuter   += edep; }
  void AddEdepWallOrCortex(G4double edep) { fEdepWall    += edep; }
  void AddEdepCytoplasm(G4double edep)    { fEdepCyto    += edep; }
  void AddEdepNucleus(G4double edep)      { fEdepNucleus += edep; }

  // Consultados por TimeStepAction durante la etapa química (1 ps a 1 us)
  G4double GetTotalCellEdep() const
  {
    return fEdepOuter + fEdepWall + fEdepCyto + fEdepNucleus;
  }

  G4int GetCurrentEventID() const { return fCurrentEventID; }

private:
  RunAction*                  fRunAction{nullptr};
  const DetectorConstruction* fDetector{nullptr};

  G4int    fCurrentEventID{0};
  G4double fEdepOuter{0.0};
  G4double fEdepWall{0.0};
  G4double fEdepCyto{0.0};
  G4double fEdepNucleus{0.0};
};

#endif

