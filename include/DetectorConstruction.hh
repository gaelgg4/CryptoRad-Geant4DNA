#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"
#include "G4SystemOfUnits.hh"

class G4VPhysicalVolume;
class G4LogicalVolume;
class G4Material;
class G4UserLimits;
class DetectorMessenger;

/**
 * @enum CellType
 * @brief Define el modelo celular activo en la simulación.
 *        - kCryptococcusStandard: Levadura típica haploide (1C) in vitro.
 *        - kHumanEpithelial: Célula epitelial cúbica polarizada diploide (2C).
 */
enum class CellType {
  kCryptococcusStandard = 0,
  kHumanEpithelial      = 1
};

/**
 * @class DetectorConstruction
 * @brief Construye los materiales bioquímicos personalizados y la geometría
 *        multicapa de C. neoformans (forma estándar) o de la célula epitelial humana.
 *
 * Capas registradas para dosimetría y seguimiento químico (ROS):
 *  1. Capa Externa Protectora:
 *     - C. neoformans: Cápsula polisacárida (99% agua + GXM 89%, GalXM 9%, MP 2%).
 *     - Humana: Glucocáliz apical/periférico (ácido siálico y glucoproteínas).
 *  2. Soporte Estructural / Pared o Membrana Externa:
 *     - C. neoformans: Pared celular (quitosano, quitina, alfa/beta-glucanos y melanina).
 *     - Humana: Complejo de membrana plasmática (colesterol 25%, esfingolípidos) y corteza de actina.
 *  3. Citoplasma (incluyendo vacuola central en C. neoformans).
 *  4. Núcleo (haploide 1C en C. neoformans vs. diploide 2C en célula humana).
 */
class DetectorConstruction : public G4VUserDetectorConstruction
{
public:
  DetectorConstruction();
  ~DetectorConstruction() override;

  G4VPhysicalVolume* Construct() override;
  void ConstructSDandField() override;

  // --- Setters configurables mediante DetectorMessenger (macros .mac) ---
  void SetCellType(const G4String& typeName);
  void SetCryptoCellDiameter(G4double val);
  void SetCryptoCapsuleThickness(G4double val);
  void SetCryptoWallThickness(G4double val);
  void SetCryptoAspectRatio(G4double val);
  void SetMelaninMassFraction(G4double val);

  void SetHumanCellWidth(G4double val);
  void SetHumanCellHeight(G4double val);
  void SetHumanGlycocalyxThickness(G4double val);

  // --- Getters para dosimetría (RunAction / EventAction / SteppingAction) ---
  CellType GetCellType() const { return fCellType; }
  G4String GetCellTypeName() const;

  G4LogicalVolume* GetOuterLayerLV() const { return fOuterLayerLV; } // Cápsula o Glucocáliz
  G4LogicalVolume* GetWallOrCortexLV() const { return fWallOrCortexLV; } // Pared fúngica o Membrana/Corteza
  G4LogicalVolume* GetCytoplasmLV() const { return fCytoplasmLV; }
  G4LogicalVolume* GetNucleusLV() const { return fNucleusLV; }

  // Masa de cada compartimento en kg (o unidades internas de Geant4) para cálculo de Dosis (Gy = J/kg)
  G4double GetOuterLayerMass() const;
  G4double GetWallOrCortexMass() const;
  G4double GetCytoplasmMass() const;
  G4double GetNucleusMass() const;

  // Fracción de captura de radicales (scavenging) por melanina/GXM
  G4double GetMelaninMassFraction() const { return fMelaninFractionInWall; }

private:
  void DefineMaterials();
  G4VPhysicalVolume* ConstructCryptococcusStandard(G4LogicalVolume* worldLV);
  G4VPhysicalVolume* ConstructHumanEpithelial(G4LogicalVolume* worldLV);

  DetectorMessenger* fMessenger{nullptr};
  CellType           fCellType{CellType::kCryptococcusStandard};

  // --- Parámetros geométricos: C. neoformans (Forma Estándar / Levadura Típica) ---
  G4double fCryptoBodyDiameter{6.0 * um};       // Rango: 4 - 10 um (promedio 5 - 7 um)
  G4double fCryptoAspectRatio{1.10};            // Morfología cuasiesférica a ovoidal (1.0 - 1.25)
  G4double fCryptoCapsuleThickness{1.2 * um};   // Rango: 0.5 - 2.0 um
  G4double fCryptoWallThickness{150.0 * nm};    // Rango: 100 - 200 nm
  G4double fCryptoMembraneThickness{8.0 * nm};  // Bicapa de ergosterol
  G4double fCryptoNucleusRadius{0.9 * um};      // Núcleo haploide (1C)
  G4double fCryptoVacuoleRadius{1.6 * um};      // Vacuola central prominente
  G4double fMelaninFractionInWall{0.10};        // 10% de masa seca en pared melaninizada

  // --- Parámetros geométricos: Célula Humana Epitelial (Cúbica de referencia) ---
  G4double fHumanCellWidth{12.0 * um};          // Rango cúbico: 10 - 15 um
  G4double fHumanCellHeight{12.0 * um};         // Rango cúbico: 10 - 15 um (aspecto ~1.0)
  G4double fHumanGlycocalyxThickness{80.0 * nm};// Cubierta externa rica en ácido siálico
  G4double fHumanCortexThickness{120.0 * nm};   // Membrana (colesterol/esfingolípidos) + corteza de actina
  G4double fHumanNucleusRadius{2.8 * um};       // Núcleo diploide (2C)

  // --- Materiales bioquímicos ---
  G4Material* fMatWater{nullptr};
  G4Material* fMatCapsuleCrypto{nullptr};
  G4Material* fMatCellWallCrypto{nullptr};
  G4Material* fMatMembraneCrypto{nullptr};
  G4Material* fMatCytoplasmCrypto{nullptr};
  G4Material* fMatNucleusCrypto{nullptr};

  G4Material* fMatGlycocalyxHuman{nullptr};
  G4Material* fMatMembraneCortexHuman{nullptr};
  G4Material* fMatCytoplasmHuman{nullptr};
  G4Material* fMatNucleusHuman{nullptr};

  // --- Punteros a Volúmenes Lógicos activos para scoring ---
  G4LogicalVolume* fWorldLV{nullptr};
  G4LogicalVolume* fOuterLayerLV{nullptr};
  G4LogicalVolume* fWallOrCortexLV{nullptr};
  G4LogicalVolume* fCytoplasmLV{nullptr};
  G4LogicalVolume* fNucleusLV{nullptr};

  G4UserLimits* fStepLimit{nullptr};
};

#endif

