#include "CS3113/cs3113.h"
#include <math.h>

/**
* Author: Jacky Chan Chi Kee
* Assignment: Simple 2D Scene
* Date due: 2025-09-27, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

//global vars
constexpr int SCREEN_WIDTH  = 1600;
constexpr int SCREEN_HEIGHT = 900;
constexpr int FPS           = 60;
constexpr float LIMIT_ANGLE = 180.0f;

//background color what a suprise
char    BG_COLOUR[] = "#000000";

//speed of edelgard orbiting dimitri
constexpr float ORBIT_RADIUS  = 200.0f;
constexpr float ORBIT_SPEED   = 0.05f;



// how fast dimitri has an heart attack
constexpr float PULSE_SPEED   = 4.0f;
constexpr float PULSE_AMOUNT  = 20.0f;

float gPrevticks     = 0.0f;

constexpr Vector2 ORIGIN = { SCREEN_WIDTH/2.0f, SCREEN_HEIGHT/2.0f };

//file paths
constexpr char LionPath[]      = "assets/dimitri_square.jpeg";
constexpr char EdelgardPath[]  = "assets/edelgard_square.jpeg";
constexpr char ClaudePath[]    = "assets/claude_square.jpeg";
bool goback = false;
AppStatus gAppStatus = RUNNING;

enum Direction { LEFT, RIGHT }; 

Texture2D gLion, gEdelgard, gClaude;

// dimitri starts in the middle
Vector2 gLionPos = ORIGIN;
Vector2 gLionScale = {100, 100};
float gLionPulseTime = 0.0f;

// edelgard annoying dimitri
Vector2 gEdelPos = gLionPos;
float gEdelAngle = 0.0f;

// claude doing his own thing
Vector2 gClaudePos = { SCREEN_WIDTH/2, SCREEN_HEIGHT/2 };
float gClaudeSpinAngle = 180.0f;
float gClaudeAngle = 0.0f;
Direction claudeDirection = RIGHT;    
Vector2 gClaudeScale = {100, 100};

void initialise() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Simple 2D Scene");
    SetTargetFPS(FPS);

    gLion = LoadTexture(LionPath);
    gEdelgard = LoadTexture(EdelgardPath);
    gClaude = LoadTexture(ClaudePath);
}

void processInput() {
    if (WindowShouldClose()) gAppStatus = TERMINATED;
}

void update() {
    //delta time for real time stuff
    float ticks = (float) GetTime();
    float deltaTime = ticks - gPrevticks;
    gPrevticks = ticks; 
    
    // pulsing, moving and background color change once dimitri reaches corner
    if (gLionPos.x < SCREEN_WIDTH && gLionPos.y < SCREEN_HEIGHT &goback==false) { //wow amazing I am using a flag
        gLionPos.x += 100.0f * deltaTime;
        gLionPos.y -= 50.0f * deltaTime;
        BG_COLOUR[1] = 'F';
    }
    if (gLionPos.x >= SCREEN_WIDTH || gLionPos.y <= 0) goback = true;
    if (goback==true){
        gLionPos.x -= 100.0f * deltaTime;
        gLionPos.y += 50.0f * deltaTime;
        BG_COLOUR[1] = '0';
        if (gLionPos.x <= 0 || gLionPos.y >= SCREEN_HEIGHT) goback = false;
    }
    
    gLionPulseTime += deltaTime * PULSE_SPEED;
    gLionScale.x = 100 + sin(gLionPulseTime) * PULSE_AMOUNT;
    gLionScale.y = 100 + sin(gLionPulseTime) * PULSE_AMOUNT;
    

    // edelguard orbiting dimitri 
    gEdelAngle += ORBIT_SPEED;
    gEdelPos.x = gLionPos.x + ORBIT_RADIUS * cos(gEdelAngle);
    gEdelPos.y = gLionPos.y + ORBIT_RADIUS * sin(gEdelAngle);

    // claude doing his own thing while ignoring dimitri and edelgards feud
    gClaudeSpinAngle += 100.0f * deltaTime * (claudeDirection == RIGHT ? 1 : -1);

    // switch direction, once limit is reached, keep him upside down lol
    if (gClaudeSpinAngle >  LIMIT_ANGLE) {
    gClaudeSpinAngle = LIMIT_ANGLE;
    claudeDirection = LEFT;}
    else if (gClaudeSpinAngle < -LIMIT_ANGLE) {
    gClaudeSpinAngle = -LIMIT_ANGLE;
    claudeDirection = RIGHT;}
    gClaudeAngle += ORBIT_SPEED/ 2.0f; 

    //ellipse is the same formula but different radius for x,y
    float orbitRadiusX = 300.0f;   
    float orbitRadiusY = 150.0f;   

    gClaudePos.x = SCREEN_WIDTH/2.0f + orbitRadiusX * cos(gClaudeAngle);
    gClaudePos.y = SCREEN_HEIGHT/2.0f + orbitRadiusY * sin(gClaudeAngle);
}

void render() {
    BeginDrawing();
    
    ClearBackground(ColorFromHex(BG_COLOUR));

    //where dimtiri starts in the program
    Rectangle lionDest = { 
        gLionPos.x, 
        gLionPos.y, 
        gLionScale.x, 
        gLionScale.y };

    Vector2 lionOrigin = { 
        gLionScale.x / 2.0f,
         gLionScale.y / 2.0f };

    //drawing the whole picture of dimitri
    Rectangle LionTextureArea{
        0.0f,
        0.0f, 
        static_cast<float>(gLion.width), 
        static_cast<float>(gLion.height),
    };
    //the classic from lecture 2/3
    DrawTexturePro(
        gLion,
        LionTextureArea,
        lionDest,
        lionOrigin,
        0.0f,
        WHITE
    );

    //where edelgard starts in the program

    Rectangle edelDest = { 
        gEdelPos.x, 
        gEdelPos.y, 
        100.0f, 
        100.0f };

    Vector2 edelOrigin = {
         50.0f, 
         50.0f };

    
    //drawing the whole picture of edelgard

    Rectangle edelArea{
        0.0f,
        0.0f, 
        static_cast<float>(gEdelgard.width), 
        static_cast<float>(gEdelgard.height),
    };
    //the classic yadadada
    DrawTexturePro(
        gEdelgard,
        edelArea,
        edelDest,
        edelOrigin,
        0.0f,
        WHITE
    );

    //where claude starts in the program
    Rectangle claudeDest = { 
        gClaudePos.x, 
        gClaudePos.y, 
        gClaudeScale.x, 
        gClaudeScale.y };
    
    Vector2 claudeOrigin = {
         gClaudeScale.x / 2.0f, 
        gClaudeScale.y / 2.0f };
    //drawing the whole picture of claude
    Rectangle claudeArea{
        0.0f,
        0.0f, 
        static_cast<float>(gClaude.width), 
        static_cast<float>(gClaude.height),
    };
    // you know the drill
    DrawTexturePro(
        gClaude,
        claudeArea,
        claudeDest,
        claudeOrigin,
        gClaudeSpinAngle,
        WHITE
    );

    EndDrawing();
}


void shutdown() {
    UnloadTexture(gLion);
    UnloadTexture(gEdelgard);
    UnloadTexture(gClaude);
    CloseWindow();
}

int main() {
    initialise();
    while (gAppStatus == RUNNING) {
        processInput();
        update();
        render();
    }
    shutdown();
    return 0;
}


