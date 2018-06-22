#ifdef _WIN32
#include <stdlib.h>
#include <windows.h>
#endif

#if __linux__ || __APPLE__
#include <gtk/gtk.h>
#include <cstdlib>
#endif

#include <cstdio>
#include <vector>
#include "vars.h"

// Current OpCode
uint16_t opcode;

// Memory
std::array<uint8_t, 4096> memory;

// Registers
std::array<uint8_t, 16> V;  // CPU Registers
uint16_t I;                 // Index Register
uint16_t pc;                // Program Counter

// Graphics
std::array<uint8_t, 64 * 32> gfx;
bool drawFlag = false;

// Timers
uint8_t delay_timer;
uint8_t sound_timer;

// Stack & stack pointer
std::array<uint8_t, 16> stack;
uint8_t sp;

// Keypad
std::array<bool, 16> key;  // TODO: Init all of them to false

// Charset
std::array<uint8_t, 80> chip8_fontset = {
    0xF0, 0x90, 0x90, 0x90, 0xF0,  // 0
    0x20, 0x60, 0x20, 0x20, 0x70,  // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0,  // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0,  // 3
    0x90, 0x90, 0xF0, 0x10, 0x10,  // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0,  // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0,  // 6
    0xF0, 0x10, 0x20, 0x40, 0x40,  // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0,  // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0,  // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90,  // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0,  // B
    0xF0, 0x80, 0x80, 0x80, 0xF0,  // C
    0xE0, 0x90, 0x90, 0x90, 0xE0,  // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0,  // E
    0xF0, 0x80, 0xF0, 0x80, 0x80   // F
};

void initialize() {
    pc = 0x200;  // ROM starts at 0x200
    opcode = 0;
    I = 0;
    sp = 0;

    // Load fontset
    for (uint8_t i = 0; i < chip8_fontset.size(); ++i) {
        memory.at(i) = chip8_fontset[i];
    }

    // Reset timers
    delay_timer = 60;
    sound_timer = 60;
}

