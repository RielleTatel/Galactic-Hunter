#ifndef Game_hpp
#define Game_hpp

#include <SDL.h>
#include <SDL_ttf.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <fstream>
#include <random>

enum class GameState {
    MENU,
    PLAYING,
    GAME_OVER,
    ENTER_NAME
};

struct HighScore {
    std::string name;
    float score;
};

struct Enemy {
    float x, y;
    float vx, vy;
    int frame;
    int frameCount;
    float frameTime;
    float frameTimer;
    int spriteW, spriteH;
};

struct EnemyNode {
    Enemy enemy;
    EnemyNode* next;
};

struct EnemyQueue {
    EnemyNode* head;
    EnemyNode* tail;
    int size;
    EnemyQueue() : head(nullptr), tail(nullptr), size(0) {}
};

struct ScoreNode {
    std::string name;
    int score;
    ScoreNode* left;
    ScoreNode* right;
    ScoreNode(const std::string& n, int s) : name(n), score(s), left(nullptr), right(nullptr) {}
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
    TTF_Font* smallFont; // For high scores display
    
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
    
    // Spaceship rotation angle
    double spaceshipAngle = 0.0;
    
    // Projectile system
    static const int MAX_PROJECTILES = 3;
    struct Projectile {
        float x, y;
        float angle;
        bool active;
    };
    Projectile projectiles[MAX_PROJECTILES];
    int projectileCount;
    SDL_Texture* beamTexture;
    bool canFire;
    float lastFireTime;
    static constexpr float FIRE_COOLDOWN = 0.5f; // Cooldown between shots in seconds
    
    // Timer system
    static constexpr float GAME_DURATION = 30.0f; // 30 seconds game duration
    float remainingTime;
    std::chrono::time_point<std::chrono::high_resolution_clock> gameStartTime;
    bool timerStarted;
    
    // Timer bar dimensions
    static constexpr int TIMER_BAR_HEIGHT = 10;
    static constexpr int TIMER_BAR_PADDING = 5;

    // High scores system
    std::vector<HighScore> highScores;
    std::string currentPlayerName;
    bool isEnteringName;
    std::string inputText;
    static constexpr int MAX_NAME_LENGTH = 20;
    
    // Helper functions
    void loadHighScores();
    void saveHighScores();
    void renderHighScores();
    void renderTimerBar();
    void renderGameOver();
    void renderNameInput();
    
    SDL_Texture* minionTexture;
    EnemyQueue enemyQueue;
    float enemySpawnTimer;
    static constexpr int MAX_ENEMIES = 10;
    static constexpr float ENEMY_SPAWN_INTERVAL = 1.5f;
    void spawnEnemy();
    void updateEnemies(float deltaTime);
    void renderEnemies();
    void clearEnemies();

    int score;
    ScoreNode* scoreRoot;
    void insertScore(ScoreNode*& root, const std::string& name, int score);
    void displayScoresDescending(ScoreNode* root, int& count);
    void clearScoreTree(ScoreNode* root);
    void saveBSTToFile(const std::string& filename);
    void loadBSTFromFile(const std::string& filename);
    int countNodes(ScoreNode* root);
    void removeLowestScore(ScoreNode*& root);
};

#endif

