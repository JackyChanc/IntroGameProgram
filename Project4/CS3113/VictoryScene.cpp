#include "VictoryScene.h"

VictoryScene::VictoryScene() : Scene({500, 300}, "#006400") {} // Dark green background

VictoryScene::VictoryScene(Vector2 origin, const char* bgHexCode) : Scene(origin, bgHexCode) {}

VictoryScene::~VictoryScene() {}

void VictoryScene::initialise() 
{
    // Reset lives for new game
    mGameState.lives = 3;
    mGameState.nextSceneID = 0;
}

void VictoryScene::update(float deltaTime) 
{
    // Press ENTER to go back to main menu
    if (IsKeyPressed(KEY_ENTER)) 
    {
        mGameState.nextSceneID = -1; // Go to main menu
    }
}

void VictoryScene::render() 
{
    ClearBackground(ColorFromHex(mBGColourHexCode));

    // Victory text
    const char* victoryText = "YOU WIN!";
    int victoryFontSize = 60;
    int victoryWidth = MeasureText(victoryText, victoryFontSize);
    DrawText(victoryText, GetScreenWidth()/2 - victoryWidth/2, GetScreenHeight()/2 - 50, victoryFontSize, GOLD);

    // Congratulations text
    const char* congratsText = "Congratulations! You completed all levels!";
    int congratsFontSize = 24;
    int congratsWidth = MeasureText(congratsText, congratsFontSize);
    DrawText(congratsText, GetScreenWidth()/2 - congratsWidth/2, GetScreenHeight()/2 + 20, congratsFontSize, WHITE);

    // Return prompt
    const char* returnText = "Press ENTER to return to main menu";
    int returnFontSize = 20;
    int returnWidth = MeasureText(returnText, returnFontSize);
    DrawText(returnText, GetScreenWidth()/2 - returnWidth/2, GetScreenHeight()/2 + 80, returnFontSize, WHITE);
}

void VictoryScene::shutdown() 
{
    // Nothing to clean up
}