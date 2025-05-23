#include "Game.hpp"
#include <iostream>
#include <SDL_image.h>
#include <unistd.h>
#include <SDL_ttf.h>
#include <cmath>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <random>
#include <fstream>
#include <functional>

using namespace std;

Game::Game() {
    isRunning = false;
    window = nullptr;
    renderer = nullptr;
    
    //For initializing the font and current state
    font = nullptr;
    smallFont = nullptr;
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
    
    // Initialize timer
    remainingTime = GAME_DURATION;
    timerStarted = false;

    // Initialize high scores
    isEnteringName = false;
    inputText = "";
    score = 0;
    scoreRoot = nullptr;
    
    // Load high scores first
    std::cout << "Loading high scores at startup..." << std::endl;
    loadBSTFromFile("highscores_bst.txt");

    // Initialize enemies
    minionTexture = nullptr;
    enemySpawnTimer = 0.0f;
    enemyQueue = EnemyQueue();
}

Game::~Game() {}

void Game::init(const char* title, int xpos, int ypos, int width, int height, bool fullscreen) {
    int flags = 0;
    
    //Initialize window
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
    
    // Initialize small font for UI elements
    smallFont = TTF_OpenFont("/Users/tatelgabrielle/Desktop/C++/FINAL-SOURCE-FILE/FINAL-SOURCE-FILE/assets/fonts/ByteBounce.ttf", 24);
    if (!smallFont) {
        cout << "Failed to load small font " << TTF_GetError() << endl;
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

    // Load minion sprite
    SDL_Surface* minionSurface = IMG_Load("/Users/tatelgabrielle/Desktop/C++/FINAL-SOURCE-FILE/FINAL-SOURCE-FILE/assets/monsters/minions.png");
    if (!minionSurface) {
        cout << "Failed to load minion sprite: " << IMG_GetError() << endl;
    } else {
        SDL_SetColorKey(minionSurface, SDL_TRUE, SDL_MapRGB(minionSurface->format, 0, 0, 0));
        minionTexture = SDL_CreateTextureFromSurface(renderer, minionSurface);
        SDL_FreeSurface(minionSurface);
        if (!minionTexture) {
            cout << "Failed to create minion texture: " << SDL_GetError() << endl;
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
                remainingTime = GAME_DURATION;
                timerStarted = true;
                gameStartTime = std::chrono::high_resolution_clock::now();
                cout << "Game Started! " << endl;
            }
        } else if (currentState == GameState::GAME_OVER) {
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN) {
                currentState = GameState::ENTER_NAME;
                isEnteringName = true;
                inputText = "";
                SDL_StartTextInput(); // Enable text input
            }
        } else if (currentState == GameState::ENTER_NAME) {
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_RETURN && !inputText.empty()) {
                    // Save the score to BST
                    insertScore(scoreRoot, inputText, score);
                    score = 0;
                    currentState = GameState::MENU;
                    isEnteringName = false;
                    SDL_StopTextInput(); // Disable text input
                    // Save only top 3 scores
                    while (countNodes(scoreRoot) > 3) {
                        removeLowestScore(scoreRoot);
                    }
                    saveBSTToFile("highscores_bst.txt");
                    std::cout << "Saved new high score: " << inputText << " " << score << std::endl;
                }
                else if (event.key.keysym.sym == SDLK_BACKSPACE && !inputText.empty()) {
                    inputText.pop_back();
                }
                else if (event.key.keysym.sym == SDLK_ESCAPE) {
                    currentState = GameState::MENU;
                    isEnteringName = false;
                    SDL_StopTextInput(); // Disable text input
                }
            }
            else if (event.type == SDL_TEXTINPUT && inputText.length() < MAX_NAME_LENGTH) {
                inputText += event.text.text;
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
    // Calculate delta time
    static auto lastTime = std::chrono::high_resolution_clock::now();
    auto now = std::chrono::high_resolution_clock::now();
    float deltaTime = std::chrono::duration<float>(now - lastTime).count();
    lastTime = now;

    if (currentState == GameState::PLAYING) {
        // Update timer
        if (timerStarted) {
            auto currentTime = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - gameStartTime).count() / 1000.0f;
            remainingTime = GAME_DURATION - elapsed;
            
            if (remainingTime <= 0) {
                currentState = GameState::GAME_OVER;
                timerStarted = false;
            }
        }

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

        // Update projectiles
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

        // Enemy spawn logic
        enemySpawnTimer += deltaTime;
        if (enemySpawnTimer >= ENEMY_SPAWN_INTERVAL) {
            spawnEnemy();
            enemySpawnTimer = 0.0f;
        }
        updateEnemies(deltaTime);
    }

    // On game over, prompt for name entry if score > 0
    if (currentState == GameState::GAME_OVER && score > 0 && !isEnteringName) {
        currentState = GameState::ENTER_NAME;
        isEnteringName = true;
        inputText = "";
        SDL_StartTextInput();
        // Save scores when game ends
        saveBSTToFile("highscores_bst.txt");
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
    
    // Render timer bar
    if (currentState == GameState::PLAYING) {
        renderTimerBar();
    }

    // Render high scores
    renderHighScores();

    // Render game over
    if (currentState == GameState::GAME_OVER) {
        renderGameOver();
    }

    // Render name input
    if (currentState == GameState::ENTER_NAME) {
        renderNameInput();
    }

    // Render enemies
    if (currentState == GameState::PLAYING) {
        renderEnemies();
    }
    
    // Render current score
    if (currentState == GameState::PLAYING) {
        SDL_Color color = {255, 255, 0, 255};
        std::stringstream ss;
        ss << "Score: " << score;
        SDL_Surface* textSurface = TTF_RenderText_Solid(smallFont, ss.str().c_str(), color);
        if (textSurface) {
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            SDL_Rect textRect = {10, 10, textSurface->w, textSurface->h};
            SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
            SDL_DestroyTexture(textTexture);
            SDL_FreeSurface(textSurface);
        }
    }

    // Render top scores
    int count = 0;
    displayScoresDescending(scoreRoot, count);

    SDL_RenderPresent(renderer);
}

