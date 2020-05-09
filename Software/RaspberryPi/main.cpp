// FLC-PCR
// Fort Lewis Collage
// James Ferguson

#include <stdio.h>
#include <iostream>
#include <string>
#include <ctime>
#include <sys/time.h>
#include <filesystem>

#include "ui.h"

// lunix serial
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>

enum states {QUIT, MAIN_MENU, EDITOR_MENU, LOAD_MENU, SAVE_MENU};

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
    UI::NumberKey keys [12];
    UI::Padding buttonCover;
    UI::Image recycleBin;
    UI::Button startStopButton;
    UI::Button loadButton;
    UI::Button saveButton;
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
    buttonPadding("img/padding/R_Grey_2.png", 5, 554, -10, 251, 500),
    keys{UI::NumberKey(560,  78, 75, 75, '7', "7"),
         UI::NumberKey(640,  78, 75, 75, '8', "8"),
         UI::NumberKey(720,  78, 75, 75, '9', "9"),
         UI::NumberKey(560, 158, 75, 75, '4', "4"),
         UI::NumberKey(640, 158, 75, 75, '5', "5"),
         UI::NumberKey(720, 158, 75, 75, '6', "6"),
         UI::NumberKey(560, 238, 75, 75, '1', "1"),
         UI::NumberKey(640, 238, 75, 75, '2', "2"),
         UI::NumberKey(720, 238, 75, 75, '3', "3"),
         UI::NumberKey(560, 318, 75, 75, '0', "0"),
         UI::NumberKey(640, 318, 75, 75, '.', "."),
         UI::NumberKey(720, 318, 75, 75, '\b', "del")},
    buttonCover("img/padding/R_Blue.png", 5, 554, -10, 251, 500),
    recycleBin("img/Recycle.png", 616, 176),
    startStopButton(665, 5, 130, 68, "Start"),
    loadButton(560, 398, 115, 50, "Load"),
    saveButton(680, 398, 115, 50, "Save"),
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
    states logic () {
      SDL_Point touchLocation; // location of touch
      static int touchTimeStart; // time touch started

      while (SDL_PollEvent(&UI::event) != 0) { // loop through all inputs
        if (UI::event.type == SDL_QUIT) { // if user hit window x button
          return QUIT; // quit
        } else if (UI::event.type == SDL_KEYDOWN) { // key presses
          switch (UI::event.key.keysym.sym) {
            case SDLK_ESCAPE: // pressed escape, quit
              return QUIT;
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
              for (int i = 0; i < 12; i++) {
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

            // load button
            SDL_Rect loadButtonRect = loadButton.getRect();
            if (SDL_PointInRect(&touchLocation, &loadButtonRect)) {
              return LOAD_MENU;
            }

            // save button
            SDL_Rect saveButtonRect = saveButton.getRect();
            if (SDL_PointInRect(&touchLocation, &saveButtonRect)) {
              return SAVE_MENU;
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
                        selectedTextBox->formatNumber();
                        selectedTextBox->deselect();
                      } 
                      // select new text box
                      selectedTextBox = cycleArray.cycles_[i]->steps_[j]->getDuration();
                      selectedTextBox->select();
                    // if tapped on the temperature text box, select it
                    } else {
                      if (selectedTextBox != nullptr) { // deselect old text box
                        selectedTextBox->formatNumber();
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
                    selectedTextBox->formatNumber();
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
      return EDITOR_MENU;
    }

    // renders the UI for the cycle editor
    void render () {
      // begin render, clear screen
      SDL_SetRenderDrawColor(UI::renderer, 74, 84, 98, 255);
      SDL_RenderClear(UI::renderer);
      
      cycleArray.render();
      buttonPadding.render();
      startStopButton.render();
      loadButton.render();
      saveButton.render();
      for (int i = 0; i < 12; i++) {
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
      // present render
      SDL_RenderPresent(UI::renderer);
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

class MainMenu {
  private:
    UI::Text mainText;
    UI::Button newButton;
    UI::Button loadButton;
  public:
    MainMenu () :
    mainText("fonts/consola.ttf", 100, 10, 10, "FLC-PCR Main Menu"),
    newButton(10, 120, 780, 100, "New Experiment"),
    loadButton(10, 230, 780, 100, "Load Experiment") {}

    states logic () {
      while (SDL_PollEvent(&UI::event) != 0) { // loop through all inputs
        if (UI::event.type == SDL_QUIT) { // if user hit window x button
          return QUIT; // quit
        } else if (UI::event.type == SDL_KEYDOWN) { // key presses
          switch (UI::event.key.keysym.sym) {
            case SDLK_ESCAPE: // pressed escape, quit
              return QUIT;
              break;
            case SDLK_s: // pressed s, take screenshot
              static int screenshotNumber;
              UI::takeScreenShot("screenshot" + std::to_string(screenshotNumber++) + ".png");
              break;
          }
        } else if (UI::event.type == SDL_FINGERDOWN) {
          SDL_Point touchLocation;
          touchLocation.x = UI::event.tfinger.x * SCREEN_WIDTH;
          touchLocation.y = UI::event.tfinger.y * SCREEN_HEIGHT;
          SDL_Rect newButtonRect = newButton.getRect();
          if (SDL_PointInRect(&touchLocation, &newButtonRect)) {
            return EDITOR_MENU;
          }
          SDL_Rect loadButtonRect = loadButton.getRect();
          if (SDL_PointInRect(&touchLocation, &loadButtonRect)) {
            return LOAD_MENU;
          }

        }
      }
      return MAIN_MENU;
    }

    void render () {
      // begin render, clear screen
      SDL_SetRenderDrawColor(UI::renderer, 139, 147, 175, 255);
      SDL_RenderClear(UI::renderer);
      
      mainText.render();
      newButton.render();
      loadButton.render();

      // present render
      SDL_RenderPresent(UI::renderer);
    }
};

class LoadMenu {
  private:
    ExperimentEditor* editor_;
    UI::Padding pathSelection_;
    int selectedPathIndex_ = -1;
    std::vector<UI::Text> experimentPaths_;
    std::string savePath_ = "/home/pi/experiments";
    float textScroll_ = 55;
    int touchTimeStart = 0;
    UI::Padding buttonPadding;
    UI::Button newButton;
    UI::Button loadButton;
    UI::Button exitButton;
    UI::Button deleteButton;
  public:
    LoadMenu (ExperimentEditor* editor) :
    pathSelection_("img/padding/S_Blue.png", 2, -2, 8, 804, 20),
    buttonPadding("img/padding/R_Grey_2.png", 5, -5, -5, 810, 51),
    newButton(5, 5, 100, 35, "New"),
    loadButton(110, 5, 100, 35, "Load"),
    exitButton(215, 5, 100, 35, "Exit"),
    deleteButton(695, 5, 100, 35, "Delete") {
      updatePaths();
      editor_ = editor;
    }
    
    void updatePaths () {
      experimentPaths_.clear();
      for (const auto & entry : std::filesystem::directory_iterator(savePath_)) {
        if (entry.path().string().substr(entry.path().string().size() - 4, 4) == ".exp") {
          UI::Text entryText("fonts/consola.ttf", 16, 0, 0, entry.path().string());
          experimentPaths_.push_back(entryText);
        }
      }
      movePaths();
    }

    void movePaths () {
      for (unsigned int i = 0; i < experimentPaths_.size(); i++) {
        experimentPaths_[i].setXY(10, i * 21 + textScroll_);
      }
      moveSelection();
    }

    void moveSelection () { 
      pathSelection_.setXY(-2, selectedPathIndex_ * 21 + textScroll_ - 2);
    }

    states logic () {
      while (SDL_PollEvent(&UI::event) != 0) { // loop through all inputs
        if (UI::event.type == SDL_QUIT) { // if user hit window x button
          return QUIT; // quit
        } else if (UI::event.type == SDL_KEYDOWN) { // key presses
          switch (UI::event.key.keysym.sym) {
            case SDLK_ESCAPE: // pressed escape, quit
              return QUIT;
              break;
            case SDLK_s: // pressed s, take screenshot
              static int screenshotNumber;
              UI::takeScreenShot("screenshot" + std::to_string(screenshotNumber++) + ".png");
              break;
          }
        } else if (UI::event.type == SDL_FINGERDOWN) {
          SDL_Point touchLocation;
          touchLocation.x = UI::event.tfinger.x * SCREEN_WIDTH;
          touchLocation.y = UI::event.tfinger.y * SCREEN_HEIGHT;
          touchTimeStart = UI::event.tfinger.timestamp;
          SDL_Rect newButtonRect = newButton.getRect();
          if (SDL_PointInRect(&touchLocation, &newButtonRect)) {
            editor_->load("/home/pi/Untitled.exp");
            return EDITOR_MENU;
          }
          SDL_Rect loadButtonRect = loadButton.getRect();
          if (SDL_PointInRect(&touchLocation, &loadButtonRect)) {
            if (selectedPathIndex_ != -1) { 
              editor_->load(experimentPaths_[selectedPathIndex_].getText());
              return EDITOR_MENU;
            }
          }
          SDL_Rect deleteButtonRect = deleteButton.getRect();
          if (SDL_PointInRect(&touchLocation, &deleteButtonRect)) {
           if (selectedPathIndex_ != -1) {
            remove(experimentPaths_[selectedPathIndex_].getText().c_str());
            selectedPathIndex_ = -1;
            updatePaths();
           } 
          }
          SDL_Rect exitButtonRect = exitButton.getRect();
          if (SDL_PointInRect(&touchLocation, &exitButtonRect)) {
            
          }
        } else if (UI::event.type == SDL_FINGERMOTION) {
          if (abs(UI::event.tfinger.dy * SCREEN_HEIGHT) > 3) {
            textScroll_ += UI::event.tfinger.dy * SCREEN_HEIGHT;
            if (textScroll_ < 55 + 21 - (int)experimentPaths_.size() * 21) {
              textScroll_ = 55 + 21 - (int)experimentPaths_.size() * 21;
            }
            if (textScroll_ > 55) {
              textScroll_ = 55;
            }
            movePaths();
          }
        } else if (UI::event.type == SDL_FINGERUP) {
          SDL_Point touchLocation;
          touchLocation.x = UI::event.tfinger.x * SCREEN_WIDTH;
          touchLocation.y = UI::event.tfinger.y * SCREEN_HEIGHT;
          
          if (UI::event.tfinger.timestamp - touchTimeStart <= 200 && touchLocation.y > 50) {
            // tap path
            selectedPathIndex_ = int(touchLocation.y - textScroll_) / 21;
            if (selectedPathIndex_ >= (int)experimentPaths_.size() || selectedPathIndex_ < 0) {
              selectedPathIndex_ = -1;
            }
            moveSelection();
          }
        }
      }
      return LOAD_MENU;
    }

    void render () {
      // begin render, clear screen
      SDL_SetRenderDrawColor(UI::renderer, 109, 117, 141, 255);
      SDL_RenderClear(UI::renderer);

      if (selectedPathIndex_ != -1) {
        pathSelection_.render();
      }

      for (unsigned int i = 0; i < experimentPaths_.size(); i++) {
        experimentPaths_[i].render();
      }

      buttonPadding.render();

      newButton.render();
      loadButton.render();
      exitButton.render();
      deleteButton.render();

      // present render
      SDL_RenderPresent(UI::renderer);
    }
};


class SaveMenu {
  private:
    ExperimentEditor* editor_;
  public:
    SaveMenu (ExperimentEditor* editor) {
      editor_ = editor;
    } 

    states logic () {
      while (SDL_PollEvent(&UI::event) != 0) { // loop through all inputs
        if (UI::event.type == SDL_QUIT) { // if user hit window x button
          return QUIT; // quit
        } else if (UI::event.type == SDL_KEYDOWN) { // key presses
          switch (UI::event.key.keysym.sym) {
            case SDLK_ESCAPE: // pressed escape, quit
              return QUIT;
              break;
            case SDLK_s: // pressed s, take screenshot
              static int screenshotNumber;
              UI::takeScreenShot("screenshot" + std::to_string(screenshotNumber++) + ".png");
              break;
          }
        } else if (UI::event.type == SDL_FINGERDOWN) {
          SDL_Point touchLocation;
          touchLocation.x = UI::event.tfinger.x * SCREEN_WIDTH;
          touchLocation.y = UI::event.tfinger.y * SCREEN_HEIGHT;

        }
      }
      return SAVE_MENU;
    }

    void render () {
      // begin render, clear screen
      SDL_SetRenderDrawColor(UI::renderer, 109, 117, 141, 255);
      SDL_RenderClear(UI::renderer);

      // present render
      SDL_RenderPresent(UI::renderer);
    }
};

int main(int argc, char* args[]) {  
  // initialize UI
  if (UI::init()) {
    return 1;
  }
 
  states state = MAIN_MENU;

  MainMenu mainMenu;
  // create experiment editor
  ExperimentEditor editor;
  editor.logic(); // add to constructor to fix
 
  LoadMenu loadMenu(&editor);
  SaveMenu saveMenu(&editor);
 
  // load default experiment data
  editor.load("/home/pi/Untitled.exp");

  // program loop
  bool run = true;
  while (run) {
    switch (state) {
      case QUIT:
        run = false;
        break;
      case MAIN_MENU:
        mainMenu.render();
        state = mainMenu.logic();
        break;
      case EDITOR_MENU:
        editor.render();
        state = editor.logic();
        break;
      case LOAD_MENU:
        loadMenu.render();
        state = loadMenu.logic();
        break;
      case SAVE_MENU:
        saveMenu.render();
        state = saveMenu.logic();
        break;
      default:
        std::cout << "ERROR: Unknown state " << state << std::endl;
        run = false;
        break;
    }
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
