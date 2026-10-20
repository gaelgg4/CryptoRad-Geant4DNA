#include "DetectorMessenger.hh"
#include "DetectorConstruction.hh"

#include "G4UIdirectory.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithADouble.hh"

DetectorMessenger::DetectorMessenger(DetectorConstruction* det)
  : G4UImessenger(), fDetector(det)
{
  fCellDir = new G4UIdirectory("/cell/");
  fCellDir->SetGuidance("Configuracion geometrica y biofisica celular (C. neoformans vs Humana).");

  fCellTypeCmd = new G4UIcmdWithAString("/cell/setType", this);
  fCellTypeCmd->SetGuidance("Selecciona el modelo celular: 'cryptococcus' (forma estandar) o 'human' (epitelial).");
  fCellTypeCmd->SetParameterName("CellType", false);
  fCellTypeCmd->SetCandidates("cryptococcus crypto yeast human epithelial hepatic");
  fCellTypeCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

  fCryptoDiameterCmd = new G4UIcmdWithADoubleAndUnit("/cell/crypto/setBodyDiameter", this);
  fCryptoDiameterCmd->SetGuidance("Diametro del cuerpo celular de C. neoformans estandar (rango tipico: 4 - 10 um).");
  fCryptoDiameterCmd->SetParameterName("BodyDiameter", false);
  fCryptoDiameterCmd->SetDefaultUnit("um");
  fCryptoDiameterCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

  fCryptoCapsuleCmd = new G4UIcmdWithADoubleAndUnit("/cell/crypto/setCapsuleThickness", this);
  fCryptoCapsuleCmd->SetGuidance("Espesor de la capsula de GXM en C. neoformans estandar (0.5 - 2.0 um).");
  fCryptoCapsuleCmd->SetParameterName("CapsuleThickness", false);
  fCryptoCapsuleCmd->SetDefaultUnit("um");
  fCryptoCapsuleCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

  fCryptoWallCmd = new G4UIcmdWithADoubleAndUnit("/cell/crypto/setWallThickness", this);
  fCryptoWallCmd->SetGuidance("Espesor de la pared celular (quitosano/glucanos/melanina: 100 - 200 nm).");
  fCryptoWallCmd->SetParameterName("WallThickness", false);
  fCryptoWallCmd->SetDefaultUnit("nm");
  fCryptoWallCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

  fCryptoAspectCmd = new G4UIcmdWithADouble("/cell/crypto/setAspectRatio", this);
  fCryptoAspectCmd->SetGuidance("Relacion de aspecto cuasiesferica/ovoidal (1.0 - 1.25).");
  fCryptoAspectCmd->SetParameterName("AspectRatio", false);
  fCryptoAspectCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

  fMelaninFractionCmd = new G4UIcmdWithADouble("/cell/crypto/setMelaninFraction", this);
  fMelaninFractionCmd->SetGuidance("Fraccion relativa de melanina en pared para captura de ROS (0.0 a 0.5).");
  fMelaninFractionCmd->SetParameterName("MelaninFraction", false);
  fMelaninFractionCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

  fHumanWidthCmd = new G4UIcmdWithADoubleAndUnit("/cell/human/setWidth", this);
  fHumanWidthCmd->SetGuidance("Ancho lateral (X, Y) de la celula epitelial humana (10 - 50 um).");
  fHumanWidthCmd->SetParameterName("Width", false);
  fHumanWidthCmd->SetDefaultUnit("um");
  fHumanWidthCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

  fHumanHeightCmd = new G4UIcmdWithADoubleAndUnit("/cell/human/setHeight", this);
  fHumanHeightCmd->SetGuidance("Altura apico-basal (Z) de la celula epitelial humana (1 - 35 um).");
  fHumanHeightCmd->SetParameterName("Height", false);
  fHumanHeightCmd->SetDefaultUnit("um");
  fHumanHeightCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

  fHumanGlycocalyxCmd = new G4UIcmdWithADoubleAndUnit("/cell/human/setGlycocalyxThickness", this);
  fHumanGlycocalyxCmd->SetGuidance("Espesor del glucocaliz humano (50 - 200 nm).");
  fHumanGlycocalyxCmd->SetParameterName("GlycocalyxThickness", false);
  fHumanGlycocalyxCmd->SetDefaultUnit("nm");
  fHumanGlycocalyxCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
}

DetectorMessenger::~DetectorMessenger()
{
  delete fCellTypeCmd;
  delete fCryptoDiameterCmd;
  delete fCryptoCapsuleCmd;
  delete fCryptoWallCmd;
  delete fCryptoAspectCmd;
  delete fMelaninFractionCmd;
  delete fHumanWidthCmd;
  delete fHumanHeightCmd;
  delete fHumanGlycocalyxCmd;
  delete fCellDir;
}

void DetectorMessenger::SetNewValue(G4UIcommand* command, G4String newValue)
{
  if (command == fCellTypeCmd) {
    fDetector->SetCellType(newValue);
  } else if (command == fCryptoDiameterCmd) {
    fDetector->SetCryptoCellDiameter(fCryptoDiameterCmd->GetNewDoubleValue(newValue));
  } else if (command == fCryptoCapsuleCmd) {
    fDetector->SetCryptoCapsuleThickness(fCryptoCapsuleCmd->GetNewDoubleValue(newValue));
  } else if (command == fCryptoWallCmd) {
    fDetector->SetCryptoWallThickness(fCryptoWallCmd->GetNewDoubleValue(newValue));
  } else if (command == fCryptoAspectCmd) {
    fDetector->SetCryptoAspectRatio(fCryptoAspectCmd->GetNewDoubleValue(newValue));
  } else if (command == fMelaninFractionCmd) {
    fDetector->SetMelaninMassFraction(fMelaninFractionCmd->GetNewDoubleValue(newValue));
  } else if (command == fHumanWidthCmd) {
    fDetector->SetHumanCellWidth(fHumanWidthCmd->GetNewDoubleValue(newValue));
  } else if (command == fHumanHeightCmd) {
    fDetector->SetHumanCellHeight(fHumanHeightCmd->GetNewDoubleValue(newValue));
  } else if (command == fHumanGlycocalyxCmd) {
    fDetector->SetHumanGlycocalyxThickness(fHumanGlycocalyxCmd->GetNewDoubleValue(newValue));
  }
}

