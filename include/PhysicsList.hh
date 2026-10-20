#ifndef PhysicsList_h
#define PhysicsList_h 1

#include "G4VModularPhysicsList.hh"
#include "globals.hh"

class G4GenericMessenger;

/**
 * @class PhysicsList
 * @brief Lista de física modular para Geant4-DNA (v11+) que acopla:
 *        1. Etapa física (estructura de trazas paso a paso hasta escala de eV):
 *           - G4EmDNAPhysics_option2 (por defecto: modelo rápido y validado de Born/Emfietzoglou)
 *           - G4EmDNAPhysics_option8 (opcional por macro: combina option2 con secciones eficaces mejoradas)
 *        2. Etapa pre-química y química (1 ps a 1 us):
 *           - G4EmDNAChemistry_option3: Radiólisis del agua, disociación molecular,
 *             difusión browniana Smoluchowski/IRT y reacciones de especies reactivas (ROS:
 *             ·OH, e-aq, H2O2, H·, H3O+, OH-, O2·-, HO2·).
 */
class PhysicsList : public G4VModularPhysicsList
{
public:
  PhysicsList();
  ~PhysicsList() override;

  void ConstructParticle() override;
  void ConstructProcess() override;

  /// Permite alternar entre "option2" y "option8" antes de /run/initialize
  void SelectDNAPhysicsOption(const G4String& optionName);

private:
  G4GenericMessenger* fMessenger{nullptr};
  G4String            fPhysOptionName{"option2"};
};

#endif

