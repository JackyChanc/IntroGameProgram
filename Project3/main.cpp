#include "CS3113/Entity.h"

/**
* Author: [Jacky Chan]
* Assignment: Lunar Lander! 
* Date due: 25-10-2025
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

// Global Constants
constexpr int SCREEN_WIDTH  = 1000;
constexpr int SCREEN_HEIGHT = 600;
constexpr int FPS           = 120;

constexpr char    BG_COLOUR[] = "#C0897E";
constexpr Vector2 ORIGIN      = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
float gFuel = 100.0f;          // fuel in percent
bool gMissionComplete = false;
bool gMissionFailed   = false;
float gMoveTime = 0.0f;        // for moving platform animation
constexpr float THRUSTER_ACCEL_X = 50.0f; // Horizontal acceleration (tune this)
constexpr float THRUSTER_ACCEL_Y = 120.0f; // Vertical thrust power (must be > 15.0f)

constexpr int   NUMBER_OF_TILES         = 20;
constexpr int   NUMBER_OF_BLOCKS        = 3;
constexpr float TILE_DIMENSION          = 50.0f;
// in m/ms², since delta time is in ms
constexpr float ACCELERATION_OF_GRAVITY = 0.5f;
constexpr float FIXED_TIMESTEP          = 1.0f / 60.0f;
constexpr float END_GAME_THRESHOLD      = 800.0f;

// Global Variables
AppStatus gAppStatus   = RUNNING;
float gPreviousTicks   = 0.0f;
float gTimeAccumulator = 0.0f;
float endTimer = 0.0f;

Entity *gRocket = nullptr;
Entity *gTiles   = nullptr;
Entity *gBlocks  = nullptr;

bool gIsPlatformActive = true;

// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Physics");

    // ----------- PROTAGONIST -----------
    std::map<Direction, std::vector<int>> animationAtlas = {
        {DOWN,  {0}},
        {LEFT,  {1}},
        {UP,    {2}},
        {RIGHT, {3}},
        {FLAME_DOWN,  {4}},   
        {FLAME_LEFT,  {5}},  
        {FLAME_UP,    {6}},  
        {FLAME_RIGHT, {7}},
    };

    float sizeRatio  = 1.0f;

    gRocket = new Entity(
        {300.0f, 50.0f}, // position
        {100.0f * sizeRatio, 120.0f},  // scale
        "assets/game/rocket.png",        // texture file address
        ATLAS,                         // atlas type
        { 2, 4 },                      // atlas dimensions
        animationAtlas,                // actual atlas
        PLAYER                         // entity type
    );

    gRocket->setJumpingPower(450.0f);
    gRocket->setColliderDimensions({
        gRocket->getScale().x / 2.0f,
        gRocket->getScale().y / 2.0f
    });
    gRocket->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});

    
    gTiles = new Entity[NUMBER_OF_TILES];// game over tiles 

    // Compute the left‑most x coordinate so that the entire row is centred
    float leftMostX = ORIGIN.x - (NUMBER_OF_TILES * TILE_DIMENSION) / 2.0f;

    for (int i = 0; i < NUMBER_OF_TILES; i++) 
    {
        gTiles[i].setTexture("assets/game/lava.png");
        gTiles[i].setEntityType(PLATFORM);
        gTiles[i].setScale({TILE_DIMENSION, TILE_DIMENSION});
        gTiles[i].setColliderDimensions({TILE_DIMENSION, TILE_DIMENSION});
        gTiles[i].setPosition({
            leftMostX + i * TILE_DIMENSION, 
            ORIGIN.y +220.0f + TILE_DIMENSION /2.0f
        });
    }

    
    gBlocks = new Entity[NUMBER_OF_BLOCKS]; // land on these to be safe

    for (int i = 0; i < NUMBER_OF_BLOCKS; i++) 
    {
        gBlocks[i].setTexture("assets/game/tile_0061.png");
        gBlocks[i].setEntityType(PLATFORM);;
        gBlocks[i].setScale({TILE_DIMENSION, TILE_DIMENSION});
        gBlocks[i].setColliderDimensions({TILE_DIMENSION, TILE_DIMENSION});
    }

    gBlocks[0].setPosition({ ORIGIN.x - TILE_DIMENSION * 3, ORIGIN.y +200.0f - TILE_DIMENSION * 3.5f });
    gBlocks[1].setPosition({ ORIGIN.x,                     ORIGIN.y +200.0f - TILE_DIMENSION * 2.0f });
    gBlocks[2].setPosition({ ORIGIN.x + TILE_DIMENSION * 3, ORIGIN.y +200.0f - TILE_DIMENSION * 4.0f });
    
    SetTargetFPS(FPS);
}

void processInput()
{
    Vector2 accel = {0.0f, ACCELERATION_OF_GRAVITY};

    if (gFuel > 0.0f)
    {
        
        if (IsKeyDown(KEY_A)) { accel.x = -THRUSTER_ACCEL_X; gFuel -= 0.05f; }  // Horizontal thrust
        else if (IsKeyDown(KEY_D)) { accel.x =  THRUSTER_ACCEL_X; gFuel -= 0.05f; } 
    }

    
    if (gFuel < 0.0f) gFuel = 0.0f; // You don't have infinite fuel

    gRocket->setAcceleration(accel);

    if (IsKeyPressed(KEY_Q) || WindowShouldClose())
        gAppStatus = TERMINATED;
}

void update() 
{
    // Delta time
    float ticks = (float) GetTime();
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks  = ticks;
    
    if (gMissionComplete || gMissionFailed)
    {
        gRocket->setVelocity({0.0f, 0.0f});
        gRocket->setAcceleration({0.0f, 0.0f});
        
        
        return; 
    }
    Vector2 rocketPos = gRocket->getPosition();
    Vector2 rocketScale = gRocket->getScale();

    float leftBound = 0;
    float rightBound = SCREEN_WIDTH;
    float topBound = 0;
    float bottomBound = SCREEN_HEIGHT;
    if (rocketPos.x < leftBound || rocketPos.x > rightBound || 
    rocketPos.y < topBound || rocketPos.y > bottomBound) {
    gMissionFailed = true; //mission failllllll, keep rocket on screen
}


    deltaTime += gTimeAccumulator; //delta time plus leftover time

    if (deltaTime < FIXED_TIMESTEP)
    {
        gTimeAccumulator = deltaTime;
        return;
    }
    
    
    gMoveTime += FIXED_TIMESTEP; //whoa moving platform
    float moveRange = 100.0f;
    float baseX = ORIGIN.x;
    gBlocks[1].setPosition({ baseX + sin(gMoveTime) * moveRange,
                            ORIGIN.y - TILE_DIMENSION * 2.5f });
    

    
    if (gRocket->isCollidingBottom()) //landed
    {
        float vx = fabs(gRocket->getVelocity().x);
        float vy = fabs(gRocket->getVelocity().y);

        
        for (int i = 0; i < NUMBER_OF_BLOCKS; i++)// oh did we land on lava or block?
        {
            Vector2 blockPos = gBlocks[i].getPosition();
            
            
            if (fabs(gRocket->getPosition().x - blockPos.x) < 30.0f &&
                fabs(gRocket->getPosition().y - blockPos.y) < 60.0f)
            {
                
                
                
                if (vy < 100.0f && vx < 60.0f) //lets gooooo
                {
                    gMissionComplete = true;
                }
                
                else // nooooo, the guy crashed
                {
                    gMissionFailed = true;
                }

                break; 
            }
        }

        
        for (int i = 0; i < NUMBER_OF_TILES; i++) // If rocket hit lava, it crashes what a suprise
        {
            Vector2 tilePos = gTiles[i].getPosition();
            if (fabs(gRocket->getPosition().x - tilePos.x) < 30.0f &&
                fabs(gRocket->getPosition().y - tilePos.y) < 60.0f)
            {
                gMissionFailed = true;
            }
        }
    }
    
    gRocket->setDirection(DOWN); // Default direction
    bool thrusting_left =  IsKeyDown(KEY_A);
    bool thrusting_right = IsKeyDown(KEY_D);
    if (thrusting_left)
    {
        gRocket->setDirection(FLAME_LEFT);  // its going left
    }
    else if (thrusting_right)
    {
        gRocket->setDirection(FLAME_RIGHT); // its going right
    }
    else if (!thrusting_left && !thrusting_right)
    {
        gRocket->setDirection(DOWN); // its going down
    }
    
    while (deltaTime >= FIXED_TIMESTEP)
    {
        gRocket->update(FIXED_TIMESTEP, gTiles, NUMBER_OF_TILES, gBlocks,
            NUMBER_OF_BLOCKS);

        gIsPlatformActive = false;

        for (int i = 0; i < NUMBER_OF_BLOCKS; i++) 
        {
            gBlocks[i].update(FIXED_TIMESTEP, nullptr, 0, nullptr, 0);
            if (gBlocks[i].isActive()) gIsPlatformActive = true;
        }

        for (int i = 0; i < NUMBER_OF_TILES; i++) 
        {
            if (!gIsPlatformActive) gTiles[i].deactivate();
            gTiles[i].update(FIXED_TIMESTEP, nullptr, 0, nullptr, 0);
        }

        deltaTime -= FIXED_TIMESTEP;
    }
}

void render(){
    BeginDrawing();
    ClearBackground(ColorFromHex(BG_COLOUR));

    gRocket->render();

    for (int i = 0; i < NUMBER_OF_TILES;  i++) gTiles[i].render();
    for (int i = 0; i < NUMBER_OF_BLOCKS; i++) gBlocks[i].render();
    
    DrawRectangle(20, 20, 200, 20, GRAY);
    DrawRectangle(20, 20, gFuel * 2, 20, GREEN);
    DrawText(TextFormat("Fuel: %.0f%%", gFuel), 230, 18, 20, BLACK);

    
    if (gMissionComplete)
        DrawText("MISSION ACCOMPLISHED!", ORIGIN.x - 200, ORIGIN.y - 50, 40, GREEN); //yay
    else if (gMissionFailed)
        DrawText("MISSION FAILED!", ORIGIN.x - 150, ORIGIN.y - 50, 40, RED); // booo

    EndDrawing();
}

void shutdown() 
{ 
    CloseWindow();
}

int main(void)
{
    initialise();

    while (gAppStatus == RUNNING)
    {
        processInput();
        update();
        render();
        if (gMissionComplete || gMissionFailed)
        {
            endTimer += GetFrameTime();
            if (endTimer >= 10.0f) // wait 10 seconds
                gAppStatus = TERMINATED;
        }
    }

    shutdown();

    return 0;
}