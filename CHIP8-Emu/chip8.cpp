#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <conio.h>
#include <windows.h>
#include "vars.h"

//Current OpCode
unsigned short opcode;

//Memory
unsigned char memory[4096];

//Registers
unsigned char V[16]; //CPU Registers
unsigned short I; //Index Register
unsigned short pc; //Program Counter

//Graphics
unsigned char gfx[64 * 32];
short drawFlag = 0;

//Timers
unsigned char delay_timer;
unsigned char sound_timer;

//Stack implementation for stack pointer
unsigned short stack[16];
unsigned short sp;

//Keypad
unsigned char key[16];

//Charset
unsigned char chip8_fontset[80] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void initialize(){
	//Initialize Memory, registers
	int iterator = 0;
	while (iterator < 16){
		V[iterator] = 0;
		iterator++;
	}

	iterator = 0;
	while (iterator < 4096){
		memory[iterator] = 0;
		iterator++;
	}

	pc = 0x200;
	opcode = 0;
	I = 0;
	sp = 0;

	// Load fontset
	for (int i = 0; i < 80; ++i)
		memory[i] = chip8_fontset[i];

	// Reset timers
	delay_timer = 60;
	sound_timer = 60;
}

BOOL WINAPI GetOpenFileName(
	_Inout_  LPOPENFILENAME lpofn
	);

char filename[100];

// pick a program file
INT ChooseProgram(PCHAR FileName)
{
	OPENFILENAME  ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = "Chip8 ROMs\0*.ch8\0All Files\0*.*\0\0";
	ofn.lpstrFile = FileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = "Please Select A CHIP8 ROM";
	ofn.Flags = OFN_NONETWORKBUTTON |
		OFN_FILEMUSTEXIST |
		OFN_HIDEREADONLY;
	if (!GetOpenFileName(&ofn))
		return(0);
	return 1;
}

void loadgame(){
	FILE *space;
	ChooseProgram(filename);
	if (fopen_s(&space, filename, "rb") != 0){ //Make optional
		printf("failed");
		exit(0);
	}
	fseek(space, 0, SEEK_END);
	long bufferSize = ftell(space);
	rewind(space);
	unsigned char * buffer = (unsigned char *)malloc(sizeof(char) * bufferSize);
	size_t result = fread(buffer, 1, bufferSize, space);
	for (int i = 0; i < bufferSize; ++i){
		memory[i + 512] = buffer[i];
	}
	fclose(space);
	free(buffer);
}