void emulateCycle() {
    bool keyPress =
        false;  // TODO: Wouldn't this reset the key press every cycle?

    // Fetch Opcode from memory in Program Counter
    opcode = memory.at(pc) << 8 | memory.at(pc + 1);

    // Decode Opcode
    switch (opcode & 0xF000) {  // Decode first hex number

        case 0x0000: {
            switch (opcode & 0x000F) {
                // 00E0	Clears the screen.
                case 0x0000: {
                    uint32_t i = 0;

                    while (i < 32 * 64) {
                        gfx.at(i) = 0;
                        i++;
                    }

                    drawFlag = true;
                    pc += 2;
                    break;
                }

                // 00EE	Returns from a subroutine.
                case 0x000E: {
                    sp--;
                    pc = stack[sp];
                    pc += 2;
                    break;
                }
            }
            break;
        }

        // 1NNN Jumps to address NNN.
        case 0x1000: {
            pc = opcode & 0x0FFF;
            break;
        }

        // 2NNN Calls subroutine at NNN.
        case 0x2000: {
            stack.at(sp) = pc;
            ++sp;
            pc = opcode & 0x0FFF;
            break;
        }

        // 3XNN Skips the next instruction if VX equals NN.
        case 0x3000: {
            pc += V.at((opcode & 0x0F00) >> 8) == (opcode & 0x00FF) ? 4 : 2;
            break;
        }

        // 4XNN Skips the next instruction if VX doesn't equal NN.
        case 0x4000: {
            pc += V.at((opcode & 0x0F00) >> 8) != (opcode & 0x00FF) ? 4 : 2;
            break;
        }

        // 5XY0 Skips the next instruction if VX equals VY.
        case 0x5000: {
            pc += V.at((opcode & 0x0F00) >> 8) == V.at((opcode & 0x00F0) >> 4)
                      ? 4
                      : 2;
            break;
        }

        // 6XNN Sets VX to NN.
        case 0x6000: {
            V.at((opcode & 0x0F00) >> 8) = opcode & 0x00FF;
            pc += 2;
            break;
        }

        // 7XNN Adds NN to VX.
        case 0x7000: {
            V.at((opcode & 0x0F00) >> 8) += opcode & 0x00FF;
            pc += 2;
            break;
        }

        case 0x8000: {
            switch (opcode & 0x000F) {
                // 8XY0	Sets VX to the value of VY.
                case (0x0000): {
                    V.at((opcode & 0x0F00) >> 8) = V.at((opcode & 0x00F0) >> 4);
                    pc += 2;
                    break;
                }

                // 8XY1	Sets VX to VX or VY.
                case (0x0001): {
                    V.at((opcode & 0x0F00) >> 8) =
                        V.at((opcode & 0x0F00) >> 8) |
                        V.at((opcode & 0x00F0) >> 4);
                    pc += 2;
                    break;
                }

                // 8XY2	Sets VX to VX and VY.
                case (0x0002): {
                    V.at((opcode & 0x0F00) >> 8) =
                        V.at((opcode & 0x0F00) >> 8) &
                        V.at((opcode & 0x00F0) >> 4);
                    pc += 2;
                    break;
                }

                // 8XY3	Sets VX to VX xor VY.
                case (0x0003): {
                    V.at((opcode & 0x0F00) >> 8) =
                        V.at((opcode & 0x0F00) >> 8) ^
                        V.at((opcode & 0x00F0) >> 4);
                    pc += 2;
                    break;
                }

                /* 8XY4	Adds VY to VX. VF is set to 1 when there's a
                   carry, and to 0 when there isn't. */
                case (0x0004): {
                    // max value is 255.
                    V.at(0xF) = V.at((opcode & 0x00F0) >> 4) >
                                        (0xFF - V.at((opcode & 0x0F00) >> 8))
                                    ? 1
                                    : 0;
                    V.at((opcode & 0x0F00) >> 8) +=
                        V.at((opcode & 0x00F0) >> 4);
                    pc += 2;
                    break;
                }

                /* 8XY5	VY is subtracted from VX. VF is set to 0 when
                   there's a borrow, and 1 when there isn't. */
                case (0x0005): {
                    V.at(0xF) = V.at((opcode & 0x0F00) >> 8) >
                                        V.at((opcode & 0x00F0) >> 4)
                                    ? 1
                                    : 0;
                    V.at((opcode & 0x0F00) >> 8) =
                        V.at((opcode & 0x0F00) >> 8) -
                        V.at((opcode & 0x00F0) >> 4);
                    pc += 2;
                    break;
                }

                /* 8XY6	Shifts VX right by one. VF is set to the value
                   of the least significant bit of VX before the shift */
                case (0x0006): {
                    V.at(0xF) = V.at((opcode & 0x0F00) >> 8) & 0x01) == 0x01 ? 1 : 0;

                    // Divide by two equals bit shift right 1 bit CREOOOO
                    V.at((opcode & 0x0F00) >> 8) =
                        V.at((opcode & 0x0F00) >> 8) / 2;
                    pc += 2;
                    break;
                }

                /* 8XY7 Sets VX to VY minus VX. VF is set to 0 when
                   there's a borrow, and 1 when there isn't. */
                case (0x0007): {
                    V.at(0xF) = V.at((opcode & 0x00F0) >> 4) >
                                        V.at((opcode & 0x0F00) >> 8)
                                    ? 1
                                    : 0;
                    V.at((opcode & 0x0F00) >> 8) =
                        V.at((opcode & 0x00F0) >> 4) -
                        V.at((opcode & 0x0F00) >> 8);
                    pc += 2;
                    break;
                }

                /* 8XYE Shifts VX left by one. VF is set to the value of
                   the most significant bit of VX before the shift. */
                case (0x000E): {
                    V.at(0xF) = V.at((opcode & 0x0F00) >> 8) & 0x80) == 0x80 ? 1 : 0;
                    // Multiply by two equals bit shift left 1 bit CREOOOO
                    V.at((opcode & 0x0F00) >> 8) =
                        V.at((opcode & 0x0F00) >> 8) * 2;
                    pc += 2;
                    break;
                }
            }
            break;
        }

        // 9XY0 Skips the next instruction if VX doesn't equal VY.
        case 0x9000: {
            pc += V.at((opcode & 0x0F00) >> 8) != V.at((opcode & 0x00F0) >> 4)
                      ? 4
                      : 2;
            break;
        }

        // ANNN Sets I to the address NNN
        case 0xA000: {
            I = (opcode & 0x0FFF);
            pc += 2;
            break;
        }

        // BNNN Jumps to the address NNN plus V0
        case 0xB000: {
            pc = (opcode & 0x0FFF) + V.at(0);
            break;
        }

        // CXNN Sets VX to a random number and NN.
        case 0xC000: {
            V.at((opcode & 0x0F00) >> 8) = (rand() % 255) & (opcode & 0x00FF);
            pc += 2;
            break;
        }

        /* DXYN Sprites stored in memory at location in index
           register (I), maximum 8bits wide. Wraps around the screen. If
           when drawn, clears a pixel, register VF is set to 1 otherwise
           it is zero. All drawing is XOR drawing (i.e. it toggles the
           screen pixels) Graphics - 64*32 screen If opcode was 0xD003 -
           draw sprite ate location 0,0 3 rows high.
           Stored at:
           memory[I] = 0x3C; // 0x3C   00111100
           memory[I + 1] = 0xC3; // 0xC3   11000011
           memory[I + 2] = 0xFF; // 0xFF 11111111 */
        case 0xD000: {
            uint32_t x =
                V.at((opcode & 0x0F00) >> 8);  // x coord is stored in VX
            uint32_t y =
                V.at((opcode & 0x00F0) >> 4);  // y coord is stored in VY
            uint32_t h = opcode & 0x000F;      // height is N

            V.at(0xF) = 0;  // Reset VF flag

            // iterate over each row
            for (uint32_t yline = 0; yline < h; yline++) {
                // Fetch pixel value stored at I
                uint32_t pixel = memory.at(I + yline);

                // iterate over each element in row
                for (uint32_t xcolumn = 0; xcolumn < 8; xcolumn++) {
                    /* checks if current pixel is set to 1. pixel var stores 8
                       actual pixels. 0x80 is 10000000. it shifts right 1 bit at
                       a time to check */
                    if ((pixel & (0x80 >> xcolumn)) != 0) {
                        // If current value is 1 - collision detected
                        if (gfx.at((x + xcolumn + ((y + yline) * 64))) == 1) {
                            V.at(0xF) = 1;
                        }
                        // XOR current value
                        gfx.at((x + xcolumn + ((y + yline) * 64))) ^= 1;
                    }
                }
            }

            drawFlag = true;  // gfx was updated so we need to draw screen
            pc += 2;
            break;
        }

        case 0xE000: {
            switch (opcode & 0x00F0) {
                // EX9E	Skips the next instruction if the key stored in VX
                // is pressed.
                case 0x0090: {
                    pc += key.at(V.at((opcode & 0x0F00) >> 8)) ? 4 : 2, break;
                }

                /* EXA1	Skips the next instruction if the key stored in
                   VX isn't pressed. */
                case 0x00A0: {
                    pc += !key.at(V.at((opcode & 0x0F00) >> 8)) ? 4 : 2;
                    break;
                }
            }
            break;
        }

        case 0xF000: {
            switch (opcode & 0x00FF) {
                // FX07	Sets VX to the value of the delay timer.
                case 0x0007: {
                    V.at((opcode & 0x0F00) >> 8) = delay_timer;
                    pc += 2;
                    break;
                }

                // FX0A	A key press is awaited, and then stored in VX.
                case 0x000A: {
                    // I think it's any key TODO
                    keyPress = false;  // No KeyPress
                    while (!keyPress) {
                        for (uint16_t i = 0; i < 16; i++) {
                            if (key.at(i)) {
                                // store i or key[i]
                                V.at((opcode & 0x0F00) >> 8) = i;
                                keyPress = true;
                                break;
                            }
                        }
                    }
                    pc += 2;
                    break;
                }

                // FX15	Sets the delay timer to VX.
                case 0x0015: {
                    delay_timer = V.at((opcode & 0x0F00) >> 8);
                    pc += 2;
                    break;
                }

                // FX18	Sets the sound timer to VX.
                case 0x0018: {
                    sound_timer = V.at((opcode & 0x0F00) >> 8);
                    pc += 2;
                    break;
                }

                // FX1E	Adds VX to I
                case 0x001E: {
                    // VF is set to 1 when range overflow (I + VX > 0xFFF),
                    // and 0 when there isn't.
                    V.at(0xF) =
                        I + V.at((opcode & 0x0F00) >> 8) > 0xFFFF ? 1 : 0;
                    I += V.at((opcode & 0x0F00) >> 8);
                    pc += 2;
                    break;
                }

                /* FX29	Sets I to the location of the sprite for the
                   character in VX. Characters 0-F (in hexadecimal) are
                   represented by a 4x5 font. */
                case 0x0029: {
                    /* character 0-F coincides with memory
                       locations * 5 */
                    I = V.at((opcode & 0x0F00) >> 8) * 5;
                    pc += 2;
                    break;
                }

                /* FX33	Stores the Binary-coded decimal representation
                   of VX, with the most significant of three digits at the
                   address in I, the middle
                   digit at I plus 1, and the least significant digit at I
                   plus 2. (In other words, take the decimal representation
                   of VX, place the hundreds digit  in memory at location in
                   I, the tens digit at location I+1, and the ones digit at
                   location I+2.) */
                case 0x0033: {
                    // Divide por 100 y devuelve la parte entera
                    memory.at(I) = V.at((opcode & 0x0F00) >> 8) / 100;
                    // Divida por 10, y devuelve el resto de dividirlo por 10
                    memory.at(I + 1) = (V.at((opcode & 0x0F00) >> 8) / 10) % 10;
                    // Resto de dividirlo por 100, y devuelve el resto de
                    // dividirlo por 10
                    memory.at(I + 2) =
                        (V.at((opcode & 0x0F00) >> 8) % 100) % 10;
                    pc += 2;
                    break;
                }

                // FX55	Stores V0 to VX in memory starting at address I
                case 0x0055: {
                    uint32_t i = 0;
                    while (i <= ((opcode & 0x0F00) >> 8)) {  // CON <=
                        // ASI DEBERIA SER memory[I+i] = V[i];
                        memory.at(I) = V.at(i);
                        I++;
                        i++;
                    }
                    // On the original interpreter, when the operation is done,
                    // I = I + X + 1.
                    I += ((opcode & 0x0F00) >> 8) + 1;
                    pc += 2;
                    break;
                }

                /* FX65	Fills V0 to VX with values from memory starting
                   I at address
                   I */
                case 0x0065: {
                    uint32_t i = 0;
                    while (i <= ((opcode & 0x0F00) >> 8)) {
                        V.at(i) = memory.at(I);
                        I++;
                        i++;
                    }
                    // On the original interpreter, when the operation is done,
                    // I = I + X + 1.
                    I += ((opcode & 0x0F00) >> 8) + 1;
                    pc += 2;
                    break;
                }
            }
            break;
        }
    }
    // Execute Opcode

    // Update timers
    if (delay_timer > 0) {
        --delay_timer;
    }

    if (sound_timer > 0) {
        --sound_timer;
    }
}
