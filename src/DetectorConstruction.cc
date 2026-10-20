#include "DetectorConstruction.hh"
#include "DetectorMessenger.hh"

#include "G4NistManager.hh"
#include "G4Material.hh"
#include "G4Element.hh"
#include "G4Box.hh"
#include "G4Orb.hh"
#include "G4Ellipsoid.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4UserLimits.hh"
#include "G4RunManager.hh"
#include "G4StateManager.hh"
#include "G4GeometryManager.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4SolidStore.hh"
#include "G4UnitsTable.hh"

#include <cmath>
#include <iomanip>

DetectorConstruction::DetectorConstruction()
  : G4VUserDetectorConstruction()
{
  fMessenger = new DetectorMessenger(this);
}

DetectorConstruction::~DetectorConstruction()
{
  delete fStepLimit;
  delete fMessenger;
}

void DetectorConstruction::DefineMaterials()
{
  // Evitar redefinir materiales si la geometría se reconstruye entre corridas
  if (fMatWater != nullptr) {
    return;
  }

  G4NistManager* nist = G4NistManager::Instance();

  // 1. Elementos atómicos básicos (C, H, O, N, P, S)
  G4Element* elH = nist->FindOrBuildElement("H");
  G4Element* elC = nist->FindOrBuildElement("C");
  G4Element* elN = nist->FindOrBuildElement("N");
  G4Element* elO = nist->FindOrBuildElement("O");
  G4Element* elP = nist->FindOrBuildElement("P");
  G4Element* elS = nist->FindOrBuildElement("S");

  // 2. Agua líquida estándar de Geant4-DNA (requerida por G4DNAMolecularMaterial)
  fMatWater = nist->FindOrBuildMaterial("G4_WATER");

  // ===========================================================================
  // MACROMOLÉCULAS DE CRYPTOCOCCUS NEOFORMANS (FORMA ESTÁNDAR)
  // ===========================================================================

  // A) Glucuronoxilomanano (GXM): 88%-90% de la masa seca capsular.
  //    Trímero de manosa [(C6H10O5)3] con ramificaciones de ácido glucurónico (C6H8O6)
  //    y xilosa (C5H8O4) -> Fórmula promedio por unidad repetitiva: C29 H44 O25
  auto* matGXM = new G4Material("GXM_Polymer", 1.52 * g/cm3, 3);
  matGXM->AddElement(elC, 29);
  matGXM->AddElement(elH, 44);
  matGXM->AddElement(elO, 25);

  // B) Galactoxilomanano (GalXM): 8%-10% de la masa seca capsular.
  //    Esqueleto de galactano con cadenas laterales de galactosa, manosa y xilosa (C22 H36 O18)
  auto* matGalXM = new G4Material("GalXM_Polymer", 1.50 * g/cm3, 3);
  matGalXM->AddElement(elC, 22);
  matGalXM->AddElement(elH, 36);
  matGalXM->AddElement(elO, 18);

  // C) Manoproteínas (MP): ~1%-2% de la masa seca capsular.
  //    Glicoproteínas ricas en manosa con trazas de azufre (cisteína/metionina): C18 H30 N3 O10 S1
  auto* matMP = new G4Material("Mannoprotein", 1.38 * g/cm3, 5);
  matMP->AddElement(elC, 18);
  matMP->AddElement(elH, 30);
  matMP->AddElement(elN, 3);
  matMP->AddElement(elO, 10);
  matMP->AddElement(elS, 1);

  // D) Quitosano: Forma desacetilada de la quitina (3 a 5 veces más abundante que la quitina).
  //    Poli-(beta-1,4-D-glucosamina): (C6 H11 N O4)n
  auto* matChitosan = new G4Material("Chitosan", 1.42 * g/cm3, 4);
  matChitosan->AddElement(elC, 6);
  matChitosan->AddElement(elH, 11);
  matChitosan->AddElement(elN, 1);
  matChitosan->AddElement(elO, 4);

  // E) Quitina: Poli-(beta-1,4-N-acetil-D-glucosamina): (C8 H13 N O5)n
  auto* matChitin = new G4Material("Chitin", 1.45 * g/cm3, 4);
  matChitin->AddElement(elC, 8);
  matChitin->AddElement(elH, 13);
  matChitin->AddElement(elN, 1);
  matChitin->AddElement(elO, 5);

  // F) Glucanos estructurales: beta-(1,3), beta-(1,6) y alfa-(1,3)-glucano: (C6 H10 O5)n
  auto* matGlucan = new G4Material("Fungal_Glucan", 1.50 * g/cm3, 3);
  matGlucan->AddElement(elC, 6);
  matGlucan->AddElement(elH, 10);
  matGlucan->AddElement(elO, 5);

  // G) Melanina (Eumelanina fúngica derivada de L-DOPA):
  //    Polímero de 5,6-dihidroxiindol (DHI) e indol-5,6-quinona: (C8 H5 N O2)n
  //    Alta densidad electrónica y fuerte capacidad antioxidante (atrapador de ·OH y e-aq)
  auto* matMelanin = new G4Material("Fungal_Melanin", 1.70 * g/cm3, 4);
  matMelanin->AddElement(elC, 8);
  matMelanin->AddElement(elH, 5);
  matMelanin->AddElement(elN, 1);
  matMelanin->AddElement(elO, 2);

  // H) Ergosterol (Esterol principal de la membrana fúngica): C28 H44 O
  auto* matErgosterol = new G4Material("Ergosterol", 1.04 * g/cm3, 3);
  matErgosterol->AddElement(elC, 28);
  matErgosterol->AddElement(elH, 44);
  matErgosterol->AddElement(elO, 1);

  // ===========================================================================
  // MACROMOLÉCULAS DE CÉLULA HUMANA EPITELIAL DE REFERENCIA
  // ===========================================================================

  // I) Ácido Siálico / Glicoproteínas del Glucocáliz (Ácido N-acetilneuramínico: C11 H19 N O9)
  auto* matSialicGlyco = new G4Material("SialicAcid_Glycoprotein", 1.40 * g/cm3, 4);
  matSialicGlyco->AddElement(elC, 11);
  matSialicGlyco->AddElement(elH, 19);
  matSialicGlyco->AddElement(elN, 1);
  matSialicGlyco->AddElement(elO, 9);

  // J) Colesterol (20%-30% de la bicapa epitelial): C27 H46 O
  auto* matCholesterol = new G4Material("Cholesterol", 1.05 * g/cm3, 3);
  matCholesterol->AddElement(elC, 27);
  matCholesterol->AddElement(elH, 46);
  matCholesterol->AddElement(elO, 1);

  // K) Fosfolípido / Esfingolípido promedio de membrana (DPPC/Esfingomielina): C40 H80 N O8 P
  auto* matPhospholipid = new G4Material("Phospholipid", 1.03 * g/cm3, 5);
  matPhospholipid->AddElement(elC, 40);
  matPhospholipid->AddElement(elH, 80);
  matPhospholipid->AddElement(elN, 1);
  matPhospholipid->AddElement(elO, 8);
  matPhospholipid->AddElement(elP, 1);

  // L) Citoqueratina / Actina (Filamentos del citoesqueleto y complejos de unión): C9 H14 N2 O3 S0.1
  auto* matCytoskeletonProtein = new G4Material("Cytoskeleton_Protein", 1.35 * g/cm3, 5);
  matCytoskeletonProtein->AddElement(elC, 45);
  matCytoskeletonProtein->AddElement(elH, 70);
  matCytoskeletonProtein->AddElement(elN, 12);
  matCytoskeletonProtein->AddElement(elO, 15);
  matCytoskeletonProtein->AddElement(elS, 1);

  // M) Cromatina Nuclear (ADN + Histonas): C10 H13 N4 O6 P
  auto* matChromatin = new G4Material("Chromatin_DNA", 1.40 * g/cm3, 5);
  matChromatin->AddElement(elC, 10);
  matChromatin->AddElement(elH, 13);
  matChromatin->AddElement(elN, 4);
  matChromatin->AddElement(elO, 6);
  matChromatin->AddElement(elP, 1);

  // ===========================================================================
  // MATERIALES COMPUESTOS POR CAPA CELULAR (ACOPLADOS CON G4_WATER PARA GEANT4-DNA)
  // Nota técnica de Geant4-DNA:
  //   1. Primero construimos la mezcla estequiométrica exacta (*_Stoich) de cada
  //      compartimento (GXM, GalXM, Quitosano, Quitina, Glucanos, Melanina, etc.)
  //      para obtener su densidad electrónica real (e-/cm3) a partir de su composición.
  //   2. Luego construimos el material activo de transporte Geant4-DNA usando
  //      G4Material(name, rho_eff, fMatWater) (con G4_WATER como baseMaterial y
  //      densidad escalada por la razón de densidad electrónica Z_eff/A_eff).
  //      Esto garantiza que G4DNAMolecularMaterial active todos los modelos de
  //      estructura de trazas (option2/option8) y radiólisis química (ROS) dentro
  //      de todas las capas celulares sin excepción.
  // ===========================================================================

  auto buildDNACompartment = [&](const G4String& name, const G4Material* stoichMat) -> G4Material* {
    G4double eDensRatio = stoichMat->GetTotNbOfElectPerVolume() / fMatWater->GetTotNbOfElectPerVolume();
    G4double effDensity = fMatWater->GetDensity() * eDensRatio;
    return new G4Material(name, effDensity, fMatWater, kStateLiquid);
  };

  // 1. Cápsula de C. neoformans (~99% agua + 1% matriz polisacárida: 89% GXM, 9% GalXM, 2% MP)
  auto* stoichCapsuleCrypto = new G4Material("Crypto_Capsule_Stoich", 1.005 * g/cm3, 4);
  stoichCapsuleCrypto->AddMaterial(fMatWater, 0.9900);
  stoichCapsuleCrypto->AddMaterial(matGXM,    0.0089);
  stoichCapsuleCrypto->AddMaterial(matGalXM,  0.0009);
  stoichCapsuleCrypto->AddMaterial(matMP,     0.0002);
  fMatCapsuleCrypto = buildDNACompartment("Crypto_Capsule_GXM", stoichCapsuleCrypto);

  // 2. Pared Celular de C. neoformans (Hidratación fisiológica 65% agua + 35% matriz sólida:
  //    Quitosano 14%, Quitina 3.5%, alfa/beta-Glucanos 14%, Melanina 3.5% [10% del peso seco])
  auto* stoichCellWallCrypto = new G4Material("Crypto_CellWall_Stoich", 1.24 * g/cm3, 5);
  stoichCellWallCrypto->AddMaterial(fMatWater,    0.650);
  stoichCellWallCrypto->AddMaterial(matChitosan,  0.140);
  stoichCellWallCrypto->AddMaterial(matGlucan,    0.140);
  stoichCellWallCrypto->AddMaterial(matChitin,    0.035);
  stoichCellWallCrypto->AddMaterial(matMelanin,   0.035);
  fMatCellWallCrypto = buildDNACompartment("Crypto_CellWall_Melanized", stoichCellWallCrypto);

  // 3. Membrana Plasmática de C. neoformans (Bicapa rica en ergosterol hidratada)
  auto* stoichMembraneCrypto = new G4Material("Crypto_Membrane_Stoich", 1.04 * g/cm3, 3);
  stoichMembraneCrypto->AddMaterial(fMatWater,       0.60);
  stoichMembraneCrypto->AddMaterial(matPhospholipid, 0.28);
  stoichMembraneCrypto->AddMaterial(matErgosterol,   0.12);
  fMatMembraneCrypto = buildDNACompartment("Crypto_PlasmaMembrane", stoichMembraneCrypto);

  // 4. Citoplasma y Vacuola de C. neoformans (~85% agua + proteínas/glucanos/vesículas)
  auto* stoichCytoplasmCrypto = new G4Material("Crypto_Cytoplasm_Stoich", 1.035 * g/cm3, 4);
  stoichCytoplasmCrypto->AddMaterial(fMatWater,        0.85);
  stoichCytoplasmCrypto->AddMaterial(matMP,            0.08);
  stoichCytoplasmCrypto->AddMaterial(matGlucan,        0.04);
  stoichCytoplasmCrypto->AddMaterial(matPhospholipid,  0.03);
  fMatCytoplasmCrypto = buildDNACompartment("Crypto_Cytoplasm", stoichCytoplasmCrypto);

  // 5. Núcleo Haploide (1C) de C. neoformans (~80% agua + 20% cromatina/proteínas nucleares)
  auto* stoichNucleusCrypto = new G4Material("Crypto_Nucleus_Stoich", 1.065 * g/cm3, 3);
  stoichNucleusCrypto->AddMaterial(fMatWater,     0.80);
  stoichNucleusCrypto->AddMaterial(matChromatin,  0.12);
  stoichNucleusCrypto->AddMaterial(matMP,         0.08);
  fMatNucleusCrypto = buildDNACompartment("Crypto_Nucleus_1C", stoichNucleusCrypto);

  // ---------------------------------------------------------------------------
  // COMPARTIMENTOS DE LA CÉLULA HUMANA EPITELIAL
  // ---------------------------------------------------------------------------

  // 6. Glucocáliz Humano (Cubierta hidratada rica en ácido siálico y proteoglicanos)
  auto* stoichGlycocalyxHuman = new G4Material("Human_Glycocalyx_Stoich", 1.02 * g/cm3, 2);
  stoichGlycocalyxHuman->AddMaterial(fMatWater,       0.92);
  stoichGlycocalyxHuman->AddMaterial(matSialicGlyco,  0.08);
  fMatGlycocalyxHuman = buildDNACompartment("Human_Glycocalyx", stoichGlycocalyxHuman);

  // 7. Complejo Membrana Apical/Basolateral + Corteza de Actina/Uniones (Colesterol 25% de lípidos)
  auto* stoichMembraneCortexHuman = new G4Material("Human_MembraneCortex_Stoich", 1.06 * g/cm3, 4);
  stoichMembraneCortexHuman->AddMaterial(fMatWater,               0.65);
  stoichMembraneCortexHuman->AddMaterial(matPhospholipid,         0.15);
  stoichMembraneCortexHuman->AddMaterial(matCholesterol,          0.06);
  stoichMembraneCortexHuman->AddMaterial(matCytoskeletonProtein,  0.14);
  fMatMembraneCortexHuman = buildDNACompartment("Human_MembraneCortex", stoichMembraneCortexHuman);

  // 8. Citoplasma Humano (Con citoqueratinas, microtúbulos y mitocondrias)
  auto* stoichCytoplasmHuman = new G4Material("Human_Cytoplasm_Stoich", 1.04 * g/cm3, 3);
  stoichCytoplasmHuman->AddMaterial(fMatWater,               0.82);
  stoichCytoplasmHuman->AddMaterial(matCytoskeletonProtein,  0.13);
  stoichCytoplasmHuman->AddMaterial(matPhospholipid,         0.05);
  fMatCytoplasmHuman = buildDNACompartment("Human_Cytoplasm", stoichCytoplasmHuman);

  // 9. Núcleo Diploide (2C) Humano (Mayor densidad de cromatina que el haploide fúngico)
  auto* stoichNucleusHuman = new G4Material("Human_Nucleus_Stoich", 1.08 * g/cm3, 3);
  stoichNucleusHuman->AddMaterial(fMatWater,               0.76);
  stoichNucleusHuman->AddMaterial(matChromatin,            0.16);
  stoichNucleusHuman->AddMaterial(matCytoskeletonProtein,  0.08);
  fMatNucleusHuman = buildDNACompartment("Human_Nucleus_2C", stoichNucleusHuman);
}

