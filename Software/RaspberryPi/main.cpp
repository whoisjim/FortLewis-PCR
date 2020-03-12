#include <stdio.h>
#include <iostream>
#include <string>
#include <ctime>
#include <sys/time.h>

// lunix serial
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>

#include "ui.h"

//class mainMenu {

//};

class ExperimentEditor {
  private:
    int serialPort_;

    enum states_ {idle_, running_};
    states_ state_ = idle_;
    int experimentIndex_ = 0;
 
    UI::CycleArray cycleArray;
    UI::Padding buttonPadding;
    UI::Padding infoBarPadding;
    UI::NumberKey keys [11];

    UI::Button startStopButton;
    timeval stepStartTime;


    UI::TextBox* selectedTextBox = nullptr;
    UI::CycleStep* newStep = nullptr;
    UI::CycleStep* heldStep = nullptr;
    UI::Cycle* heldCycle = nullptr;

  public:

    void startExperiment () {
      state_ = running_;
      experimentIndex_ = -1;
      writeSerial("on\n");
      nextStep(); 
    }

    void updateStep () {
      timeval currentTime;
      gettimeofday(&currentTime, NULL);
      double currentDuration = (currentTime.tv_sec - stepStartTime.tv_sec) + (currentTime.tv_usec - stepStartTime.tv_usec) * 1e-6;
      if (currentDuration > std::stoi(cycleArray.getStep(experimentIndex_)->getDuration()->getText())) {
        nextStep();
      }
    }

    void nextStep () {
      experimentIndex_ += 1;
      try {
        UI::CycleStep* step = cycleArray.getStep(experimentIndex_);
        writeSerial("pt" + step->getTemperature()->getText() + "\n");
        gettimeofday(&stepStartTime, NULL);
      } catch (...) {
        std::cout << "invalid index " << experimentIndex_ << std::endl;
      }
    }

    void abortExperiment () {
      writeSerial("off\n");
      state_ = idle_;
    }

    ExperimentEditor ():
    cycleArray(5, 5),
    buttonPadding("img/grey_padding.png", 6, 555, -10, 250, 500),
    infoBarPadding("img/default_padding.png", 6, -10, 453, 820, 36),
    keys{UI::NumberKey(560, 133, 75, 75, '7', "7"),
         UI::NumberKey(640, 133, 75, 75, '8', "8"),
         UI::NumberKey(720, 133, 75, 75, '9', "9"),
         UI::NumberKey(560, 213, 75, 75, '4', "4"),
         UI::NumberKey(640, 213, 75, 75, '5', "5"),
         UI::NumberKey(720, 213, 75, 75, '6', "6"),
         UI::NumberKey(560, 293, 75, 75, '1', "1"),
         UI::NumberKey(640, 293, 75, 75, '2', "2"),
         UI::NumberKey(720, 293, 75, 75, '3', "3"),
         UI::NumberKey(560, 373, 75, 75, '0', "0"),
         UI::NumberKey(640, 373, 155, 75, '\b', "del")},
    startStopButton(695, 5, 100, 50, "on/\noff"){
      cycleArray.addCycle(0, new UI::Cycle());
      cycleArray.addCycle(0, new UI::Cycle());
      cycleArray.addCycle(0, new UI::Cycle());
      cycleArray.cycles_[0]->addStep(0, new UI::CycleStep());
      cycleArray.cycles_[1]->addStep(0, new UI::CycleStep());
      cycleArray.cycles_[1]->addStep(1, new UI::CycleStep());
      cycleArray.cycles_[1]->addStep(2, new UI::CycleStep());
      cycleArray.cycles_[2]->addStep(0, new UI::CycleStep());

      openSerial();
    }

    void openSerial () {
      serialPort_ = open("/dev/ttyACM0", O_RDWR);

      if (serialPort_ < 0) {
        printf("Error %i opening serial port: %s\n", errno, strerror(errno));
      }

      struct termios tty;
      memset(&tty, 0, sizeof(tty));

      if (tcgetattr(serialPort_, &tty) != 0) {
        printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
      }

      tty.c_cflag &= ~(PARENB | CSTOPB | CRTSCTS);
      tty.c_cflag |= CREAD | CLOCAL | CS8;

      tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHONL | ISIG);

      tty.c_iflag &= ~(IXON | IXOFF | IXANY | IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);

      tty.c_oflag &= ~(OPOST | ONLCR);

      tty.c_cc[VTIME] = 1;
      tty.c_cc[VMIN] = 0;

      cfsetispeed(&tty, B9600);
      cfsetospeed(&tty, B9600);

      if (tcsetattr(serialPort_, TCSANOW, &tty) != 0) {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
      }
    }

    void closeSerial () {
      close(serialPort_);
    }

    std::string readSerial () {
      char readBuffer [256];
      memset(&readBuffer, '\0', sizeof(readBuffer));
      read(serialPort_, &readBuffer, sizeof(readBuffer));
      return std::string(readBuffer);
    }

    void writeSerial (std::string message) {
      write(serialPort_, message.c_str(), sizeof(message.c_str()));
    }
    
    // 0 run, 1 end
    int logic () {
      SDL_Point touchLocation;
      static int touchTimeStart;

      while (SDL_PollEvent(&UI::event) != 0) {
        if (UI::event.type == SDL_QUIT) {
          return 1;
        } else if (UI::event.type == SDL_KEYDOWN) {
          switch (UI::event.key.keysym.sym) {
            case SDLK_ESCAPE:
              return 1;
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
          SDL_Rect startStopButtonRect = startStopButton.getRect();
          if (SDL_PointInRect(&touchLocation, &startStopButtonRect)) {
            if (state_ == running_) {
              abortExperiment(); 
            } else {
              startExperiment();
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
          if (abs(touchDelta.x) + abs(touchDelta.y) > 5) {
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
      if (newStep == heldStep || newStep == nullptr) {
        newStep = new UI::CycleStep(565, 5);
      }
      cycleArray.removeEmptyCycles();
      
      if (state_ == running_) {
        updateStep();
      }

      return 0;
    } 

    void render () { 
      cycleArray.render();
      buttonPadding.render();
      startStopButton.render();
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
    }

    ~ExperimentEditor () {
      delete selectedTextBox;
      delete newStep;
      delete heldStep;
      delete heldCycle;
      closeSerial();
    }
};

//class ExperimentManager {

//};

int main(int argc, char* args[]) {

  if (UI::init()) {
    return 1;
  }

  ExperimentEditor editor;

  bool run = true;
  while (run) {
    SDL_SetRenderDrawColor(UI::renderer, 90, 105, 136, 255);
    SDL_RenderClear(UI::renderer);

    if (editor.logic()) {
      break;
    }
    editor.render();

    if (SDL_GetError()[0] != '\0') {
      std::cout << "SDL ERROR: " << SDL_GetError() << std::endl;
      SDL_ClearError();
    }
    SDL_RenderPresent(UI::renderer);
  }
  SDL_DestroyWindow(UI::window);
  SDL_Quit();

  return 0;
}
