// FLC-PCR
// Fort Lewis Collage
// James Ferguson

#include <stdio.h>
#include <iostream>
#include <string>
#include <ctime>
#include <sys/time.h>

#include "ui.h"

// lunix serial
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>

// class for performing logic and rendering of the experiment editor where users can edit and run experiments
class ExperimentEditor {
  private:
    int serialPort_; // the serial port for communication to atmega328p
    bool atTemperature_; // is the system at the target temperature
    bool timerStarted_; // has the timer for the current step started
    timeval stepStartTime; // the time that the current step got to temperature

    enum states_ {idle_, running_, done_, abort_}; // system states
    states_ state_ = idle_; // current system state<F3>
    int experimentIndex_ = 0; // current temperature step position
 
    // UI element declarations
    UI::CycleArray cycleArray;
    
    // keypad, start/stop, and new step elements
    UI::Padding buttonPadding;
    UI::NumberKey keys [11];
    UI::Padding buttonCover;
    UI::Image recycleBin;
    UI::Button startStopButton; 
    UI::Text dragToAdd;
    UI::TextBox* selectedTextBox = nullptr;
    UI::CycleStep* newStep = nullptr;
    UI::CycleStep* heldStep = nullptr;
    UI::Cycle* heldCycle = nullptr;
    
    // info bar and contained elements
    UI::Padding infoBarPadding;
    UI::Image statusIndicator;
    UI::Padding progressBarPadding;
    UI::Padding progressBar;
    UI::Text targetTemperature;

  public:
    ExperimentEditor ():
    cycleArray(5, 5),
    buttonPadding("img/padding/R_Grey_3.png", 5, 555, -10, 250, 500),
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
    buttonCover("img/padding/R_Blue.png", 5, 555, -10, 250, 500),
    recycleBin("img/Recycle.png", 616, 176),
    startStopButton(665, 5, 130, 47, "Start"),
    dragToAdd("fonts/consola.ttf", 16, 560, 5, "Drag to Add"),
    infoBarPadding("img/padding/R_Grey_1.png", 5, -10, 453, 820, 36),
    statusIndicator("img/Red_Light.png", 5, 459), 
    progressBarPadding("img/padding/S_Grey_2.png", 2, 26, 459, 69, 16),
    progressBar("img/padding/S_Red.png", 2, 26, 459, 0, 16),
    targetTemperature("fonts/consola.ttf", 16, 100, 459, "Idle") {
      openSerial();
    }

    // turn on and start sending temperature data to plc
    void startExperiment () {
      if (cycleArray.size() == 0) {
        return;
      }
      if (selectedTextBox != nullptr) {
        selectedTextBox->deselect();
        selectedTextBox = nullptr;
      }
      state_ = running_;
      experimentIndex_ = -1;
      writeSerial("on\n");
      statusIndicator.setTexture("img/Green_Light.png");
      progressBar.setTexture("img/padding/S_Green.png", 2);
      startStopButton.setText("Abort");
      gettimeofday(&stepStartTime, NULL);
      nextStep(); 
    }

    // check if an update to plc temperature is readdy
    void updateStep () {
      timeval currentTime;
      gettimeofday(&currentTime, NULL);
      double currentDuration = (currentTime.tv_sec - stepStartTime.tv_sec) + (currentTime.tv_usec - stepStartTime.tv_usec) * 1e-6;
      
      static int cnt;
      cnt = (cnt + 1) % 120;
      if (cnt == 59) {
        writeSerial("d\n");
      } else if (cnt == 119) {
        std::string serialString = readSerial(); 
        
        std::string sub = "none";
        for (unsigned int i = 0; i < serialString.length(); i++) {
          if (serialString[i] == ' ') {
            sub = serialString.substr(0, i);
            UI::CycleStep* step = cycleArray.getStep(experimentIndex_);
            if (std::stof(sub) - std::stof(step->getTemperature()->getText()) < 5) {
              atTemperature_ = true;
            }
            break;
          }
        }  
      }

      if (atTemperature_ && !timerStarted_) {
        gettimeofday(&stepStartTime, NULL);
        timerStarted_ = true;
      }
      
      if (timerStarted_ && currentDuration > std::stoi(cycleArray.getStep(experimentIndex_)->getDuration()->getText())) {
        nextStep();
        gettimeofday(&stepStartTime, NULL);
      }
    }

    // set plc to next temperature in experiment
    void nextStep () {
      experimentIndex_ += 1;
      
      progressBar.setWH(69 * (float)experimentIndex_ / (float)cycleArray.size(), 16);

      try {
        UI::CycleStep* step = cycleArray.getStep(experimentIndex_);
        targetTemperature.setText(step->getTemperature()->getText() + "\xB0" + "C");
        writeSerial("pt" + step->getTemperature()->getText() + "\n");
        atTemperature_ = false;
        timerStarted_ = false;
      } catch (...) {
        finishExperiment();
      }
    }

