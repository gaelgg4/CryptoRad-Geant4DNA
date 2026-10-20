#ifndef ActionInitialization_h
#define ActionInitialization_h 1

#include "G4VUserActionInitialization.hh"

class DetectorConstruction;

/**
 * @class ActionInitialization
 * @brief Instancia y registra todas las clases de acción de usuario tanto para el
 *        hilo maestro (Master) como para los hilos trabajadores (Workers), e inicializa
 *        el planificador temporal químico (G4Scheduler) de Geant4-DNA (1 ps a 1 us).
 */
class ActionInitialization : public G4VUserActionInitialization
{
public:
  explicit ActionInitialization(const DetectorConstruction* detConstruction);
  ~ActionInitialization() override = default;

  void BuildForMaster() const override;
  void Build() const override;

private:
  const DetectorConstruction* fDetector{nullptr};
};

#endif

