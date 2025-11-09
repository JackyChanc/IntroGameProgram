#include "Menu.h"

// Declare gLives as extern since it's defined in main.cpp
extern int gLives;

Menu::Menu() : Scene({800, 450}, "#2C3E50") 
{
    mGameState.nextSceneID = 0;
}

Menu::Menu(Vector2 origin, const char *bgHexCode) : Scene(origin, bgHexCode)
{
    mGameState.nextSceneID = 0;
}

Menu::~Menu() {}

void Menu::initialise() 
{
    
    
    // Initialise menu state and camera so BeginMode2D in main.cpp renders correctly
    
    mGameState.camera = { 0 };
    mGameState.camera.target = mOrigin;
    mGameState.camera.offset = mOrigin;
    mGameState.camera.rotation = 0.0f;
    mGameState.camera.zoom = 1.0f;
}

void Menu::update(float deltaTime) 
{
    if (IsKeyPressed(KEY_ENTER)) 
    {
        gLives = 3; // Reset lives when starting a new game
        mGameState.nextSceneID = 1;
    }
}

void Menu::render() 
{
    ClearBackground(ColorFromHex(mBGColourHexCode));

    // title
    const char *title = "PROJ 4 Rise of AI";
    int titleFontSize = 60;
    int titleWidth = MeasureText(title, titleFontSize);
    DrawText(title, (int)(mOrigin.x - titleWidth / 2), (int)(mOrigin.y - 80), titleFontSize, WHITE);

    // prompt
    const char *prompt = "Press ENTER to Start";
    int promptFontSize = 30;
    int promptWidth = MeasureText(prompt, promptFontSize);
    DrawText(prompt, (int)(mOrigin.x - promptWidth / 2), (int)(mOrigin.y + 10), promptFontSize, BLUE);
}

void Menu::shutdown() 
{
    
}
