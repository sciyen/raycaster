#include "renderer.h"
#include <math.h>
#include "raycaster_data.h"

uint16_t Renderer::RecursiveTraceFrame(
    uint32_t *fb,    // In, Frame buffer
    int x,           // In, screen X
    uint16_t up,     // In, upper screen position
    uint16_t down,   // In, lower screen position
    uint8_t offset)  // In, downscale
{
    uint8_t sso;   // top point of wall
    uint8_t tc;    // x axis of texture (256 -> 64)
    uint8_t tn;    // texture number
    uint16_t tso;  // y axis of texture
    uint16_t tst;  // texture step (currentlt not used)
    uint32_t *lb = fb + x + up * SCREEN_WIDTH;  // frame buffer

    _rc->Trace(x, &sso, &tn, &tc, &tso, &tst);

    auto tx = static_cast<int>(tc >> 2);
    if (sso >= HORIZON_HEIGHT)
        sso = HORIZON_HEIGHT;

    // render top sky
    for (int y = up; y < HORIZON_HEIGHT - sso; y++) {
        if (offset > 0)
            *lb = DOWN_SCALE(*lb) +
                  DOWN_SCALE(GetARGB(96 + (HORIZON_HEIGHT - y)));
        else
            *lb = GetARGB(96 + (HORIZON_HEIGHT - y));
        lb += SCREEN_WIDTH;
    }

    // render obstacle
    for (int y = 0; y < sso * 2; y++) {
        // paint texture pixel
        uint16_t ty =
            (tso + (((TEXTURE_SIZE / 2) << 10) - tso) * y / sso) >> 10;

        uint16_t tv;
        uint32_t color;
        if (tn / 2 < NUMBER_OF_GRAY_TEXTURE) {
            // gray scale texture
            tv = (g_texture_gray[tn / 2])[(ty << 6) + tx];
            color = GetARGB(tv);
        } else {
            // colored texture
            tv = (g_texture_color[tn / 2 - 1])[(ty << 6) + tx];
            color = GetARGB_color(tv);
        }

        if (offset > 0)
            *lb = DOWN_SCALE(*lb) + DOWN_SCALE(color);
        else
            *lb = color;

        lb += SCREEN_WIDTH;
    }

    // render bottom sky
    for (int y = HORIZON_HEIGHT + sso; y < down; y++) {
        if (offset > 0)
            *lb =
                DOWN_SCALE(*lb) + DOWN_SCALE(GetARGB(96 + y - HORIZON_HEIGHT));
        else
            *lb = GetARGB(96 + y - HORIZON_HEIGHT);
        lb += SCREEN_WIDTH;
    }
    return sso;
}

void Renderer::TraceFrame(Game *g, uint32_t *fb)
{
    _rc->Start(static_cast<uint16_t>(g->playerX * 256.0f),
               static_cast<uint16_t>(g->playerY * 256.0f),
               static_cast<int16_t>(g->playerA / (2.0f * M_PI) * 1024.0f));

    for (int x = 0; x < SCREEN_WIDTH; x++) {
        uint16_t sso = RecursiveTraceFrame(fb, x, 0, SCREEN_HEIGHT, 0);
        if (g->godMode > 0)
            sso = RecursiveTraceFrame(fb, x, HORIZON_HEIGHT - sso,
                                      HORIZON_HEIGHT + sso, 1);
    }
}

void Renderer::RenderGame(Game *g, uint32_t *fb)
{
    static float time = 0, offset = 0;
    if (g->moving > 0) {
        time += 0.02f;
        offset = (10 * (sin(time) + 1));
    } else {
        offset *= 0.99f;
    }

    // rendering hand and gun
    if (g->pose == POSE_SQUAT) {
        const int sx = SCREEN_WIDTH / 2 - TEXTURE_GUN_CENTER_WIDTH / 2 + 1;
        const int sy = SCREEN_HEIGHT - TEXTURE_GUN_CENTER_HEIGHT;
        uint32_t *lb = fb + sy * SCREEN_WIDTH + sx;
        for (int j = 0; j < TEXTURE_GUN_CENTER_HEIGHT; j++) {
            for (int i = 0; i < TEXTURE_GUN_CENTER_WIDTH; i++) {
                auto tv =
                    g_texture_gun_center[j * TEXTURE_GUN_CENTER_WIDTH + i];

                if ((tv & 0x8000) == 0)
                    *lb = GetARGB_color(tv);
                lb++;
            }
            lb += SCREEN_WIDTH - TEXTURE_GUN_SIDE_WIDTH;
        }
    } else {
        uint32_t *lb =
            fb +
            (SCREEN_HEIGHT - TEXTURE_GUN_SIDE_HEIGHT + (uint8_t) offset) *
                SCREEN_WIDTH +
            (SCREEN_WIDTH - TEXTURE_GUN_SIDE_WIDTH);
        for (int j = 0; j < TEXTURE_GUN_SIDE_HEIGHT - (uint8_t) offset; j++) {
            for (int i = 0; i < TEXTURE_GUN_SIDE_WIDTH; i++) {
                auto tv = g_texture_gun_side[j * TEXTURE_GUN_SIDE_WIDTH + i];

                if ((tv & 0x8000) == 0)
                    *lb = GetARGB_color(tv);
                lb++;
            }
            lb += SCREEN_WIDTH - TEXTURE_GUN_SIDE_WIDTH;
        }
    }

    // rendering aiming point
    uint32_t *lb = fb + (SCREEN_HEIGHT / 2) * SCREEN_WIDTH + (SCREEN_WIDTH / 2);
    for (int l = 0; l < ARM_POINT_LEN; l++) {
        float stable = (g->pose == POSE_SQUAT ? 0.2f : 1);
        uint8_t len = offset * stable + l + ARM_POINT_RAD * stable;
        *(lb - len * SCREEN_WIDTH) = ARM_POINT_COLOR;
        *(lb + len * SCREEN_WIDTH) = ARM_POINT_COLOR;
        *(lb - len) = ARM_POINT_COLOR;
        *(lb + len) = ARM_POINT_COLOR;
    }
}