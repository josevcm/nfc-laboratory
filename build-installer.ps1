$BuildPath="bin/installer"

if (!(Test-Path $BuildPath)) {
    New-Item $BuildPath -ItemType Directory
}

# Copy installer meta data
Copy-Item -Force -Recurse bin/config   $BuildPath
Copy-Item -Force -Recurse bin/packages $BuildPath

# Copy application resources
Copy-Item -Force -Recurse dat/conf   $BuildPath/packages/nfc-lab/data
# Copy-Item -Force -Recurse dat/fonts  $BuildPath/packages/nfc-lab/data

# Copy application external libraries
Copy-Item -Force dll/usb-1.0.20/x86_64-w64-mingw32/bin/*.dll $BuildPath/packages/nfc-lab/data/

# Copy application executable
Copy-Item -Force cmake-build-release/src/nfc-app/app-qt/nfc-lab.exe $BuildPath/packages/nfc-lab/data/

# Create QT deployment from executable
windeployqt.exe --debug --force --compiler-runtime --no-translations --no-system-d3d-compiler --no-angle --no-opengl-sw $BuildPath/packages/nfc-lab/data/nfc-lab.exe
# windeployqt.exe --release --force --compiler-runtime --no-translations --no-system-d3d-compiler --no-angle --no-opengl-sw $BuildPath/packages/nfc-lab/data/nfc-lab.exe

# Create QT installer
binarycreator.exe -c $BuildPath/config/config.xml -p $BuildPath/packages $BuildPath/nfc-lab-2.6.6-x86_64.exe
