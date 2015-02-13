#ifdef _WIN32
#include <windows.h>
#include <stdlib.h>
#include <SDL.h>
#endif
#ifdef linux
#include <cstdlib>
#include <gtk/gtk.h>
#include "SDL.h"
#endif
#include <stdio.h>
//#include <string.h>
//#include <conio.h>
#include "vars.h"

//Screen dimension constants
const int SCREEN_WIDTH = 580;
const int SCREEN_HEIGHT = 320;

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

void emulateCycle();
void initialize();
void loadgame();

bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		{
			printf("Warning: Linear texture filtering not enabled!");
		}

		//Create window
		gWindow = SDL_CreateWindow("CHIP8-Emu", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (gWindow == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			//Create renderer for window
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if (gRenderer == NULL)
			{
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
			}
		}
	}

	return success;
}

int main(int argc, char* args[])
{
	//Start up SDL and create window
	if (!init())
	{
		printf("Failed to initialize!\n");
	}
	else
	{
		//Main loop flag
		bool quit = false;
		initialize();
		#ifdef linux
		gtk_init(&argc, &args);
		#endif
		loadgame();

		//Event handler
		SDL_Event e;

		while (!quit)
		{
			//Handle events on queue
			while (SDL_PollEvent(&e) != 0)
			{
				if (e.type == SDL_QUIT)
					quit = true;
				if (e.type == SDL_KEYDOWN){
					switch (e.key.keysym.sym){
					case (SDLK_1) :
						key[0] = 1;
						break;
					case (SDLK_2) :
						key[1] = 1;
						break;
					case (SDLK_3) :
						key[2] = 1;
						break;
					case (SDLK_4) :
						key[3] = 1;
						break;
					case (SDLK_q) :
						key[4] = 1;
						break;
					case (SDLK_w) :
						key[5] = 1;
						break;
					case (SDLK_e) :
						key[6] = 1;
						break;
					case (SDLK_r) :
						key[7] = 1;
						break;
					case (SDLK_a) :
						key[8] = 1;
						break;
					case (SDLK_s) :
						key[9] = 1;
						break;
					case (SDLK_d) :
						key[10] = 1;
						break;
					case (SDLK_f) :
						key[11] = 1;
						break;
					case (SDLK_z) :
						key[12] = 1;
						break;
					case (SDLK_x) :
						key[13] = 1;
						break;
					case (SDLK_c) :
						key[14] = 1;
						break;
					case (SDLK_v) :
						key[15] = 1;
						break;
					}
				}
				if (e.type == SDL_KEYUP){
					switch (e.key.keysym.sym){
					case (SDLK_1) :
						key[0] = 0;
						break;
					case (SDLK_2) :
						key[1] = 0;
						break;
					case (SDLK_3) :
						key[2] = 0;
						break;
					case (SDLK_4) :
						key[3] = 0;
						break;
					case (SDLK_q) :
						key[4] = 0;
						break;
					case (SDLK_w) :
						key[5] = 0;
						break;
					case (SDLK_e) :
						key[6] = 0;
						break;
					case (SDLK_r) :
						key[7] = 0;
						break;
					case (SDLK_a) :
						key[8] = 0;
						break;
					case (SDLK_s) :
						key[9] = 0;
						break;
					case (SDLK_d) :
						key[10] = 0;
						break;
					case (SDLK_f) :
						key[11] = 0;
						break;
					case (SDLK_z) :
						key[12] = 0;
						break;
					case (SDLK_x) :
						key[13] = 0;
						break;
					case (SDLK_c) :
						key[14] = 0;
						break;
					case (SDLK_v) :
						key[15] = 0;
						break;
					}
				}
			}

			emulateCycle();

			if (drawFlag == 1){
				//Clear screen
				SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);
				SDL_RenderClear(gRenderer);
				int chgline = 0;
				int chgcol = 0;
				//Render red filled quad
				for (int i = 0; i < 64 * 32; i++){
					if ((i % 64) == 0){
						chgline += 9;
						chgcol = 0;
					}
					if (gfx[i] == 1){
						SDL_Rect fillRect = { chgcol, chgline, 10, 10 };
						SDL_SetRenderDrawColor(gRenderer, 0x00, 0xFF, 0x00, 0xFF);
						SDL_RenderFillRect(gRenderer, &fillRect);
					}
					chgcol += 9;
				}
				//Update screen
				SDL_RenderPresent(gRenderer);
				drawFlag = 0;
			}
		}
	}

	//Free resources and close SDL
	//close();

	return 0;
}