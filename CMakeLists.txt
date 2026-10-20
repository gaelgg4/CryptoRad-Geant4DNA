cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(CryptoRadG4DNA VERSION 1.0.0 LANGUAGES CXX)

#----------------------------------------------------------------------------
# Estándar de C++ requerido (Geant4 11+ utiliza C++17 por defecto)
#----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# En macOS con CommandLineTools 15+, asegurar la ruta de cabeceras libc++ (c++/v1) del SDK
if(APPLE AND EXISTS "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include/c++/v1")
  add_compile_options(-isystem /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include/c++/v1)
endif()

# Ruta por defecto de Geant4 11.2.1 en el sistema local si no se especifica -DGeant4_DIR
if(NOT Geant4_DIR)
  set(Geant4_DIR "/Users/gaelgarciachavez/HongoSim/geant4-install/lib/cmake/Geant4" CACHE PATH "Geant4 CMake directory" FORCE)
endif()

#----------------------------------------------------------------------------
# Buscar el paquete Geant4 con soporte de interfaz gráfica (ui_all) y visualización (vis_all)
#----------------------------------------------------------------------------
option(WITH_GEANT4_UIVIS "Build simulation with Geant4 UI and Vis drivers" ON)
if(WITH_GEANT4_UIVIS)
  find_package(Geant4 REQUIRED ui_all vis_all)
else()
  find_package(Geant4 REQUIRED)
endif()

#----------------------------------------------------------------------------
# Configurar definiciones y directorios de inclusión de Geant4 y del proyecto
#----------------------------------------------------------------------------
include(${Geant4_USE_FILE})
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${Geant4_INCLUDE_DIRS})

#----------------------------------------------------------------------------
# Localizar archivos fuente (.cc) y cabeceras (.hh)
#----------------------------------------------------------------------------
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

#----------------------------------------------------------------------------
# Definir el ejecutable principal y enlazar librerías de Geant4
#----------------------------------------------------------------------------
add_executable(crypto_rad_sim main.cc ${sources} ${headers})
target_link_libraries(crypto_rad_sim ${Geant4_LIBRARIES})

#----------------------------------------------------------------------------
# Copiar scripts de macros (.mac, .in) al directorio de compilación (build)
#----------------------------------------------------------------------------
file(GLOB MACRO_FILES
  "${PROJECT_SOURCE_DIR}/macros/*.mac"
  "${PROJECT_SOURCE_DIR}/macros/*.in"
)
foreach(_script ${MACRO_FILES})
  get_filename_component(_script_name ${_script} NAME)
  configure_file(
    ${_script}
    ${PROJECT_BINARY_DIR}/${_script_name}
    COPYONLY
  )
endforeach()

#----------------------------------------------------------------------------
# Instalación opcional del binario
#----------------------------------------------------------------------------
install(TARGETS crypto_rad_sim DESTINATION bin)