G4VPhysicalVolume* DetectorConstruction::Construct()
{
  DefineMaterials();

  // Limpiar almacenes geométricos si se modifica la geometría en tiempo de ejecución
  G4GeometryManager::GetInstance()->OpenGeometry();
  G4PhysicalVolumeStore::GetInstance()->Clean();
  G4LogicalVolumeStore::GetInstance()->Clean();
  G4SolidStore::GetInstance()->Clean();

  // Mundo de simulación (Medio acuoso extracelular de 30 um de lado)
  G4double worldHalfSize = 15.0 * um;
  auto* worldSolid = new G4Box("World_Solid", worldHalfSize, worldHalfSize, worldHalfSize);
  fWorldLV = new G4LogicalVolume(worldSolid, fMatWater, "World_LV");
  auto* worldPV = new G4PVPlacement(
    nullptr, G4ThreeVector(), fWorldLV, "World_PV", nullptr, false, 0, true);

  auto* worldVis = new G4VisAttributes(G4Colour(0.9, 0.9, 1.0, 0.05));
  worldVis->SetVisibility(false);
  fWorldLV->SetVisAttributes(worldVis);

  // Construir la célula seleccionada (C. neoformans estándar o Humana epitelial)
  if (fCellType == CellType::kCryptococcusStandard) {
    ConstructCryptococcusStandard(fWorldLV);
  } else {
    ConstructHumanEpithelial(fWorldLV);
  }

  // Límite de paso máximo de 10 nm dentro de los volúmenes celulares para alta resolución espacial
  delete fStepLimit;
  fStepLimit = new G4UserLimits(10.0 * nm);
  if (fOuterLayerLV)   fOuterLayerLV->SetUserLimits(fStepLimit);
  if (fWallOrCortexLV) fWallOrCortexLV->SetUserLimits(fStepLimit);
  if (fCytoplasmLV)    fCytoplasmLV->SetUserLimits(fStepLimit);
  if (fNucleusLV)      fNucleusLV->SetUserLimits(fStepLimit);

  return worldPV;
}