    // reached end of experiment shut plc down
    void finishExperiment () {
      writeSerial("off\n");
      targetTemperature.setText("Finished");
      statusIndicator.setTexture("img/Blue_Light.png");
      progressBar.setTexture("img/padding/S_Blue.png", 2);
      state_ = done_;
      startStopButton.setText("Reset");
    } 

    // pematurely end experiment turn plc off
    void abortExperiment () {
      writeSerial("off\n");
      targetTemperature.setText("Aborted");
      statusIndicator.setTexture("img/Red_Light.png");
      progressBar.setTexture("img/padding/S_Red.png", 2);
      state_ = abort_;
      startStopButton.setText("Reset");
    }

    // re enables experiment editiog
    void resetExperiment() {
      experimentIndex_ = 0;
      progressBar.setWH(0, 16);
      targetTemperature.setText("Idle");
      statusIndicator.setTexture("img/Red_Light.png");
      progressBar.setTexture("img/padding/S_Red.png", 2);
      state_ = idle_;
      startStopButton.setText("Start");
    }
    
    // open /dev/ttyACM0 as linux serial with baud rate 115200
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

      tty.c_cc[VTIME] = 0;
      tty.c_cc[VMIN] = 0;

      cfsetispeed(&tty, B115200);
      cfsetospeed(&tty, B115200);

