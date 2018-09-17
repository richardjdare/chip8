#include <iostream>
#include <SDL.h>
#include "chip8.h"

//----------------------------------------------------------------------------
// Chip8 main.cpp 2018 Richard Dare - www.richardjdare.com
// Chip8 Emulator using SDL2
//----------------------------------------------------------------------------

using namespace std;

const int screenWidth = 640;
const int screenHeight = 320;
const int framerate = 60;
const int singleFrameMs = 1000 / framerate;
// 500 hz
const int clockSpeed = 500;
const int ticksPerFrame = singleFrameMs / (1000 / clockSpeed);
const int pixelWidth =  screenWidth / Chip8::screenWidth;
const int pixelHeight = screenHeight / Chip8::screenHeight;

//----------------------------------------------------------------------------
// prototypes
//----------------------------------------------------------------------------
void render(Chip8 *theChip8, SDL_Renderer *renderer);
void updateKey(Chip8 *theChip8, SDL_Keycode sdlKeycode, Chip8::KeyStatus keyStatus);
void drawPixel(SDL_Renderer *renderer, int x, int y, int pixelWidth, int pixelHeight);

//----------------------------------------------------------------------------
// main
//----------------------------------------------------------------------------
int main(int argc, char * argv[])
{
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		cout << "SDL initialization failed. SDL Error: " << SDL_GetError() << endl;
	}

	SDL_Window *window = SDL_CreateWindow("Chip8 Emulator", 
		SDL_WINDOWPOS_UNDEFINED, 
		SDL_WINDOWPOS_UNDEFINED, 
		screenWidth, 
		screenHeight, 
		SDL_WINDOW_SHOWN);

	if (window == nullptr)
	{
		cout << "Could not initialize window" << endl;
	}

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	if (renderer == nullptr)
	{
		cout << "Could not initialize renderer" << endl;
	}

	SDL_RenderSetLogicalSize(renderer, screenWidth, screenHeight);

	Chip8 myChip8 = Chip8();

	myChip8.reset();
	myChip8.load("../chip8/roms/invaders.rom");

	bool quit = false;
	SDL_Event e;

	Uint32 lastTime = SDL_GetTicks();

	while (!quit)
	{
		// lets run the whole thing at 60fps and dial up the
		// cpu tick to 500hz or something. I dont think anyone knows
		// what the actual times are!

		Uint32 startTime = SDL_GetTicks();
		Uint32 frametime = startTime - lastTime;

		// process sdl events
		while (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_QUIT)
			{
				quit = true;
			}
			if (e.type == SDL_KEYDOWN)
			{
				updateKey(&myChip8, e.key.keysym.sym, Chip8::key_down);
			}
			if (e.type == SDL_KEYUP)
			{
				updateKey(&myChip8, e.key.keysym.sym, Chip8::key_up);
			}
		}

		// we want to run at 500hz, so perform as many ticks as
		// necessary given the current framerate
		for (int i = 0; i < ticksPerFrame; i++)
		{
			myChip8.tick();
		}

		// timers run at 60hz
		myChip8.updateTimers();

		// is it time to update the screen?
		// We only draw when Chip8 tells us to
		if (myChip8.willDraw())
		{
			//clear screen 
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			SDL_RenderClear(renderer);

			render(&myChip8, renderer);
			myChip8.drawFlag = false;
		}

		// are we playing a beep?
		if (myChip8.willBeep())
		{
			cout << "BEEP!" << endl;
		}


		// Limit frame rate ( I *think* this is how you do it with sdl?)
		if (SDL_GetTicks() - startTime < singleFrameMs)
		{
			SDL_Delay(singleFrameMs - (SDL_GetTicks() - startTime));
		}

		lastTime = startTime;

		SDL_RenderPresent(renderer);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

//----------------------------------------------------------------------------
// drawPixel
//----------------------------------------------------------------------------
void drawPixel(SDL_Renderer *renderer, int x, int y,int width,int height)
{
	SDL_Rect r;
	r.x = x * width;
	r.y = y * height;
	r.w = pixelWidth;
	r.h = pixelHeight;

	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderFillRect(renderer, &r);
}

//----------------------------------------------------------------------------
// render
//----------------------------------------------------------------------------
void render(Chip8 *theChip8, SDL_Renderer *renderer)
{
	for (int y = 0; y < Chip8::screenHeight; y++)
	{
		for (int x = 0; x < Chip8::screenWidth; x++)
		{
			unsigned char pxl = theChip8->gfx[y * Chip8::screenWidth + x];
			if (pxl == 1)
			{
				drawPixel(renderer, x, y, pixelWidth, pixelHeight);
			}
		}
	}
}

//----------------------------------------------------------------------------
// updateKey
//----------------------------------------------------------------------------
void updateKey(Chip8 *theChip8, SDL_Keycode sdlKeycode, Chip8::KeyStatus keyStatus)
{
	switch (sdlKeycode) 
	{
	case SDLK_1:
		theChip8->keys[0] = keyStatus;
		break;
	case SDLK_2:
		theChip8->keys[1] = keyStatus;
		break;
	case SDLK_3:
		theChip8->keys[2] = keyStatus;
		break;
	case SDLK_4:
		theChip8->keys[3] = keyStatus;
		break;
	case SDLK_q:
		theChip8->keys[4] = keyStatus;
		break;
	case SDLK_w:
		theChip8->keys[5] = keyStatus;
		break;
	case SDLK_e:
		theChip8->keys[6] = keyStatus;
		break;
	case SDLK_r:
		theChip8->keys[7] = keyStatus;
		break;
	case SDLK_a:
		theChip8->keys[8] = keyStatus;
		break;
	case SDLK_s:
		theChip8->keys[9] = keyStatus;
		break;
	case SDLK_d:
		theChip8->keys[10] = keyStatus;
		break;
	case SDLK_f:
		theChip8->keys[11] = keyStatus;
		break;
	case SDLK_z:
		theChip8->keys[12] = keyStatus;
		break;
	case SDLK_x:
		theChip8->keys[13] = keyStatus;
		break;
	case SDLK_c:
		theChip8->keys[14] = keyStatus;
		break;
	case SDLK_v:
		theChip8->keys[15] = keyStatus;
		break;
	}
}