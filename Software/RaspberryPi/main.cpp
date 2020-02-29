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
  
  UI::Padding buttonPadding("img/grey_padding.png", 6, 555, -10, 250, 500);
  UI::Key keys [11] = {UI::Key(560, 100, 75, 75, '7', "7"),
                       UI::Key(640, 100, 75, 75, '8', "8"),
                       UI::Key(720, 100, 75, 75, '9', "9"),
                       UI::Key(560, 180, 75, 75, '4', "4"),
                       UI::Key(640, 180, 75, 75, '5', "5"),
                       UI::Key(720, 180, 75, 75, '6', "6"),
                       UI::Key(560, 260, 75, 75, '1', "1"),
                       UI::Key(640, 260, 75, 75, '2', "2"),
                       UI::Key(720, 260, 75, 75, '3', "3"),
                       UI::Key(560, 340, 75, 75, '0', "0"),
                       UI::Key(640, 340, 155, 75, '\b', "del")}; 

  UI::Padding infoBarPadding("img/default_padding.png", 6, -10, 454, 820, 36); 
  UI::TextBox errorText(5, 459, 790, 16, "No Errors");
  UI::CycleArray cycleArray(5, 5);

  UI::TextBox* selectedTextBox = nullptr;
  
  UI::CycleStep* newStep = new UI::CycleStep(565, 5);
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

      // touch start
      } else if (UI::event.type == SDL_FINGERDOWN) {
        touchLocation.x = UI::event.tfinger.x * SCREEN_WIDTH;
        touchLocation.y = UI::event.tfinger.y * SCREEN_HEIGHT;
        touchTimeStart = UI::event.tfinger.timestamp;
        if (selectedTextBox != nullptr) {
          for (int i = 0; i < 11; i++) {
            SDL_Rect keyRect = keys[i].getRect();
            if (SDL_PointInRect(&touchLocation, &keyRect)) {
              keys[i].press(selectedTextBox);
            }
          }
        }

      // touch move
      } else if (UI::event.type == SDL_FINGERMOTION) {
        touchLocation.x = UI::event.tfinger.x * SCREEN_WIDTH;
        touchLocation.y = UI::event.tfinger.y * SCREEN_HEIGHT;
        SDL_Point touchDelta;
        touchDelta.x = UI::event.tfinger.dx * SCREEN_WIDTH;
        touchDelta.y = UI::event.tfinger.dy * SCREEN_HEIGHT;
        SDL_Rect buttonPaddingRect = buttonPadding.getRect();
        if (SDL_PointInRect(&touchLocation, &buttonPaddingRect)) {
          SDL_Rect newStepRect = newStep->getRect();
          if (SDL_PointInRect(&touchLocation, &newStepRect)) {
            heldStep = newStep;
          }
        } else {
          if (heldStep == nullptr && heldCycle == nullptr) {
            for (unsigned int i = 0; i < cycleArray.cycles_.size(); i++) {
              for (unsigned int j = 0; j < cycleArray.cycles_[i]->steps_.size(); j++) {
                SDL_Rect stepRect = cycleArray.cycles_[i]->steps_[j]->getRect();
                if (SDL_PointInRect(&touchLocation, &stepRect)) {
                  heldStep = cycleArray.cycles_[i]->removeStep(j);
                }
              }
              SDL_Rect cycleRect = cycleArray.cycles_[i]->getRect();
              if (SDL_PointInRect(&touchLocation, &cycleRect) && touchLocation.y < cycleRect.y + 36) {
                heldCycle = cycleArray.removeCycle(i);
              } 
            }
            float cycleArrayPosX = touchDelta.x + cycleArray.getPoint().x; 
            if (cycleArrayPosX < -110 * (int)cycleArray.cycles_.size() + 345) {
              cycleArrayPosX = -110 * (int)cycleArray.cycles_.size() + 345;
            }
            if (cycleArrayPosX > 5) {
              cycleArrayPosX = 5;
            }
            cycleArray.setXY(cycleArrayPosX, 5);
          }
        }
        if (heldCycle != nullptr) {
          heldCycle->setXY(touchLocation.x - 50, touchLocation.y - 23);
        }
        if (heldStep != nullptr) {
          heldStep->setXY(touchLocation.x - 50, touchLocation.y - 23);
        }

      // touch end
      } else if (UI::event.type == SDL_FINGERUP) {
        touchLocation.x = UI::event.tfinger.x * SCREEN_WIDTH;
        touchLocation.y = UI::event.tfinger.y * SCREEN_HEIGHT;
        if (UI::event.tfinger.timestamp - touchTimeStart <= 500) { // tap 
          for (unsigned int i = 0; i < cycleArray.cycles_.size(); i++) {
            for (unsigned int j = 0; j < cycleArray.cycles_[i]->steps_.size(); j++) {
              SDL_Rect stepRect = cycleArray.cycles_[i]->steps_[j]->getRect();
              if (SDL_PointInRect(&touchLocation, &stepRect)) {
                if (touchLocation.y > stepRect.y + stepRect.h / 2) {
                  if (selectedTextBox != nullptr) {
                    selectedTextBox->deselect();
                  } 
                  selectedTextBox = cycleArray.cycles_[i]->steps_[j]->getDuration();
                  selectedTextBox->select(); 
                } else {
                  if (selectedTextBox != nullptr) {
                    selectedTextBox->deselect();
                  }
                  selectedTextBox = cycleArray.cycles_[i]->steps_[j]->getTemperature();
                  selectedTextBox->select();
                }
              }
            }
            SDL_Rect cycleRect = cycleArray.cycles_[i]->getRect();
            if (SDL_PointInRect(&touchLocation, &cycleRect) && touchLocation.y < cycleRect.y + 36) {
              if (selectedTextBox != nullptr) {
                selectedTextBox->deselect();
              }
              selectedTextBox = cycleArray.cycles_[i]->getNumberOfCycles();
              selectedTextBox->select();
            }
          }
        }
        SDL_Rect buttonPaddingRect = buttonPadding.getRect();
        if (SDL_PointInRect(&touchLocation, &buttonPaddingRect)) {
          if (heldStep != nullptr) {
            delete heldStep;
            heldStep = nullptr;
          } else if (heldCycle != nullptr) {
            delete heldCycle;
            heldCycle = nullptr;
          }
        }
        if (heldStep != nullptr) {
          SDL_Point cycleArrayPos = cycleArray.getPoint();
          for (unsigned int i = 0; i < cycleArray.cycles_.size(); i++) {
            if (touchLocation.x - cycleArrayPos.x < (int)i * 110 + 110) {
              for (unsigned int j = 0; j < cycleArray.cycles_[i]->steps_.size(); j++) {
                if (touchLocation.y - cycleArrayPos.y < (int)j * 52 + 54) {
                  cycleArray.cycles_[i]->addStep(j, heldStep);
                  heldStep = nullptr;
                  break;
                }
              }
              if (heldStep != nullptr) {
                cycleArray.cycles_[i]->addStep(cycleArray.cycles_[i]->steps_.size(), heldStep);
                heldStep = nullptr;
              }
              break;
            }
          }
          if (heldStep != nullptr) {
            cycleArray.addCycle(cycleArray.cycles_.size(), new UI::Cycle());
            cycleArray.cycles_[cycleArray.cycles_.size() - 1]->addStep(0, heldStep);
            heldStep = nullptr;
          }
        }
        if (heldCycle != nullptr) {
          SDL_Point cycleArrayPos = cycleArray.getPoint();
          for (unsigned int i = 0; i < cycleArray.cycles_.size(); i++) {
            if (touchLocation.x - cycleArrayPos.x < (int)i * 110 + 55) {
              cycleArray.addCycle(i, heldCycle);
              heldCycle = nullptr;
              break;
            }
          }
          if (heldCycle != nullptr) {
            cycleArray.addCycle(cycleArray.cycles_.size(), heldCycle);
            heldCycle = nullptr;
          }
        }
      }
    }

    if (newStep == heldStep) {
      newStep = new UI::CycleStep(565, 5);
    }
    cycleArray.removeEmptyCycles(); 

    cycleArray.render();
    buttonPadding.render();
    for (int i = 0; i < 11; i++) {
      keys[i].render();
    }
    infoBarPadding.render();
    newStep->render();
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
