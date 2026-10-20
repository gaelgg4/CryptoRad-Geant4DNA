#include "PhysicsList.hh"

#include "G4EmDNAPhysics_option2.hh"
#include "G4EmDNAPhysics_option8.hh"
#include "G4EmDNAChemistry_option3.hh"
#include "G4PhysicsConstructorRegistry.hh"
#include "G4GenericMessenger.hh"
#include "G4SystemOfUnits.hh"

PhysicsList::PhysicsList()
  : G4VModularPhysicsList()
{
  // Cut por defecto de 1 nm para resolución a escala subcelular y de ADN
  SetDefaultCutValue(1.0 * nm);
  SetVerboseLevel(1);

  // 1. Registrar constructor físico de estructura de trazas Geant4-DNA (por defecto: option2)
  RegisterPhysics(new G4EmDNAPhysics_option2());

  // 2. Registrar constructor químico de Geant4-DNA (radiólisis y reacciones de ROS: 1 ps a 1 us)
  //    G4EmDNAChemistry_option3 incluye canales completos de ·OH, e-aq, H2O2, H·, H3O+, OH-
  RegisterPhysics(new G4EmDNAChemistry_option3());

  // 3. Comando de macro para cambiar entre G4EmDNAPhysics_option2 y G4EmDNAPhysics_option8
  fMessenger = new G4GenericMessenger(this, "/dna/physics/",
                                      "Seleccion de modelo fisico de Geant4-DNA");
  auto& selectCmd = fMessenger->DeclareMethod(
    "selectOption",
    &PhysicsList::SelectDNAPhysicsOption,
    "Selecciona el constructor fisico de Geant4-DNA: 'option2' u 'option8'");
  selectCmd.SetParameterName("optionName", false);
  selectCmd.SetCandidates("option2 option8");
  selectCmd.SetStates(G4State_PreInit);
}

PhysicsList::~PhysicsList()
{
  delete fMessenger;
}

void PhysicsList::SelectDNAPhysicsOption(const G4String& optionName)
{
  if (optionName == fPhysOptionName) {
    return;
  }

  if (optionName == "option2") {
    ReplacePhysics(new G4EmDNAPhysics_option2());
    fPhysOptionName = "option2";
    G4cout << "[PhysicsList] Física electromagnética cambiada a G4EmDNAPhysics_option2" << G4endl;
  } else if (optionName == "option8") {
    ReplacePhysics(new G4EmDNAPhysics_option8());
    fPhysOptionName = "option8";
    G4cout << "[PhysicsList] Física electromagnética cambiada a G4EmDNAPhysics_option8" << G4endl;
  }
}

void PhysicsList::ConstructParticle()
{
  G4VModularPhysicsList::ConstructParticle();
}

void PhysicsList::ConstructProcess()
{
  G4VModularPhysicsList::ConstructProcess();
}

