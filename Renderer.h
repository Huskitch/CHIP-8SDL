#pragma once
#include "CPU.h"
#include <SDL.h>

class Renderer
{
public:
	Renderer();
	~Renderer();

	void CreateWindow(int width, int height);
	void Draw(CPU* cpu);
	void ProcessInputs(CPU* cpu);

private:
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* texture;

	uint32_t pixels[2048];

	int width;
	int height;

	uint8_t keyMap[16] =
	{
		SDLK_x,
		SDLK_1,
		SDLK_2,
		SDLK_3,
		SDLK_q,
		SDLK_w,
		SDLK_e,
		SDLK_a,
		SDLK_s,
		SDLK_d,
		SDLK_z,
		SDLK_c,
		SDLK_4,
		SDLK_r,
		SDLK_f,
		SDLK_v
	};
};