void emulateCycle(){
	unsigned short h;
	unsigned short x;
	unsigned short y;
	unsigned short pixel;
	int iterator;
	int keyPress;

	// Fetch Opcode from memory in Program Counter
	opcode = memory[pc] << 8 | memory[pc + 1];

	// Decode Opcode
	switch (opcode & 0xF000){ //Decode first hex number
	case 0x0000:
		switch (opcode & 0x000F){
		case 0x0000: //00E0	Clears the screen.
			iterator = 0;
			while (iterator < 32 * 64){
				gfx[iterator] = 0;
				iterator++;
			}
			drawFlag = 1;
			pc += 2;
			break;

		case 0x000E: //00EE	Returns from a subroutine.
			sp--;
			pc = stack[sp];
			pc += 2; //I FORGOT TO INCREASE PROGRAM COUNTER
			break;
		}
		break;

	case 0x1000: //1NNN	Jumps to address NNN.
		pc = opcode & 0x0FFF;
		break;

	case 0x2000: //2NNN	Calls subroutine at NNN.
		stack[sp] = pc;
		++sp;
		pc = opcode & 0x0FFF;
		break;

	case 0x3000: //3XNN	Skips the next instruction if VX equals NN.
		if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
			pc += 4;
		else
			pc += 2;
		break;

	case 0x4000: //4XNN	Skips the next instruction if VX doesn't equal NN.
		if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
			pc += 4;
		else
			pc += 2;
		break;

	case 0x5000: //5XY0	Skips the next instruction if VX equals VY.
		if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
			pc += 4;
		else
			pc += 2;
		break;

	case 0x6000: //6XNN	Sets VX to NN.
		V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
		pc += 2;
		break;

	case 0x7000: //7XNN	Adds NN to VX.
		V[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);
		pc += 2;
		break;

	case 0x8000:
		switch (opcode & 0x000F){
		case (0x0000) : //8XY0	Sets VX to the value of VY.
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case (0x0001) : //8XY1	Sets VX to VX or VY.
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] | V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case (0x0002) : //8XY2	Sets VX to VX and VY.
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] & V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case (0x0003) : //8XY3	Sets VX to VX xor VY.
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] ^ V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case (0x0004) : //8XY4	Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't.
			if (V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8])) //max value is 255. 
				V[0xF] = 1; //carry
			else
				V[0xF] = 0;
			V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case (0x0005) : //8XY5	VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
			if (V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4])
				V[0xF] = 1;
			else
				V[0xF] = 0;
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] - V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case (0x0006) : //8XY6	Shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift
			if ((V[(opcode & 0x0F00) >> 8] & 0x01) == 0x01)
				V[0xF] = 1;
			else
				V[0xF] = 0;
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] / 2; //Divide by two equals bit shift right 1 bit CREOOOO
			pc += 2;
			break;

		case (0x0007) : //8XY7	Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
			if (V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8])
				V[0xF] = 1;
			else
				V[0xF] = 0;
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case (0x000E) : //8XYE	Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift.
			if ((V[(opcode & 0x0F00) >> 8] & 0x80) == 0x80)
				V[0xF] = 1;
			else
				V[0xF] = 0;
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] * 2; //Multiply by two equals bit shift left 1 bit CREOOOO
			pc += 2;
			break;
		}
		break;
	case 0x9000: // 9XY0	Skips the next instruction if VX doesn't equal VY.
		if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
			pc += 4;
		else
			pc += 2;
		break;

	case 0xA000: // ANNN: Sets I to the address NNN
		I = (opcode & 0x0FFF);
		pc += 2;
		break;

	case 0xB000: //BNNN	Jumps to the address NNN plus V0
		pc = (opcode & 0x0FFF) + V[0];
		break;

	case 0xC000: //CXNN	Sets VX to a random number and NN.
		V[(opcode & 0x0F00) >> 8] = (rand() % 255) & (opcode & 0x00FF);
		pc += 2;
		break;

	case 0xD000: /*DXYN	Sprites stored in memory at location in index register (I), maximum 8bits wide. Wraps around the screen. If when drawn,
				 clears a pixel, register VF is set to 1 otherwise it is zero. All drawing is XOR drawing (i.e. it toggles the screen pixels)
				 Graphics - 64*32 screen
				 If opcode was 0xD003 - draw sprite ate location 0,0 3 rows high. Stored at:
				 memory[I]     = 0x3C;    0x3C   00111100     ****
				 memory[I + 1] = 0xC3;    0xC3   11000011   **    **
				 memory[I + 2] = 0xFF;    0xFF   11111111   ********
				 */
		x = V[(opcode & 0x0F00) >> 8]; //x coord is stored in VX
		y = V[(opcode & 0x00F0) >> 4]; //y coord is stored in VY
		h = opcode & 0x000F; //height is N

		V[0xF] = 0; //Reset VF flag

		for (int yline = 0; yline < h; yline++){ //iterate over each row
			pixel = memory[I + yline]; //Fetch pixel value stored at I
			for (int xcolumn = 0; xcolumn < 8; xcolumn++){ //iterate over each element in row
				if ((pixel & (0x80 >> xcolumn)) != 0){ //checks if current pixel is set to 1. pixel var stores 8 actual pixels. 0x80 is 10000000. it shifts right 1 bit at a time to check
					if (gfx[(x + xcolumn + ((y + yline) * 64))] == 1) //If current value is 1 - collision detected
						V[0xF] = 1;
					gfx[(x + xcolumn + ((y + yline) * 64))] ^= 1; //XOR current value
				}
			}
		}

		drawFlag = 1; //gfx was updated so we need to draw screen
		pc += 2;
		break;

	case 0xE000:
		switch (opcode & 0x00F0){
		case 0x0090: //EX9E	Skips the next instruction if the key stored in VX is pressed.
			if (key[V[(opcode & 0x0F00) >> 8]] != 0)
				pc += 4;
			else
				pc += 2;
			break;
		case 0x00A0: //EXA1	Skips the next instruction if the key stored in VX isn't pressed.
			if (key[V[(opcode & 0x0F00) >> 8]] == 0)
				pc += 4;
			else
				pc += 2;
			break;
		}
		break;

	case 0xF000:
		switch (opcode & 0x00FF){
		case 0x0007: //FX07	Sets VX to the value of the delay timer.
			V[(opcode & 0x0F00) >> 8] = delay_timer;
			pc += 2;
			break;

		case 0x000A: //FX0A	A key press is awaited, and then stored in VX.
			//I think it's any key TODO
			keyPress = 0; //No KeyPress
			while (keyPress == 0){
				for (int i = 0; i < 16; i++){
					if (key[i] == 1){
						V[(opcode & 0x0F00) >> 8] = i; //store i or key[i]????????
						keyPress = 1;
						break;
					}
				}
			}
			pc += 2;
			break;

		case 0x0015: //FX15	Sets the delay timer to VX.
			delay_timer = V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case 0x0018: //FX18	Sets the sound timer to VX.
			sound_timer = V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case 0x001E: //FX1E	Adds VX to I
			// VF is set to 1 when range overflow (I+VX>0xFFF), and 0 when there isn't.
			if (I + V[(opcode & 0x0F00) >> 8] > 0xFFFF)
				V[0xF] = 1;
			else
				V[0xF] = 0;
			I += V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case 0x0029: //FX29	Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font.
			I = V[(opcode & 0x0F00) >> 8] * 5; //character 0-F coincides with memory locations*5 ORIGINAL CODE WITH MEMORY, [((opcode & 0x0F00) >> 8)*5]
			pc += 2;
			break;

		case 0x0033: //FX33	Stores the Binary-coded decimal representation of VX, with the most significant of three digits at the address in I, the middle 
			//digit at I plus 1, and the least significant digit at I plus 2. (In other words, take the decimal representation of VX, place the hundreds digit 
			//in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2.)
			memory[I] = V[(opcode & 0x0F00) >> 8] / 100; //Divide por 100 y devuelve la parte entera
			memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10; //Divida por 10, y devuelve el resto de dividirlo por 10
			memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10; //Resto de dividirlo por 100, y devuelve el resto de dividirlo por 10
			pc += 2;
			break;

		case 0x0055: //FX55	Stores V0 to VX in memory starting at address I
			iterator = 0;
			while (iterator <= ((opcode & 0x0F00) >> 8)){ //CON <=
				memory[I] = V[iterator]; // ASI DEBERIA SER memory[I+iterator] = V[iterator];
				I++;
				iterator++;
			}
			// On the original interpreter, when the operation is done, I = I + X + 1.
			I += ((opcode & 0x0F00) >> 8) + 1;
			pc += 2;
			break;

		case 0x0065: //FX65	Fills V0 to VX with values from memory starting at address I
			iterator = 0;
			while (iterator <= ((opcode & 0x0F00) >> 8)){
				V[iterator] = memory[I];
				I++;
				iterator++;
			}
			// On the original interpreter, when the operation is done, I = I + X + 1.
			I += ((opcode & 0x0F00) >> 8) + 1;
			pc += 2;
			break;
		}
		break;
	}
	// Execute Opcode

	// Update timers
	if (delay_timer > 0){
		--delay_timer;
	}
	if (sound_timer>0)
		--sound_timer;
}