cmake_minimum_required(VERSION 3.16)
project(VoleMachineSimulator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# Existing C++ simulator backend sources (unmodified)
set(SIMULATOR_SOURCES
    ALU.cpp
    Cpu.cpp
    Instructions.cpp
    Memory.cpp
    Programloader.cpp
    RegisterFile.cpp
    Screen.cpp
    Volemachine.cpp
)

add_library(vole_simulator_core STATIC ${SIMULATOR_SOURCES} VoleMachine.h)
target_include_directories(vole_simulator_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Qt Widgets GUI sources
set(GUI_SOURCES
    gui/main.cpp
    gui/MainWindow.cpp
    gui/MainWindow.h
    gui/HomePage.cpp
    gui/HomePage.h
    gui/CellFlashAnimator.cpp
    gui/CellFlashAnimator.h
    gui/ProgramControlsPanel.cpp
    gui/ProgramControlsPanel.h
    gui/MachineStatusPanel.cpp
    gui/MachineStatusPanel.h
    gui/ScreenPanel.cpp
    gui/ScreenPanel.h
    gui/RegisterGridPanel.cpp
    gui/RegisterGridPanel.h
    gui/MemoryMatrixPanel.cpp
    gui/MemoryMatrixPanel.h
    gui/ValueInspectorPanel.cpp
    gui/ValueInspectorPanel.h
    gui/ThemeConfig.h
    resources/resources.qrc
)

add_executable(VoleMachineSimulator WIN32 ${GUI_SOURCES})
target_include_directories(VoleMachineSimulator PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/gui
)
target_link_libraries(VoleMachineSimulator PRIVATE
    vole_simulator_core
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

# Copy resources folder next to executable for runtime stylesheet/background loading
add_custom_command(TARGET VoleMachineSimulator POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/resources"
        "$<TARGET_FILE_DIR:VoleMachineSimulator>/resources"
    COMMENT "Copying resources directory to build folder..."
)

if(WIN32)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${Qt6_DIR}/../../../bin")
    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET VoleMachineSimulator POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                --no-translations
                --no-system-d3d-compiler
                --no-opengl-sw
                "$<TARGET_FILE:VoleMachineSimulator>"
            COMMENT "Deploying Qt runtime libraries to build directory via windeployqt..."
        )
    endif()
endif()
