import sys
from geant4_pybind import *

def main():
    # Inicializa la interfaz de usuario de Geant4
    ui = G4UIExecutive(len(sys.argv), sys.argv)

    # Inicia la sesión (esto abrirá la ventana gráfica, usualmente Qt o OpenGL)
    ui.SessionStart()

if __name__ == "__main__":
    main()