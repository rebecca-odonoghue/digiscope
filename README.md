# Digiscope - Team Project II 2016
A digital oscilloscope designed for use with Digiscope device for Windows and OS X. 

## Build Instructions
### Using Qt Creator to Build
1. Open Digiscope\Digiscope.pro using Qt Creator
2. Open the Projects tab on the left hand side
3. Change the Release build directory to Digiscope\build
4. Ensure that the selected build is Release in the bottom left corner
5. Click the Build button in the bottom left corner

### Deploying the Qt Libraries on Windows
1. Add $QT_INSTALL_DIR\Qt5.5.0\5.5\mingw492_32\bin to the PATH environment variable.
2. Open the Windows Command Prompt to the build directory 
3. Run windeployqt Digiscope.exe 
