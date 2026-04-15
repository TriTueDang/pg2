#!/bin/bash

# Zjištění operačního systému
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    PRESET="msvc-vcpkg"
    EXE="./build/pg2"
elif [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" ]]; then
    PRESET="windows-vcpkg"
    EXE="./build/pg2.exe"
else
    echo "Unsupported OS: $OSTYPE"
    exit 1
fi

echo "Building project using preset: $PRESET..."

# Pokus o konfiguraci
cmake --preset "$PRESET"
if [ $? -ne 0 ]; then
    echo "Configuration failed! Cleaning build directory and retrying..."
    rm -rf build
    cmake --preset "$PRESET"
    if [ $? -ne 0 ]; then
        echo "Configuration failed again!"
        exit 1
    fi
fi

# Sestavení (KROK: Kontrola kompilace)
echo "------------------------------------------"
echo "STEP 1: COMPILATION CHECK"
echo "------------------------------------------"
cmake --build build --target pg2
if [ $? -ne 0 ]; then
    echo "------------------------------------------"
    echo "BUILD FAILED! Please check the errors above."
    echo "------------------------------------------"
    exit 1
fi
echo "BUILD SUCCESSFUL!"

# Spuštění testů (KROK: Kontrola integrity)
echo "------------------------------------------"
echo "STEP 2: INTEGRITY TESTS"
echo "------------------------------------------"
if [ -f "$EXE" ]; then
    $EXE --test
    if [ $? -ne 0 ]; then
        echo "------------------------------------------"
        echo "TESTS FAILED! Application will not start."
        echo "------------------------------------------"
        exit 1
    fi
    echo "TESTS PASSED!"
else
    echo "Executable not found for tests!"
    exit 1
fi

echo "------------------------------------------"
echo "STEP 3: RUNNING GAME"
echo "------------------------------------------"
echo "Starting Chicken Gun Story..."
$EXE
