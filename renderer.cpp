#include "renderer.h"
#include <math.h>
#include "raycaster_data.h"

void Renderer::TraceFrame(Game *g, uint32_t *fb)
{
    _rc->Start(static_cast<uint16_t>(g->playerX * 256.0f),
               static_cast<uint16_t>(g->playerY * 256.0f),
               static_cast<int16_t>(g->playerA / (2.0f * M_PI) * 1024.0f));

    for (int x = 0; x < SCREEN_WIDTH; x++) {
        uint8_t sso;
        uint8_t tc;
        uint8_t tn;
        uint16_t tso;
        uint16_t tst;
        uint32_t *lb = fb + x;

        _rc->Trace(x, &sso, &tn, &tc, &tso, &tst);

        const auto tx = static_cast<int>(tc >> 2);
        int16_t ws = HORIZON_HEIGHT - sso;
        if (ws < 0) {
            ws = 0;
            sso = HORIZON_HEIGHT;
        }
        uint16_t to = tso;
        uint16_t ts = tst;

        for (int y = 0; y < ws; y++) {
            *lb = GetARGB(96 + (HORIZON_HEIGHT - y));
            lb += SCREEN_WIDTH;
        }

        for (int y = 0; y < sso * 2; y++) {
            // paint texture pixel
            uint16_t ty =
                (tso + (((TEXTURE_SIZE / 2) << 10) - tso) * y / sso) >> 10;

            uint16_t tv;
            if (tn / 2 < NUMBER_OF_GRAY_TEXTURE) {
                // gray scale texture
                tv = (g_texture_gray[tn / 2])[(ty << 6) + tx];
                *lb = GetARGB(tv);
            } else {
                // colored texture
                tv = (g_texture_color[tn / 2 - 1])[(ty << 6) + tx];
                *lb = GetARGB_color(tv);
            }

            // View with different direction
            if (tn % 2 == 1 && *lb > 0) {
                // dark wall
                *lb = (*lb & 0xFEFEFE) >> 1;
            }
            to += ts;

            lb += SCREEN_WIDTH;
        }

        for (int y = 0; y < ws; y++) {
            *lb = GetARGB(96 + (HORIZON_HEIGHT - (ws - y)));
            lb += SCREEN_WIDTH;
        }
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