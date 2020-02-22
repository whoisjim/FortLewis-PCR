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
	
	UI::Padding testPadD("img/default_padding.png", 6, 120, 120, 0, 0);
	UI::Padding testPadG("img/grey_padding.png", 6, 100, 100, 10, 10);
	UI::Padding testPadK("img/dark_padding.png", 6, 80, 80, 20, 20);
	UI::Padding testPadS("img/sharp_padding.png", 6, 60, 60, 30, 30);
	UI::Padding testPadC("img/sharp_selection_padding.png", 6, 40, 40, 40, 40);



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

		testPadD.render();
		testPadG.render();
		testPadK.render();
		testPadS.render();
		testPadC.render();
		
		SDL_RenderPresent(UI::renderer);
	
	}
	SDL_DestroyWindow(UI::window);
	SDL_Quit();
	return 0;
}