G4VPhysicalVolume* DetectorConstruction::ConstructCryptococcusStandard(G4LogicalVolume* worldLV)
{
  // Semiejes del cuerpo celular (relación de aspecto cuasiesférica/ovoidal 1.0 - 1.25)
  G4double rBodyXY = 0.5 * fCryptoBodyDiameter;
  G4double rBodyZ  = rBodyXY * fCryptoAspectRatio;

  // 1. CÁPSULA POLISACÁRIDA (GXM + GalXM + MP en 99% agua)
  G4double rCapXY = rBodyXY + fCryptoCapsuleThickness;
  G4double rCapZ  = rBodyZ  + fCryptoCapsuleThickness;

  auto* capsuleSolid = new G4Ellipsoid("Crypto_Capsule_Solid", rCapXY, rCapXY, rCapZ);
  fOuterLayerLV = new G4LogicalVolume(capsuleSolid, fMatCapsuleCrypto, "OuterLayer_LV");
  auto* capsulePV = new G4PVPlacement(
    nullptr, G4ThreeVector(), fOuterLayerLV, "Crypto_Capsule_PV", worldLV, false, 0, true);

  // 2. PARED CELULAR MELANINIZADA (Quitosano + Quitina + Glucanos + Melanina)
  auto* wallSolid = new G4Ellipsoid("Crypto_Wall_Solid", rBodyXY, rBodyXY, rBodyZ);
  fWallOrCortexLV = new G4LogicalVolume(wallSolid, fMatCellWallCrypto, "WallOrCortex_LV");
  new G4PVPlacement(
    nullptr, G4ThreeVector(), fWallOrCortexLV, "Crypto_Wall_PV", fOuterLayerLV, false, 0, true);

  // 3. CITOPLASMA (Interior a la pared celular; incluye efecto de bicapa con ergosterol)
  G4double rCytoXY = rBodyXY - fCryptoWallThickness;
  G4double rCytoZ  = rBodyZ  - fCryptoWallThickness;

  auto* cytoSolid = new G4Ellipsoid("Crypto_Cytoplasm_Solid", rCytoXY, rCytoXY, rCytoZ);
  fCytoplasmLV = new G4LogicalVolume(cytoSolid, fMatCytoplasmCrypto, "Cytoplasm_LV");
  new G4PVPlacement(
    nullptr, G4ThreeVector(), fCytoplasmLV, "Crypto_Cytoplasm_PV", fWallOrCortexLV, false, 0, true);

  // 4. NÚCLEO HAPLOIDE (1C) desplazado ligeramente por la vacuola central prominente
  G4double nucPosZ = 0.35 * rCytoZ;
  auto* nucleusSolid = new G4Orb("Crypto_Nucleus_Solid", fCryptoNucleusRadius);
  fNucleusLV = new G4LogicalVolume(nucleusSolid, fMatNucleusCrypto, "Nucleus_LV");
  new G4PVPlacement(
    nullptr, G4ThreeVector(0., 0., nucPosZ), fNucleusLV, "Crypto_Nucleus_PV", fCytoplasmLV, false, 0, true);

  // Atributos visuales (Colores diferenciados por capa biológica)
  fOuterLayerLV->SetVisAttributes(new G4VisAttributes(G4Colour(0.2, 0.8, 1.0, 0.25)));   // Azul claro translúcido (Cápsula GXM)
  fWallOrCortexLV->SetVisAttributes(new G4VisAttributes(G4Colour(0.45, 0.28, 0.15, 0.55))); // Marrón oscuro (Pared con melanina)
  fCytoplasmLV->SetVisAttributes(new G4VisAttributes(G4Colour(0.3, 0.9, 0.4, 0.30)));    // Verde suave (Citoplasma)
  fNucleusLV->SetVisAttributes(new G4VisAttributes(G4Colour(0.9, 0.2, 0.2, 0.85)));      // Rojo intenso (Núcleo 1C)

  G4cout << "\n==================================================================\n"
         << " [DetectorConstruction] Geometria activa: Cryptococcus neoformans (Forma Estandar)\n"
         << "   * Diametro del cuerpo celular (XY): " << G4BestUnit(fCryptoBodyDiameter, "Length") << "\n"
         << "   * Relacion de aspecto (Z/XY)      : " << fCryptoAspectRatio << "\n"
         << "   * Espesor de capsula (GXM)        : " << G4BestUnit(fCryptoCapsuleThickness, "Length")
         << " (Indice capsular t_cap/r_cuerpo = " << (fCryptoCapsuleThickness / rBodyXY) << ")\n"
         << "   * Espesor de pared celular        : " << G4BestUnit(fCryptoWallThickness, "Length") << "\n"
         << "   * Radio nuclear haploide (1C)     : " << G4BestUnit(fCryptoNucleusRadius, "Length") << "\n"
         << "==================================================================\n" << G4endl;

  return capsulePV;
}

