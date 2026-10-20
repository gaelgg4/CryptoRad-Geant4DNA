#include "PrimaryGeneratorAction.hh"

#include "G4Event.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4GenericMessenger.hh"
#include "G4PhysicalConstants.hh"
#include "Randomize.hh"

#include <cmath>

PrimaryGeneratorAction::PrimaryGeneratorAction()
  : G4VUserPrimaryGeneratorAction()
{
  G4int nParticles = 1;
  fParticleGun = new G4ParticleGun(nParticles);

  // Configuración por defecto: Electrón de 20 keV (dentro del rango óptimo de Geant4-DNA)
  // viajando en dirección +Z desde el exterior de la cápsula/glucocáliz hacia el centro celular.
  G4ParticleDefinition* particle = G4ParticleTable::GetParticleTable()->FindParticle("e-");
  fParticleGun->SetParticleDefinition(particle);
  fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0., 0., 1.));
  fParticleGun->SetParticleEnergy(20.0 * keV);
  fParticleGun->SetParticlePosition(G4ThreeVector(0., 0., fBeamStartZ));

  // Comandos de macro (/beam/...) para controlar el ancho del microhaz y la posición Z inicial
  fMessenger = new G4GenericMessenger(this, "/beam/", "Configuracion geometrica del microhaz primario");

  auto& radiusCmd = fMessenger->DeclarePropertyWithUnit(
    "spotRadius", "um", fBeamSpotRadius,
    "Radio del disco transversal de irradiacion (0 = haz puntual axial)");
  radiusCmd.SetParameterName("radius", false);
  radiusCmd.SetRange("radius>=0.");

  auto& zPosCmd = fMessenger->DeclarePropertyWithUnit(
    "startZ", "um", fBeamStartZ,
    "Coordenada Z inicial desde donde se dispara el haz hacia +Z");
  zPosCmd.SetParameterName("z0", false);
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
  delete fMessenger;
  delete fParticleGun;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event)
{
  G4double x0 = 0.0;
  G4double y0 = 0.0;

  // Si se define un radio de haz > 0, muestrear uniformemente en un disco circular XY
  if (fBeamSpotRadius > 0.0) {
    G4double r   = fBeamSpotRadius * std::sqrt(G4UniformRand());
    G4double phi = twopi * G4UniformRand();
    x0 = r * std::cos(phi);
    y0 = r * std::sin(phi);
  }

  fParticleGun->SetParticlePosition(G4ThreeVector(x0, y0, fBeamStartZ));
  fParticleGun->GeneratePrimaryVertex(event);
}

