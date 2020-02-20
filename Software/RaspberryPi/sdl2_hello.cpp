#include <stdio.h>
#include "ui.h"

TTF_Font* font;
bool run = true;

int main(int argc, char* args[]) {

	if (UI::init()) {
		return 1;
	}

	font = TTF_OpenFont("fonts/consola.ttf", 28);
	SDL_Surface* temp = TTF_RenderText_Solid(font, "Test", {255, 255, 255, 255});
	SDL_Texture* fontTexture = SDL_CreateTextureFromSurface(UI::renderer, temp);
	SDL_FreeSurface(temp);

	while (run) {
		while (SDL_PollEvent(&UI::event) != 0) {
			if (UI::event.type == SDL_QUIT) {
				run = false;
			} else if (UI::event.type == SDL_KEYDOWN) {
				switch (UI::event.key.keysym.sym) {
					case SDLK_ESCAPE:
						run = false;
						break;
				}
			}
		}
		SDL_SetRenderDrawColor(UI::renderer, 50, 50, 50, 255);
		SDL_RenderClear(UI::renderer);
		
		SDL_RenderCopy(UI::renderer, fontTexture, NULL, NULL);
		
		SDL_RenderPresent(UI::renderer);
	
	}
	SDL_DestroyWindow(UI::window);
	SDL_Quit();
	return 0;
}
