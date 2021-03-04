g++ -g -o flcpcr main.cpp ui.cpp serial.cpp $(SDL/sdl2-config --cflags --libs) -lSDL2_image -lSDL2_ttf -pthread -std=c++17 -lstdc++fs -Wall