G4VPhysicalVolume* DetectorConstruction::ConstructHumanEpithelial(G4LogicalVolume* worldLV)
{
  // Dimensiones de célula epitelial cúbica/polarizada
  G4double halfW = 0.5 * fHumanCellWidth;
  G4double halfH = 0.5 * fHumanCellHeight;

  // 1. GLUCOCÁLIZ EXTERNO (Cubierta rica en ácido siálico, glucoproteínas y proteoglicanos)
  G4double outerHalfW = halfW + fHumanGlycocalyxThickness;
  G4double outerHalfH = halfH + fHumanGlycocalyxThickness;

  auto* glycoSolid = new G4Box("Human_Glycocalyx_Solid", outerHalfW, outerHalfW, outerHalfH);
  fOuterLayerLV = new G4LogicalVolume(glycoSolid, fMatGlycocalyxHuman, "OuterLayer_LV");
  auto* glycoPV = new G4PVPlacement(
    nullptr, G4ThreeVector(), fOuterLayerLV, "Human_Glycocalyx_PV", worldLV, false, 0, true);

  // 2. COMPLEJO MEMBRANA PLASMÁTICA (25% Colesterol + Esfingolípidos) Y CORTEZA DE ACTINA/UNIONES
  auto* cortexSolid = new G4Box("Human_MembraneCortex_Solid", halfW, halfW, halfH);
  fWallOrCortexLV = new G4LogicalVolume(cortexSolid, fMatMembraneCortexHuman, "WallOrCortex_LV");
  new G4PVPlacement(
    nullptr, G4ThreeVector(), fWallOrCortexLV, "Human_MembraneCortex_PV", fOuterLayerLV, false, 0, true);

  // 3. CITOPLASMA POLARIZADO (Con citoqueratinas I y II, microtúbulos y mitocondrias)
  G4double cytoHalfW = halfW - fHumanCortexThickness;
  G4double cytoHalfH = halfH - fHumanCortexThickness;

  auto* cytoSolid = new G4Box("Human_Cytoplasm_Solid", cytoHalfW, cytoHalfW, cytoHalfH);
  fCytoplasmLV = new G4LogicalVolume(cytoSolid, fMatCytoplasmHuman, "Cytoplasm_LV");
  new G4PVPlacement(
    nullptr, G4ThreeVector(), fCytoplasmLV, "Human_Cytoplasm_PV", fWallOrCortexLV, false, 0, true);

  // 4. NÚCLEO DIPLOIDE (2C) posicionado central-basalmente
  G4double nucPosZ = -0.15 * cytoHalfH;
  auto* nucleusSolid = new G4Orb("Human_Nucleus_Solid", fHumanNucleusRadius);
  fNucleusLV = new G4LogicalVolume(nucleusSolid, fMatNucleusHuman, "Nucleus_LV");
  new G4PVPlacement(
    nullptr, G4ThreeVector(0., 0., nucPosZ), fNucleusLV, "Human_Nucleus_PV", fCytoplasmLV, false, 0, true);

  // Atributos visuales
  fOuterLayerLV->SetVisAttributes(new G4VisAttributes(G4Colour(0.95, 0.85, 0.2, 0.25)));  // Amarillo suave (Glucocáliz)
  fWallOrCortexLV->SetVisAttributes(new G4VisAttributes(G4Colour(1.0, 0.5, 0.1, 0.45)));  // Naranja (Membrana/Corteza)
  fCytoplasmLV->SetVisAttributes(new G4VisAttributes(G4Colour(0.85, 0.55, 0.75, 0.30)));  // Rosa/Violeta (Citoplasma epitelial)
  fNucleusLV->SetVisAttributes(new G4VisAttributes(G4Colour(0.2, 0.3, 0.95, 0.85)));      // Azul intenso (Núcleo diploide 2C)

  G4double volumeUm3 = (fHumanCellWidth * fHumanCellWidth * fHumanCellHeight) / (um * um * um);
  G4double areaUm2   = 2.0 * (fHumanCellWidth * fHumanCellWidth + 2.0 * fHumanCellWidth * fHumanCellHeight) / (um * um);

  G4cout << "\n==================================================================\n"
         << " [DetectorConstruction] Geometria activa: Celula Humana Epitelial (Cubica Polarizada)\n"
         << "   * Ancho lateral (X, Y)            : " << G4BestUnit(fHumanCellWidth, "Length") << "\n"
         << "   * Altura apico-basal (Z)          : " << G4BestUnit(fHumanCellHeight, "Length") << "\n"
         << "   * Volumen celular                 : " << volumeUm3 << " um3\n"
         << "   * Relacion Area/Volumen (A/V)     : " << (areaUm2 / volumeUm3) << " um^-1\n"
         << "   * Espesor de glucocaliz           : " << G4BestUnit(fHumanGlycocalyxThickness, "Length") << "\n"
         << "   * Radio nuclear diploide (2C)     : " << G4BestUnit(fHumanNucleusRadius, "Length") << "\n"
         << "==================================================================\n" << G4endl;

  return glycoPV;
}

