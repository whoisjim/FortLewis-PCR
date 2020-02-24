#include <stdio.h>
#include <iostream>
#include "ui.h"

bool run = true;

int main(int argc, char* args[]) {

  if (UI::init()) {
    return 1;
  }
  
  SDL_Point touchLocation;
  int touchTimeStart;
  
  UI::Padding buttonPadding("img/grey_padding.png", 6, 560, -10, 250, 500);
  UI::Padding infoBarPadding("img/default_padding.png", 6, -10, 454, 820, 36); 
  UI::TextBox errorText(5, 459, 790, 16, "No Errors");
  
  UI::CycleArray cycleArray(10, 10);
  cycleArray.addCycle(0, UI::Cycle());
  cycleArray.addCycle(0, UI::Cycle());
  cycleArray.addCycle(0, UI::Cycle());
  cycleArray.cycles_[0].addStep(0, UI::CycleStep());
  cycleArray.cycles_[0].addStep(0, UI::CycleStep());
  cycleArray.cycles_[1].addStep(0, UI::CycleStep());
  cycleArray.cycles_[1].addStep(0, UI::CycleStep());
  cycleArray.cycles_[1].addStep(0, UI::CycleStep());
  cycleArray.cycles_[2].addStep(0, UI::CycleStep());

  UI::TextBox* selectedTextBox = nullptr;

  UI::CycleStep* heldStep = nullptr;
  UI::Cycle* heldCycle = nullptr;

  while (run) {
    SDL_SetRenderDrawColor(UI::renderer, 90, 105, 136, 255);
    SDL_RenderClear(UI::renderer);
    while (SDL_PollEvent(&UI::event) != 0) {
      if (UI::event.type == SDL_QUIT) {
        run = false;
      } else if (UI::event.type == SDL_KEYDOWN) {
        switch (UI::event.key.keysym.sym) {
          case SDLK_ESCAPE:
            run = false;
            break;
        }
      } else if (UI::event.type == SDL_FINGERDOWN) {
        touchLocation.x = UI::event.tfinger.x * SCREEN_WIDTH;
        touchLocation.y = UI::event.tfinger.y * SCREEN_HEIGHT;
        touchTimeStart = UI::event.tfinger.timestamp;        

      } else if (UI::event.type == SDL_FINGERMOTION) {
        touchLocation.x = UI::event.tfinger.x * SCREEN_WIDTH;
        touchLocation.y = UI::event.tfinger.y * SCREEN_HEIGHT;
        if (heldStep == nullptr && heldCycle == nullptr) {
          for (unsigned int i = 0; i < cycleArray.cycles_.size(); i++) {
            for (unsigned int j = 0; j < cycleArray.cycles_[i].steps_.size(); j++) {
              SDL_Rect stepRect = cycleArray.cycles_[i].steps_[j].getRect();
              if (SDL_PointInRect(&touchLocation, &stepRect)) {
                heldStep = new UI::CycleStep(cycleArray.cycles_[i].removeStep(j));
              }
            }
            SDL_Rect cycleRect = cycleArray.cycles_[i].getRect();
            if (SDL_PointInRect(&touchLocation, &cycleRect) && touchLocation.y < cycleRect.y + 36) {
              heldCycle = new UI::Cycle(cycleArray.removeCycle(i));
            } 
          }
        } else {
          if (heldStep != nullptr) {
            heldStep->setXY(touchLocation.x - 50, touchLocation.y - 23);
          }
          if (heldCycle != nullptr) {
            heldCycle->setXY(touchLocation.x - 50, touchLocation.y - 23);
          }
        }

      } else if (UI::event.type == SDL_FINGERUP) {
        touchLocation.x = UI::event.tfinger.x * SCREEN_WIDTH;
        touchLocation.y = UI::event.tfinger.y * SCREEN_HEIGHT;
        if (UI::event.tfinger.timestamp - touchTimeStart <= 500) { // tap
          for (unsigned int i = 0; i < cycleArray.cycles_.size(); i++) {
            for (unsigned int j = 0; j < cycleArray.cycles_[i].steps_.size(); j++) {
              SDL_Rect stepRect = cycleArray.cycles_[i].steps_[j].getRect();
              if (SDL_PointInRect(&touchLocation, &stepRect)) {
                if (touchLocation.y > stepRect.y + stepRect.h / 2) {
                  if (selectedTextBox != nullptr) {
                    selectedTextBox->deselect();
                  } 
                  selectedTextBox = cycleArray.cycles_[i].steps_[j].getDuration();
                  selectedTextBox->select(); 
                } else {
                  if (selectedTextBox != nullptr) {
                    selectedTextBox->deselect();
                  }
                  selectedTextBox = cycleArray.cycles_[i].steps_[j].getTemperature();
                  selectedTextBox->select();
                }
              }
            }
            SDL_Rect cycleRect = cycleArray.cycles_[i].getRect();
            if (SDL_PointInRect(&touchLocation, &cycleRect) && touchLocation.y < cycleRect.y + 36) {
              if (selectedTextBox != nullptr) {
                selectedTextBox->deselect();
              }
              selectedTextBox = cycleArray.cycles_[i].getNumberOfCycles();
              selectedTextBox->select();
            }
          }
        }
        if (heldStep != nullptr) {
          SDL_Point cycleArrayPos = cycleArray.getPoint();
          for (unsigned int i = 0; i < cycleArray.cycles_.size(); i++) {
            if (touchLocation.x - cycleArrayPos.x < (int)i * 110 + 110) {
              for (unsigned int j = 0; j < cycleArray.cycles_[i].steps_.size(); j++) {
                if (touchLocation.y - cycleArrayPos.y < (int)j * 52 + 54) {
                  cycleArray.cycles_[i].addStep(j, *heldStep);
                  delete heldStep;
                  heldStep = nullptr;
                  break;
                }
              }
              if (heldStep != nullptr) {
                cycleArray.cycles_[i].addStep(cycleArray.cycles_[i].steps_.size(), *heldStep);
                delete heldStep;
                heldStep = nullptr;
              }
              break;
            }
          }
        }
        if (heldCycle != nullptr) {
          SDL_Point cycleArrayPos = cycleArray.getPoint();
          for (unsigned int i = 0; i < cycleArray.cycles_.size(); i++) {
            if (touchLocation.x - cycleArrayPos.x < (int)i * 110 + 55) {
              cycleArray.addCycle(i, *heldCycle);
              delete heldCycle;
              heldCycle = nullptr;
              break;
            }
          }
          if (heldCycle != nullptr) {
            cycleArray.addCycle(cycleArray.cycles_.size(), *heldCycle);
            delete heldCycle;
            heldCycle = nullptr;
          }
        }
      }
    }
     
    cycleArray.render();

    buttonPadding.render();
    infoBarPadding.render();

    if (heldStep != nullptr) {
      heldStep->render();
    }

    if (heldCycle != nullptr) {
      heldCycle->render();
    }

    if (SDL_GetError()[0] != '\0') {
      std::cout << "SDL ERROR: " << SDL_GetError() << std::endl;
      errorText.setText(SDL_GetError());
      SDL_ClearError();
    }
    errorText.render();
    SDL_RenderPresent(UI::renderer);
  }
  SDL_DestroyWindow(UI::window);
  SDL_Quit();
  return 0;
}
