#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 480

namespace UI {
  SDL_Window* window = NULL;
  SDL_Renderer* renderer = NULL;
  SDL_Surface* screenSurface = NULL;
  SDL_Event event;

  std::vector<std::string> paths;
  std::vector<SDL_Texture*> textures;

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
    return 0;
  }

  class Padding {
    public:
      int x_, y_, w_, h_;
      
      Padding (const char* path, int border, int w, int h, int x = 0, int y = 0) {
        bool textureLoaded = false;
        for (int i = 0; i < paths.size(); i++) {
          if (paths[i] == path) {
            textureLoaded = true;
            textureID_ = i;
            break;
          }
        }
        if (!textureLoaded) {
          textureID_ = paths.size();
          paths.push_back(path);
          SDL_Surface* tempSurface = IMG_Load(path);
          textures.push_back(SDL_CreateTextureFromSurface(renderer , tempSurface));
          SDL_FreeSurface(tempSurface);
        }

        x_ = x;
        y_ = y;
        w_ = w;
        h_ = h;
        border_ = border;
      }
      void render () {
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
      std::string getPath() {
        return paths[textureID_];
      }
    private:
      int textureID_;
      int border_;

  };
}
