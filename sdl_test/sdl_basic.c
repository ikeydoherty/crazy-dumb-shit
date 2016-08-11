/*
 * This file is part of crazy-dumb-shit
 *
 * Copyright © 2016 Ikey Doherty <ikey@solus-project.com>
 *
 * sdl_basic is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#define _GNU_SOURCE

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include <stdlib.h>

#include "util.h"

DEF_AUTOFREE(SDL_Window, SDL_DestroyWindow)
DEF_AUTOFREE(TTF_Font, TTF_CloseFont)
DEF_AUTOFREE(SDL_Surface, SDL_FreeSurface)
DEF_AUTOFREE(SDL_Texture, SDL_DestroyTexture)
DEF_AUTOFREE(SDL_Renderer, SDL_DestroyRenderer)
DEF_AUTOFREE(char, free)

/**
 * Just in case we fail to find it.
 */
#define DEFAULT_REFRESH_RATE 60

/**
 * Convenience wrapper for working with fonts.
 */
static inline SDL_Texture *render_text(SDL_Renderer *ren, TTF_Font *font, SDL_Color color,
                                       const char *displ, SDL_Rect *optimal_size)
{
        /* Our FPS is updated once a second, using Blended won't hurt here, else we'd
         * use Solid. */
        autofree(SDL_Surface) *surface = TTF_RenderText_Blended(font, displ, color);
        if (!surface) {
                return NULL;
        }
        /* Store the size here for SDL_RenderCopy */
        if (optimal_size) {
                SDL_Rect opt = {.x = 0, .y = 0, .w = surface->w, .h = surface->h };
                *optimal_size = opt;
        }
        return SDL_CreateTextureFromSurface(ren, surface);
}

/**
 * Limiter to common hz
 */
static inline int pref_hz(int in)
{
        if (in <= 30) {
                return 30;
        } else if (in <= 55) {
                return 50;
        } else if (in < 100) {
                return 60;
        }
        return in;
}

/**
 * Quickly get the actual refresh rate for the given window
 */
static inline int get_refresh_rate(SDL_Window *window)
{
        SDL_DisplayMode mode = { 0 };
        if (SDL_GetWindowDisplayMode(window, &mode) != 0) {
                fprintf(stderr, "get_refresh_rate(): %s\n", SDL_GetError());
                fprintf(stderr, "Falling back to default %udHz\n", DEFAULT_REFRESH_RATE);
                return DEFAULT_REFRESH_RATE;
        }
        return pref_hz(mode.refresh_rate);
}

/**
 * Main outer loop of the entire app
 */
static bool main_loop(void)
{
        autofree(SDL_Window) *window = NULL;
        autofree(SDL_Renderer) *ren = NULL;
        autofree(TTF_Font) *font = NULL;
        /* Game loop. */
        bool running = false;
        SDL_Color fg_color = {.r = 255, .g = 255, .b = 255, .a = 255 };
        int rfhz;

        /* Create our font first */
        font = TTF_OpenFont("assets/fonts/Hack-Regular.ttf", 16);
        if (!font) {
                fprintf(stderr, "Failed to load font: %s\n", TTF_GetError());
                return false;
        }

        /* New 800x600 window, centered, initially hidden */
        window = SDL_CreateWindow("sdl_basic",
                                  SDL_WINDOWPOS_CENTERED,
                                  SDL_WINDOWPOS_CENTERED,
                                  800,
                                  600,
                                  SDL_WINDOW_HIDDEN);

        if (!window) {
                fprintf(stderr, "Failed to create SDL Window: %s\n", SDL_GetError());
                return false;
        }

        /* Set up the renderer + vsync */
        ren = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (!ren) {
                fprintf(stderr, "Failed to create SDL Renderer: %s\n", SDL_GetError());
                return false;
        }

        /* Get the refresh rate */
        rfhz = get_refresh_rate(window);
        fprintf(stdout, "sdl_basic @ %dHz\n", rfhz);

        /* Main render loop */
        running = true;
        SDL_ShowWindow(window);

        /* FPS Tracking (primitive + no averaging) */
        uint32_t frames = 0;
        uint32_t start_time = SDL_GetTicks();
        float fps_track = 0.0f;

        uint frame_ticks = (uint)(1000 / rfhz);

        while (running) {
                SDL_Event event = { 0 };
                autofree(SDL_Texture) *fps_counter = NULL;
                ++frames;
                uint32_t render_start = SDL_GetTicks();

                /* Handle events on this iteration */
                while (SDL_PollEvent(&event)) {
                        switch (event.type) {
                        case SDL_QUIT:
                                running = false;
                                break;
                        default:
                                /* Not fussed */
                                break;
                        }
                }
                /* Get fps label */
                autofree(char) *fps_label = NULL;
                if (!asprintf(&fps_label, "%.2f fps", fps_track)) {
                        abort();
                }
                SDL_Rect render_rect;
                fps_counter = render_text(ren, font, fg_color, fps_label, &render_rect);
                SDL_Rect source_rect = render_rect;
                /* Only modify x/y on the render, don't spazz the source, it warps */
                render_rect.x = 10;
                render_rect.y = 10;

                /* Clear the renderer, and now copy the FPS out */
                SDL_SetRenderDrawColor(ren, 125, 125, 255, 255);
                SDL_RenderClear(ren);
                SDL_RenderCopy(ren, fps_counter, &source_rect, &render_rect);
                SDL_RenderPresent(ren);

                /* Frame limit */
                if ((SDL_GetTicks() - render_start) < frame_ticks) {
                        SDL_Delay(frame_ticks - (SDL_GetTicks() - render_start));
                }

                fps_track = (float)frames / ((float)(SDL_GetTicks() - start_time) / 1000.0f);
        }
        return true;
}

/**
 * Main entry, ensuring cleanups take place
 */
int main(void)
{
        /* Try to init SDL */
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
                fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
                return EXIT_FAILURE;
        }
        /* Try now to init TTF */
        if (TTF_Init() != 0) {
                fprintf(stderr, "Failed to initialise font system: %s\n", TTF_GetError());
                return EXIT_FAILURE;
        }
        /* Initialise PNG */
        if (IMG_Init(0 & IMG_INIT_PNG) != 0) {
                fprintf(stderr, "Failed to initialise image system: %s\n", IMG_GetError());
                return EXIT_FAILURE;
        }
        bool success = main_loop();
        SDL_Quit();
        TTF_Quit();
        IMG_Quit();
        return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
