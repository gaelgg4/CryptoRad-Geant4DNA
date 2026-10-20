#ifndef RunAction_h
#define RunAction_h 1

#include "G4UserRunAction.hh"
#include "G4Accumulable.hh"
#include "globals.hh"

class G4Run;
class DetectorConstruction;

/**
 * @class RunAction
 * @brief Configura G4AnalysisManager para exportar resultados en formato CSV
 *        y acumula las dosis totales absorbidas (Gy) por capa celular durante la corrida.
 *
 * Archivos CSV generados automáticamente por G4AnalysisManager:
 *  - CryptoRad_Output_nt_LayerMicrodosimetry.csv (Dosis en Gy y Edep en eV por capa)
 *  - CryptoRad_Output_nt_ROS_Kinetics_GValues.csv (G-values de ·OH, e-aq, H2O2 de 1 ps a 1 us)
 */
class RunAction : public G4UserRunAction
{
public:
  explicit RunAction(const DetectorConstruction* detector);
  ~RunAction() override = default;

  void BeginOfRunAction(const G4Run* run) override;
  void EndOfRunAction(const G4Run* run) override;

  void AddEventEdep(G4double edepOuter, G4double edepWall,
                    G4double edepCyto,  G4double edepNucleus);

private:
  const DetectorConstruction* fDetector{nullptr};

  G4Accumulable<G4double> fSumEdepOuter{0.0};
  G4Accumulable<G4double> fSumEdepWall{0.0};
  G4Accumulable<G4double> fSumEdepCyto{0.0};
  G4Accumulable<G4double> fSumEdepNucleus{0.0};
};

#endif

