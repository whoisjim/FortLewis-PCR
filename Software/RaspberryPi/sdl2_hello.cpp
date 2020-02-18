#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Surface* screenSurface = NULL;
SDL_Event event;

SDL_Texture* textures[1];
TTF_Font* font;
bool run = true;

void dispError() {
	SDL_Surface* fonttemp = TTF_RenderText_Solid(font, SDL_GetError(), {255, 0, 0, 255});
	SDL_Texture* fontTexture = SDL_CreateTextureFromSurface(renderer, fonttemp);
	SDL_FreeSurface(fonttemp);
	SDL_Rect* dest = new SDL_Rect;
	dest -> x = 16;
	dest -> y = 16;
	dest -> h = 28;
	dest -> w = 1888;
	SDL_RenderCopy(renderer, fontTexture, NULL, dest);
}

int main(int argc, char* args[]) {
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		fprintf(stderr, "could not initialize sdl2: %s\n", SDL_GetError());
		return 1;
	}
	window = SDL_CreateWindow("FLC:PCR", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_FULLSCREEN);
	if (window == NULL) {
		fprintf(stderr, "could not create window: %s\n", SDL_GetError());
		return 1;
	}
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL) {
		fprintf(stderr, "could not create renderer: %s\n", SDL_GetError());
		return 1;
	}
	if (TTF_Init() == -1) {
		fprintf(stderr, "could not initialize ttf: %s\n", SDL_GetError());
		return 1;
	}

	font = TTF_OpenFont("fonts/consola.ttf", 28);
	SDL_Surface* temp = TTF_RenderText_Solid(font, "Test", {255, 255, 255, 255});
	SDL_Texture* fontTexture = SDL_CreateTextureFromSurface(renderer, temp);
	SDL_FreeSurface(temp);

	temp = IMG_Load("img/rect.png");
	textures[0] = SDL_CreateTextureFromSurface(renderer, temp);
	SDL_FreeSurface(temp);
	SDL_Rect * source = NULL;
	
	SDL_Rect * dest = new SDL_Rect;
	dest -> x = 16;
	dest -> y = 64;
	dest -> h = 16;
	dest -> w = 16;

	while (run) {
		while (SDL_PollEvent(&event) != 0) {
			if (event.type == SDL_QUIT) {
				run = false;
			} else if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
					case SDLK_ESCAPE:
						run = false;
						break;
				}
			}
		}
		SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
		SDL_RenderClear(renderer);
		
		SDL_RenderCopy(renderer, fontTexture, source, source);
		SDL_RenderCopy(renderer, textures[0], source, dest);
		
		dispError();
		
		SDL_RenderPresent(renderer);
	
	}
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
