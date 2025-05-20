#include "Game.hpp"
#include <iostream>
#include <SDL_image.h>
#include <unistd.h>
#include <SDL_ttf.h> 

using namespace std;

Game::Game() {
    isRunning = false;
    window = nullptr;
    renderer = nullptr;
    
    //For initializing the font and current state
    font = nullptr;
    currentState = GameState::MENU;
}

Game::~Game() {}

void Game::init(const char* title, int xpos, int ypos, int width, int height, bool fullscreen) {
    int flags = 0;
    if (fullscreen) {
        flags = SDL_WINDOW_FULLSCREEN;
    }
    if (SDL_Init(SDL_INIT_EVERYTHING) == 0) {
        cout << "Subsystems Initialised!..." << std::endl;
        window = SDL_CreateWindow(title, xpos, ypos, width, height, flags);
        if (window) {
            cout << "Window created!" << endl;
        }
        renderer = SDL_CreateRenderer(window, -1, 0);
        if(renderer) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            cout << "Renderer craeted " << endl;
        }
        isRunning = true;
    } else {
        isRunning = false;
    }
    
    // Print current working directory for debugging
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        cout << "Current working directory: " << cwd << endl;
    }
    
    SDL_Surface* bgSurface = IMG_Load("/Users/tatelgabrielle/Desktop/C++/FINAL-SOURCE-FILE/FINAL-SOURCE-FILE/assets/background/bg_space_seamless_1.png");
    
    if (!bgSurface) {
        cout << "Failed to load background image! SDL_image Error " << IMG_GetError() << endl;
    }
    
    backgroundTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);
    SDL_FreeSurface(bgSurface);
    
    
    // Initialize SDL_ttf and load font
    if (TTF_Init() == -1) {
        cout << "Failed to initialize SDL_ttf: " << TTF_GetError() << endl;
    }
    font = TTF_OpenFont("/Users/tatelgabrielle/Desktop/C++/FINAL-SOURCE-FILE/FINAL-SOURCE-FILE/assets/fonts/ByteBounce.ttf", 96);

    if (!font) {
        cout << "Failed to load font " << TTF_GetError() << endl;
    }
    
}

void Game::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            isRunning = false;
        }
        
        if (currentState == GameState::MENU) {
            if (event.type == SDL_MOUSEBUTTONDOWN || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN)) {
                currentState = GameState::PLAYING;
                cout << "Game Started! " << endl;
            }
        }
    }
}

void Game::update() {
    if (currentState == GameState::PLAYING) {

    }
}

void Game::render() {
    SDL_RenderClear(renderer);
    
    //Draws the background image of the project 
    if (backgroundTexture) {
        float zoomW = 1.11f; // Adjust width ratio here
        float zoomH = 1.0f; // Adjust height ratio here
        int w, h;
        SDL_GetWindowSize(window, &w, &h);
        int destW = w * zoomW;
        int destH = h * zoomH;
        SDL_Rect destRect;
        destRect.w = destW;
        destRect.h = destH;
        destRect.x = (w - destW) / 2;
        destRect.y = (h - destH) / 2;
        SDL_RenderCopy(renderer, backgroundTexture, NULL, &destRect);
    }

    // RENDER TEXT ON MENU
    if (currentState == GameState::MENU) {
        if (!font) {
            cout << "[ERROR] Font not loaded!" << endl;
        } else {
            SDL_Color color = {255, 0, 0};
            SDL_Surface* textSurface = TTF_RenderText_Solid(font, "Start Game", color);
            if (!textSurface) {
                cout << "[ERROR] Failed to create text surface: " << TTF_GetError() << endl;
            } else {
                SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                if (!textTexture) {
                    std::cout << "[ERROR] Failed to create text texture: " << SDL_GetError() << std::endl;
                } else {
                    int w, h;
                    SDL_GetWindowSize(window, &w, &h);
                    SDL_Rect textRect;
                    textRect.w = textSurface->w;
                    textRect.h = textSurface->h;
                    textRect.x = (w - textRect.w) / 2;
                    textRect.y = (h - textRect.h) / 2;
                    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
                    SDL_DestroyTexture(textTexture);
                }
                SDL_FreeSurface(textSurface);
            }
        }
        
        if (currentState == GameState::MENU) {
            cout << "MENU: PRES ENTER TO START" << endl;
        }
    }
    
    SDL_RenderPresent(renderer);
}

void Game::clean() {
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
    cout << "Game cleaned!" << endl;
    
    
    //Clean FONT
    if (font) {
        TTF_CloseFont(font);
        font = nullptr;
    }

    TTF_Quit();

    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
    cout << "Game cleaned!" << endl;
}

