#ifndef LEVELC_H
#define LEVELC_H

#include "LevelA.h"  // For LEVEL_WIDTH and LEVEL_HEIGHT
#include <vector>

class LevelC : public Scene {
private:
    // Level layout 
    unsigned int mLevelData[LEVEL_WIDTH * LEVEL_HEIGHT] = {
        1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,
        1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        1,0,0,0,0,0,0,1,1,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,1,1,1,1,1,1,1,1,1,
        1,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        1,1,1,1,1,1,1,1,1,0,0,1,0,1,1,0,0,0,0,0,0,1,0,1,1,1,1,1,1,1
    };

    std::vector<Entity*> mEnemies;
    Entity* mChest = nullptr;
    float mSpawnTimer = 0.0f;
    Sound mDeathSound = {0};
    Sound mVictorySound = {0};

public:
    static constexpr float TILE_DIMENSION = 50.0f;
    static constexpr float ACCELERATION_OF_GRAVITY = 981.0f;
    static constexpr float END_GAME_THRESHOLD = 800.0f;

    LevelC();
    LevelC(Vector2 origin, const char* bgHexCode);
    ~LevelC();

    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;
};

#endif // LEVELC_H