      if (tcsetattr(serialPort_, TCSANOW, &tty) != 0) {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
      }
    }

    // close serial port
    void closeSerial () {
      close(serialPort_);
    }

    // read data from serial port
    std::string readSerial () {
      char readBuffer [256];
      memset(&readBuffer, '\0', sizeof(readBuffer));
      read(serialPort_, &readBuffer, 256);
      return std::string(readBuffer);
    }

    // write data to serial port
    void writeSerial (std::string message) {
      write(serialPort_, message.c_str(), sizeof(message.c_str()));
    }
    
    // performs editor input and logic returns 1 for program termination
    int logic () {
      SDL_Point touchLocation; // location of touch
      static int touchTimeStart; // time touch started

      while (SDL_PollEvent(&UI::event) != 0) { // loop through all inputs
        if (UI::event.type == SDL_QUIT) { // if user hit window x button
          return 1; // quit
        } else if (UI::event.type == SDL_KEYDOWN) { // key presses
          switch (UI::event.key.keysym.sym) {
            case SDLK_ESCAPE: // pressed escape, quit
              return 1;
              break;
            case SDLK_s: // pressed s, take screenshot
              static int screenshotNumber;
              UI::takeScreenShot("screenshot" + std::to_string(screenshotNumber++) + ".png");
              break;
          }

        // if touch start
        } else if (UI::event.type == SDL_FINGERDOWN) {
          
          // get event location and start time
          touchLocation.x = UI::event.tfinger.x * SCREEN_WIDTH;
          touchLocation.y = UI::event.tfinger.y * SCREEN_HEIGHT;
          touchTimeStart = UI::event.tfinger.timestamp;

          // only use buttons if not holding anything
          if (heldStep == nullptr && heldCycle == nullptr) {
            
            // type numbers if a text box is selected and a putton is pressed
            if (selectedTextBox != nullptr) {
              for (int i = 0; i < 11; i++) {
                SDL_Rect keyRect = keys[i].getRect();
                if (SDL_PointInRect(&touchLocation, &keyRect)) {
                  keys[i].press(selectedTextBox);
                }
              }
            }

            // check if start/abort/reset button is pressed
            SDL_Rect startStopButtonRect = startStopButton.getRect();
            if (SDL_PointInRect(&touchLocation, &startStopButtonRect)) {
              if (state_ == running_) {
                abortExperiment(); 
              } else if (state_ == done_ || state_ == abort_) {
                resetExperiment();
              } else {
                startExperiment();
              }
            }
          }

        // touch move
        } else if (UI::event.type == SDL_FINGERMOTION) {
          
          // get event location and velocity
          touchLocation.x = UI::event.tfinger.x * SCREEN_WIDTH;
          touchLocation.y = UI::event.tfinger.y * SCREEN_HEIGHT;
          SDL_Point touchDelta;
          touchDelta.x = UI::event.tfinger.dx * SCREEN_WIDTH;
          touchDelta.y = UI::event.tfinger.dy * SCREEN_HEIGHT;
          
          // only allow motion if an experiment is not running
          if (state_ != running_) {

            // if touch speed is grater than 5
            SDL_Rect buttonPaddingRect = buttonPadding.getRect();
            if (abs(touchDelta.x) + abs(touchDelta.y) > 5) {
              
              // if touch is over button padding
              if (SDL_PointInRect(&touchLocation, &buttonPaddingRect)) {
                SDL_Rect newStepRect = newStep->getRect();

                // if touching new step pick it up and not holding anything
                if (SDL_PointInRect(&touchLocation, &newStepRect) && heldStep == nullptr && heldCycle == nullptr) {
                  heldStep = newStep;
                }
              } else { // touch is not on the button padding
                // if not holding anything
                if (heldStep == nullptr && heldCycle == nullptr) {
                  
                  // look at all the cycles
                  for (unsigned int i = 0; i < cycleArray.cycles_.size(); i++) {
                    // look at all the steps of this cycle
                    for (unsigned int j = 0; j < cycleArray.cycles_[i]->steps_.size(); j++) {
                      
                      // if touchin this step pick it up
                      SDL_Rect stepRect = cycleArray.cycles_[i]->steps_[j]->getRect();
                      if (SDL_PointInRect(&touchLocation, &stepRect)) {
                        heldStep = cycleArray.cycles_[i]->removeStep(j);
                      }
                    }

                    // if touching this cycle pick it up
                    SDL_Rect cycleRect = cycleArray.cycles_[i]->getRect();
                    if (SDL_PointInRect(&touchLocation, &cycleRect) && touchLocation.y < cycleRect.y + 36) {
                      heldCycle = cycleArray.removeCycle(i);
                    }
                  }

                  // pan around cycle view
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

            //move held cycle or step if any
            if (heldCycle != nullptr) {
              heldCycle->setXY(touchLocation.x - 50, touchLocation.y - 23);
            }
            if (heldStep != nullptr) {
              heldStep->setXY(touchLocation.x - 50, touchLocation.y - 23);
            }
          }

        // touch end
        } else if (UI::event.type == SDL_FINGERUP) {
          
          // record touch location
          touchLocation.x = UI::event.tfinger.x * SCREEN_WIDTH;
          touchLocation.y = UI::event.tfinger.y * SCREEN_HEIGHT;
          
          // if not holding anuthing and the system is not running
          if (heldStep == nullptr && heldCycle == nullptr && state_ != running_) {
            
            // if the touch event was under half a seccond
            if (UI::event.tfinger.timestamp - touchTimeStart <= 500) { // tap
              
              // look at all the cycles
              for (unsigned int i = 0; i < cycleArray.cycles_.size(); i++) {
                
                // look at all the steps in this cycle
                for (unsigned int j = 0; j < cycleArray.cycles_[i]->steps_.size(); j++) {
                  
                  // if taped on this step
                  SDL_Rect stepRect = cycleArray.cycles_[i]->steps_[j]->getRect();
                  if (SDL_PointInRect(&touchLocation, &stepRect)) {
                    // if tapped on the duration text box, select it
                    if (touchLocation.y > stepRect.y + stepRect.h / 2) {
                      if (selectedTextBox != nullptr) { // deselect old text box
                        selectedTextBox->deselect();
                      } 
                      // select new text box
                      selectedTextBox = cycleArray.cycles_[i]->steps_[j]->getDuration();
                      selectedTextBox->select();
                    // if tapped on the temperature text box, select it
                    } else {
                      if (selectedTextBox != nullptr) { // deselect old text box
                        selectedTextBox->deselect();
                      }
                      // select new text box
                      selectedTextBox = cycleArray.cycles_[i]->steps_[j]->getTemperature();
                      selectedTextBox->select();
                    }
                  }
                }
                // if tapped on the number of cycles text box, select it
                SDL_Rect cycleRect = cycleArray.cycles_[i]->getRect();
                if (SDL_PointInRect(&touchLocation, &cycleRect) && touchLocation.y < cycleRect.y + 36) {
                  if (selectedTextBox != nullptr) { // deselect old text box
                    selectedTextBox->deselect();
                  }
                  // select new text box
                  selectedTextBox = cycleArray.cycles_[i]->getNumberOfCycles();
                  selectedTextBox->select();
                }
              }
            }
          }

          // if touch end is over button padding delete
          SDL_Rect buttonPaddingRect = buttonPadding.getRect();
          if (SDL_PointInRect(&touchLocation, &buttonPaddingRect)) {
            // delete whatever, if any is held
            if (heldStep != nullptr) {
              delete heldStep;
              heldStep = nullptr;
            } else if (heldCycle != nullptr) {
              delete heldCycle;
              heldCycle = nullptr;
            }
          }

          // if holding a step and not over button padding
          if (heldStep != nullptr) {
            SDL_Point cycleArrayPos = cycleArray.getPoint();
            // look at all the cycles
            for (unsigned int i = 0; i < cycleArray.cycles_.size(); i++) {
              if (touchLocation.x - cycleArrayPos.x < (int)i * 110 + 110) {
                // look at all the steps in this cycle
                for (unsigned int j = 0; j < cycleArray.cycles_[i]->steps_.size(); j++) {
                  // if touch is inside cycle, put step inside cycle
                  if (touchLocation.y - cycleArrayPos.y < (int)j * 52 + 54) {
                    cycleArray.cycles_[i]->addStep(j, heldStep);
                    heldStep = nullptr;
                    break;
                  }
                }
                // touch is below cycle, add step to end of cycle 
                if (heldStep != nullptr) {
                  cycleArray.cycles_[i]->addStep(cycleArray.cycles_[i]->steps_.size(), heldStep);
                  heldStep = nullptr;
                }
                break;
              }
            }
            // touch is not on any cycle, create new cycle at end and place step in it
            if (heldStep != nullptr) {
              cycleArray.addCycle(cycleArray.cycles_.size(), new UI::Cycle());
              cycleArray.cycles_[cycleArray.cycles_.size() - 1]->addStep(0, heldStep);
              heldStep = nullptr;
            }
          }

          // if holding cycle and not over button padding
          if (heldCycle != nullptr) {
            SDL_Point cycleArrayPos = cycleArray.getPoint();
            // look at all the cycles
            for (unsigned int i = 0; i < cycleArray.cycles_.size(); i++) {
              // if touch is inside cycleArray, put cycle inside the array
              if (touchLocation.x - cycleArrayPos.x < (int)i * 110 + 55) {
                cycleArray.addCycle(i, heldCycle);
                heldCycle = nullptr;
                break;
              }
            }
            // touch outside cycle array, put cycle at the end of cycle array
            if (heldCycle != nullptr) {
              cycleArray.addCycle(cycleArray.cycles_.size(), heldCycle);
              heldCycle = nullptr;
            }
          }
        }
      }

      // if the new step was taken make a new one
      if (newStep == heldStep || newStep == nullptr) {
        newStep = new UI::CycleStep(560, 26);
      }

      // remove empty cycle from the cycle array
      cycleArray.removeEmptyCycles();

      // update plc if an experiment is running
      if (state_ == running_) {
        updateStep();
      }
      return 0;
    }

    // renders the UI for the cycle editor
    void render () { 
      cycleArray.render();
      buttonPadding.render();
      startStopButton.render();
      for (int i = 0; i < 11; i++) {
        keys[i].render();
      }
      
      dragToAdd.render();
      newStep->render();

      if (heldStep != nullptr || heldCycle != nullptr) {
        buttonCover.render();
        recycleBin.render();
      }

      infoBarPadding.render();
      statusIndicator.render();
      progressBarPadding.render();

      int progressBarSize = progressBar.getRect().w;
      if (progressBarSize >= 3) {
        progressBar.render();
      }
      targetTemperature.render();
 
      if (heldStep != nullptr) {
        heldStep->render();
      }
      if (heldCycle != nullptr) {
        heldCycle->render();
      }
    }

    // save experiment in cycle array
    void save (std::string path) {
      cycleArray.save(path);
    }

    // load experiment into cycle array
    void load (std::string path) {
      cycleArray.load(path);
    }

    ~ExperimentEditor () {
      delete selectedTextBox;
      delete newStep;
      delete heldStep;
      delete heldCycle;
      closeSerial();
    }
};

int main(int argc, char* args[]) {
  
  // initialize UI
  if (UI::init()) {
    return 1;
  }

  // create experiment editor
  ExperimentEditor editor;
  
  // load default experiment data
  editor.load("testsave.exp");
  
  // program loop
  bool run = true;
  while (run) {

    // run experimet editor logic
    if (editor.logic()) {
      break;
    }
    
    // begin render, clear screen
    SDL_SetRenderDrawColor(UI::renderer, 74, 84, 98, 255);
    SDL_RenderClear(UI::renderer);
 
    // draw editor
    editor.render();
 
    // present render
    SDL_RenderPresent(UI::renderer);
  
    // check for errors dump to console
    if (SDL_GetError()[0] != '\0') {
      std::cout << "SDL ERROR: " << SDL_GetError() << std::endl;
      SDL_ClearError();
    }
  }
  
  // clean up
  SDL_DestroyWindow(UI::window);
  SDL_Quit();

  return 0;
}
