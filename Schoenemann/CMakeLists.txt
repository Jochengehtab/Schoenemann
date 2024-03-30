# CMakeList.txt: CMake-Projekt für "Schoenemann". Schließen Sie die Quelle ein, und definieren Sie
# projektspezifische Logik hier.
#

# Fügen Sie der ausführbaren Datei dieses Projekts eine Quelle hinzu.
add_executable (Schoenemann "Schoenemann.cpp" "Schoenemann.h" "Evaluate.cpp" "Evaluate.h" "Search.cpp" "Search.h"         "movegen/benchmark.hpp" "timeman.cpp" "timeman.h" "psqt.cpp" "psqt.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Schoenemann PROPERTY CXX_STANDARD 20)
endif()

# TODO: Fügen Sie bei Bedarf Tests hinzu, und installieren Sie Ziele.
