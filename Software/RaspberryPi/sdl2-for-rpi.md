From https://gist.github.com/Lokathor/e6fec720b722b8a6f78e399698cae6e4

Here is an explanation of how to install a version of SDL2 that will let you run graphical programs straight from the console on a raspberry pi (including hardware acceleration). I got this from [a blog post on choccyhobnob](https://choccyhobnob.com/sdl2-2-0-8-on-raspberry-pi/).

As a reminder, the RPI "hardware acceleration" is OpenGL ES 2. Some sites talk about an "experimental" OpenGL driver that you can install. That driver is for desktop mode only (X11). You do not need it to run SDL2 out of the console. In fact if you have it configured for use then it will _hinder_ console based graphical programs from being able to use hardware acceleration (Retropie won't even start up, for example, until you disable the experimental driver and reboot).

Clear the default sdl2 installation (which is compiled for desktop mode):

```sh
sudo apt-get remove -y --force-yes libsdl2-dev
sudo apt-get autoremove -y
# There's also a version of 2.0.8 which you should archive
sudo mv /usr/lib/arm-linux-gnueabihf/libSDL2-2.0.so.0 /usr/lib/arm-linux-gnueabihf/libSDL2-2.0.so.0.backup
```

Then we use `apt-get` to pull in all the ~~junk~~ dependencies we'll need

```sh
sudo apt-get install libfontconfig-dev qt5-default automake mercurial libtool libfreeimage-dev \
  libopenal-dev libpango1.0-dev libsndfile-dev libudev-dev libtiff5-dev libwebp-dev libasound2-dev \
  libaudio-dev libxrandr-dev libxcursor-dev libxi-dev libxinerama-dev libxss-dev libesd0-dev \
  freeglut3-dev libmodplug-dev libsmpeg-dev libjpeg-dev
```

Go to the directory where you want to download these soruce files to.

First we'll install sdl2 itself

```sh
hg clone https://hg.libsdl.org/SDL
cd SDL
./autogen.sh
./configure --disable-pulseaudio --disable-esd --disable-video-mir \
  --disable-video-wayland --disable-video-opengl --host=arm-raspberry-linux-gnueabihf --prefix=/usr
make
sudo make install
```

You might get some warnings about unknown options, that's fine, it doesn't block the build or install.

Now we _download and unpack_ all the add-on libs.

(We won't use `SDL2_net` because Rust already has networking in `std`)

```sh
cd ..
wget http://www.libsdl.org/projects/SDL_image/release/SDL2_image-2.0.2.tar.gz
wget http://www.libsdl.org/projects/SDL_mixer/release/SDL2_mixer-2.0.2.tar.gz
wget http://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-2.0.14.tar.gz

tar zxvf SDL2_image-2.0.2.tar.gz
tar zxvf SDL2_mixer-2.0.2.tar.gz
tar zxvf SDL2_ttf-2.0.14.tar.gz
```

Then we _build and install_ all the add-on libs

```sh
cd SDL2_image-2.0.2 
./autogen.sh 
./configure --prefix=/usr
make 
sudo make install
cd ..

cd SDL2_mixer-2.0.2 
./autogen.sh 
./configure --prefix=/usr
make 
sudo make install
cd ..

cd SDL2_ttf-2.0.14
./autogen.sh
./configure --prefix=/usr
make
sudo make install
cd ..
```

To test that things are correct we should of course build and run a "hello world". I found [this one](https://gist.github.com/fschr/92958222e35a823e738bb181fe045274) which worked for me.

`sdl2_hello.cpp`:
```cpp
#include <SDL2/SDL.h>
#include <stdio.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

int main(int argc, char* args[]) {
  SDL_Window* window = NULL;
  SDL_Surface* screenSurface = NULL;
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    fprintf(stderr, "could not initialize sdl2: %s\n", SDL_GetError());
    return 1;
  }
  window = SDL_CreateWindow(
                            "hello_sdl2",
                            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                            SCREEN_WIDTH, SCREEN_HEIGHT,
                            SDL_WINDOW_SHOWN
                            );
  if (window == NULL) {
    fprintf(stderr, "could not create window: %s\n", SDL_GetError());
    return 1;
  }
  screenSurface = SDL_GetWindowSurface(window);
  SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));
  SDL_UpdateWindowSurface(window);
  SDL_Delay(2000);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
```

Once that's set you use the `sdl2-config` program to get the right compiler flags:

```sh
g++ -o sdl2_hello sdl2_hello.c `sdl2-config --cflags --libs`
```