void Game::renderTimerBar() {
    int winW, winH;
    SDL_GetWindowSize(window, &winW, &winH);

    // Draw background bar
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_Rect bgRect = {
        TIMER_BAR_PADDING,
        TIMER_BAR_PADDING,
        winW - (2 * TIMER_BAR_PADDING),
        TIMER_BAR_HEIGHT
    };
    SDL_RenderFillRect(renderer, &bgRect);

    // Draw remaining time bar
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_Rect timeRect = {
        TIMER_BAR_PADDING,
        TIMER_BAR_PADDING,
        static_cast<int>((winW - (2 * TIMER_BAR_PADDING)) * (remainingTime / GAME_DURATION)),
        TIMER_BAR_HEIGHT
    };
    SDL_RenderFillRect(renderer, &timeRect);
}

void Game::renderHighScores() {
    if (!smallFont) return;

    SDL_Color color = {255, 255, 255, 255};
    
    // Render "Top Scores" header
    SDL_Surface* headerSurface = TTF_RenderText_Solid(smallFont, "Top 3 Scores", color);
    if (headerSurface) {
        SDL_Texture* headerTexture = SDL_CreateTextureFromSurface(renderer, headerSurface);
        SDL_Rect headerRect = {10, 50, headerSurface->w, headerSurface->h};
        SDL_RenderCopy(renderer, headerTexture, NULL, &headerRect);
        SDL_DestroyTexture(headerTexture);
        SDL_FreeSurface(headerSurface);
    }

    // Render scores
    int count = 0;
    displayScoresDescending(scoreRoot, count);
    
    // If no scores are displayed, show a message
    if (count == 0) {
        SDL_Surface* noScoresSurface = TTF_RenderText_Solid(smallFont, "No high scores yet!", color);
        if (noScoresSurface) {
            SDL_Texture* noScoresTexture = SDL_CreateTextureFromSurface(renderer, noScoresSurface);
            SDL_Rect noScoresRect = {10, 80, noScoresSurface->w, noScoresSurface->h};
            SDL_RenderCopy(renderer, noScoresTexture, NULL, &noScoresRect);
            SDL_DestroyTexture(noScoresTexture);
            SDL_FreeSurface(noScoresSurface);
        }
    }
}

void Game::renderGameOver() {
    if (!font) return;

    SDL_Color color = {255, 0, 0, 255};
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, "Game Over hit enter", color);
    
    if (textSurface) {
        int winW, winH;
        SDL_GetWindowSize(window, &winW, &winH);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_Rect textRect = {
            (winW - textSurface->w) / 2,
            (winH - textSurface->h) / 2,
            textSurface->w,
            textSurface->h
        };
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
        SDL_DestroyTexture(textTexture);
        SDL_FreeSurface(textSurface);
    }
}

