#pragma once

#include "game.h"
#include "raycaster.h"
#include "raycaster_data.h"

class Renderer
{
    RayCaster *_rc;

    inline static uint32_t GetARGB(uint8_t brightness)
    {
        return (brightness << 16) + (brightness << 8) + brightness;
    }

    inline static uint32_t GetARGB_color(uint16_t color)
    {
        return ((color & TEXTURE_R_MASK) << TEXTURE_R_OFFSET) +
               ((color & TEXTURE_G_MASK) << TEXTURE_G_OFFSET) +
               ((color & TEXTURE_B_MASK) << TEXTURE_B_OFFSET);
    }

    const uint8_t *g_texture_gray[NUMBER_OF_GRAY_TEXTURE] = {g_texture8};

    const uint16_t *g_texture_color[2] = {g_texture8_computer, g_texture8_cat};

public:
    void TraceFrame(Game *g, uint32_t *frameBuffer);
    Renderer(RayCaster *rc) { _rc = rc; };
    ~Renderer(){};
};
