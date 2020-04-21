// UI
// contains functions and classes for initializing SDL2
// and rendering UI elements for FLC:PCR

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 480

namespace UI {  
  extern SDL_Window* window; // sdl2 window created by init()
  extern SDL_Renderer* renderer; // sdl2 renderer created by init()
  extern SDL_Event event; // sdl2 event for handeling user input 

  extern std::vector<std::string> texturePaths; // contains the path of each loaded texture
  extern std::vector<SDL_Texture*> textures; // contains each loaded texture

  extern std::vector<std::string> fontPaths; // contans the path and fontsize of each loaded font
  extern std::vector<TTF_Font*> fonts; // contains each loaded font

  int init(); // initializes sdls
  bool takeScreenShot(std::string path);
  
  // for drawing a rectangle based on image file.
  // streaches center and edges while leaving corners at the initial aspect ratio.
  class Padding {
    public:
      // path : image file path
      // border : the distance in pixels from the edge of the image to not streach
      // x : upper left of rectangle's screen x position
      // y : upper left of rectangle's screen y position
      // w : width of rectangle
      // h : height of rectangle
      Padding (const char* path, int border, int x, int y, int w, int h);
      void setTexture (const char* path, int border); // change texture
      void render (); // draw rectangle to screen
      void setXY (int x, int y); // move rectange
      void setWH (int w, int h); // resize rectangle
      SDL_Rect getRect (); // get SDL_Rect based on location and size
    private:
      int textureID_;
      int border_;
      int x_, y_, w_, h_;
  };

  // for rendering text
  class Text {
    public:
      // path : font file path must be .ttf
      // size : size of font in  pixels
      // x : upper left of text's screen x position
      // y : upper left of text's screen y position
      // text : the string of charicters to be rendered
      Text (const char* path, int size, int x, int y, std::string text = "");
      void setText (std::string text); // sets a new string
      std::string getText (); // get the curent string 
      void render (); // renders text
      void setXY (int x, int y); // move text
      ~Text();
    private:
      std::string text_;
      SDL_Texture* texture_;
      int fontID_;
      int x_, y_;
  };

  // for rendering an unscaled image
  class Image {
    public:
      // path : image file path
      // x : upper left of image's screen x position
      // y : upper left of image's screen y position 
      Image(const char* path, int x, int y);
      void setTexture (const char* path); // change texture
      void render (); // renders the image
      void setXY (int x, int y); // sets the image location
    private:
      int x_, y_;
      int textureID_;
  };
  
  // for rendering text with a padding with visual selection identifier 
  class TextBox {
    public:
      // x : upper left of text box's screen x position
      // y : upper left of text box's screen y position
      // w : width of text box
      // h : height of text box 
      // text : the string of charicters to be rendered
      TextBox (int x, int y, int w, int h, std::string text = "");
      void setText (std::string text); // sets a new string
      std::string getText (); // get the curent string
      void render (); // renders the text box
      void select (); // sets the visual selection indicator to selected
      void deselect (); // returns the visual selection indicator to normal
      void setXY (int x, int y); // sets the text box location
      SDL_Rect getRect (); // get SDL_Rect based on location and size
    private:
      Padding padding_;
      Text text_;
      int x_, y_, w_, h_;
  };

  // for visually representing a single temperature step wuthin a thermal cycle
  class CycleStep {
    public:
      // x : upper left of CycleStep's screen x position
      // y : upper left of CycleStep's screen y position
      CycleStep (int x = 0, int y = 0);
      void render (); // renders the CycleStep
      void setXY (int x, int y); // sets the CycleStep location
      SDL_Rect getRect (); // get SDL_Rect based on location and size
      TextBox* getTemperature (); // gets the TextBox associated with the step's temperature
      TextBox* getDuration (); //  gets the TextBox associated with the step's duration
    private:
      int x_, y_;
      Padding padding_;
      Image temperatureImage_;
      Image durationImage_;
      TextBox temperature_;
      TextBox duration_;
  };
  
