$BuildDebugPath="cmake-build-debug/src/nfc-app/app-qt/"
$BuildReleasePath="cmake-build-release/src/nfc-app/app-qt/"

if (Test-Path "$BuildDebugPath/nfc-lab.exe") {

    # Copy application resources
    Copy-Item -Force -Recurse dat/conf   $BuildDebugPath
    Copy-Item -Force -Recurse dat/fonts  $BuildDebugPath

    # Copy application external libraries
    Copy-Item -Force dll/glew-2.1.0/x86_64-w64-mingw32/bin/*.dll      $BuildDebugPath
    Copy-Item -Force dll/usb-1.0.20/x86_64-w64-mingw32/bin/*.dll      $BuildDebugPath
    Copy-Item -Force dll/freetype-2.11.0/x86_64-w64-mingw32/bin/*.dll $BuildDebugPath

    windeployqt.exe --debug --compiler-runtime --no-translations --no-system-d3d-compiler --no-angle --no-opengl-sw $BuildDebugPath/nfc-lab.exe
}

if (Test-Path "$BuildReleasePath/nfc-lab.exe") {

    # Copy application resources
    Copy-Item -Force -Recurse dat/conf   $BuildReleasePath
    Copy-Item -Force -Recurse dat/fonts  $BuildReleasePath

    # Copy application external libraries
    Copy-Item -Force dll/glew-2.1.0/x86_64-w64-mingw32/bin/*.dll      $BuildReleasePath
    Copy-Item -Force dll/usb-1.0.20/x86_64-w64-mingw32/bin/*.dll      $BuildReleasePath
    Copy-Item -Force dll/freetype-2.11.0/x86_64-w64-mingw32/bin/*.dll $BuildReleasePath

    windeployqt.exe --release --compiler-runtime --no-translations --no-system-d3d-compiler --no-angle --no-opengl-sw $BuildReleasePath/nfc-lab.exe
}