void DetectorConstruction::ConstructSDandField()
{
  // El registro de dosis y especies químicas se realiza en SteppingAction y TimeStepAction
}

// --- Setters con notificación al G4RunManager para reconstruir geometría ---

void DetectorConstruction::SetCellType(const G4String& typeName)
{
  if (typeName == "cryptococcus" || typeName == "crypto" || typeName == "yeast") {
    fCellType = CellType::kCryptococcusStandard;
  } else if (typeName == "human" || typeName == "epithelial" || typeName == "hepatic") {
    fCellType = CellType::kHumanEpithelial;
  } else {
    G4Exception("DetectorConstruction::SetCellType", "InvalidCellType", JustWarning,
                ("Tipo celular no reconocido: " + typeName + ". Use 'cryptococcus' o 'human'.").c_str());
    return;
  }
  if (G4StateManager::GetStateManager()->GetCurrentState() != G4State_PreInit) {
    G4RunManager::GetRunManager()->ReinitializeGeometry();
  }
}

void DetectorConstruction::SetCryptoCellDiameter(G4double val)
{
  if (val > 0.) {
    fCryptoBodyDiameter = val;
    if (G4StateManager::GetStateManager()->GetCurrentState() != G4State_PreInit) {
      G4RunManager::GetRunManager()->ReinitializeGeometry();
    }
  }
}

