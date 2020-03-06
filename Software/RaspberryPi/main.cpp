#include <stdio.h>
#include <iostream>
#include "ui.h"

bool run = true; // is the application running

int main(int argc, char* args[]) {

  if (UI::init()) { // initialize SDL
    return 1;
  }
  
  SDL_Point touchLocation; // the locatio of a touch
  int touchTimeStart; // the time the curent touch started
  
  // create padding behind keypad
  UI::Padding buttonPadding("img/grey_padding.png", 6, 555, -10, 250, 500);
  
  // create keys for typing numbers
  UI::NumberKey keys [11] = {UI::NumberKey(560, 133, 75, 75, '7', "7"),
                             UI::NumberKey(640, 133, 75, 75, '8', "8"),
                             UI::NumberKey(720, 133, 75, 75, '9', "9"),
                             UI::NumberKey(560, 213, 75, 75, '4', "4"),
                             UI::NumberKey(640, 213, 75, 75, '5', "5"),
                             UI::NumberKey(720, 213, 75, 75, '6', "6"),
                             UI::NumberKey(560, 293, 75, 75, '1', "1"),
                             UI::NumberKey(640, 293, 75, 75, '2', "2"),
                             UI::NumberKey(720, 293, 75, 75, '3', "3"),
                             UI::NumberKey(560, 373, 75, 75, '0', "0"),
                             UI::NumberKey(640, 373, 155, 75, '\b', "del")}; 

  // add padding for info at the bottom
  UI::Padding infoBarPadding("img/default_padding.png", 6, -10, 453, 820, 36);
  
  // add text box for displaying SDL errors
  UI::TextBox errorText(5, 459, 790, 16, "No Errors");
  
  // create the array of cycles for curent experiment
  UI::CycleArray cycleArray(5, 5);

  // points to the text box to type in
  UI::TextBox* selectedTextBox = nullptr;
  
  // for creating new cycle steps
  UI::CycleStep* newStep = new UI::CycleStep(565, 5);
  
  // for moving cycles and cycle steps
  UI::CycleStep* heldStep = nullptr;
  UI::Cycle* heldCycle = nullptr;

  while (run) { // loop for as long as the application is running
    // clear the screen
    SDL_SetRenderDrawColor(UI::renderer, 90, 105, 136, 255);
    SDL_RenderClear(UI::renderer);
	
    // get user input
    while (SDL_PollEvent(&UI::event) != 0) {
      if (UI::event.type == SDL_QUIT) { // quit if SDL_QUIT not used for command line raspberry pi
        run = false;
      } else if (UI::event.type == SDL_KEYDOWN) { // quit if escape is pressed
        switch (UI::event.key.keysym.sym) {
          case SDLK_ESCAPE:
            run = false;
            break;
        }

      // if touch start
      } else if (UI::event.type == SDL_FINGERDOWN) {
	// record location and time
        touchLocation.x = UI::event.tfinger.x * SCREEN_WIDTH;
        touchLocation.y = UI::event.tfinger.y * SCREEN_HEIGHT;
        touchTimeStart = UI::event.tfinger.timestamp;
		
	// type numbers when keys are pressed and a text box is selected
        if (selectedTextBox != nullptr) {
          for (int i = 0; i < 11; i++) {
            SDL_Rect keyRect = keys[i].getRect();
            if (SDL_PointInRect(&touchLocation, &keyRect)) {
              keys[i].press(selectedTextBox);
            }
          }
        }

      // if touch move
      } else if (UI::event.type == SDL_FINGERMOTION) {
	// record touch location and velocity 
        touchLocation.x = UI::event.tfinger.x * SCREEN_WIDTH;
        touchLocation.y = UI::event.tfinger.y * SCREEN_HEIGHT;
        SDL_Point touchDelta;
        touchDelta.x = UI::event.tfinger.dx * SCREEN_WIDTH;
        touchDelta.y = UI::event.tfinger.dy * SCREEN_HEIGHT;

	// if touch speed is grater than 5
        SDL_Rect buttonPaddingRect = buttonPadding.getRect();
        if (abs(touchDelta.x) + abs(touchDelta.y) > 5) {
	  // if touch is over button padding
          if (SDL_PointInRect(&touchLocation, &buttonPaddingRect)) {
            SDL_Rect newStepRect = newStep->getRect();
            if (SDL_PointInRect(&touchLocation, &newStepRect)) { // if touching newStep pick it up
              heldStep = newStep;
            }
          } else { // if touch is not over button padding
            if (heldStep == nullptr && heldCycle == nullptr) { // if not holding anything
	      // look at all the cycles and cycle steps and pick up if touching one
              for (unsigned int i = 0; i < cycleArray.cycles_.size(); i++) {
                for (unsigned int j = 0; j < cycleArray.cycles_[i]->steps_.size(); j++) {
                  SDL_Rect stepRect = cycleArray.cycles_[i]->steps_[j]->getRect();
                  if (SDL_PointInRect(&touchLocation, &stepRect)) { // pick up cycle step
                    heldStep = cycleArray.cycles_[i]->removeStep(j);
                  }
                }
                SDL_Rect cycleRect = cycleArray.cycles_[i]->getRect();
                if (SDL_PointInRect(&touchLocation, &cycleRect) && touchLocation.y < cycleRect.y + 36) { // pick up cycle
                  heldCycle = cycleArray.removeCycle(i);
                } 
              }
	      // pan around view
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
        }

        // move held cycle or cycle step 
        if (heldCycle != nullptr) {
          heldCycle->setXY(touchLocation.x - 50, touchLocation.y - 23);
        }
        if (heldStep != nullptr) {
          heldStep->setXY(touchLocation.x - 50, touchLocation.y - 23);
        }

      // if touch end
      } else if (UI::event.type == SDL_FINGERUP) {
        // record touch location
        touchLocation.x = UI::event.tfinger.x * SCREEN_WIDTH;
        touchLocation.y = UI::event.tfinger.y * SCREEN_HEIGHT;
	// if touch time was less than half a seccond, meaning a tap
        if (UI::event.tfinger.timestamp - touchTimeStart <= 500) {
          // look at all the cycle array text boxes to see of one of them was tapped
          for (unsigned int i = 0; i < cycleArray.cycles_.size(); i++) {
            for (unsigned int j = 0; j < cycleArray.cycles_[i]->steps_.size(); j++) {
              SDL_Rect stepRect = cycleArray.cycles_[i]->steps_[j]->getRect();
              if (SDL_PointInRect(&touchLocation, &stepRect)) {
                if (touchLocation.y > stepRect.y + stepRect.h / 2) {// tapped cycle step duration text box
                  if (selectedTextBox != nullptr) { // deselect old text box
                    selectedTextBox->deselect();
                  }
		  // select new text box
                  selectedTextBox = cycleArray.cycles_[i]->steps_[j]->getDuration();
                  selectedTextBox->select(); 
                } else { // tapped cycle step temperature text box
                  if (selectedTextBox != nullptr) {  // deselect old text box
                    selectedTextBox->deselect();
                  }
		  // select new text box
                  selectedTextBox = cycleArray.cycles_[i]->steps_[j]->getTemperature();
                  selectedTextBox->select();
                }
              }
            }
            SDL_Rect cycleRect = cycleArray.cycles_[i]->getRect();
            if (SDL_PointInRect(&touchLocation, &cycleRect) && touchLocation.y < cycleRect.y + 36) { // tapped number of cycles text box
              if (selectedTextBox != nullptr) { // deselect old text box
                selectedTextBox->deselect();
              }
	      // select new text box
              selectedTextBox = cycleArray.cycles_[i]->getNumberOfCycles();
              selectedTextBox->select();
            }
          }
        }
	// if touch end is over button padding
        SDL_Rect buttonPaddingRect = buttonPadding.getRect();
        if (SDL_PointInRect(&touchLocation, &buttonPaddingRect)) {\
	  // delete held cycle or cycle step
          if (heldStep != nullptr) {
            delete heldStep;
            heldStep = nullptr;
          } else if (heldCycle != nullptr) {
            delete heldCycle;
            heldCycle = nullptr;
          }
        }
	// place held cycle step
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
	// place held cycle
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

    // create new cycle step if the last one was taken
    if (newStep == heldStep) {
      newStep = new UI::CycleStep(565, 5);
    }

    // remove any empty cycles from cycle array
    cycleArray.removeEmptyCycles(); 

    // render UI elements
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

    // check for SDL errors
    if (SDL_GetError()[0] != '\0') { // update error text if new error has occured 
      std::cout << "SDL ERROR: " << SDL_GetError() << std::endl;
      errorText.setText(SDL_GetError());
      SDL_ClearError();
    }
    errorText.render(); // render error
    SDL_RenderPresent(UI::renderer); // present render
  }
  // quit SDL
  SDL_DestroyWindow(UI::window);
  SDL_Quit();
  return 0;
}
