#include "LevelC.h"
#include <map>

extern int gLives;

LevelC::LevelC() : Scene({0.0f}, nullptr)
{
    
}

LevelC::LevelC(Vector2 origin, const char* bgHexCode) : Scene(origin, bgHexCode) {}

LevelC::~LevelC()
{
    shutdown();
}

void LevelC::initialise()
{
    

    // Reset core state for a clean run
    mSpawnTimer = 0.0f;
    mGameState.nextSceneID = 0;
    mGameState.showGameOver = false;
    mGameState.lives = gLives;

    // Audio setup
    mGameState.bgm = LoadMusicStream("assets/game/BarrenLand.wav");
    mDeathSound = LoadSound("assets/game/death_noise.wav");
    mVictorySound = LoadSound("assets/game/victory.wav");
    SetSoundVolume(mDeathSound, 0.33f);
    SetMusicVolume(mGameState.bgm, 0.33f);
    PlayMusicStream(mGameState.bgm);
    mGameState.jumpSound = LoadSound("assets/game/SFX_Jump_01.wav");

    //Chest goal + map visuals 
    Vector2 chestPos = {
        mOrigin.x - (LEVEL_WIDTH / 2.0f * TILE_DIMENSION) + 35 * TILE_DIMENSION,
        mOrigin.y - (LEVEL_HEIGHT / 2.0f * TILE_DIMENSION) + 6 * TILE_DIMENSION
    };
    mChest = new Entity(chestPos, {TILE_DIMENSION, TILE_DIMENSION}, "assets/game/chest.png", NPC);

    mGameState.map = new Map(
        LEVEL_WIDTH, LEVEL_HEIGHT,
        (unsigned int*)mLevelData,
        "assets/game/dirt.png",
        TILE_DIMENSION,
        1, 1,
        mOrigin
    );

    //Player setup: animations, collider, gravity, etc.
    std::map<Direction, std::vector<int>> playerAnimationAtlas = {
        {LEFT,  {8, 9, 10, 11}},
        {RIGHT, {12, 13, 14, 15}},
        {DOWN,  {0}},
        {UP,    {0}}
    };

    constexpr float PLAYER_WIDTH = TILE_DIMENSION;
    constexpr float PLAYER_HEIGHT = TILE_DIMENSION;

    mGameState.player = new Entity(
        {150.0f, 300.0f},
        {PLAYER_WIDTH, PLAYER_HEIGHT},
        "assets/game/character.jpg",
        ATLAS,
        {4, 4},
        playerAnimationAtlas,
        PLAYER
    );

    mGameState.player->setJumpingPower(600.0f);
    mGameState.player->setColliderDimensions({PLAYER_WIDTH * 0.6f, PLAYER_HEIGHT * 0.8f});
    mGameState.player->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});

    //Camera follows player side-scrolling style
    mGameState.camera = {0};
    mGameState.camera.target = mGameState.player->getPosition();
    mGameState.camera.offset = mOrigin;
    mGameState.camera.rotation = 0.0f;
    mGameState.camera.zoom = 1.0f;

    // Clear old enemies (if retrying level)
    for (Entity* e : mEnemies) delete e;
    mEnemies.clear();

    // Enemies: one of each different AI behavior for final challenge
    Entity* wanderer = new Entity({mOrigin.x + 200.0f, mOrigin.y - 200.0f}, {100.0f, 100.0f}, "assets/game/Ghost.png", NPC);
    wanderer->setAIType(WANDERER);
    wanderer->setAIState(WALKING);
    wanderer->setColliderDimensions({60.0f, 60.0f});
    wanderer->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});
    mEnemies.push_back(wanderer);

    Entity* follower = new Entity({mOrigin.x + 400.0f, mOrigin.y - 200.0f}, {100.0f, 100.0f}, "assets/game/follower.png", NPC);
    follower->setAIType(FOLLOWER);
    follower->setAIState(IDLE);
    follower->setSpeed(150.0f);
    follower->setColliderDimensions({60.0f, 60.0f});
    follower->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});
    mEnemies.push_back(follower);

    // Jumper enemy jumps around to reach player
    Entity* Bat = new Entity({mOrigin.x + 650.0f, mOrigin.y - 250.0f}, {100.0f, 100.0f}, "assets/game/bat.png", NPC);
    Bat->setAIType(JUMPER);
    Bat->setAIState(IDLE);
    Bat->setSpeed(150.0f);
    Bat->setJumpingPower(400.0f);
    Bat->setColliderDimensions({60.0f, 60.0f});
    Bat->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});
    mEnemies.push_back(Bat);
}