void DetectorConstruction::SetCryptoCapsuleThickness(G4double val)
{
  if (val > 0.) {
    fCryptoCapsuleThickness = val;
    if (G4StateManager::GetStateManager()->GetCurrentState() != G4State_PreInit) {
      G4RunManager::GetRunManager()->ReinitializeGeometry();
    }
  }
}

void DetectorConstruction::SetCryptoWallThickness(G4double val)
{
  if (val > 0.) {
    fCryptoWallThickness = val;
    if (G4StateManager::GetStateManager()->GetCurrentState() != G4State_PreInit) {
      G4RunManager::GetRunManager()->ReinitializeGeometry();
    }
  }
}

void DetectorConstruction::SetCryptoAspectRatio(G4double val)
{
  if (val >= 1.0 && val <= 1.5) {
    fCryptoAspectRatio = val;
    if (G4StateManager::GetStateManager()->GetCurrentState() != G4State_PreInit) {
      G4RunManager::GetRunManager()->ReinitializeGeometry();
    }
  }
}

void DetectorConstruction::SetMelaninMassFraction(G4double val)
{
  if (val >= 0.0 && val <= 0.5) {
    fMelaninFractionInWall = val;
  }
}

void DetectorConstruction::SetHumanCellWidth(G4double val)
{
  if (val > 0.) {
    fHumanCellWidth = val;
    if (G4StateManager::GetStateManager()->GetCurrentState() != G4State_PreInit) {
      G4RunManager::GetRunManager()->ReinitializeGeometry();
    }
  }
}

