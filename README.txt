# Vole Machine Simulator — Setup and Run Guide

## 1. Project Overview

This is a C++ desktop application that simulates the Vole Machine architecture through a graphical user interface developed using Qt Widgets. It allows users to load and execute Vole Machine programs, run instructions step by step or continuously, and interact with the CPU registers and memory. The interface also provides a simulated output screen and execution status information.

The project uses CMake for configuration and building.

## 2. Requirements

Before building the project, ensure that the following tools are installed and configured:

- **C++ Compiler:** A compiler supporting C++17.
- **CMake:** Used to configure and build the project.
- **Qt:** A compatible Qt installation with the Qt Widgets module, configured so that CMake can locate it.

Check that CMake is installed by running the following command in a terminal:

```bash
cmake --version
```

The command should display the installed CMake version.

## 3. Building the Project

1. Extract the submitted project ZIP file.
2. Open a terminal in the project directory containing `CMakeLists.txt`.
3. Configure the project:

   ```bash
   cmake -S . -B build
   ```

4. Build the application:

   ```bash
   cmake --build build
   ```

CMake automatically creates the `build` directory, so it does not need to be created manually or included in the submitted ZIP file.

## 4. Running the Application

After the build completes successfully, locate and run the generated Vole Machine Simulator executable inside the `build` directory.

For a typical MinGW build, the executable will be located in the build directory. Its exact name depends on the target configured in `CMakeLists.txt`.

Depending on the compiler and build configuration, it may instead be located in a subdirectory such as `build/Debug/` or `build/Release/`.

## 5. Using the Application

1. Launch the Vole Machine Simulator.
2. Click **START PROGRAM** on the home screen to open the simulator workspace.
3. Load a program from a file or enter instructions manually.
4. Set the starting memory address if needed.
5. Click **Load Program** to load the instructions into memory.
6. Use **Step** to execute one instruction at a time, or **Run All** to execute the program continuously.
7. Monitor the CPU registers, program counter (PC), instruction register (IR), memory contents, and simulated screen output during execution.
8. Use **Halt** to stop execution when needed.
9. Use **Reset Machine** to reset the simulator and start a new execution.

## 6. Troubleshooting

- **CMake is not recognized:** Install CMake from the [official CMake website](https://cmake.org/download/) or use Windows Package Manager:

  ```powershell
  winget install --id Kitware.CMake -e
  ```

  Restart the terminal after installation and verify it using `cmake --version`.

- **CMake cannot find Qt:** Ensure that Qt is installed with the required modules and that its installation path is configured for CMake. If necessary, specify the Qt installation directory using `CMAKE_PREFIX_PATH` when configuring the project.

- **The application fails to start because Qt DLLs are missing:** Run the application from a Qt-configured environment or deploy the required Qt libraries using Qt's `windeployqt` tool.

- **The executable cannot be found:** Check the generated `build` directory and its configuration-specific subdirectories.

- **The program fails to load:** Ensure that the program file contains valid Vole Machine instructions and that the selected starting address is within the valid memory range.