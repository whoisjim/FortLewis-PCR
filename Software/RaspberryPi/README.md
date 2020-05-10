# Raspberry Pi
## Todo
- [X] fix load floating point problem
- [X] add floating pont support to num key
- [X] add estamated time to completion
- [X] create save and load dialogue
- [X] create main menu
- [X] add shift functionality to save keybord
- [X] add are you sure dialog
- [X] add save reminder
- [ ] add folders to save / load
- [ ] add external directories to save / load
- [ ] polish save and load menu
- [ ] polish save keybord
- [ ] polish main menu
- [ ] add error checking
- [ ] setup program auto launch
- [ ] add button amimations
- [ ] try to inprove microcontroller communication speed
- [ ] fix memory ishues
## Installing for Raspberry Pi B Vi.2 Raspbian Buster Lite February 2020
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
Compile with g++ 8.3.0
```
g++ -o flcpcr main.cpp ui.cpp $(SDL/sdl2-config --cflags --libs) -lSDL2_image -lSDL2_ttf -std=c++17 -lstdc++fs -Wall
```
Run
```
./flcpcr
```