void LevelC::update(float deltaTime)
{
    UpdateMusicStream(mGameState.bgm);

    // Freeze world if player lost
    if (mGameState.showGameOver)
    {
        if (IsKeyPressed(KEY_ENTER))
        {
            // Reset lives and send player back to menu
            gLives = 3;
            mGameState.lives = gLives;
            mGameState.showGameOver = false;
            mGameState.nextSceneID = -1;
            
        }
        return;
    }

    //Standard gameplay
    if (mGameState.nextSceneID == 0)
    {
        mGameState.player->update(deltaTime, nullptr, mGameState.map, nullptr, 0);

        for (Entity* enemy : mEnemies)
            enemy->update(deltaTime, mGameState.player, mGameState.map, nullptr, 0);

        // Keep the camera smoothly tracking the player horizontally
        Vector2 currentPlayerPosition = {mGameState.player->getPosition().x, mOrigin.y};
        panCamera(&mGameState.camera, &currentPlayerPosition);

        // Death conditions: fall to your doom or get caught by danger
        bool fell = mGameState.player->getPosition().y > END_GAME_THRESHOLD;
        bool hitEnemy = false;
        for (Entity* enemy : mEnemies)
        {
            if (mGameState.player->collidesWith(enemy))
            {
                hitEnemy = true;
                break;
            }
        }

        if (fell || hitEnemy)
        {
            PlaySound(mDeathSound);

            if (gLives > 1)
            {
                // Try again — lose a life
                gLives--;
                mGameState.lives = gLives;
                this->initialise();
            }
            else
            {
                // Out of chances — game over screen
                gLives = 0;
                mGameState.lives = 0;
                mGameState.showGameOver = true;
            }
            return;
        }

        // Player reached chest — go to victory scene!
        if (mChest && mGameState.player->collidesWith(mChest))
        {
            PlaySound(mVictorySound);
            mGameState.nextSceneID = 4;
            return;
        }
    }
}

void LevelC::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode));

    // Game over overlay (victory handled in separate scene)
    if (mGameState.showGameOver)
    {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), {0, 0, 0, 180});

        const char* text = "GAME OVER";
        int fontSize = 60;
        int textWidth = MeasureText(text, fontSize);
        DrawText(text, GetScreenWidth()/2 - textWidth/2, GetScreenHeight()/2 - 50, fontSize, RED);

        const char* returnText = "Press ENTER to return to main menu";
        int smallFont = 24;
        int returnWidth = MeasureText(returnText, smallFont);
        DrawText(returnText, GetScreenWidth()/2 - returnWidth/2, GetScreenHeight()/2 + 30, smallFont, WHITE);
        return;
    }

    // Gameplay visuals: map, player, enemies, chest
    BeginMode2D(mGameState.camera);
    if (mGameState.map) mGameState.map->render();
    if (mGameState.player) mGameState.player->render();
    for (Entity* enemy : mEnemies) enemy->render();
    if (mChest) mChest->render();
    EndMode2D();

    // On-screen lives indicator
    char livesText[32];
    snprintf(livesText, sizeof(livesText), "Lives: %d", gLives);
    DrawText(livesText, 20, 20, 24, WHITE);
}

void LevelC::shutdown()
{
    // Free gameplay memory
    delete mGameState.player;
    delete mGameState.map;

    for (Entity* enemy : mEnemies)
        delete enemy;
    mEnemies.clear();

    if (mChest)
    {
        delete mChest;
        mChest = nullptr;
    }

    // Free audio resources
    UnloadMusicStream(mGameState.bgm);
    UnloadSound(mGameState.jumpSound);
    UnloadSound(mDeathSound);
    UnloadSound(mVictorySound);
}
