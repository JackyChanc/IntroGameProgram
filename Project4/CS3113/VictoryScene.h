#ifndef VICTORYSCENE_H
#define VICTORYSCENE_H

#include "Scene.h"

class VictoryScene : public Scene {
public:
    VictoryScene();
    VictoryScene(Vector2 origin, const char* bgHexCode);
    ~VictoryScene();

    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;
};

#endif // VICTORYSCENE_H