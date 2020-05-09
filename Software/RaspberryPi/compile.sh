g++ -g -o flcpcr main.cpp ui.cpp $(SDL/sdl2-config --cflags --libs) -lSDL2_image -lSDL2_ttf -std=c++17 -lstdc++fs -Wall
