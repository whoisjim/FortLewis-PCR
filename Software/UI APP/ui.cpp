#include "ui.h"
#include <iostream>
#include <fstream>
#include <ctype.h>

namespace UI {
  SDL_Window* window = NULL;
  SDL_Renderer* renderer = NULL;
  SDL_Event event;

  std::vector<std::string> texturePaths;
  std::vector<SDL_Texture*> textures;

  std::vector<std::string> fontPaths;
  std::vector<TTF_Font*> fonts;
 
  int init() {
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
      fprintf(stderr, "could not initialize sdl2: %s\n", SDL_GetError());
      return 1;
    }
    window = SDL_CreateWindow("FLC:PCR", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (window == NULL) {
      fprintf(stderr, "could not create window: %s\n", SDL_GetError());
      return 1;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
      fprintf(stderr, "could not create renderer: %s\n", SDL_GetError());
      return 1;
    }
    if (TTF_Init() == -1) {
      fprintf(stderr, "could not initialize ttf: %s\n", SDL_GetError());
      return 1;
    }
    //SDL_ShowCursor(SDL_DISABLE);
    return 0;
  }

  bool takeScreenShot(std::string path) {
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, 800, 480, 32, SDL_PIXELFORMAT_RGB888);
    SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_RGB888, surface->pixels, surface->pitch);
    IMG_SavePNG(surface, path.c_str());
    SDL_FreeSurface(surface);
    return true;
  }

  Padding::Padding (const char* path, int border, int x, int y, int w, int h) {
    setTexture(path, border);
    x_ = x;
    y_ = y;
    w_ = w;
    h_ = h;
  }

  void quit() {
    for (unsigned int i = 0; i < textures.size(); i++) {
      SDL_DestroyTexture(textures[i]);
    }
    texturePaths.clear();
    textures.clear();

    for (unsigned int i = 0; i < fonts.size(); i++) {
      TTF_CloseFont(fonts[i]);
    }
    fontPaths.clear();
    fonts.clear();

    SDL_DestroyWindow(window);
    SDL_Quit();
  }

  void Padding::setTexture (const char* path, int border) {
    border_ = border;
    bool textureLoaded = false;
    for (unsigned int i = 0; i < texturePaths.size(); i++) {
      if (texturePaths[i] == path) {
        textureLoaded = true;
        textureID_ = i;
        break;
      }
    }
    if (!textureLoaded) {
      textureID_ = texturePaths.size();
      texturePaths.push_back(path);
      SDL_Surface* tempSurface = IMG_Load(path);
      textures.push_back(SDL_CreateTextureFromSurface(renderer, tempSurface));
      SDL_FreeSurface(tempSurface);
    }
  }

  void Padding::render () {
    SDL_Rect source;
    SDL_Rect destination;
    int imgW;
    int imgH;
    SDL_QueryTexture(textures[textureID_], NULL, NULL, &imgW, &imgH);

    source.x = 0;
    source.y = 0;
    source.w = border_;
    source.h = border_;
    destination.x = x_;
    destination.y = y_;
    destination.w = border_;
    destination.h = border_;
    SDL_RenderCopy(renderer, textures[textureID_], &source, &destination);

    source.x = border_;
    source.y = 0;
    source.w = imgW - 2 * border_;
    source.h = border_;
    destination.x = x_ + border_;
    destination.y = y_;
    destination.w = w_ - 2 * border_;
    destination.h = border_;
    SDL_RenderCopy(renderer, textures[textureID_], &source, &destination);

    source.x = imgW - border_;
    source.y = 0;
    source.w = border_;
    source.h = border_;
    destination.x = x_ + w_ - border_;
    destination.y = y_;
    destination.w = border_;
    destination.h = border_;
    SDL_RenderCopy(renderer, textures[textureID_], &source, &destination);

    source.x = 0;
    source.y = border_;
    source.w = border_;
    source.h = imgH - 2 * border_;
    destination.x = x_;
    destination.y = y_ + border_;
    destination.w = border_;
    destination.h = h_ - 2 * border_;
    SDL_RenderCopy(renderer, textures[textureID_], &source, &destination);

    source.x = border_;
    source.y = border_;
    source.w = imgW - 2 * border_;
    source.h = imgH - 2 * border_;
    destination.x = x_ + border_;
    destination.y = y_ + border_;
    destination.w = w_ - 2 * border_;
    destination.h = h_ - 2 * border_;
    SDL_RenderCopy(renderer, textures[textureID_], &source, &destination);

    source.x = imgW - border_;
    source.y = border_;
    source.w = border_;
    source.h = imgH - 2 * border_;
    destination.x = x_ + w_ - border_;
    destination.y = y_ + border_;
    destination.w = border_;
    destination.h = h_ - 2 * border_;
    SDL_RenderCopy(renderer, textures[textureID_], &source, &destination);
    
    source.x = 0;
    source.y = imgH - border_;
    source.w = border_;
    source.h = border_;
    destination.x = x_;
    destination.y = y_ + h_ - border_;
    destination.w = border_;
    destination.h = border_;
    SDL_RenderCopy(renderer, textures[textureID_], &source, &destination);

    source.x = border_;
    source.y = imgH - border_;
    source.w = imgW - 2 * border_;
    source.h = border_;
    destination.x = x_ + border_;
    destination.y = y_ + h_ - border_;
    destination.w = w_ - 2 * border_;
    destination.h = border_;
    SDL_RenderCopy(renderer, textures[textureID_], &source, &destination);

    source.x = imgW - border_;
    source.y = imgH - border_;
    source.w = border_;
    source.h = border_;
    destination.x = x_ + w_ - border_;
    destination.y = y_ + h_ - border_;
    destination.w = border_;
    destination.h = border_;
    SDL_RenderCopy(renderer, textures[textureID_], &source, &destination);
  }

  void Padding::setXY (int x, int y) {
    x_ = x;
    y_ = y;
  }

  void Padding::setWH (int w, int h) {
    w_ = w;
    h_ = h;
  }

  SDL_Rect Padding::getRect () {
    SDL_Rect rect;
    rect.x = x_;
    rect.y = y_;
    rect.w = w_;
    rect.h = h_;
    return rect;
  }

  Text::Text (const char* path, int size, int x, int y, std::string text, int horizontalJustification, int verticalJustification) {
    bool fontLoaded = false;
    for (unsigned int i = 0; i < fontPaths.size(); i++) {
      if (fontPaths[i] == path + std::to_string(size)) {
        fontLoaded = true;
        fontID_ = i;
        break;
      }
    }
    if (!fontLoaded) {
      fontID_ = fontPaths.size();
      fontPaths.push_back(path + std::to_string(size));
      fonts.push_back(TTF_OpenFont(path, size));
    }

    horizontalJustification_ = horizontalJustification;
    verticalJustification_ = verticalJustification; 
    x_ = x;
    y_ = y;
    texture_ = NULL;
    setText(text);
  }

  Text::Text (const Text& copy) {
    fontID_ = copy.fontID_;
    x_ = copy.x_;
    y_ = copy.y_;
    texture_ = NULL;
    horizontalJustification_ = copy.horizontalJustification_;
    verticalJustification_ = copy.verticalJustification_;
    setText(copy.text_);
  }

  void Text::setText (std::string text) {
    text_ = text;
    SDL_Surface* tempSurface = NULL; 
    if (text.size() > 0) {
      tempSurface = TTF_RenderText_Blended(fonts[fontID_], text_.c_str(), {6, 6, 8, 255});
    }
    if (texture_ != NULL) {
      SDL_DestroyTexture(texture_);
    }
    if (tempSurface != NULL) {
      texture_ = SDL_CreateTextureFromSurface(renderer , tempSurface);
      SDL_FreeSurface(tempSurface);
    } else {
      texture_ = NULL;
    }
  }

  std::string Text::getText () {
    return text_;
  }

  void Text::render () {
    if (texture_ != NULL) {
      SDL_Rect destination;
      int imgW;
      int imgH;
      SDL_QueryTexture(texture_, NULL, NULL, &imgW, &imgH);
      if (horizontalJustification_ < 0) {
        destination.x = x_ - imgW;
      } else if (horizontalJustification_ == 0) {
        destination.x = x_ - imgW / 2;
      } else {
        destination.x = x_;
      }
      if (verticalJustification_ < 0) {
        destination.y = y_ - imgH;
      } else if (verticalJustification_ == 0) {
        destination.y = y_ - imgH / 2;
      } else {
        destination.y = y_;
      }
      destination.w = imgW;
      destination.h = imgH;
      SDL_RenderCopy(renderer, texture_, NULL, &destination);
    }
  }
  
  void Text::setXY (int x, int y) {
    x_ = x;
    y_ = y;
  }

  void Text::formatNumber () {
    if (text_.size() == 0) {
      setText("0");
    }
    try {
      if (std::stof(text_) == 0) {
        setText("0");
      } 
    } catch (...) {
      setText("0");
    }
    bool hasDecimal = false;
    for (unsigned int i = 0; i < text_.size(); i++) {
      if (text_[i] == '.') {
        hasDecimal = true;
        break;
      }
    }
    if (hasDecimal) {
      for (unsigned int i = text_.size() - 1; i >= 0; i--) {
        if (text_[i] == '.') {
          text_.pop_back();
          break;
        } else if (text_[i] != '0') {
          break;
        } 
        text_.pop_back();
      }
      setText(text_);
    }
  }

  Text::~Text () {
    SDL_DestroyTexture(texture_);
  }

  Image::Image (const char* path, int x, int y) {
    bool textureLoaded = false;
    for (unsigned int i = 0; i < texturePaths.size(); i++) {
      if (texturePaths[i] == path) {
        textureLoaded = true;
        textureID_ = i;
        break;
      }
    }
    if (!textureLoaded) {
      textureID_ = texturePaths.size();
      texturePaths.push_back(path);
      SDL_Surface* tempSurface = IMG_Load(path);
      textures.push_back(SDL_CreateTextureFromSurface(renderer , tempSurface));
      SDL_FreeSurface(tempSurface);
    }
    x_ = x;
    y_ = y;
  }

  void Image::setTexture (const char* path) {
    bool textureLoaded = false;
    for (unsigned int i = 0; i < texturePaths.size(); i++) {
      if (texturePaths[i] == path) {
        textureLoaded = true;
        textureID_ = i;
        break;
      }
    }
    if (!textureLoaded) {
      textureID_ = texturePaths.size();
      texturePaths.push_back(path);
      SDL_Surface* tempSurface = IMG_Load(path);
      textures.push_back(SDL_CreateTextureFromSurface(renderer, tempSurface));
      SDL_FreeSurface(tempSurface);
    }
  }

  void Image::render () {
    SDL_Rect destination;
    int imgW;
    int imgH;
    SDL_QueryTexture(textures[textureID_], NULL, NULL, &imgW, &imgH);
    
    destination.x = x_;
    destination.y = y_;
    destination.w = imgW;
    destination.h = imgH;
    SDL_RenderCopy(renderer, textures[textureID_], NULL, &destination);
  }

  void Image::setXY (int x, int y) {
    x_ = x;
    y_ = y;
  }
 
  TextBox::TextBox (int x, int y, int w, int h, std::string text) :
  padding_("img/padding/S_Grey_3.png", 2, x, y, w, h),
  text_("fonts/Inconsolata-Medium.ttf", h, x + 2, y, text) {
    x_ = x;
    y_ = y;
    w_ = w;
    h_ = h;
  }

  void TextBox::setText (std::string text) {
    text_.setText(text);
  }

  std::string TextBox::getText () {
    return text_.getText();
  }

  void TextBox::render () {
    padding_.render();
    text_.render();
  }

  void TextBox::select () {
    padding_.setTexture("img/padding/S_Blue.png", 2);
  }

  void TextBox::deselect () {
    padding_.setTexture("img/padding/S_Grey_3.png", 2);
  }

  void TextBox::setXY (int x, int y) {
    x_ = x;
    y_ = y;
    padding_.setXY(x, y);
    text_.setXY(x + 2, y);
  }

  SDL_Rect TextBox::getRect () {
    SDL_Rect rect;
    rect.x = x_;
    rect.y = y_;
    rect.w = w_;
    rect.h = h_;
    return rect;
  }

  void TextBox::formatNumber () {
    text_.formatNumber();
  }

  CycleStep::CycleStep (int x, int y) :
  padding_("img/padding/R_Grey_1.png", 5, x, y, 100, 47),
  temperatureImage_("img/Thermometer.png", x + 5, y + 5),
  durationImage_("img/Clock.png", x + 5, y + 26),
  temperature_(x + 26, y + 5, 52, 16, "0"),
  duration_(x + 26, y + 26, 52, 16, "0"),
  c_("fonts/Inconsolata-Medium.ttf", 16, x + 79, y + 5, "\xB0" + std::string(1, 'C')),
  s_("fonts/Inconsolata-Medium.ttf", 16, x + 84, y + 26, "S") {
    x_ = x;
    y_ = y;
  }

  void CycleStep::render () {
    padding_.render();
    temperatureImage_.render();
    durationImage_.render();
    temperature_.render();
    duration_.render();
    c_.render();
    s_.render();
  }

  void CycleStep::setXY (int x, int y) {
    padding_.setXY(x, y);
    temperatureImage_.setXY(x + 5, y + 5);
    durationImage_.setXY(x + 5, y + 26);
    temperature_.setXY(x + 26, y + 5);
    duration_.setXY(x + 26, y + 26);
    c_.setXY(x + 79, y + 5);
    s_.setXY(x + 84, y + 26);
    x_ = x;
    y_ = y;
  }

  SDL_Rect CycleStep::getRect () {
    SDL_Rect rect;
    rect.x = x_;
    rect.y = y_;
    rect.w = 100;
    rect.h = 47;
    return rect;
  }

  TextBox* CycleStep::getTemperature () {
    return &temperature_;
  }

  TextBox* CycleStep::getDuration () {
    return &duration_;
  }

  Cycle::Cycle (int x, int y) :
  padding_("img/padding/R_Grey_2.png", 5, x, y, 100, 26),
  cycleImage_("img/Cycle.png", x + 5, y + 5),
  numberOfCycles_(x + 26, y + 5, 69, 16, "0") {
    x_ = x;
    y_ = y;
  }

  void Cycle::render () {
    padding_.render();
    cycleImage_.render();
    numberOfCycles_.render();
    for(unsigned int i = 0; i < steps_.size(); i++) {
      steps_[i]->render();
    }
  }

  void Cycle::setXY (int x, int y) {
    x_ = x;
    y_ = y;
    padding_.setXY(x, y);
    cycleImage_.setXY(x + 5, y + 5);
    numberOfCycles_.setXY(x + 26, y + 5);
    for(unsigned int i = 0; i < steps_.size(); i++) {
      steps_[i]->setXY(x + 5, y + 25 + i * 52);
    }
  }
  
  void Cycle::addStep (int index, CycleStep* step) {
    steps_.insert(steps_.begin() + index, step);
    padding_.setWH(100, 26 + steps_.size() * 52);
    for(unsigned int i = 0; i < steps_.size(); i++) {
      steps_[i]->setXY(x_ + 5, y_ + 25 + i * 52);
    }
  }

  CycleStep* Cycle::removeStep (int index) {
    CycleStep* step = steps_[index];
    steps_.erase(steps_.begin() + index);
    padding_.setWH(100, 26 + steps_.size() * 52);
    for(unsigned int i = 0; i < steps_.size(); i++) {
      steps_[i]->setXY(x_ + 5, y_ + 25 + i * 52);
    }
    return step;
  }

  CycleStep* Cycle::getStep (int index) {
    return steps_[index];
  }

  SDL_Rect Cycle::getRect () {
    return padding_.getRect();
  }

  TextBox* Cycle::getNumberOfCycles () {
    return &numberOfCycles_;
  }

  Cycle::~Cycle () {
    for(auto i : steps_) {
      delete i;
    }
    steps_.clear();
  }

  CycleArray::CycleArray (int x, int y) {
    x_ = x;
    y_ = y;
  }

  void CycleArray::render () {
    for(unsigned int i = 0; i < cycles_.size(); i++) {
      cycles_[i]->render();
    }
  }

  void CycleArray::setXY (int x, int y) {
    x_ = x;
    y_ = y;
    for(unsigned int i = 0; i < cycles_.size(); i++) {
      cycles_[i]->setXY(x_ + i * 110, y_);
    }
  }

  void CycleArray::addCycle (int index, Cycle* cycle) {
    cycles_.insert(cycles_.begin() + index, cycle);
    for(unsigned int i = 0; i < cycles_.size(); i++) {
      cycles_[i]->setXY(x_ + i * 110, y_);
    }
  }

  Cycle* CycleArray::removeCycle (int index) {
    Cycle* cycle = cycles_[index];
    cycles_.erase(cycles_.begin() + index);
    for(unsigned int i = 0; i < cycles_.size(); i++) {
      cycles_[i]->setXY(x_ + i * 110, y_);
    }
    return cycle;
  }

  SDL_Point CycleArray::getPoint () {
    SDL_Point temp = {x_, y_};
    return temp;
  }

  void CycleArray::removeEmptyCycles () {
    if (cycles_.size() != 0) {
      for(unsigned int i = cycles_.size(); i > 0; i--) {
        if (cycles_[i - 1]->steps_.size() == 0) {
          Cycle* deleteMe = removeCycle(i - 1);
          delete deleteMe;
        }
      }
    }
  }
  
  CycleStep* CycleArray::getStep (int index) {
    int size = 0;
    for (unsigned int i = 0; i < cycles_.size(); i++) {
    size += cycles_[i]->steps_.size() * std::stoi(cycles_[i]->getNumberOfCycles()->getText());
      if (index < size) {
        size -= cycles_[i]->steps_.size() * std::stoi(cycles_[i]->getNumberOfCycles()->getText());
        index -= size;
        return cycles_[i]->getStep(index % (cycles_[i]->steps_.size() ));
      }
          
    }
    throw "cycle array index out of range";
  }

  int CycleArray::size () {
    int size = 0;
    for (unsigned int i = 0; i < cycles_.size(); i++) {
      size += cycles_[i]->steps_.size() * std::stoi(cycles_[i]->getNumberOfCycles()->getText());
    }
    return size;
  }

  void CycleArray::load (std::string path) {
    clear();
    std::ifstream inFile;
    inFile.open(path);
    int i = -1;
    int j = 0;
    while (!inFile.eof()) {
      if (inFile.peek() == '\n') {
        inFile.get();
      } else if (inFile.peek() == '\t') { 
        inFile.get();
        float temperature;
        float duration;
        inFile >> temperature;
        inFile >> duration;
        cycles_[i]->addStep(j, new CycleStep());
        cycles_[i]->steps_[j]->getTemperature()->setText(std::to_string(temperature));
        cycles_[i]->steps_[j]->getDuration()->setText(std::to_string(duration));
        cycles_[i]->steps_[j]->getTemperature()->formatNumber();
        cycles_[i]->steps_[j]->getDuration()->formatNumber();
        j++;
      } else {
        i++;
        j = 0;
        float cycles;
        inFile >> cycles;
        addCycle(i, new Cycle()); 
        cycles_[i]->getNumberOfCycles()->setText(std::to_string(cycles)); 
        cycles_[i]->getNumberOfCycles()->formatNumber();
      }
    }
    inFile.close();
  }

  void CycleArray::save (std::string path) {
    std::ofstream outFile;
    outFile.open(path);
    for(unsigned int i = 0; i < cycles_.size(); i++) {
      outFile << cycles_[i]->getNumberOfCycles()->getText() << "\n";
      for(unsigned int j = 0; j < cycles_[i]->steps_.size(); j++) {
        outFile << "\t" << cycles_[i]->steps_[j]->getTemperature()->getText() << " " << cycles_[i]->steps_[j]->getDuration()->getText() << "\n";
      }
    }

    outFile.close();
  }

  void CycleArray::clear () {
    for(auto i : cycles_) {
      delete i;
    }
    cycles_.clear();
  }

  float CycleArray::getRemainingTime (int index) {
    const float DEGREES_PER_SECCOND = 1.11;
    float time = 0;
    if (index >= size()) {
      return 0;
    }
    for (int i = index; i < size() - 1; i++) {
      time += std::stof(getStep(i)->getDuration()->getText());
      time += abs(std::stof(getStep(i)->getTemperature()->getText()) - std::stof(getStep(i + 1)->getTemperature()->getText()) ) / DEGREES_PER_SECCOND; 
    }
    time += std::stof(getStep(size() - 1)->getDuration()->getText());
    return time;
  }

  CycleArray::~CycleArray () {
    clear();
  }

  Key::Key (int x, int y, int w, int h, char ch, std::string text):
  padding_("img/padding/R_Grey_1.png", 5, x, y, w, h),
  text_("fonts/Inconsolata-Medium.ttf", h - 10, x + w / 2, y + h / 2, text, 0, 0) {
    x_ = x;
    y_ = y;
    ch_ = ch;
  }

  Key::Key (int x, int y, int w, int h, char ch, std::string text, int fontSize):
  padding_("img/padding/R_Grey_1.png", 5, x, y, w, h),
  text_("fonts/Inconsolata-Medium.ttf", fontSize, x + w / 2, y + h / 2, text, 0, 0) {
    x_ = x;
    y_ = y;
    ch_ = ch;
  }

  void Key::render () {
    padding_.render();   
    text_.render();
  }

  void Key::press (TextBox* target, bool caps) {
    if (ch_ == '\b') {
      std::string text = target->getText();
      if (text.size() > 0) {
        target->setText(text.substr(0, text.size() - 1));
      }
    } else {
      if (caps) {
        target->setText(target->getText() + (char)toupper(ch_));
      } else {
        target->setText(target->getText() + ch_);
      }
    }
  }

  SDL_Rect Key::getRect () {
    return padding_.getRect();
  }

  void Key::setText(std::string text) {
    text_.setText(text);
  }

  void Key::setCh(char ch) {
    ch_ = ch;
  }

  void Key::setXY (int x, int y) {
    SDL_Rect rect = padding_.getRect();
    x_ = x;
    y_ = y;
    text_.setXY(x + rect.w / 2, y + rect.h / 2);
    padding_.setXY(x, y);
  }

  NumberKey::NumberKey (int x, int y, int w, int h, char ch, std::string text):
  padding_("img/padding/R_Grey_1.png", 5, x, y, w, h),
  text_("fonts/Inconsolata-Medium.ttf", h - 10, x + w / 2, y + h / 2, text, 0, 0) {
    x_ = x;
    y_ = y;
    ch_ = ch;
  }

  NumberKey::NumberKey (int x, int y, int w, int h, char ch, std::string text, int fontSize):
  padding_("img/padding/R_Grey_1.png", 5, x, y, w, h),
  text_("fonts/Inconsolata-Medium.ttf", fontSize, x + w / 2, y + h / 2, text, 0, 0) {
    x_ = x;
    y_ = y;
    ch_ = ch;
  }

  void NumberKey::render () {
    padding_.render();   
    text_.render();
  }

  void NumberKey::press (TextBox* target) {
    std::string text = target->getText();
    if (ch_ == '\b') {
      if (text.size() > 0) {
        target->setText(text.substr(0, text.size() - 1));
      }
    } else if (text.size() < 5) {
      if (ch_ == '.') {
        bool hasDecimal = false;
        for (unsigned int i = 0; i < text.size(); i++) {
          if (text[i] == '.') {
            hasDecimal = true;
            break;
          }
        }
        if (!hasDecimal) {
          if (text == "0") {
            target->setText(".");
          } else {
            target->setText(text + '.');
          }
        }
      } else {
        if (text == "0") {
          target->setText(std::string(1, ch_));
        } else {
          target->setText(text + ch_);
        }
      }
    }
  }

  SDL_Rect NumberKey::getRect () {
    return padding_.getRect();
  }

  void NumberKey::setText(std::string text) {
    text_.setText(text);
  }

  void NumberKey::setCh(char ch) {
    ch_ = ch;
  }

  void NumberKey::setXY (int x, int y) { 
    SDL_Rect rect = padding_.getRect();
    x_ = x;
    y_ = y;
    text_.setXY(x + rect.w / 2, y + rect.h / 2);
    padding_.setXY(x, y);
  }

  Button::Button (int x, int y, int w, int h, std::string text):
  padding_("img/padding/R_Grey_1.png", 5, x, y, w, h),
  text_("fonts/Inconsolata-Medium.ttf", h - 10, x + w / 2, y + h / 2, text, 0, 0) {
    x_ = x;
    y_ = y;
  }

  Button::Button (int x, int y, int w, int h, std::string text, int fontSize):
  padding_("img/padding/R_Grey_1.png", 5, x, y, w, h),
  text_("fonts/Inconsolata-Medium.ttf", fontSize, x + w / 2, y + h / 2, text, 0, 0) {
    x_ = x;
    y_ = y;
  }

  void Button::render () {
    padding_.render();   
    text_.render();
  }

  void Button::press () {
    
  }

  void Button::setText(std::string text) {
    text_.setText(text);
  }

  SDL_Rect Button::getRect () {
    return padding_.getRect();
  }

  void Button::setXY (int x, int y) {
    SDL_Rect rect = padding_.getRect();
    x_ = x;
    y_ = y;
    text_.setXY(x + rect.w / 2, y + rect.h / 2);
    padding_.setXY(x, y);
  }
}
