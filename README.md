# CHIP8-Emu

[![Build Status](https://travis-ci.com/hugo19941994/chip8-emu.svg?branch=master)](https://travis-ci.com/hugo19941994/chip8-emu)

A CHIP8 emulator written in C and SDL.

## Key Bindings
The key bindings are:

    1 2 3 4
    q w e r
    a s d f
    z x c v

## Compiling
### Windows
Dependencies: SDL2

Open the project in Visual Studio and compile

### Linux and MacOS
Dependencies: SDL2, GTK2

Compile using:

```bash
g++ chip8.cpp main.cpp loader.cpp vars.h `sdl2-config --cflags --libs` `pkg-config --cflags --libs gtk+-2.0`
```


## Screenshot
![BRIX](https://raw.githubusercontent.com/hugo19941994/CHIP8-Emu/master/images/BRIX.png "BRIX")
