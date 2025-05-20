
#ifndef Game_hpp
#define Game_hpp

#include <SDL.h>
#include <SDL_ttf.h>
#include <iostream>

enum class GameState {
    MENU,
    PLAYING
}; 

class Game {
public:
    Game();
    ~Game();

    void init(const char* title, int xpos, int ypos, int width, int height, bool fullscreen);
    void handleEvents();
    void update();
    void render();
    void clean();

    bool running() { return isRunning; }

private:
    bool isRunning;
    SDL_Window* window;
    SDL_Renderer* renderer;
    
    //background variables
    SDL_Texture* backgroundTexture;
    
    //Game text variable
    GameState currentState;
    TTF_Font* font; 
};

#endif

