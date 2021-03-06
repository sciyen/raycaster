#include <SDL.h>
#include <stdio.h>
#include <fstream>
#include <iostream>

#include "game.h"
#include "raycaster.h"
#include "raycaster_fixed.h"
#include "raycaster_float.h"
#include "renderer.h"

using namespace std;

static void DrawBuffer(SDL_Renderer *sdlRenderer,
                       SDL_Texture *sdlTexture,
                       uint32_t *fb,
                       int dx)
{
    int pitch = 0;
    void *pixelsPtr;
    if (SDL_LockTexture(sdlTexture, NULL, &pixelsPtr, &pitch)) {
        throw runtime_error("Unable to lock texture");
    }
    memcpy(pixelsPtr, fb, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));
    SDL_UnlockTexture(sdlTexture);
    SDL_Rect r;
    r.x = dx * SCREEN_SCALE;
    r.y = 0;
    r.w = SCREEN_WIDTH * SCREEN_SCALE;
    r.h = SCREEN_HEIGHT * SCREEN_SCALE;
    SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &r);
}

static bool ProcessEvent(const SDL_Event &event,
                         int *moveDirection,
                         int *rotateDirection,
                         Game *game)
{
    if (event.type == SDL_QUIT) {
        return true;
    } else if ((event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) &&
               event.key.repeat == 0) {
        auto k = event.key;
        auto p = event.type == SDL_KEYDOWN;
        switch (k.keysym.sym) {
        case SDLK_ESCAPE:
            return true;
            break;
        case SDLK_UP:
            *moveDirection = p ? 1 : 0;
            break;
        case SDLK_DOWN:
            *moveDirection = p ? -1 : 0;
            break;
        case SDLK_LEFT:
            *rotateDirection = p ? -1 : 0;
            break;
        case SDLK_RIGHT:
            *rotateDirection = p ? 1 : 0;
            break;
        case SDLK_e:
            game->pose = p ? POSE_SQUAT : POSE_STAND;
            break;
        case SDLK_g:
            game->godMode = p ? 1 : 0;
            break;
        default:
            break;
        }
    }
    return false;
}

void printPerformanceInfo(float floatRenderSec,
                          float fixedRenderSec,
                          float overallSec)
{
    const float smoothFactor = 0.95f;
    static auto lastPrintTick = SDL_GetPerformanceCounter();
    static float floatRC = floatRenderSec;
    static float fixedRC = fixedRenderSec;
    static float overRC = overallSec;
    auto currentTick = SDL_GetPerformanceCounter();
    const static auto tickFrequency = SDL_GetPerformanceFrequency();
    floatRC = smoothFactor * floatRC + (1 - smoothFactor) * floatRenderSec;
    fixedRC = smoothFactor * fixedRC + (1 - smoothFactor) * fixedRenderSec;
    overRC = smoothFactor * overRC + (1 - smoothFactor) * overallSec;
    if ((currentTick - lastPrintTick) / static_cast<float>(tickFrequency) >
        0.2f) {
        printf("fixed: %.6f(s), float: %.6f(s), FPS: %.6f(s)\n", fixedRC,
               floatRC, 1 / overRC);
        lastPrintTick = currentTick;
    }
}

int main(int argc, char *args[])
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    } else {
        SDL_Window *sdlWindow =
            SDL_CreateWindow("RayCaster [fixed-point vs. floating-point]",
                             SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                             SCREEN_SCALE * (SCREEN_WIDTH * 2 + 1),
                             SCREEN_SCALE * SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

        if (sdlWindow == NULL) {
            printf("Window could not be created! SDL_Error: %s\n",
                   SDL_GetError());
        } else {
            Game game;
            RayCasterFloat floatCaster;
            Renderer floatRenderer(&floatCaster);
            uint32_t floatBuffer[SCREEN_WIDTH * SCREEN_HEIGHT];
            RayCasterFixed fixedCaster;
            Renderer fixedRenderer(&fixedCaster);
            uint32_t fixedBuffer[SCREEN_WIDTH * SCREEN_HEIGHT];
            int moveDirection = 0;
            int rotateDirection = 0;
            bool isExiting = false;
            const static auto tickFrequency = SDL_GetPerformanceFrequency();
            auto tickCounter = SDL_GetPerformanceCounter();
            SDL_Event event;

            SDL_Renderer *sdlRenderer =
                SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED);
            SDL_Texture *fixedTexture = SDL_CreateTexture(
                sdlRenderer, SDL_PIXELFORMAT_ARGB8888,
                SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
            SDL_Texture *floatTexture = SDL_CreateTexture(
                sdlRenderer, SDL_PIXELFORMAT_ARGB8888,
                SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

            while (!isExiting) {
                /* Float point render start */
                const auto renderFloatTickStart = SDL_GetPerformanceCounter();
                floatRenderer.TraceFrame(&game, floatBuffer);
                floatRenderer.RenderGame(&game, floatBuffer);
                const auto renderFloatTickEnd = SDL_GetPerformanceCounter();
                const auto floatRenderSeconds =
                    (renderFloatTickEnd - renderFloatTickStart) /
                    static_cast<float>(tickFrequency);

                /* Fixed point render start */
                const auto renderFixedTickStart = SDL_GetPerformanceCounter();
                fixedRenderer.TraceFrame(&game, fixedBuffer);
                fixedRenderer.RenderGame(&game, fixedBuffer);
                const auto renderFixedTickEnd = SDL_GetPerformanceCounter();
                const auto fixedRenderSeconds =
                    (renderFixedTickEnd - renderFixedTickStart) /
                    static_cast<float>(tickFrequency);

                DrawBuffer(sdlRenderer, fixedTexture, fixedBuffer, 0);
                DrawBuffer(sdlRenderer, floatTexture, floatBuffer,
                           SCREEN_WIDTH + 1);

                SDL_RenderPresent(sdlRenderer);

                if (SDL_PollEvent(&event)) {
                    isExiting = ProcessEvent(event, &moveDirection,
                                             &rotateDirection, &game);
                }

                /* Calculating the overall performance */
                const auto nextCounter = SDL_GetPerformanceCounter();
                const auto seconds = (nextCounter - tickCounter) /
                                     static_cast<float>(tickFrequency);
                tickCounter = nextCounter;
                game.Move(moveDirection, rotateDirection, seconds);
                printPerformanceInfo(floatRenderSeconds, fixedRenderSeconds,
                                     seconds);
            }
            SDL_DestroyTexture(floatTexture);
            SDL_DestroyTexture(fixedTexture);
            SDL_DestroyRenderer(sdlRenderer);
            SDL_DestroyWindow(sdlWindow);
        }
    }

    SDL_Quit();
    return 0;
}