void Game::renderNameInput() {
    if (!smallFont) return;

    int winW, winH;
    SDL_GetWindowSize(window, &winW, &winH);

    // Render background box
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
    SDL_Rect bgRect = {
        winW/4,
        winH/2 - 50,
        winW/2,
        100
    };
    SDL_RenderFillRect(renderer, &bgRect);

    // Render border
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &bgRect);

    // Render prompt
    SDL_Color color = {255, 255, 255, 255};
    std::string prompt = "Enter your name: " + inputText;
    
    // Add blinking cursor
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime.time_since_epoch()).count();
    if ((elapsed / 500) % 2 == 0) {
        prompt += "|";
    }
    
    SDL_Surface* textSurface = TTF_RenderText_Solid(smallFont, prompt.c_str(), color);
    if (textSurface) {
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_Rect textRect = {
            (winW - textSurface->w) / 2,
            (winH - textSurface->h) / 2,
            textSurface->w,
            textSurface->h
        };
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
        SDL_DestroyTexture(textTexture);
        SDL_FreeSurface(textSurface);
    }

    // Render instructions
    std::string instructions = "Press ENTER to confirm or ESC to cancel";
    SDL_Surface* instrSurface = TTF_RenderText_Solid(smallFont, instructions.c_str(), color);
    if (instrSurface) {
        SDL_Texture* instrTexture = SDL_CreateTextureFromSurface(renderer, instrSurface);
        SDL_Rect instrRect = {
            (winW - instrSurface->w) / 2,
            (winH - instrSurface->h) / 2 + 40,
            instrSurface->w,
            instrSurface->h
        };
        SDL_RenderCopy(renderer, instrTexture, NULL, &instrRect);
        SDL_DestroyTexture(instrTexture);
        SDL_FreeSurface(instrSurface);
    }
}

void Game::loadHighScores() {
    std::ifstream file("highscores.txt");
    if (file.is_open()) {
        highScores.clear();
        std::string name;
        float score;
        while (file >> name >> score) {
            highScores.push_back({name, score});
        }
        file.close();
        
        // Sort and keep only top 3 scores
        std::sort(highScores.begin(), highScores.end(), 
            [](const HighScore& a, const HighScore& b) {
                return a.score > b.score;
            });
        if (highScores.size() > 3) {
            highScores.resize(3);
        }
    }
}

void Game::saveHighScores() {
    // Sort scores in descending order
    std::sort(highScores.begin(), highScores.end(), 
        [](const HighScore& a, const HighScore& b) {
            return a.score > b.score;
        });
    
    // Keep only top 3 scores
    if (highScores.size() > 3) {
        highScores.resize(3);
    }
    
    std::ofstream file("highscores.txt");
    if (file.is_open()) {
        for (const auto& score : highScores) {
            file << score.name << " " << score.score << "\n";
        }
        file.close();
    }
}

void Game::spawnEnemy() {
    if (enemyQueue.size >= MAX_ENEMIES) return;
    int winW, winH;
    SDL_GetWindowSize(window, &winW, &winH);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> xDist(0, winW - 32);
    std::uniform_real_distribution<float> yDist(0, winH / 2);
    std::uniform_real_distribution<float> vDist(-0.5f, 0.5f);
    std::uniform_real_distribution<float> bonusDist(0, 1);
    Enemy enemy;
    enemy.x = xDist(gen);
    enemy.y = yDist(gen);
    enemy.vx = vDist(gen);
    enemy.vy = vDist(gen);
    if (enemy.vx == 0 && enemy.vy == 0) enemy.vx = 1.0f;
    enemy.frame = rand() % 40; // Start at a random frame
    enemy.frameCount = 40;   // 8 columns x 5 rows = 40 frames
    enemy.frameTime = 0.25f; // slower animation
    enemy.frameTimer = 0.0f;
    enemy.spriteW = 23; // use 23 for all columns to avoid cropping
    enemy.spriteH = 66;
    enemy.frameCount = 12;
    enemy.frameTime = 0.15f;
    enemy.health = 2; // Initialize health to 2
    enemy.maxHealth = 2; // Default maxHealth
    enemy.isTimeBonus = (bonusDist(gen) < 0.1); // 10% chance for time bonus sprite
    if (score >= 600) {
        enemy.health = 3;
        enemy.maxHealth = 3;
    }
    EnemyNode* node = new EnemyNode{enemy, nullptr};
    if (!enemyQueue.tail) {
        enemyQueue.head = enemyQueue.tail = node;
    } else {
        enemyQueue.tail->next = node;
        enemyQueue.tail = node;
    }
    enemyQueue.size++;
}

