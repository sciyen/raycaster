#pragma once

#include <stdint.h>

#define ARM_POINT_LEN 10
#define ARM_POINT_RAD 10
#define ARM_POINT_COLOR 0x00FFFFFF

class Game
{
public:
    int moving;
    void Move(int m, int r, float seconds);

    float playerX, playerY, playerA;

    Game();
    ~Game();
};
