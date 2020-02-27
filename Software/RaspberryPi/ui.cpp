#include "ui.h"

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
    window = SDL_CreateWindow("FLC:PCR", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_FULLSCREEN);
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
    SDL_ShowCursor(SDL_DISABLE);
    return 0;
  }

  Padding::Padding (const char* path, int border, int x, int y, int w, int h) {
    setTexture(path, border);
    x_ = x;
    y_ = y;
    w_ = w;
    h_ = h;
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

  Text::Text (const char* path, int size, int x, int y, std::string text) {
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
    x_ = x;
    y_ = y;
    setText(text);
  }

  void Text::setText (std::string text) {
    text_ = text;
    SDL_Surface* tempSurface = TTF_RenderText_Blended(fonts[fontID_], text_.c_str(), {24, 20, 37, 255});
    texture_ = SDL_CreateTextureFromSurface(renderer , tempSurface);
    SDL_FreeSurface(tempSurface);
  }

  std::string Text::getText () {
    return text_;
  }

  void Text::render () {
    SDL_Rect destination;
    int imgW;
    int imgH;
    SDL_QueryTexture(texture_, NULL, NULL, &imgW, &imgH);
    
    destination.x = x_;
    destination.y = y_;
    destination.w = imgW;
    destination.h = imgH;
    SDL_RenderCopy(renderer, texture_, NULL, &destination);
  }
  
  void Text::setXY (int x, int y) {
    x_ = x;
    y_ = y;
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
  padding_("img/sharp_padding.png", 6, x, y, w, h),
  text_("fonts/consola.ttf", h, x + 6, y + 2, text) {
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
    padding_.setTexture("img/sharp_selection_padding.png", 6);
  }

  void TextBox::deselect () {
    padding_.setTexture("img/sharp_padding.png", 6);
  }

  void TextBox::setXY (int x, int y) {
    x_ = x;
    y_ = y;
    padding_.setXY(x, y);
    text_.setXY(x + 6, y + 2);
  }

  SDL_Rect TextBox::getRect () {
    SDL_Rect rect;
    rect.x = x_;
    rect.y = y_;
    rect.w = w_;
    rect.h = h_;
    return rect;
  }

  CycleStep::CycleStep (int x, int y) :
  padding_("img/default_padding.png", 6, x, y, 100, 47),
  temperatureImage_("img/thermometer.png", x + 5, y + 5),
  durationImage_("img/clock.png", x + 5, y + 26),
  temperature_(x + 26, y + 5, 69, 16, "0"),
  duration_(x + 26, y + 26, 69, 16, "0") {
    x_ = x;
    y_ = y;
  }

  void CycleStep::render () {
    padding_.render();
    temperatureImage_.render();
    durationImage_.render();
    temperature_.render();
    duration_.render();
  }

  void CycleStep::setXY (int x, int y) {
    padding_.setXY(x, y);
    temperatureImage_.setXY(x + 5, y + 5);
    durationImage_.setXY(x + 5, y + 26);
    temperature_.setXY(x + 26, y + 5);
    duration_.setXY(x + 26, y + 26);
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
  padding_("img/grey_padding.png", 6, x, y, 100, 26),
  cycleImage_("img/cycle.png", x + 5, y + 5),
  numberOfCycles_(x + 26, y + 5, 69, 16, "1") {
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

  SDL_Rect Cycle::getRect () {
    return padding_.getRect();
  }

  TextBox* Cycle::getNumberOfCycles () {
    return &numberOfCycles_;
  }

  Cycle::~Cycle () {
    for(unsigned int i = 0; i < steps_.size(); i++) {
      delete steps_[i];
    }
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

  CycleArray::~CycleArray () {
    for(unsigned int i = 0; i < cycles_.size(); i++) {
      delete cycles_[i];
    }
  }
}
