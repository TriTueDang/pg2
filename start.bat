@echo off
setlocal enabledelayedexpansion

set PRESET=windows-vcpkg
set EXE=build\pg2.exe

echo Building project using preset: %PRESET%...

cmake --preset %PRESET%
if %ERRORLEVEL% neq 0 (
    echo Configuration failed! Cleaning build directory and retrying...
    if exist build (
        rd /s /q build
    )
    cmake --preset %PRESET%
    if %ERRORLEVEL% neq 0 (
        echo Configuration failed again!
        pause
        exit /b %ERRORLEVEL%
    )
)

cmake --build build --target pg2
if %ERRORLEVEL% neq 0 (
    echo ------------------------------------------
    echo BUILD FAILED! Please check the errors above.
    echo ------------------------------------------
    pause
    exit /b %ERRORLEVEL%
)

echo ------------------------------------------
echo BUILD SUCCESSFUL!
echo ------------------------------------------

if exist %EXE% (
    echo Running internal tests...
    %EXE% --test
    if %ERRORLEVEL% neq 0 (
        echo ------------------------------------------
        echo TESTS FAILED! Application will not start.
        echo ------------------------------------------
        pause
        exit /b %ERRORLEVEL%
    )
    echo TESTS PASSED!
    echo ------------------------------------------
    
    echo Starting Chicken Gun Story...
    %EXE%
) else (
    echo Executable not found: %EXE%
    pause
    exit /b 1
)