  // for visually representing a thermal cycle that can be repeated
  // is made up of CycleSteps
  class Cycle {
    public:
      // x : upper left of CycleStep's screen x position
      // y : upper left of CycleStep's screen y position
      // index : the index to add or remove CycleStep at
      // step : CycleStep to add to this Cycle
      Cycle (int x = 0, int y = 0);
      void render (); // renders the Cycle
      void setXY (int x, int y); // sets the Cycle location
      void addStep (int index, CycleStep* step); // adds a CycleStep to this Cycle
      CycleStep* getStep (int index);
      CycleStep* removeStep (int index); // removes and returns a CycleStep from this Cycle
      SDL_Rect getRect (); // get SDL_Rect based on location and size
      TextBox* getNumberOfCycles (); // gets the TextBox associated with number of times to repeat this cycle
      std::vector<CycleStep*> steps_; // the CycleSteps containd by this Cycle
      ~Cycle();
    private:
      int x_, y_;
      Padding padding_;
      Image cycleImage_;
      TextBox numberOfCycles_;
  };
 
  // for containing a series of Cycles
  class CycleArray {
    public:
      // x : upper left of Cycle's screen x position
      // y : upper left of Cycle's screen y position
      // index : the index to add or remove Cycle at
      // step : Cycle to add to this CycleArray
      std::vector<Cycle*> cycles_; // the Cycles contained by this CycleArray
      CycleArray (int x = 0, int y = 0);
      void render (); // renders each contained cycle
      void setXY (int x, int y); // sets the location of all contained Cycles
      void addCycle (int index, Cycle* cycle); // adds a Cycle to this CycleArray
      Cycle* removeCycle (int index); // removes and returns a Cycle from this CycleArray
      SDL_Point getPoint (); // returns the location of this CycleArray
      CycleStep* getStep (int index);
      int size (); // returns the number of steps in the experiment
      void removeEmptyCycles (); // removes all empty cycles in this array
      void load (std::string path);
      void save (std::string path);
      void clear (); // empties array
      ~CycleArray();
    private:
      int x_, y_;
  };

  // for creating keys that type charicters into text boxes
  class Key {
    public:
      // x : upper left of key's screen x position
      // y : upper left of key's screen y position
      // w : width of the key
      // h : height of the key
      // text : the text on the button
      // ch : the cahricter to add to the textbox '\b' for delete
      // target : the text box to add number to
      Key (int x, int y, int w, int h, char ch, std::string text);
      void render (); // renders this number key
      void press (TextBox* target); // sends sumber to pointed text box
      SDL_Rect getRect (); // get SDL_Rect based on location and size
    private:
      int x_, y_;
      char ch_;
      Padding padding_;
      Text text_;
  };

  // for creating keys that type numbers into text box
  class NumberKey {
    public:
      // x : upper left of key's screen x position
      // y : upper left of key's screen y position
      // w : width of the key
      // h : height of the key
      // text : the text on the button
      // ch : the number to add to the textbox '\b' for delete
      // target : the text box to add number to
      NumberKey (int x, int y, int w, int h, char ch, std::string text);
      void render (); // renders this number key
      void press (TextBox* target); // sends sumber to pointed text box
      SDL_Rect getRect (); // get SDL_Rect based on location and size
    private:
      int x_, y_;
      char ch_;
      Padding padding_;
      Text text_;
  };

  // for creating a button that does something when pressed
  class Button {
    public:
      // x : upper left of the buttons's screen x position
      // y : upper left of the buttons's screen y position
      // w : width of the button
      // h : height of the button
      // text : the text on the button
      Button (int x, int y, int w, int h, std::string text);
      void render (); // renders this button
      void press (); // nothing ath the momennt
      void setText(std::string text);
      SDL_Rect getRect (); // get SDL_Rect based on location and size
    private:
      int x_, y_;
      Padding padding_;
      Text text_;
  };
}
