#include <stdio.h>
#include "ui.h"

bool run = true;

int main(int argc, char* args[]) {

  if (UI::init()) {
    return 1;
  }
  { 
    UI::CycleStep testStep0(10, 10);
    UI::CycleStep testStep1(120, 10);

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
      static float pos = 120;
      pos += 0.1;
      testStep1.setXY(pos, 10);

      testStep0.render();
      testStep1.render();
    
      SDL_RenderPresent(UI::renderer);
    } 
  }
  SDL_DestroyWindow(UI::window);
  SDL_Quit();
  return 0;
}
