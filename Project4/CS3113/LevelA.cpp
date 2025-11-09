#include "LevelA.h"
#include <map>

extern int gLives;

LevelA::LevelA() : Scene({0.0f}, nullptr) {}

LevelA::LevelA(Vector2 origin, const char *bgHexCode) : Scene(origin, bgHexCode) {}

LevelA::~LevelA() { shutdown(); }

void LevelA::initialise()
{
    // Reset level-specific timers and state flags
    mSpawnTimer = 0.0f;
    mGameState.nextSceneID = 0;
    mGameState.showGameOver = false;

    // Sync lives with global counter so player suffering is consistent
    mGameState.lives = gLives;

    // Load and play level background music + death/jump audio effects
    mGameState.bgm = LoadMusicStream("assets/game/BarrenLand.wav");
    mDeathSound = LoadSound("assets/game/death_noise.wav");
    SetSoundVolume(mDeathSound, 0.33f);
    SetMusicVolume(mGameState.bgm, 0.33f);
    PlayMusicStream(mGameState.bgm);

    mGameState.jumpSound = LoadSound("assets/game/SFX_Jump_01.wav");

    // Place treasure chest somewhere not too easy to reach
    Vector2 chestPos = {
        mOrigin.x - (LEVEL_WIDTH/2.0f * TILE_DIMENSION) + 25 * TILE_DIMENSION,
        mOrigin.y - (LEVEL_HEIGHT/2.0f * TILE_DIMENSION) + 8 * TILE_DIMENSION
    };
    mChest = new Entity(chestPos, {TILE_DIMENSION, TILE_DIMENSION}, "assets/game/chest.png", NPC);

    // Load level layout and collision tiles
    mGameState.map = new Map(
        LEVEL_WIDTH, LEVEL_HEIGHT,
        (unsigned int*) mLevelData,
        "assets/game/dirt.png",
        TILE_DIMENSION,
        1, 1,
        mOrigin
    );

    // Basic animation setup for player: left/right walking + idle defaults
    std::map<Direction, std::vector<int>> playerAnimationAtlas = {
        {LEFT,  {8, 9, 10, 11}},
        {RIGHT, {12, 13, 14, 15}},
        {DOWN,  {0}},
        {UP,    {0}}
    };

    // Create player in the world with gravity and collider so physics is real-ish
    constexpr float PLAYER_WIDTH  = TILE_DIMENSION;
    constexpr float PLAYER_HEIGHT = TILE_DIMENSION;

    mGameState.player = new Entity(
        {200.0f, 200.0f},
        {PLAYER_WIDTH, PLAYER_HEIGHT},
        "assets/game/character.jpg",
        ATLAS,
        {4, 4},
        playerAnimationAtlas,
        PLAYER
    );

    mGameState.player->setJumpingPower(550.0f);
    mGameState.player->setColliderDimensions({PLAYER_WIDTH * 0.6f, PLAYER_HEIGHT * 0.8f});
    mGameState.player->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});

    // Camera tries its best to track player movement
    mGameState.camera = {0};
    mGameState.camera.target = mGameState.player->getPosition();
    mGameState.camera.offset = mOrigin;
    mGameState.camera.rotation = 0.0f;
    mGameState.camera.zoom = 1.0f;

    // Remove leftover enemies from previous life and spawn fresh evil
    for (Entity* e : mEnemies) delete e;
    mEnemies.clear();

    // A test enemy that loves walking off cliffs (WANDERER AI)
    Entity* wanderer = new Entity({mOrigin.x + 150.0f, mOrigin.y - 200.0f}, {100.0f, 100.0f}, "assets/game/Ghost.png", NPC);
    wanderer->setAIType(WANDERER);
    wanderer->setAIState(WALKING);
    wanderer->setColliderDimensions({60.0f, 60.0f});
    wanderer->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});
    mEnemies.push_back(wanderer);
}

void LevelA::update(float deltaTime)
{
    UpdateMusicStream(mGameState.bgm);

    // If player already died too many times, wait for ENTER to restart
    if (mGameState.showGameOver)
    {
        if (IsKeyPressed(KEY_ENTER))
        {
            gLives = 3;
            mGameState.lives = gLives;
            mGameState.showGameOver = false;
            mGameState.nextSceneID = -1; // return to menu
        }
        return; // freeze gameplay until player accepts fate
    }

    // Normal gameplay update only when level is active
    if (mGameState.nextSceneID == 0)
    {
        // Player physics + tile collisions + jumping etc.
        mGameState.player->update(deltaTime, nullptr, mGameState.map, nullptr, 0);

        // Enemies roam around trying to ruin player’s day
        for (Entity* enemy : mEnemies)
            enemy->update(deltaTime, mGameState.player, mGameState.map, nullptr, 0);

        // Camera tries to follow player horizontally
        Vector2 currentPlayerPosition = { mGameState.player->getPosition().x, mOrigin.y };
        panCamera(&mGameState.camera, &currentPlayerPosition);

        // Check death conditions: falling off map OR hugging enemies
        bool hitEnemy = false;
        for (Entity* enemy : mEnemies)
        {
            if (mGameState.player->collidesWith(enemy)) {
                hitEnemy = true;
                break;
            }
        }

        if (mGameState.player->getPosition().y > END_GAME_THRESHOLD || hitEnemy)
        {
            PlaySound(mDeathSound);

            if (gLives > 1)
            {
                gLives--;
                mGameState.lives = gLives;
                this->initialise(); // quick respawn with fresh scene
            }
            else
            {
                gLives = 0;
                mGameState.lives = 0;
                mGameState.showGameOver = true;
            }
        }

        // Win condition: reach the shiny chest and profit 🎉
        if (mChest != nullptr && mGameState.player->collidesWith(mChest))
        {
            mGameState.nextSceneID = 2;
        }
    }
}

void LevelA::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode));

    // Game over UI overlay, big red shame text
    if (mGameState.showGameOver) {
        const char* gameOverText = "GAME OVER";
        int fontSize = 60;
        int textWidth = MeasureText(gameOverText, fontSize);
        DrawText(gameOverText,
                 GetScreenWidth()/2 - textWidth/2,
                 GetScreenHeight()/2 - 50,
                 fontSize, RED);

        const char* returnText = "Press ENTER to return to main menu";
        int smallFontSize = 24;
        int returnWidth = MeasureText(returnText, smallFontSize);
        DrawText(returnText,
                 GetScreenWidth()/2 - returnWidth/2,
                 GetScreenHeight()/2 + 30,
                 smallFontSize, WHITE);
        return;
    }

    // Render player + enemies + chest inside camera world-space
    BeginMode2D(mGameState.camera);
        if (mGameState.map != nullptr) mGameState.map->render();
        if (mGameState.player != nullptr) mGameState.player->render();
        for (Entity* enemy : mEnemies)
            enemy->render();
        if (mChest != nullptr) mChest->render();
    EndMode2D();

    // UI overlay showing how many mistakes player can still make
    char livesText[32];
    snprintf(livesText, sizeof(livesText), "Lives: %d", gLives);
    DrawText(livesText, 20, 20, 24, WHITE);
}

void LevelA::shutdown()
{
    // Some good cleanup so memory doesn’t explode
    delete mGameState.player;
    delete mGameState.map;

    for (Entity* enemy : mEnemies)
        delete enemy;
    mEnemies.clear();

    if (mChest != nullptr) {
        delete mChest;
        mChest = nullptr;
    }

    // Unload audio out of RAM properly
    UnloadMusicStream(mGameState.bgm);
    UnloadSound(mGameState.jumpSound);
    UnloadSound(mDeathSound);
}
