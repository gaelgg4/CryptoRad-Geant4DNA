#ifndef DetectorMessenger_h
#define DetectorMessenger_h 1

#include "G4UImessenger.hh"
#include "globals.hh"

class DetectorConstruction;
class G4UIdirectory;
class G4UIcmdWithAString;
class G4UIcmdWithADoubleAndUnit;
class G4UIcmdWithADouble;

/**
 * @class DetectorMessenger
 * @brief Interfaz de comandos de macro (/cell/...) para alternar entre
 *        Cryptococcus neoformans (forma estándar) y célula epitelial humana,
 *        así como ajustar espesores de cápsula, pared celular y dimensiones.
 */
class DetectorMessenger : public G4UImessenger
{
public:
  explicit DetectorMessenger(DetectorConstruction* det);
  ~DetectorMessenger() override;

  void SetNewValue(G4UIcommand* command, G4String newValue) override;

private:
  DetectorConstruction*      fDetector{nullptr};
  G4UIdirectory*             fCellDir{nullptr};

  G4UIcmdWithAString*        fCellTypeCmd{nullptr};
  G4UIcmdWithADoubleAndUnit* fCryptoDiameterCmd{nullptr};
  G4UIcmdWithADoubleAndUnit* fCryptoCapsuleCmd{nullptr};
  G4UIcmdWithADoubleAndUnit* fCryptoWallCmd{nullptr};
  G4UIcmdWithADouble*        fCryptoAspectCmd{nullptr};
  G4UIcmdWithADouble*        fMelaninFractionCmd{nullptr};

  G4UIcmdWithADoubleAndUnit* fHumanWidthCmd{nullptr};
  G4UIcmdWithADoubleAndUnit* fHumanHeightCmd{nullptr};
  G4UIcmdWithADoubleAndUnit* fHumanGlycocalyxCmd{nullptr};
};

#endif

