#include "CS3113/cs3113.h"
#include <string>
#include <cstdlib>
#include <cmath>
using namespace std;

/**
* Author: Jacky Chan Chi Kee
* Assignment: Fire Emblem Pong
* Date due: 2025-10-13, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

//made by nintendo pls don't sue me
constexpr char BALL_FP[] = "assets/Claude_square.jpeg";          // Claude is the ball
constexpr char P1_PADDLE_FP[] = "assets/dimitri_square.jpeg";   // Player 1
constexpr char P2_PADDLE_FP[] = "assets/edelgard_square.jpeg";  // Player 2 / AI
constexpr char BACKGROUND_FP[] = "assets/Garreg_Mach_Monastery.jpeg"; // Background image

//constants
constexpr int SCREEN_WIDTH = 960;
constexpr int SCREEN_HEIGHT = 540;

constexpr int PADDLE_WIDTH = 25;
constexpr int PADDLE_HEIGHT = 80;
constexpr int BALL_SIZE = 20;

constexpr int PADDLE_SPEED = 250;   // How fast our heroes move
constexpr int BALL_SPEED = 350;     // How fast Claude flies
constexpr int FONT_SIZE = 40;
constexpr int MAX_BALLS = 3;        // chaos I can do anything

//padle and ball structs 
struct Paddle {
    Vector2 position;
    Vector2 scale;
    float speed;
    Texture2D texture;
};

struct Ball {
    Vector2 position;
    Vector2 velocity;
    Vector2 scale;
    float speed;
    Texture2D texture;
    bool isActive;
};

//switches and states
Paddle player1Paddle, player2Paddle;
Ball balls[MAX_BALLS];
GameStatus currentGameState = GameStatus(RUNNING);

bool isSinglePlayer = false;
int numActiveBalls = 1;              // Start with 1 ball


// textures for paddles and ball
Texture2D player1PaddleTexture;
Texture2D player2PaddleTexture;
Texture2D ballTexture;
Texture2D backgroundTexture;

//box to box collision 
bool isColliding(const Vector2 *positionA, const Vector2 *scaleA,
                 const Vector2 *positionB, const Vector2 *scaleB)
{
    float xDistance = fabs(positionA->x - positionB->x) - ((scaleA->x + scaleB->x) / 2.0f);
    float yDistance = fabs(positionA->y - positionB->y) - ((scaleA->y + scaleB->y) / 2.0f);

    return (xDistance < 0.0f && yDistance < 0.0f); // classic AABB collision
}

//reset ball when game start
void resetBall(Ball &ball) {
    ball.position = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f }; // center spawn
    float dirX = (rand() % 2 == 0) ? 1.0f : -1.0f;  // random left/right
    float dirY = (rand() % 2 == 0) ? 0.7f : -0.7f;  // small angle
    ball.velocity = { dirX, dirY };
}


void initialise() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "CS-UY 3113: Fire Emblem Pong");
    SetTargetFPS(60);
    srand(time(NULL));

    // Load textures for characters and ball
    player1PaddleTexture = LoadTexture(P1_PADDLE_FP);
    player2PaddleTexture = LoadTexture(P2_PADDLE_FP);
    ballTexture          = LoadTexture(BALL_FP);
    backgroundTexture    = LoadTexture(BACKGROUND_FP);

    // Setup paddles
    player1Paddle = { {PADDLE_WIDTH, SCREEN_HEIGHT/2.0f}, {PADDLE_WIDTH, PADDLE_HEIGHT}, (float)PADDLE_SPEED, player1PaddleTexture };
    player2Paddle = { {SCREEN_WIDTH - PADDLE_WIDTH, SCREEN_HEIGHT/2.0f}, {PADDLE_WIDTH, PADDLE_HEIGHT}, (float)PADDLE_SPEED, player2PaddleTexture };

    // Setup balls
    for (int i=0; i<MAX_BALLS; ++i) {
        balls[i] = { {SCREEN_WIDTH/2.0f, SCREEN_HEIGHT/2.0f}, {0,0}, {BALL_SIZE, BALL_SIZE}, (float)BALL_SPEED, ballTexture, false };
    }
    balls[0].isActive = true;
    resetBall(balls[0]);
}

//input
void processInput(float deltaTime) {
    // Player 1 controls
    if (IsKeyDown(KEY_W)) player1Paddle.position.y -= player1Paddle.speed * deltaTime;
    if (IsKeyDown(KEY_S)) player1Paddle.position.y += player1Paddle.speed * deltaTime;

    // Player 2 controls (if 2-player mode)
    if (!isSinglePlayer) {
        if (IsKeyDown(KEY_UP)) player2Paddle.position.y -= player2Paddle.speed * deltaTime;
        if (IsKeyDown(KEY_DOWN)) player2Paddle.position.y += player2Paddle.speed * deltaTime;
    }

    //Toggle single-player mode
    if (IsKeyPressed(KEY_T)) {
    isSinglePlayer = !isSinglePlayer;
}

    //num of claude
    if (IsKeyDown(KEY_ONE)) numActiveBalls = 1;
    if (IsKeyDown(KEY_TWO)) numActiveBalls = 2;
    if (IsKeyDown(KEY_THREE)) numActiveBalls = 3;

    //Activate/deactivate claude
    for (int i=0; i<MAX_BALLS; ++i) {
        if (i < numActiveBalls) {
            if (!balls[i].isActive) {
                balls[i].isActive = true;
                resetBall(balls[i]);
            }
        } else balls[i].isActive = false;
    }
}

//AI for lonely people like me
void updateAI(float deltaTime) {
    if (!isSinglePlayer) return;

    static int direction = 1; //1 down, -1 up

    //auto move edelgard
    player2Paddle.position.y += direction * player2Paddle.speed * deltaTime;

    //reverse direction if screen edge is hit
    if (player2Paddle.position.y < PADDLE_HEIGHT / 2.0f) {
        player2Paddle.position.y = PADDLE_HEIGHT / 2.0f;
        direction = 1;
    }
    else if (player2Paddle.position.y > SCREEN_HEIGHT - PADDLE_HEIGHT / 2.0f) {
        player2Paddle.position.y = SCREEN_HEIGHT - PADDLE_HEIGHT / 2.0f;
        direction = -1;
    }
}

//update
void update(float deltaTime) {
    if (currentGameState != RUNNING) return;

    updateAI(deltaTime);

    //Keep paddles in bounds
    if (player1Paddle.position.y < PADDLE_HEIGHT / 2.0f) player1Paddle.position.y = PADDLE_HEIGHT / 2.0f;
    if (player1Paddle.position.y > SCREEN_HEIGHT - PADDLE_HEIGHT / 2.0f) player1Paddle.position.y = SCREEN_HEIGHT - PADDLE_HEIGHT / 2.0f;
    if (player2Paddle.position.y < PADDLE_HEIGHT / 2.0f) player2Paddle.position.y = PADDLE_HEIGHT / 2.0f;
    if (player2Paddle.position.y > SCREEN_HEIGHT - PADDLE_HEIGHT / 2.0f) player2Paddle.position.y = SCREEN_HEIGHT - PADDLE_HEIGHT / 2.0f;

    // Update balls
    for (int i = 0; i < MAX_BALLS; ++i) {
        Ball &ball = balls[i];
        if (!ball.isActive) continue;

        // Move the ball
        ball.position.x += ball.velocity.x * ball.speed * deltaTime;
        ball.position.y += ball.velocity.y * ball.speed * deltaTime;

        //Wall collisions
        if (ball.position.y - BALL_SIZE / 2.0f < 0) { ball.position.y = BALL_SIZE / 2.0f; ball.velocity.y *= -1; }
        if (ball.position.y + BALL_SIZE / 2.0f > SCREEN_HEIGHT) { ball.position.y = SCREEN_HEIGHT - BALL_SIZE / 2.0f; ball.velocity.y *= -1; }

        // Paddle collisions
        if (isColliding(&player1Paddle.position, &player1Paddle.scale, &ball.position, &ball.scale) && ball.velocity.x < 0) ball.velocity.x *= -1;
        if (isColliding(&player2Paddle.position, &player2Paddle.scale, &ball.position, &ball.scale) && ball.velocity.x > 0) ball.velocity.x *= -1;

        // GAME OVAAAAAAA
        if (ball.position.x - BALL_SIZE / 2.0f < 0) { ball.isActive = false; currentGameState = GameStatus(GAME_OVER_P2_WIN); return; }
        if (ball.position.x + BALL_SIZE / 2.0f > SCREEN_WIDTH) { ball.isActive = false; currentGameState = GameStatus(GAME_OVER_P1_WIN); return; }
    }
}

//render
void render() {
    BeginDrawing();
    ClearBackground(BLACK);

    // Draw background
    DrawTexturePro(backgroundTexture,
                   {0, 0, (float)backgroundTexture.width, (float)backgroundTexture.height},
                   {SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT},
                   {SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f}, 0.0f, WHITE);

    // Draw paddles
    DrawTexturePro(player1Paddle.texture,
                   {0, 0, (float)player1Paddle.texture.width, (float)player1Paddle.texture.height},
                   {player1Paddle.position.x, player1Paddle.position.y, player1Paddle.scale.x, player1Paddle.scale.y},
                   {player1Paddle.scale.x / 2, player1Paddle.scale.y / 2}, 0.0f, WHITE);

    DrawTexturePro(player2Paddle.texture,
                   {0, 0, (float)player2Paddle.texture.width, (float)player2Paddle.texture.height},
                   {player2Paddle.position.x, player2Paddle.position.y, player2Paddle.scale.x, player2Paddle.scale.y},
                   {player2Paddle.scale.x / 2, player2Paddle.scale.y / 2}, 0.0f, WHITE);

    // Draw balls
    for (int i = 0; i < MAX_BALLS; ++i) {
        if (balls[i].isActive) {
            DrawTexturePro(balls[i].texture,
                           {0, 0, (float)balls[i].texture.width, (float)balls[i].texture.height},
                           {balls[i].position.x, balls[i].position.y, balls[i].scale.x, balls[i].scale.y},
                           {balls[i].scale.x / 2, balls[i].scale.y / 2}, 0.0f, WHITE);
        }
    }

    // UI 
    const char* modeText = isSinglePlayer ? "Mode: 1-Player (AI) | Press T to switch"
                                          : "Mode: 2-Player | Press T to switch";
    const char* ballText = TextFormat("Balls: %d | Press 1, 2, or 3", numActiveBalls);

    DrawText(modeText, 10, 10, 20, WHITE);
    DrawText(ballText, 10, 35, 20, WHITE);
    DrawText("Player 1: W/S", 10, SCREEN_HEIGHT - 30, 20, WHITE);
    DrawText("Player 2: UP/DOWN", SCREEN_WIDTH - 200, SCREEN_HEIGHT - 30, 20, WHITE);

    //Game Over 
    if (currentGameState != RUNNING) {
        const char* gameOverText = (currentGameState == GAME_OVER_P1_WIN)
                                   ? "PLAYER 1 WINS!"
                                   : "PLAYER 2 WINS!";
        DrawText(gameOverText, SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2 - 60, FONT_SIZE, WHITE);
        DrawText("Press R to Reset", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 10, 30, WHITE);
    }

    EndDrawing();
}


//shutdown
void shutdown() {
    UnloadTexture(player1PaddleTexture);
    UnloadTexture(player2PaddleTexture);
    UnloadTexture(ballTexture);
    UnloadTexture(backgroundTexture);
    CloseWindow();
}

//main
int main() {
    initialise();
    float gpreviousTicks = 0.0f;

    while (!WindowShouldClose()) {
        //typicak delta Tieme
        float ticks = (float)GetTime();
        float deltaTime = ticks - gpreviousTicks;
        gpreviousTicks = ticks;

        processInput(deltaTime);

        if (currentGameState != RUNNING && IsKeyDown(KEY_R)) {
            currentGameState = GameStatus(RUNNING);
            numActiveBalls = 1;
            balls[0].isActive = true;
            resetBall(balls[0]);
            for(int i=1;i<MAX_BALLS;++i) balls[i].isActive = false;
        }

        update(deltaTime);
        render();
    }

    shutdown();
    return 0;
}