void Game::updateEnemies(float deltaTime) {
    EnemyNode* prev = nullptr;
    EnemyNode* curr = enemyQueue.head;
    int winW, winH;
    SDL_GetWindowSize(window, &winW, &winH);
    while (curr) {
        Enemy& e = curr->enemy;
        e.x += e.vx;
        e.y += e.vy;
        // Bounce off edges
        if (e.x < 0) { e.x = 0; e.vx = -e.vx; }
        if (e.x > winW - e.spriteW) { e.x = winW - e.spriteW; e.vx = -e.vx; }
        if (e.y < 0) { e.y = 0; e.vy = -e.vy; }
        if (e.y > winH - e.spriteH) { e.y = winH - e.spriteH; e.vy = -e.vy; }
        // Check collision with projectiles
        bool destroyed = false;
        for (int i = 0; i < MAX_PROJECTILES; i++) {
            if (projectiles[i].active) {
                SDL_Rect projRect = {static_cast<int>(projectiles[i].x - 16), static_cast<int>(projectiles[i].y - 64), 32, 128};
                SDL_Rect minionRect = {static_cast<int>(e.x), static_cast<int>(e.y), 45, 66};
                if (SDL_HasIntersection(&projRect, &minionRect)) {
                    projectiles[i].active = false;
                    if (e.health > 1) {
                        e.health--;
                    } else {
                        if (e.isTimeBonus) {
                            remainingTime += 4.0f; // Add 4 seconds for time bonus sprite
                        }
                        score += 50;
                        destroyed = true;
                    }
                    break;
                }
            }
        }
        if (destroyed) {
            // Remove minion from queue
            if (prev) {
                prev->next = curr->next;
                if (curr == enemyQueue.tail) enemyQueue.tail = prev;
                delete curr;
                curr = prev->next;
            } else {
                enemyQueue.head = curr->next;
                if (curr == enemyQueue.tail) enemyQueue.tail = nullptr;
                delete curr;
                curr = enemyQueue.head;
            }
            enemyQueue.size--;
            continue;
        }
        prev = curr;
        curr = curr->next;
        e.frameTimer += deltaTime;
        if (e.frameTimer >= e.frameTime) {
            e.frame = (e.frame + 1) % 6; // 3 columns x 2 rows = 6 frames
            e.frameTimer = 0.0f;
        }
    }
}

void Game::renderEnemies() {
    if (!minionTexture) return;
    EnemyNode* curr = enemyQueue.head;
    while (curr) {
        Enemy& e = curr->enemy;
        const int framesPerRow = 3;
        int frameX = e.frame % framesPerRow;
        int frameY = e.frame / framesPerRow;
        SDL_Rect srcRect = {frameX * 45, frameY * 66, 45, 66};
        SDL_Rect destRect = {static_cast<int>(e.x), static_cast<int>(e.y), 60, 100}; // 2x scale
        SDL_RenderCopy(renderer, minionTexture, &srcRect, &destRect);
        // Draw health bar above minion
        int barWidth = 60;
        int barHeight = 8;
        int barX = destRect.x;
        int barY = destRect.y - barHeight - 4;
        // Background (gray)
        SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
        SDL_Rect bgRect = {barX, barY, barWidth, barHeight};
        SDL_RenderFillRect(renderer, &bgRect);
        // Health (green or red)
        int healthWidth = (e.health * barWidth) / e.maxHealth; // Use maxHealth for scaling
        SDL_SetRenderDrawColor(renderer, e.health == e.maxHealth ? 0 : 255, e.health == e.maxHealth ? 255 : 0, 0, 255); // green if full, red if not
        SDL_Rect healthRect = {barX, barY, healthWidth, barHeight};
        SDL_RenderFillRect(renderer, &healthRect);
        // Border (white)
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &bgRect);
        // If time bonus sprite, draw a blue border
        if (e.isTimeBonus) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
            SDL_RenderDrawRect(renderer, &destRect);
        }
        curr = curr->next;
    }
}

