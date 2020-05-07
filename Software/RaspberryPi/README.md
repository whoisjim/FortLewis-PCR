# Raspberry Pi
##  Todo
- [ ] fix load floating point error
- [ ] add estamated time to completion
- [ ] create save and load dialogue
- [ ] create main menu
- [ ] setup program auto launch
- [ ] add button amimations
- [ ] try to inprove microcontroller communication speed
## Installing
install SDL2 and dependencies
```
sudo apt-get install mercurial libfontconfig-dev qt5-default automake mercurial libtool libfreeimage-dev libopenal-dev libpango1.0-dev libsndfile-dev libudev-dev libtiff5-dev libwebp-dev libasound2-dev libaudio-dev libxrandr-dev libxcursor-dev libxi-dev libxinerama-dev libxss-dev libesd0-dev freeglut3-dev libmodplug-dev libsmpeg-dev libjpeg-dev
hg clone https://hg.libsdl.org/SDL
cd SDL
./autogen.sh
./configure --disable-pulseaudio --disable-esd --disable-video-mir --disable-video-wayland --disable-video-opengl --host=arm-raspberry-linux-gnueabihf --prefix=/usr
make
sudo make install
cd ..
sudo apt-get install libsdl2-image-dev
sudo apt-get install libsdl2-ttf-dev
```
Compile
```
g++ -o flcpcr main.cpp ui.cpp $(SDL/sdl2-config --cflags --libs) -lSDL2_image -lSDL2_ttf -Wall
```
Run
```
./flcpcr
```
