#include <cmath>
#include <cstdlib>

#include "game.h"
#include "raycaster.h"
#include "raycaster_data.h"

void Game::Move(int m, int r, float seconds)
{
    moving = m;
    playerA += 0.05f * r * seconds * 25.0f;

    float try_move_x = 0.5f * m * sin(playerA) * seconds * 5.0f;
    float try_move_y = 0.5f * m * cos(playerA) * seconds * 5.0f;
    int x = playerX + try_move_x;
    int y = playerY + try_move_y;
    if (((g_map32[(x / OBSTACLES_PER_ELEMENT) + (y * ELEMENTS_PER_ROW)] >>
          ((OBSTACLES_PER_ELEMENT - x % OBSTACLES_PER_ELEMENT) *
           OBSTACLE_SIZE)) &
         OBSTACLE_MASK) == 0) {
        playerX += try_move_x;
        playerY += try_move_y;
    } else {
        playerX -= try_move_x;
        playerY -= try_move_y;
    }

    while (playerA < 0) {
        playerA += 2.0f * M_PI;
    }
    while (playerA >= 2.0f * M_PI) {
        playerA -= 2.0f * M_PI;
    }

    if (playerX < 1) {
        playerX = 1.01f;
    } else if (playerX > MAP_X - 2) {
        playerX = MAP_X - 2 - 0.01f;
    }
    if (playerY < 1) {
        playerY = 1.01f;
    } else if (playerY > MAP_Y - 2) {
        playerY = MAP_Y - 2 - 0.01f;
    }
}

Game::Game()
{
    pose = POSE_STAND;
    moving = 0;
    godMode = 0;
    playerX = 23.03f;
    playerY = 6.8f;
    playerA = 5.25f;
}

Game::~Game() {}
