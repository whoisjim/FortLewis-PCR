sudo apt-get update
sudo apt-get upgrade
# Installs SDL2 in the current directory
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

# wget http://www.libsdl.org/projects/SDL_image/release/SDL2_image-2.0.2.tar.gz
# wget http://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-2.0.14.tar.gz
# tar zxvf SDL2_image-2.0.2.tar.gz
# tar zxvf SDL2_ttf-2.0.14.tar.gz
# cd SDL2_image-2.0.2 
# ./autogen.sh 
# ./configure --prefix=/usr
# make 
# sudo make install
# cd ..
# cd SDL2_ttf-2.0.14
# ./autogen.sh
# ./configure --prefix=/usr
# make
# sudo make install
# cd ..
