#include "RunAction.hh"
#include "DetectorConstruction.hh"

#include "G4AnalysisManager.hh"
#include "G4Run.hh"
#include "G4AccumulableManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"

#include <iomanip>

RunAction::RunAction(const DetectorConstruction* detector)
  : G4UserRunAction(),
    fDetector(detector)
{
  // Registrar acumuladores multihilo para el resumen estadístico de dosis
  auto* accumulableManager = G4AccumulableManager::Instance();
  accumulableManager->RegisterAccumulable(fSumEdepOuter);
  accumulableManager->RegisterAccumulable(fSumEdepWall);
  accumulableManager->RegisterAccumulable(fSumEdepCyto);
  accumulableManager->RegisterAccumulable(fSumEdepNucleus);

  // Configurar G4AnalysisManager con salida nativa a CSV
  auto* analysisManager = G4AnalysisManager::Instance();
  analysisManager->SetDefaultFileType("csv");
  analysisManager->SetVerboseLevel(1);
  analysisManager->SetFileName("CryptoRad_Output");

  // ---------------------------------------------------------------------------
  // Ntuple ID = 0: Microdosimetría física por capa celular (1 fila por evento)
  // ---------------------------------------------------------------------------
  analysisManager->CreateNtuple("LayerMicrodosimetry",
                                "Energia depositada (eV) y Dosis absorbida (Gy) por capa celular");
  analysisManager->CreateNtupleIColumn("EventID");                 // Col 0
  analysisManager->CreateNtupleSColumn("CellType");                // Col 1
  analysisManager->CreateNtupleDColumn("Edep_OuterLayer_eV");      // Col 2 (Cápsula GXM o Glucocáliz)
  analysisManager->CreateNtupleDColumn("Edep_WallOrCortex_eV");    // Col 3 (Pared Melanina o Corteza/Membrana)
  analysisManager->CreateNtupleDColumn("Edep_Cytoplasm_eV");       // Col 4 (Citoplasma)
  analysisManager->CreateNtupleDColumn("Edep_Nucleus_eV");         // Col 5 (Núcleo 1C o 2C)
  analysisManager->CreateNtupleDColumn("Dose_OuterLayer_Gy");      // Col 6
  analysisManager->CreateNtupleDColumn("Dose_WallOrCortex_Gy");    // Col 7
  analysisManager->CreateNtupleDColumn("Dose_Cytoplasm_Gy");       // Col 8
  analysisManager->CreateNtupleDColumn("Dose_Nucleus_Gy");         // Col 9
  analysisManager->FinishNtuple();

  // ---------------------------------------------------------------------------
  // Ntuple ID = 1: Cinética química de ROS y G-values (1 ps a 1 us)
  // ---------------------------------------------------------------------------
  analysisManager->CreateNtuple("ROS_Kinetics_GValues",
                                "Rendimiento quimico temporal de ROS (1 ps a 1 us)");
  analysisManager->CreateNtupleIColumn("EventID");                        // Col 0
  analysisManager->CreateNtupleSColumn("CellType");                       // Col 1
  analysisManager->CreateNtupleDColumn("Time_ps");                        // Col 2
  analysisManager->CreateNtupleDColumn("Total_Edep_Cell_eV");             // Col 3
  analysisManager->CreateNtupleIColumn("N_OH");                           // Col 4
  analysisManager->CreateNtupleIColumn("N_e_aq");                         // Col 5
  analysisManager->CreateNtupleIColumn("N_H2O2");                         // Col 6
  analysisManager->CreateNtupleDColumn("G_OH_raw");                       // Col 7
  analysisManager->CreateNtupleDColumn("G_e_aq_raw");                     // Col 8
  analysisManager->CreateNtupleDColumn("G_H2O2_raw");                     // Col 9
  analysisManager->CreateNtupleDColumn("G_OH_Effective_PostScavenging");  // Col 10
  analysisManager->CreateNtupleIColumn("N_OH_InNucleus");                 // Col 11
  analysisManager->FinishNtuple();
}

void RunAction::BeginOfRunAction(const G4Run* /*run*/)
{
  G4AccumulableManager::Instance()->Reset();

  auto* analysisManager = G4AnalysisManager::Instance();
  analysisManager->OpenFile();
}

void RunAction::EndOfRunAction(const G4Run* run)
{
  G4int nofEvents = run->GetNumberOfEvent();
  if (nofEvents == 0) {
    auto* analysisManager = G4AnalysisManager::Instance();
    analysisManager->Write();
    analysisManager->CloseFile();
    return;
  }

  // Fusionar acumuladores de todos los hilos trabajadores
  G4AccumulableManager::Instance()->Merge();

  if (IsMaster() && fDetector != nullptr) {
    G4double edepOuter   = fSumEdepOuter.GetValue();
    G4double edepWall    = fSumEdepWall.GetValue();
    G4double edepCyto    = fSumEdepCyto.GetValue();
    G4double edepNucleus = fSumEdepNucleus.GetValue();

    G4double massOuter   = fDetector->GetOuterLayerMass();
    G4double massWall    = fDetector->GetWallOrCortexMass();
    G4double massCyto    = fDetector->GetCytoplasmMass();
    G4double massNucleus = fDetector->GetNucleusMass();

    G4double doseOuterGy   = (massOuter > 0.)   ? (edepOuter / massOuter) / gray   : 0.;
    G4double doseWallGy    = (massWall > 0.)    ? (edepWall / massWall) / gray     : 0.;
    G4double doseCytoGy    = (massCyto > 0.)    ? (edepCyto / massCyto) / gray     : 0.;
    G4double doseNucleusGy = (massNucleus > 0.) ? (edepNucleus / massNucleus) / gray : 0.;

    G4cout << "\n==========================================================================\n"
           << " RESUMEN MICRODOSIMETRICO DE LA CORRIDA (" << nofEvents << " eventos)\n"
           << " Modelo Celular Activo: " << fDetector->GetCellTypeName() << "\n"
           << "--------------------------------------------------------------------------\n"
           << std::fixed << std::setprecision(4)
           << " 1. Capa Externa (Capsula GXM / Glucocaliz):\n"
           << "    - Edep acumulada : " << edepOuter / keV << " keV\n"
           << "    - Dosis absorbida: " << doseOuterGy << " Gy\n"
           << " 2. Pared Celular Melaninizada / Complejo Membrana-Corteza:\n"
           << "    - Edep acumulada : " << edepWall / keV << " keV\n"
           << "    - Dosis absorbida: " << doseWallGy << " Gy\n"
           << " 3. Citoplasma:\n"
           << "    - Edep acumulada : " << edepCyto / keV << " keV\n"
           << "    - Dosis absorbida: " << doseCytoGy << " Gy\n"
           << " 4. Nucleo (1C Haploide vs 2C Diploide):\n"
           << "    - Edep acumulada : " << edepNucleus / keV << " keV\n"
           << "    - Dosis absorbida: " << doseNucleusGy << " Gy\n"
           << "==========================================================================\n"
           << G4endl;
  }

  // Escribir y cerrar archivos CSV
  auto* analysisManager = G4AnalysisManager::Instance();
  analysisManager->Write();
  analysisManager->CloseFile();
}

void RunAction::AddEventEdep(G4double edepOuter, G4double edepWall,
                             G4double edepCyto,  G4double edepNucleus)
{
  fSumEdepOuter   += edepOuter;
  fSumEdepWall    += edepWall;
  fSumEdepCyto    += edepCyto;
  fSumEdepNucleus += edepNucleus;
}

