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
    
    //For initializing the space ship sprite variable
    playerTexture = nullptr;
    
    //For initializing the cursor sprite variable
    cursorTexture = nullptr;
    
    // Initialize projectile system
    beamTexture = nullptr;
    projectileCount = MAX_PROJECTILES;
    canFire = true;
    lastFireTime = 0.0f;
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        projectiles[i].active = false;
    }
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
    
    // Load space ship sprite
    playerTexture = IMG_LoadTexture(renderer, "/Users/tatelgabrielle/Desktop/C++/FINAL-SOURCE-FILE/FINAL-SOURCE-FILE/assets/sprites/fighter.png");
    if (!playerTexture) {
        cout << "Failed to load player sprite: " << IMG_GetError() << endl;
    }
    
    // Load cursor sprite with color keying for magenta background
    SDL_Surface* cursorSurface = IMG_Load("/Users/tatelgabrielle/Desktop/C++/FINAL-SOURCE-FILE/FINAL-SOURCE-FILE/assets/sprites/cursor.png");
    if (!cursorSurface) {
        cout << "Failed to load cursor sprite: " << IMG_GetError() << endl;
    } else {
        SDL_SetColorKey(cursorSurface, SDL_TRUE, SDL_MapRGB(cursorSurface->format, 255, 0, 255));
        cursorTexture = SDL_CreateTextureFromSurface(renderer, cursorSurface);
        SDL_FreeSurface(cursorSurface);
        if (!cursorTexture) {
            cout << "Failed to create cursor texture: " << SDL_GetError() << endl;
        }
    }
            // Set initial cursor position to center of window
            int winW, winH;
            SDL_GetWindowSize(window, &winW, &winH);
            cursorX = (winW - 32) / 2.0f;
            cursorY = (winH - 32) / 2.0f;
            cursorVelX = 0.0f;
            cursorVelY = 0.0f;
    
    // Load beam texture
    SDL_Surface* beamSurface = IMG_Load("/Users/tatelgabrielle/Desktop/C++/FINAL-SOURCE-FILE/FINAL-SOURCE-FILE/assets/sprites/beams.png");
    if (!beamSurface) {
        cout << "Failed to load beam sprite: " << IMG_GetError() << endl;
    } else {
        // Set color key for magenta background
        SDL_SetColorKey(beamSurface, SDL_TRUE, SDL_MapRGB(beamSurface->format, 255, 0, 255));
        beamTexture = SDL_CreateTextureFromSurface(renderer, beamSurface);
        SDL_FreeSurface(beamSurface);
        if (!beamTexture) {
            cout << "Failed to create beam texture: " << SDL_GetError() << endl;
        }
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
        
        // Handle cursor movement in PLAYING state
        if (currentState == GameState::PLAYING) {
            const float speed = 0.5f; // Increased speed for better responsiveness
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_LEFT:
                        cursorVelX = -speed;
                        break;
                    case SDLK_RIGHT:
                        cursorVelX = speed;
                        break;
                    case SDLK_UP:
                        cursorVelY = -speed;
                        break;
                    case SDLK_DOWN:
                        cursorVelY = speed;
                        break;
                }
            }
            
            // Handle cursor movement in PLAYING state
            if (event.type == SDL_KEYUP) {
                switch (event.key.keysym.sym) {
                    case SDLK_LEFT:
                        if (cursorVelX < 0) cursorVelX = 0;
                        break;
                    case SDLK_RIGHT:
                        if (cursorVelX > 0) cursorVelX = 0;
                        break;
                    case SDLK_UP:
                        if (cursorVelY < 0) cursorVelY = 0;
                        break;
                    case SDLK_DOWN:
                        if (cursorVelY > 0) cursorVelY = 0;
                        break;
                }
            }
            
            
            // Handle projectile firing
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) {
                if (canFire && projectileCount > 0) {
                    // Find first inactive projectile
                    for (int i = 0; i < MAX_PROJECTILES; i++) {
                        if (!projectiles[i].active) {
                            // Get spaceship position
                            int winW, winH;
                            SDL_GetWindowSize(window, &winW, &winH);
                            int spriteW = 68;
                            int spriteH = 68;
                            float spaceshipX = (winW - spriteW) / 2.0f;
                            float spaceshipY = winH - spriteH - 10.0f;

                            // Initialize projectile
                            projectiles[i].x = spaceshipX + spriteW/2;
                            projectiles[i].y = spaceshipY + spriteH/2;
                            projectiles[i].angle = spaceshipAngle;
                            projectiles[i].active = true;
                            
                            // Decrease projectile count
                            projectileCount--;
                            
                            // If no projectiles left, disable firing
                            if (projectileCount == 0) {
                                canFire = false;
                            }
                            break;
                        }
                    }
                }
                
                
            }
        }
    }
}

