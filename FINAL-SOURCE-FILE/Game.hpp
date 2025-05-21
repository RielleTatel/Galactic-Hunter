
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
    
    //background variable
    SDL_Texture* backgroundTexture;
    
    //Game text variable
    TTF_Font* font;
    
    //current state variable
    GameState currentState;
    
    // space ship sprite variable
    SDL_Texture* playerTexture;

    // cursor sprite variable
    SDL_Texture* cursorTexture;

        // Cursor position and velocity
        float cursorX = 0.0f;
        float cursorY = 0.0f;
        float cursorVelX = 0.0f;
        float cursorVelY = 0.0f;
};

#endif

