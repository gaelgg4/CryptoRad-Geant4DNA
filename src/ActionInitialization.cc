#include "ActionInitialization.hh"
#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "EventAction.hh"
#include "SteppingAction.hh"
#include "StackingAction.hh"
#include "TimeStepAction.hh"
#include "DetectorConstruction.hh"

#include "G4DNAChemistryManager.hh"
#include "G4Scheduler.hh"
#include "G4SystemOfUnits.hh"

ActionInitialization::ActionInitialization(const DetectorConstruction* detConstruction)
  : G4VUserActionInitialization(),
    fDetector(detConstruction)
{}

void ActionInitialization::BuildForMaster() const
{
  // En modo multihilo (MT), el hilo Master solo ejecuta RunAction para consolidar y exportar CSVs
  SetUserAction(new RunAction(fDetector));
}

void ActionInitialization::Build() const
{
  // 1. Generador de partículas primarias
  SetUserAction(new PrimaryGeneratorAction());

  // 2. Gestión de corrida y configuración de G4AnalysisManager (archivos CSV)
  auto* runAction = new RunAction(fDetector);
  SetUserAction(runAction);

  // 3. Acumulación por evento de microdosis por capa y G-values temporales
  auto* eventAction = new EventAction(runAction, fDetector);
  SetUserAction(eventAction);

  // 4. Registro paso a paso de deposición de energía física (Cápsula/Glucocáliz, Pared/Corteza, Citoplasma, Núcleo)
  SetUserAction(new SteppingAction(eventAction, fDetector));

  // 5. Disparo automático de la etapa química al terminar las trazas físicas del evento
  SetUserAction(new StackingAction());

  // 6. Inicialización del motor químico de Geant4-DNA (1 ps a 1 us)
  if (G4DNAChemistryManager::IsActivated()) {
    G4Scheduler::Instance()->SetUserAction(new TimeStepAction(eventAction, fDetector));
    G4Scheduler::Instance()->SetEndTime(1.0 * microsecond);
    G4Scheduler::Instance()->SetVerbose(0);
  }
}

