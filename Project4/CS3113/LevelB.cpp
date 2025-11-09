#include "LevelB.h"
#include <map>

extern int gLives;

LevelB::LevelB() : Scene({0.0f}, nullptr)
{
    
}

LevelB::LevelB(Vector2 origin, const char* bgHexCode) : Scene(origin, bgHexCode) {}

LevelB::~LevelB()
{
    shutdown();
}

void LevelB::initialise()
{
    

    // Reset state so Level B behaves cleanly every time
    mSpawnTimer = 0.0f;
    mGameState.nextSceneID = 0;
    mGameState.showGameOver = false;
    mGameState.lives = gLives;

    // --- Background music + painful sound effects ---
    mGameState.bgm = LoadMusicStream("assets/game/BarrenLand.wav");
    mDeathSound = LoadSound("assets/game/death_noise.wav");
    SetSoundVolume(mDeathSound, 0.33f);
    SetMusicVolume(mGameState.bgm, 0.33f);
    PlayMusicStream(mGameState.bgm);
    mGameState.jumpSound = LoadSound("assets/game/SFX_Jump_01.wav");

    // Treasure chest placed slightly harder than Level A
    Vector2 chestPos = {
        mOrigin.x - (LEVEL_WIDTH / 2.0f * TILE_DIMENSION) + 33 * TILE_DIMENSION,
        mOrigin.y - (LEVEL_HEIGHT / 2.0f * TILE_DIMENSION) + 6 * TILE_DIMENSION
    };
    mChest = new Entity(chestPos, {TILE_DIMENSION, TILE_DIMENSION}, "assets/game/chest.png", NPC);

    // --- Bring in the world geometry so gravity actually matters ---
    mGameState.map = new Map(
        LEVEL_WIDTH, LEVEL_HEIGHT,
        (unsigned int*)mLevelData,
        "assets/game/dirt.png",
        TILE_DIMENSION,
        1, 1,
        mOrigin
    );

    // --- Player setup: upgraded jump for more heroic platforming ---
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

    mGameState.player->setJumpingPower(600.0f);  // Level B is jumpier
    mGameState.player->setColliderDimensions({PLAYER_WIDTH * 0.6f, PLAYER_HEIGHT * 0.8f});
    mGameState.player->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});

    // --- Camera: loyal companion watching from a fixed height ---
    mGameState.camera = {0};
    mGameState.camera.target = mGameState.player->getPosition();
    mGameState.camera.offset = mOrigin;
    mGameState.camera.rotation = 0.0f;
    mGameState.camera.zoom = 1.0f;

    // Clear any leftovers from previous unlucky lives
    for (Entity* e : mEnemies) delete e;
    mEnemies.clear();

    // --- Enemy Squad: Level B edition ---
    
    // Wanderer: aimless ghost who walks and regrets nothing
    Entity* wanderer = new Entity({mOrigin.x + 200.0f, mOrigin.y - 200.0f}, {100.0f, 100.0f}, "assets/game/Ghost.png", NPC);
    wanderer->setAIType(WANDERER);
    wanderer->setAIState(WALKING);
    wanderer->setColliderDimensions({60.0f, 60.0f});
    wanderer->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});
    mEnemies.push_back(wanderer);

    // Follower: casually stalks player when close enough
    Entity* follower = new Entity({mOrigin.x + 200.0f, mOrigin.y - 200.0f}, {100.0f, 100.0f}, "assets/game/follower.png", NPC);
    follower->setAIType(FOLLOWER);
    follower->setAIState(IDLE);
    follower->setSpeed(150.0f);
    follower->setColliderDimensions({60.0f, 60.0f});
    follower->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});
    mEnemies.push_back(follower);

    // Chaser: FOLLOWER but faster and angrier
    Entity* chaser = new Entity({mOrigin.x + 650.0f, mOrigin.y - 250.0f}, {100.0f, 100.0f}, "assets/game/follower.png", NPC);
    chaser->setAIType(FOLLOWER);
    chaser->setAIState(IDLE);
    chaser->setSpeed(180.0f);
    chaser->setColliderDimensions({60.0f, 60.0f});
    chaser->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});
    mEnemies.push_back(chaser);
}

void LevelB::update(float deltaTime)
{
    UpdateMusicStream(mGameState.bgm);

    // Game Over screen active → input locked except ENTER restart option
    if (mGameState.showGameOver)
    {
        if (IsKeyPressed(KEY_ENTER))
        {
            gLives = 3;
            mGameState.lives = gLives;
            mGameState.showGameOver = false;
            mGameState.nextSceneID = -1; // back to main menu
        }
        return;
    }

    if (mGameState.nextSceneID == 0)
    {
        // Main gameplay update loop
        mGameState.player->update(deltaTime, nullptr, mGameState.map, nullptr, 0);

        for (Entity* enemy : mEnemies)
            enemy->update(deltaTime, mGameState.player, mGameState.map, nullptr, 0);

        // Keep camera centered-ish horizontally
        Vector2 currentPlayerPosition = {mGameState.player->getPosition().x, mOrigin.y};
        panCamera(&mGameState.camera, &currentPlayerPosition);

        // Player death causes: falling to the abyss or high-fiving a ghost
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
                gLives--;
                mGameState.lives = gLives;
                
                this->initialise(); // respawn with fresh souls
            }
            else
            {
                gLives = 0;
                mGameState.lives = 0;
                mGameState.showGameOver = true;
            }
        }

        // Win if player reaches the chest and claims the loot
        if (mChest && mGameState.player->collidesWith(mChest))
            mGameState.nextSceneID = 3;
    }
}

void LevelB::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode));

    // Show harsh reality when player loses all lives
    if (mGameState.showGameOver)
    {
        const char* gameOverText = "GAME OVER";
        int fontSize = 60;
        int textWidth = MeasureText(gameOverText, fontSize);
        DrawText(gameOverText, GetScreenWidth() / 2 - textWidth / 2,
                 GetScreenHeight() / 2 - 50, fontSize, RED);

        const char* returnText = "Press ENTER to return to main menu";
        int smallFontSize = 24;
        int returnWidth = MeasureText(returnText, smallFontSize);
        DrawText(returnText, GetScreenWidth() / 2 - returnWidth / 2,
                 GetScreenHeight() / 2 + 30, smallFontSize, WHITE);
        return;
    }

    // World-space rendering via camera
    BeginMode2D(mGameState.camera);
    if (mGameState.map) mGameState.map->render();
    if (mGameState.player) mGameState.player->render();
    for (Entity* enemy : mEnemies) enemy->render();
    if (mChest) mChest->render();
    EndMode2D();

    // UI showing remaining survival chances
    char livesText[32];
    snprintf(livesText, sizeof(livesText), "Lives: %d", gLives);
    DrawText(livesText, 20, 20, 24, WHITE);
}

void LevelB::shutdown()
{
    // Cleanup so system RAM doesn't cry
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

    // Properly unload audio assets too
    UnloadMusicStream(mGameState.bgm);
    UnloadSound(mGameState.jumpSound);
    UnloadSound(mDeathSound);
}
