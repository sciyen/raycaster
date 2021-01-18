// floating-point implementation for testing/comparison

#include "raycaster_float.h"
#include <math.h>

uint8_t RayCasterFloat::IsWall(float rayX, float rayY)
{
    float mapX = 0;
    float mapY = 0;
    float offsetX = modff(rayX, &mapX);
    float offsetY = modff(rayY, &mapY);
    int tileX = static_cast<int>(mapX);
    int tileY = static_cast<int>(mapY);

    if (tileX < 0 || tileY < 0 || tileX >= MAP_X - 1 || tileY >= MAP_Y - 1) {
        return true;
    }

    return (g_map32[(tileX / OBSTACLES_PER_ELEMENT) +
                    (tileY * ELEMENTS_PER_ROW)] >>
            ((OBSTACLES_PER_ELEMENT - tileX % OBSTACLES_PER_ELEMENT) *
             OBSTACLE_SIZE)) &
           OBSTACLE_MASK;
}

float RayCasterFloat::Distance(float playerX,      // In, Player location X
                               float playerY,      // In, Player location Y
                               float rayA,         // In, Player location Angle
                               float *hitOffset,   // Out,
                               int *hitDirection)  // Out,
{
    // Player location angle normalizarion
    while (rayA < 0) {
        rayA += 2.0f * M_PI;
    }
    while (rayA >= 2.0f * M_PI) {
        rayA -= 2.0f * M_PI;
    }

    // Split the player location into fractional part(offset) and
    // integer part(tile).
    float rayX = playerX;
    float rayY = playerY;
    float tileX = 0;
    float tileY = 0;
    float offsetX = modff(rayX, &tileX);
    float offsetY = modff(rayY, &tileY);

    float vecX = 1 - offsetY;  // The case that 3pi/2 ~ pi/2
    float vecY = 1 - offsetX;  // The case that 0 ~ pi
    int tileStepX = 1;         // The case that 0 ~ pi
    int tileStepY = 1;         // The case that 3pi/2 ~ pi/2

    // Generate directional unit vector according to player angle
    if (rayA > M_PI) {
        tileStepX = -1;
        vecY = (offsetX == 0) ? 1 : offsetX;
    }
    if (rayA > M_PI_2 && rayA < 3 * M_PI_2) {
        tileStepY = -1;
        vecX = (offsetY == 0) ? 1 : offsetY;
    }

    // Calculate the starting delta
    float startDeltaX = vecX * tan(rayA) * tileStepY;
    float startDeltaY = vecY / tan(rayA) * tileStepX;

    float interceptX = rayX + startDeltaX;
    float interceptY = rayY + startDeltaY;
    float stepX = fabs(tan(rayA)) * tileStepX;
    float stepY = fabs(1 / tan(rayA)) * tileStepY;
    bool verticalHit = false;
    bool horizontalHit = false;
    bool somethingDone = false;

    do {
        somethingDone = false;
        while (((tileStepY == 1 && (interceptY <= tileY + 1)) ||
                (tileStepY == -1 && (interceptY >= tileY)))) {
            somethingDone = true;
            tileX += tileStepX;
            if (*hitDirection = IsWall(tileX, interceptY)) {
                verticalHit = true;
                rayX = tileX + (tileStepX == -1 ? 1 : 0);
                rayY = interceptY;
                *hitOffset = interceptY;
                // Use odd number to indicate different hit direction
                *hitDirection = (*hitDirection - 1) * 2 + 1;
                break;
            }
            interceptY += stepY;
        }
        while (!verticalHit && ((tileStepX == 1 && (interceptX <= tileX + 1)) ||
                                (tileStepX == -1 && (interceptX >= tileX)))) {
            somethingDone = true;
            tileY += tileStepY;
            if (*hitDirection = IsWall(interceptX, tileY)) {
                horizontalHit = true;
                rayX = interceptX;
                *hitOffset = interceptX;
                rayY = tileY + (tileStepY == -1 ? 1 : 0);
                *hitDirection = (*hitDirection - 1) * 2;
                break;
            }
            interceptX += stepX;
        }
    } while ((!horizontalHit && !verticalHit) && somethingDone);

    if (!somethingDone) {
        return 0;
    }

    float deltaX = rayX - playerX;
    float deltaY = rayY - playerY;
    return sqrt(deltaX * deltaX + deltaY * deltaY);
}

void RayCasterFloat::Trace(uint16_t screenX,
                           uint8_t *screenY,
                           uint8_t *textureNo,
                           uint8_t *textureX,
                           uint16_t *textureY,
                           uint16_t *textureStep)
{
    float hitOffset;
    int hitDirection;
    float deltaAngle = atanf(((int16_t) screenX - SCREEN_WIDTH / 2.0f) /
                             (SCREEN_WIDTH / (2.0f * tanf(FOV_X / 2.0f))));
    float lineDistance = Distance(_playerX, _playerY, _playerA + deltaAngle,
                                  &hitOffset, &hitDirection);
    float distance = lineDistance * cos(deltaAngle);
    float dum;
    *textureX = (uint8_t)(256.0f * modff(hitOffset, &dum));
    *textureNo = hitDirection;
    *textureY = 0;
    *textureStep = 0;
    if (distance > 0) {
        auto txs = 2.0f * INV_FACTOR / distance;
        if (txs > SCREEN_HEIGHT) {
            *screenY = SCREEN_HEIGHT / 2;
            *textureY = static_cast<uint16_t>(
                TEXTURE_SIZE * (txs - SCREEN_HEIGHT) / 2 / txs * (1 << 10));
        } else
            *screenY = txs / 2;
    } else {
        *screenY = 0;
    }
}

void RayCasterFloat::Start(uint16_t playerX, uint16_t playerY, int16_t playerA)
{
    _playerX = (playerX / 1024.0f) * 4.0f;
    _playerY = (playerY / 1024.0f) * 4.0f;
    _playerA = (playerA / 1024.0f) * 2.0f * M_PI;
}

RayCasterFloat::RayCasterFloat() : RayCaster() {}

RayCasterFloat::~RayCasterFloat() {}
