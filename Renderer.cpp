#include "Renderer.h"

Renderer::Renderer()
{

}

Renderer::~Renderer()
{
}

void Renderer::CreateWindow(int width, int height)
{
	window = NULL;

	SDL_Init(SDL_INIT_EVERYTHING);
	window = SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);

	renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_RenderSetLogicalSize(renderer, width, height);

	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);
}

void Renderer::Draw(CPU* cpu)
{
	if (cpu->drawn)
	{
		cpu->drawn = false;

		for (int i = 0; i < 2048; i++)
		{
			uint8_t pixel = cpu->pixels[i];
			pixels[i] = (0x00FFFFFF * pixel) | 0xFF000000;
		}

		SDL_UpdateTexture(texture, NULL, pixels, 64 * sizeof(Uint32));

		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
	}
}

void Renderer::ProcessInputs(CPU* cpu)
{
	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		if (e.type == SDL_KEYDOWN)
		{
			for (int i = 0; i < 16; i++)
			{
				if (e.key.keysym.sym == keyMap[i])
					cpu->keys[i] = 1;
			}
		}

		if (e.type == SDL_KEYUP)
		{
			for (int i = 0; i < 16; i++)
			{
				if (e.key.keysym.sym == keyMap[i])
					cpu->keys[i] = 0;
			}
		}
	}
}
