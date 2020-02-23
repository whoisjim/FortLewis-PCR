#include <stdio.h>
#include <iostream>
#include "ui.h"

bool run = true;

int main(int argc, char* args[]) {

  if (UI::init()) {
    return 1;
  }
  { 
    UI::Cycle testCycle(10, 10);
  
    testCycle.addStep(0, UI::CycleStep());
    testCycle.addStep(1, UI::CycleStep());

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
      SDL_SetRenderDrawColor(UI::renderer, 90, 105, 136, 255);
      SDL_RenderClear(UI::renderer);

      testCycle.render();
    
      SDL_RenderPresent(UI::renderer);
      if (SDL_GetError()[0] != '\0') {
        std::cout << "SDL ERROR: " << SDL_GetError() << std::endl;
        SDL_ClearError();
      }
    } 
  }
  SDL_DestroyWindow(UI::window);
  SDL_Quit();
  return 0;
}
