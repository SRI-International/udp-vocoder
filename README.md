# udp-vocoder
UDP broadcaster and receiver for opus audio packets on Windows 10/11
Encodes audio in broadcaster and decodes audio in receiver.
This project was compiled using MSVC using the VScode IDE. Instructions on how to compile are below.

# Dependencies
 The folder `inlclude\` and `lib\` has the header files and the precompiled static libraries respectively. 
 This project uses Audio Codec Opus and Portaudio to access microphone and speakers


# Precompiled Binaries
There are precompiled binaries for both broadcaster and receiver. 

# Compiling for Windows

1. Install [Microsoft Build Tools](https://aka.ms/vs/17/release/vs_BuildTools.exe)
2. Once installed, check off "Desktop development with C++" 
3. Install VScode with default settings
    - Recommended to install Microsoft C/C++ extensions for debugging and IntelliSense
4. Download or install the `udp-vocoder` project

**The following step is neccessary every time if you want to compile the code**

5. Go to the start menu and search up "Developer Command Prompt for VS2022" or (if you install a new version of it VS202X) 
6. Use `cd [path]` in the terminal to your project folder.
7. Use `code .` to open up VScode. 
8. Open `broadcaster.cpp` and go to the menu ribbon and go to "Run" -> "Run without Debugging"
9. Repeat step 8 with `receiver.cpp`
10. Run `broadcaster.exe` and `receiver.exe` (can open receiver.exe on a seperate device connected to the same network)
