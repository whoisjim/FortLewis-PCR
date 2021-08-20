# FLC-PCR UI APP

This application handles user interaction, experiment management, cycle timing and communicating with `PCRControl.ino` to perform PCR. Currently only compatible with linux. It was designed to run on Raspian Lite but should be compatible with other Linux distros and desktop environments.

## Raspberry Pi Version Setup

### 1. Install Rasbian Lite

[You can download a Rasbian image and formatting software here](https://www.raspberrypi.org/software/). Write the image to a SD card for the Raspberry Pi. Make sure your a using the light version of the OS. This version does not include a desktop environment. Once you have flashed the OS image to a sd card. start it up and connect it to the internet.

In addition normal setup you may want to change the keybord layout to that of the one you have. Raspberry Pi's default to a UK layout.

### 2. Download This Repository

Install git and clone the repository with the following commands

```sh
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install git
git clone https://github.com/whoisjim/FortLewis-PCR
```

### 3. Install Dependencies For The Command Line

In order to run SDL2 (the rendering library) without a desktop environment a specially compiled version must be used. To do so run the following commands.

```sh
sudo apt-get update
sudo apt-get upgrade
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

### 4. Compile With g++ 8.3.0 For The Command Line

The following command will compile the FLC-PCR UI app. You may change `SDL/sdl2-config` to a different path depending on where you installed SDL2.

```sh
g++ -o flcpcr main.cpp ui.cpp $(SDL/sdl2-config --cflags --libs) -lSDL2_image -lSDL2_ttf -std=c++17 -lstdc++fs -Wall
```

### 5. Running the FLC-PCR UI App

Running the following command will run the FLC-PCR UI app after you have compiled it. Pressing escape will close the program.

```sh
./flcpcr
```

When the system is finished this program should autorun on startup. The FLC-PCR UI app expects the ATmega328p to be mounted at `/dev/ttyUSB0` in order to communicate with `PCRControl.ino`. This is defined in `Main.cpp` but should get moved to a config file in the future.

## Desktop Environment Version Setup

If you want to run this with a desktop environment, the following commands install SDL2 and compile the FLC-PCR UI app. All other steps should be the same.

### Install Dependencies For X11

```sh
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install libsdl2-dev
sudo apt-get install libsdl2-image-dev
sudo apt-get install libsdl2-ttf-dev
```

### Compile With g++ 8.3.0 For X11

```sh
g++ -o flcpcr main.cpp ui.cpp -lSDL2 -lSDL2_image -lSDL2_ttf -std=c++17 -lstdc++fs -Wall
```