void Game::clearEnemies() {
    EnemyNode* curr = enemyQueue.head;
    while (curr) {
        EnemyNode* next = curr->next;
        delete curr;
        curr = next;
    }
    enemyQueue.head = enemyQueue.tail = nullptr;
    enemyQueue.size = 0;
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

    if (smallFont) {
        TTF_CloseFont(smallFont);
        smallFont = nullptr;
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

    if (minionTexture) {
        SDL_DestroyTexture(minionTexture);
        minionTexture = nullptr;
    }
    clearEnemies();
}

void Game::insertScore(ScoreNode*& root, const std::string& name, int score) {
    if (!root) {
        root = new ScoreNode(name, score);
        return;
    }
    if (score > root->score) {
        insertScore(root->left, name, score);
    } else {
        insertScore(root->right, name, score);
    }
}

void Game::displayScoresDescending(ScoreNode* root, int& count) {
    if (!root || count >= 3) return;
    
    // First traverse the left subtree (higher scores)
    displayScoresDescending(root->left, count);
    
    // Then display the current node if we haven't shown 3 scores yet
    if (count < 3) {
        SDL_Color color = {255, 255, 255, 255};
        std::stringstream ss;
        ss << (count + 1) << ". " << root->name << " [Score: " << root->score << "]";
        SDL_Surface* textSurface = TTF_RenderText_Solid(smallFont, ss.str().c_str(), color);
        if (textSurface) {
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            SDL_Rect textRect = {10, 80 + count * 30, textSurface->w, textSurface->h};
            SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
            SDL_DestroyTexture(textTexture);
            SDL_FreeSurface(textSurface);
        }
        count++;
    }
    
    // Finally traverse the right subtree (lower scores)
    displayScoresDescending(root->right, count);
}

void Game::clearScoreTree(ScoreNode* root) {
    if (!root) return;
    clearScoreTree(root->left);
    clearScoreTree(root->right);
    delete root;
}

void Game::saveBSTToFile(const std::string& filename) {
    std::string fullPath = "/Users/tatelgabrielle/Desktop/C++/FINAL-SOURCE-FILE/FINAL-SOURCE-FILE/" + filename;
    std::cout << "Attempting to save scores to file: " << fullPath << std::endl;
    
    // Create file if it doesn't exist
    std::ofstream file(fullPath, std::ios::out | std::ios::app);
    if (!file.is_open()) {
        std::cout << "Failed to open file for writing: " << fullPath << std::endl;
        std::cout << "Error: " << strerror(errno) << std::endl;
        return;
    }
    
    // Clear the file contents before writing
    file.close();
    file.open(fullPath, std::ios::out | std::ios::trunc);
    
    // Save in descending order
    std::function<void(ScoreNode*)> save = [&](ScoreNode* node) {
        if (!node) return;
        save(node->left);
        file << node->name << " " << node->score << "\n";
        std::cout << "Saving score: " << node->name << " " << node->score << std::endl;
        save(node->right);
    };
    save(scoreRoot);
    file.close();
    std::cout << "Finished saving scores to file" << std::endl;
}

void Game::loadBSTFromFile(const std::string& filename) {
    std::string fullPath = "/Users/tatelgabrielle/Desktop/C++/FINAL-SOURCE-FILE/FINAL-SOURCE-FILE/" + filename;
    std::cout << "Attempting to load scores from file: " << fullPath << std::endl;
    
    clearScoreTree(scoreRoot);
    scoreRoot = nullptr;
    
    std::ifstream file(fullPath);
    if (!file.is_open()) {
        std::cout << "Failed to open file for reading: " << fullPath << std::endl;
        std::cout << "Error: " << strerror(errno) << std::endl;
        return;
    }
    
    std::string name;
    int scoreVal;
    while (file >> name >> scoreVal) {
        std::cout << "Loading score: " << name << " " << scoreVal << std::endl;
        insertScore(scoreRoot, name, scoreVal);
    }
    file.close();
    std::cout << "Finished loading scores from file" << std::endl;
}

int Game::countNodes(ScoreNode* root) {
    if (!root) return 0;
    return 1 + countNodes(root->left) + countNodes(root->right);
}

void Game::removeLowestScore(ScoreNode*& root) {
    if (!root) return;
    if (!root->right) {
        ScoreNode* temp = root;
        root = root->left;
        delete temp;
    } else {
        removeLowestScore(root->right);
    }
}
