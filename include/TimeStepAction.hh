#ifndef TimeStepAction_h
#define TimeStepAction_h 1

#include "G4UserTimeStepAction.hh"
#include "globals.hh"
#include <vector>

class EventAction;
class DetectorConstruction;

/**
 * @class TimeStepAction
 * @brief Monitorea la evolución temporal de las especies reactivas de oxígeno (ROS:
 *        ·OH, e-aq, H2O2) desde 1 picosegundo (1 ps) hasta 1 microsegundo (1 us).
 *
 * Calcula en cada ventana de tiempo:
 *  - Conteo absoluto de radicales (N_OH, N_e_aq, N_H2O2) y radicales ·OH dentro del núcleo.
 *  - G-values estándar (moléculas por cada 100 eV depositados en la célula).
 *  - G(·OH) efectivo post-scavenging considerando la cinética de captura antioxidante
 *    por la melanina de la pared celular y el GXM capsular de C. neoformans.
 */
class TimeStepAction : public G4UserTimeStepAction
{
public:
  TimeStepAction(EventAction* eventAction, const DetectorConstruction* detector);
  ~TimeStepAction() override = default;

  TimeStepAction(const TimeStepAction& other);
  TimeStepAction& operator=(const TimeStepAction& other) = delete;

  void StartProcessing() override;
  void UserPreTimeStepAction() override {}
  void UserPostTimeStepAction() override;
  void UserReactionAction(const G4Track& /*trackA*/,
                          const G4Track& /*trackB*/,
                          const std::vector<G4Track*>* /*products*/) override {}
  void EndProcessing() override;

private:
  void RecordROSYieldsAtTime(G4double currentTimePs);

  EventAction*                fEventAction{nullptr};
  const DetectorConstruction* fDetector{nullptr};

  std::vector<G4double>       fTimeCheckpointsPs;
  std::size_t                 fNextCheckpointIdx{0};
};

#endif

