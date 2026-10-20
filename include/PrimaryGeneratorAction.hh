#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "globals.hh"
#include "G4SystemOfUnits.hh"

class G4ParticleGun;
class G4Event;
class G4GenericMessenger;

/**
 * @class PrimaryGeneratorAction
 * @brief Configura el haz primario de radiación ionizante (electrones, protones, alfa).
 *
 * Permite tanto un haz puntual axial (por defecto en Z = -12 um apuntando hacia +Z)
 * como un microhaz circular uniforme de radio configurable (/beam/setSpotRadius)
 * para irradiar la sección transversal de C. neoformans o de la célula epitelial humana.
 */
class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
public:
  PrimaryGeneratorAction();
  ~PrimaryGeneratorAction() override;

  void GeneratePrimaries(G4Event* event) override;

  const G4ParticleGun* GetParticleGun() const { return fParticleGun; }
  void SetBeamSpotRadius(G4double radius) { fBeamSpotRadius = radius; }
  void SetBeamStartZ(G4double zPos) { fBeamStartZ = zPos; }

private:
  G4ParticleGun*      fParticleGun{nullptr};
  G4GenericMessenger* fMessenger{nullptr};
  G4double            fBeamSpotRadius{0.0 * um};
  G4double            fBeamStartZ{-10.0 * um};
};

#endif

