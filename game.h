#pragma once

#include <stdint.h>

#define ARM_POINT_LEN 10
#define ARM_POINT_RAD 10
#define ARM_POINT_COLOR 0x00FFFFFF

enum PlayerPose { POSE_STAND, POSE_SQUAT };

class Game
{
public:
    PlayerPose pose;
    int moving;
    int godMode;
    void Move(int m, int r, float seconds);

    float playerX, playerY, playerA;

    Game();
    ~Game();
};
