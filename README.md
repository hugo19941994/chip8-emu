CHIP8-Emu
=========

A CHIP8 emulator written in C and SDL.

##Key Bindings
The key bindings are:

    1 2 3 4
    q w e r
    a s d f
    z x c v

##Compiling
###Windows
Dependencies: SDL2

Open the project in Visual Studio and compile
###Linux
Dependencies: SDL2, GTK3

Compile using:
g++ -o prog chip8.cpp main.cpp loader.cpp vars.h -I/usr/include/SDL2 -lSDL2main -lSDL2 \`pkg-config --cflags --libs gtk+-2.0\`

##Screenshot
![BRIX](https://raw.githubusercontent.com/hugo19941994/CHIP8-Emu/master/images/BRIX.png "BRIX")
