#pragma once

#include <stdint.h>

/* specify the precalcuated tables */
#define TABLES_320

#define SCREEN_WIDTH (uint16_t) 320
#define SCREEN_HEIGHT (uint16_t) 256
#define SCREEN_SCALE 2

// Field of view
#define FOV_X (double) (M_PI / 2)
#define FOV_Y (double) (M_PI / 2)

#define WALL_HEIGHT 1.0f
#define INV_FACTOR \
    (float) (WALL_HEIGHT * SCREEN_WIDTH / (4.0f * tanf(FOV_X / 2)))
//#define INV_FACTOR (float) (SCREEN_WIDTH * 95.0f / 320.0f)

#define LOOKUP_TBL
#define LOOKUP8(tbl, offset) tbl[offset]
#define LOOKUP16(tbl, offset) tbl[offset]

#define TEXTURE_XS 6
#define TEXTURE_SIZE (uint16_t)(1 << TEXTURE_XS)  // 2^6

// The datatype used to restore map (g_map)
#define MAP_ELEMENT_SIZE 3  // 2^3 = 8, for uint8_t
//#define MAP_ELEMENT_SIZE 5  // 2^5 = 32, for uint32_t

// Map mask for obtaining bits in x axis
#define MAP_ELEMENT_MASK ((1 << MAP_ELEMENT_SIZE) - 1)

#define MAP_XS (uint8_t) 5
// Map x direction size (number of bits)
#define MAP_X (uint8_t)(2 << MAP_XS)
#define MAP_Y (uint8_t) 32
#define INV_FACTOR_INT ((uint16_t)(SCREEN_WIDTH * 75))
#define MIN_DIST (int) ((150 * ((float) SCREEN_WIDTH / (float) SCREEN_HEIGHT)))
#define HORIZON_HEIGHT (SCREEN_HEIGHT / 2)
#define INVERT(x) (uint8_t)((x ^ 255) + 1)
#define ABS(x) (x < 0 ? -x : x)

class RayCaster
{
public:
    virtual void Start(uint16_t playerX, uint16_t playerY, int16_t playerA) = 0;

    virtual void Trace(uint16_t screenX,
                       uint8_t *screenY,
                       uint8_t *textureNo,
                       uint8_t *textureX,
                       uint16_t *textureY,
                       uint16_t *textureStep) = 0;

    RayCaster(){};

    ~RayCaster(){};
};
