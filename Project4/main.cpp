/**
 * Author: [Jacky Chan]
 * Assignment: Rise of the AI
 * Date due: 2025-11-08, 11:59pm
 * I pledge that I have completed this assignment without
 * collaborating with anyone else, in conformance with the
 * NYU School of Engineering Policies and Procedures on
 * Academic Misconduct.
 **/

#include "CS3113/Menu.h"
#include "CS3113/LevelB.h"
#include "CS3113/LevelA.h"
#include "CS3113/LevelC.h" 
#include "CS3113/VictoryScene.h"

// Global Constants
constexpr int SCREEN_WIDTH     = 1000,
              SCREEN_HEIGHT    = 600,
              FPS              = 120,
              NUMBER_OF_LEVELS = 5; // menu + levelA + levelB + levelC + victory scene
              VictoryScene *gVictoryScene = nullptr;

constexpr Vector2 ORIGIN      = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
            
constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;

// Global Variables
AppStatus gAppStatus   = RUNNING;
float gPreviousTicks   = 0.0f,
      gTimeAccumulator = 0.0f;

Scene *gCurrentScene = nullptr;
std::vector<Scene*> gLevels = {};

LevelA *gLevelA = nullptr;
LevelB *gLevelB = nullptr;
LevelC *gLevelC = nullptr;
Menu  *gMenu   = nullptr;


// Global lives for the whole game (persist across levels)
int gLives = 3;

// Function Declarations
void switchToScene(Scene *scene);
void initialise();
void processInput();
void update();
void render();
void shutdown();

void switchToScene(Scene *scene)
{   
    gCurrentScene = scene;
    gCurrentScene->initialise();
    gCurrentScene->getState().nextSceneID = 0;
}

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Scenes");
    InitAudioDevice();

    // Create menu and levels. Menu is at index 0; levels follow.
    gMenu   = new Menu(ORIGIN, "#121212");
    gLevelA = new LevelA(ORIGIN, "#C0897E");
    gLevelB = new LevelB(ORIGIN, "#0a77cfff");
    gLevelC = new LevelC(ORIGIN, "#011627"); 
    gVictoryScene = new VictoryScene(ORIGIN, "#006400");
    
    gLevels.push_back(gMenu);   // 0 -> menu
    gLevels.push_back(gLevelA); // 1 -> level A
    gLevels.push_back(gLevelB); // 2 -> level B
    gLevels.push_back(gLevelC); // 3 -> level C
    gLevels.push_back(gVictoryScene); // 4 -> victory scene

    // Start at the menu
    switchToScene(gLevels[0]);

    SetTargetFPS(FPS);
}

void processInput() 
{

    // Only process player movement if the current scene has a player entity
    if (gCurrentScene->getState().player != nullptr)
    {
        gCurrentScene->getState().player->resetMovement();

        if      (IsKeyDown(KEY_A)) {gCurrentScene->getState().player->moveLeft();
                                    }
        else if (IsKeyDown(KEY_D)) {gCurrentScene->getState().player->moveRight();
                                    }

        if (IsKeyPressed(KEY_W) && 
            gCurrentScene->getState().player->isCollidingBottom())
        {
            gCurrentScene->getState().player->jump();
            PlaySound(gCurrentScene->getState().jumpSound);
        }

        if (GetLength(gCurrentScene->getState().player->getMovement()) > 1.0f) 
            gCurrentScene->getState().player->normaliseMovement();
    }

    if (IsKeyPressed(KEY_Q) || WindowShouldClose()) gAppStatus = TERMINATED;
}

void update() 
{
    float ticks = (float) GetTime();
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks  = ticks;

    deltaTime += gTimeAccumulator;

    if (deltaTime < FIXED_TIMESTEP)
    {
        gTimeAccumulator = deltaTime;
        return;
    }

    while (deltaTime >= FIXED_TIMESTEP)
    {
        gCurrentScene->update(FIXED_TIMESTEP);
        deltaTime -= FIXED_TIMESTEP;
    }
}

void render()
{
    BeginDrawing();

    gCurrentScene->render();

    EndDrawing();
}

void shutdown() 
{
    for (Scene* s : gLevels)
    delete s;
    gLevels.clear();
    

    CloseAudioDevice();
    CloseWindow();
}

int main(void)
{
    initialise();

    while (gAppStatus == RUNNING)
    {
        processInput();
        update();

        if (gCurrentScene->getState().nextSceneID != 0)
{
    int id = gCurrentScene->getState().nextSceneID;
    
    if (id == -1) {
        // Special case: go back to menu
        switchToScene(gLevels[0]);
    } else {
        printf("DEBUG: About to switch to scene %d, gLives = %d\n", id, gLives);
        switchToScene(gLevels[id]);
    }
}
    

        render();
    }

    shutdown();

    return 0;
}