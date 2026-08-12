# cpp-lib

[![CI](https://github.com/bang346/cpp-lib/actions/workflows/ci.yml/badge.svg)](https://github.com/bang346/cpp-lib/actions/workflows/ci.yml)

A modular C++17 library containing reusable components for embedded systems, motor control, communication protocols, hardware abstraction, and general-purpose utilities.

The project is organized as a collection of independent CMake targets that can be used together or integrated selectively into another CMake project.

## Features

### Mathematics and Motor Control

* CRC calculation
* Clarke and Park transformations
* Space Vector PWM (SVPWM)
* PID control
* General control-system components
* Six-step BLDC motor commutation

### Communication

* Binary serialization and deserialization
* Message encoding and decoding
* Frame handling
* Bus master components
* Binary containers
* Communication interfaces

### Hardware Abstraction

* Bus interface
* Coder interface
* Delay interface
* GPIO interface
* SPI mock implementation for unit testing
* STM32-specific components under development

### Device Drivers

* DRV8353 three-phase gate-driver interface

### Helper Utilities

* CSV parsing
* Pretty-printing utilities

## Requirements

* A compiler with C++17 support
* CMake 3.23 or newer
* Git
* Internet access during the initial CMake configuration

The initial configuration downloads external dependencies using CMake `FetchContent`, including GoogleTest and `csv-parser`.

## Repository Structure

```text
cpp-lib/
├── Core/
│   ├── Communication/       Communication and protocol components
│   ├── Control/             PID and control-system components
│   ├── Interface/           Hardware-independent interfaces
│   ├── Math/                CRC, Clarke/Park and SVPWM
│   ├── Motor/SixStep/       Six-step motor commutation
│   └── parts/DRV8353/       DRV8353 gate-driver interface
├── Hal/
│   ├── mock/                Mock implementations for unit tests
│   └── stm32/               STM32-specific implementations
├── Helper/
│   ├── csv_parser/          CSV parser wrapper
│   └── prittyprinting/      Output formatting utilities
├── tests/                   GoogleTest-based unit tests
└── CMakeLists.txt           Top-level CMake configuration
```

## Building

Clone the repository:

```bash
git clone https://github.com/bang346/cpp-lib.git
cd cpp-lib
```

Configure and build the library:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

For a debug build:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
```

Or to use Presets: 

```bash
cmake --list-presets
cmake --preset clang-asan
cmake --build --preset clang-asan
```

To Configure all:
```bash
Start-Job { cmake --preset msvc-asan }; Start-Job { cmake --preset gcc-asan } ; Start-Job { cmake --preset clang-asan }
```

To Build all:
```bash
Start-Job { cmake --build --preset msvc-asan }; Start-Job { cmake --build --preset gcc-asan } ; Start-Job { cmake --build --preset clang-asan }
```

## Building and Running the Tests

Enable the tests during configuration:

```bash
cmake -S . -B build -DCPPLIB_BUILD_TESTS=ON -DCPPLIB_TARGET=WINDOWS -DCMAKE_BUILD_TYPE=Debug -DCPPLIB_BUILD_HARDWARETESTS=ON
```

```bash
cmake -S . -B build \
    -DCPPLIB_BUILD_TESTS=ON \
    -DCPPLIB_TARGET=STM32 \
    -DCMAKE_BUILD_TYPE=Debug
```

Build the project:

```bash
cmake --build build --config Debug
```

Run all tests:

```bash
ctest --test-dir build -C Debug --output-on-failure
```

On a single-config generator such as Ninja, the configuration can also be built and tested with:

```bash
cmake -S . -B build -G Ninja \
    -DCPPLIB_BUILD_TESTS=ON \
    -DCMAKE_BUILD_TYPE=Debug

cmake --build build
ctest --test-dir build --output-on-failure
```

## Using cpp-lib in Another CMake Project

Add the repository to your project, for example as a Git submodule:

```bash
git submodule add https://github.com/bang346/cpp-lib.git external/cpp-lib
git submodule update --init --recursive
```

Add it from your root `CMakeLists.txt`:

```cmake
add_subdirectory(external/cpp-lib)
```

Link the required modules to your target:

```cmake
add_executable(my_application
    src/main.cpp
)

target_link_libraries(my_application
    PRIVATE
        motor::sixstep
        parts::DRV8353
        helper::csv_parser
        helper::prittyprinting
)
```

Only link the modules required by your application.

## CMake Options

| Option               | Default | Description                                                                                     |
| -------------------- | ------: | ----------------------------------------------------------------------------------------------- |
| `CPPLIB_BUILD_TESTS` |   `OFF` | Builds the GoogleTest-based unit tests when the project is configured as the top-level project. |

Example:

```bash
cmake -S . -B build -DCPPLIB_BUILD_TESTS=ON
```

## Continuous Integration

GitHub Actions automatically configures, builds, and tests the project on:

* Ubuntu
* Windows

The workflow runs for pushes and pull requests targeting the `main` branch.

## Project Status

This repository is under active development. Interfaces, target names, and module structure may change while the library evolves.

## Contributing

Contributions, bug reports, and improvement suggestions are welcome.

A typical contribution workflow is:

1. Fork the repository.
2. Create a feature branch.
3. Add or update tests.
4. Verify that the project builds successfully.
5. Open a pull request against `main`.

## License

No license has been specified yet. Add a `LICENSE` file before distributing or reusing the project outside its intended environment.