void DetectorConstruction::SetHumanCellHeight(G4double val)
{
  if (val > 0.) {
    fHumanCellHeight = val;
    if (G4StateManager::GetStateManager()->GetCurrentState() != G4State_PreInit) {
      G4RunManager::GetRunManager()->ReinitializeGeometry();
    }
  }
}

void DetectorConstruction::SetHumanGlycocalyxThickness(G4double val)
{
  if (val > 0.) {
    fHumanGlycocalyxThickness = val;
    if (G4StateManager::GetStateManager()->GetCurrentState() != G4State_PreInit) {
      G4RunManager::GetRunManager()->ReinitializeGeometry();
    }
  }
}

G4String DetectorConstruction::GetCellTypeName() const
{
  return (fCellType == CellType::kCryptococcusStandard)
           ? "Cryptococcus_neoformans_Standard"
           : "Human_Epithelial_Reference";
}

// --- Cálculo exacto de la masa neta de cada capa (descontando volúmenes hijos) ---

G4double DetectorConstruction::GetOuterLayerMass() const
{
  if (!fOuterLayerLV || !fWallOrCortexLV) return 0.;
  G4double outerVol = fOuterLayerLV->GetSolid()->GetCubicVolume()
                    - fWallOrCortexLV->GetSolid()->GetCubicVolume();
  return outerVol * fOuterLayerLV->GetMaterial()->GetDensity();
}

G4double DetectorConstruction::GetWallOrCortexMass() const
{
  if (!fWallOrCortexLV || !fCytoplasmLV) return 0.;
  G4double wallVol = fWallOrCortexLV->GetSolid()->GetCubicVolume()
                   - fCytoplasmLV->GetSolid()->GetCubicVolume();
  return wallVol * fWallOrCortexLV->GetMaterial()->GetDensity();
}

G4double DetectorConstruction::GetCytoplasmMass() const
{
  if (!fCytoplasmLV || !fNucleusLV) return 0.;
  G4double cytoVol = fCytoplasmLV->GetSolid()->GetCubicVolume()
                   - fNucleusLV->GetSolid()->GetCubicVolume();
  return cytoVol * fCytoplasmLV->GetMaterial()->GetDensity();
}

G4double DetectorConstruction::GetNucleusMass() const
{
  if (!fNucleusLV) return 0.;
  return fNucleusLV->GetSolid()->GetCubicVolume() * fNucleusLV->GetMaterial()->GetDensity();
}

