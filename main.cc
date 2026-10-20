#include "DetectorConstruction.hh"
#include "PhysicsList.hh"
#include "ActionInitialization.hh"

#include "G4RunManagerFactory.hh"
#include "G4UImanager.hh"
#include "G4UIExecutive.hh"
#include "G4VisExecutive.hh"
#include "G4DNAChemistryManager.hh"
#include "Randomize.hh"

/**
 * @file main.cc
 * @brief Punto de entrada de la simulación Geant4-DNA (v11+) para comparar microdosimetría
 *        y cinética de ROS (1 ps a 1 us) entre Cryptococcus neoformans (forma estándar)
 *        y una célula epitelial humana de referencia.
 *
 * Modos de ejecución:
 *  - Modo interactivo (con interfaz gráfica Qt/OpenGL):
 *      ./crypto_rad_sim
 *  - Modo batch (ejecución de macro):
 *      ./crypto_rad_sim run_cryptococcus.mac
 *      ./crypto_rad_sim run_human_epithelial.mac
 */
int main(int argc, char** argv)
{
  // 1. Detectar modo interactivo (si no se pasan argumentos por línea de comandos)
  G4UIExecutive* ui = nullptr;
  if (argc == 1) {
    ui = new G4UIExecutive(argc, argv);
  }

  // 2. Semilla del generador de números aleatorios (MixMax por defecto en Geant4 11+)
  G4Random::setTheSeed(20261019L);

  // 3. Crear RunManager en modo Serial para consolidar los Ntuples CSV en un único archivo
  //    y ejecutar el integrador químico G4Scheduler de forma determinista por evento
  auto* runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::Serial);

  // 4. Instanciar y registrar las clases principales de la simulación
  auto* detector = new DetectorConstruction();
  runManager->SetUserInitialization(detector);

  auto* physicsList = new PhysicsList();
  runManager->SetUserInitialization(physicsList);

  // Activar el gestor químico de Geant4-DNA antes de inicializar ActionInitialization
  G4DNAChemistryManager::Instance()->SetChemistryActivation(true);

  runManager->SetUserInitialization(new ActionInitialization(detector));

  // 5. Obtener puntero al gestor de interfaz de usuario y ejecutar macros
  G4UImanager* UImanager = G4UImanager::GetUIpointer();

  if (!ui) {
    // Modo Batch: ejecutar el archivo .mac pasado como argumento
    G4String command  = "/control/execute ";
    G4String fileName = argv[1];
    UImanager->ApplyCommand(command + fileName);
  } else {
    // Modo Interactivo: inicializar el gestor de visualización 3D y cargar vis.mac
    G4VisManager* visManager = new G4VisExecutive("Quiet");
    visManager->Initialize();
    UImanager->ApplyCommand("/control/execute vis.mac");
    ui->SessionStart();
    delete visManager;
    delete ui;
  }

  // 6. Liberar memoria y finalizar
  delete runManager;

  return 0;
}