void Game::update() {
    if (currentState == GameState::PLAYING) {
        // Update cursor position
        cursorX += cursorVelX;
        cursorY += cursorVelY;
        // Clamp cursor position to window bounds
        int winW, winH;
        SDL_GetWindowSize(window, &winW, &winH);
        if (cursorX < 0) cursorX = 0;
        if (cursorY < 0) cursorY = 0;
        if (cursorX > winW - 32) cursorX = winW - 32;
        if (cursorY > winH - 32) cursorY = winH - 32;

        // Calculate angle between spaceship and cursor
        int spriteW = 68;
        int spriteH = 68;
        float spaceshipX = (winW - spriteW) / 2.0f;
        float spaceshipY = winH - spriteH - 10.0f;
        
        // Calculate angle in radians
        float dx = cursorX - spaceshipX;
        float dy = cursorY - spaceshipY;
        
        spaceshipAngle = atan2(dy, dx) * 180.0 / M_PI;
        // Adjust angle to match sprite's default orientation
        spaceshipAngle += 90.0;

        // Update projectiles || PORJECTILE FIRING SYSTEM 
        bool allProjectilesOffScreen = true;
        for (int i = 0; i < MAX_PROJECTILES; i++) {
            if (projectiles[i].active) {
                // Move projectile in its direction
                float speed = 2.0f;
                float radianAngle = (projectiles[i].angle - 90.0f) * M_PI / 180.0f;
                projectiles[i].x += cos(radianAngle) * speed;
                projectiles[i].y += sin(radianAngle) * speed;

                // Check if projectile is off screen
                if (projectiles[i].x < 0 || projectiles[i].x > winW ||
                    projectiles[i].y < 0 || projectiles[i].y > winH) {
                    projectiles[i].active = false;
                } else {
                    allProjectilesOffScreen = false;
                }
            }
        }

        // If all projectiles are off screen, replenish the stack
        if (allProjectilesOffScreen && !canFire) {
            projectileCount = MAX_PROJECTILES;
            canFire = true;
        }
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
    
    
    //RENDER PlAYER SPRITE || SPACE SHIP
    if (currentState == GameState::PLAYING && playerTexture) {
        int winW, winH;
        SDL_GetWindowSize(window, &winW, &winH);

        int spriteW = 68; // width of your sprite
        int spriteH = 68; // height of your sprite

        SDL_Rect destRect;
        destRect.w = spriteW;
        destRect.h = spriteH;
        destRect.x = (winW - spriteW) / 2;
        destRect.y = winH - spriteH - 10; // 10px from bottom

        // Create a point for rotation center
        SDL_Point center = {spriteW / 2, spriteH / 2};
        
        // Render the spaceship with rotation
        SDL_RenderCopyEx(renderer, playerTexture, NULL, &destRect, spaceshipAngle, &center, SDL_FLIP_NONE);
    }

  //RENDER CURSOR SPRITE
  if (currentState == GameState::PLAYING && cursorTexture) {
    int spriteW = 32; // width of one sprite
    int spriteH = 32; // height of one sprite

    // Source rectangle: middle sprite (orange)
    SDL_Rect srcRect;
    srcRect.x = 0;
    srcRect.y = 32;
    srcRect.w = spriteW;
    srcRect.h = spriteH;

    // Destination rectangle: use cursorX, cursorY
    SDL_Rect destRect;
    destRect.w = spriteW;
    destRect.h = spriteH;
    destRect.x = static_cast<int>(cursorX);
    destRect.y = static_cast<int>(cursorY);

    SDL_RenderCopy(renderer, cursorTexture, &srcRect, &destRect);
  }
    
    // Render projectiles
    if (currentState == GameState::PLAYING && beamTexture) {
        for (int i = 0; i < MAX_PROJECTILES; i++) {
            if (projectiles[i].active) {
                // Source rectangle for the green beam
                SDL_Rect srcRect = {0, 0, 32, 128}; // Adjust these values based on your sprite sheet
                
                // Destination rectangle
                SDL_Rect destRect;
                destRect.w = 32;
                destRect.h = 128;
                destRect.x = static_cast<int>(projectiles[i].x - destRect.w/2);
                destRect.y = static_cast<int>(projectiles[i].y - destRect.h/2);

                // Render the beam with rotation
                SDL_Point center = {destRect.w/2, destRect.h/2};
                SDL_RenderCopyEx(renderer, beamTexture, &srcRect, &destRect,
                               projectiles[i].angle, &center, SDL_FLIP_NONE);
            }
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
    
    //CLEAN SPACE SHIP SPRITE
    if (playerTexture) {
      SDL_DestroyTexture(playerTexture);
      playerTexture = nullptr;
    }

    //CLEAN CURSOR SPRITE
    if (cursorTexture) {
      SDL_DestroyTexture(cursorTexture);
      cursorTexture = nullptr;
    }
    
    // Clean beam texture
    if (beamTexture) {
        SDL_DestroyTexture(beamTexture);
        beamTexture = nullptr;
    }
}

