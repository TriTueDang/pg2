@echo off
echo Starting Chicken Gun Story with High Performance GPU...

:: Force high performance on some systems via environment variables
set DRI_PRIME=1
set __NV_PRIME_RENDER_OFFLOAD=1
set __GLX_VENDOR_LIBRARY_NAME=nvidia

if exist "build\pg2.exe" (
    "build\pg2.exe"
) else (
    if exist "build\Debug\pg2.exe" (
        "build\Debug\pg2.exe"
    ) else (
        if exist "build\Release\pg2.exe" (
            "build\Release\pg2.exe"
        ) else (
            echo Error: pg2.exe not found! Please build the project first.
            pause
        )
    )
)
pause
