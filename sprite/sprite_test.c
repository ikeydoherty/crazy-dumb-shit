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

#include <stdbool.h>
#include <stdlib.h>

/**
 * Initialise the game libs
 */
bool game_init(void)
{
        return false;
}

/**
 * Tear down/free the game libs
 */
void game_destroy(void)
{
}

/**
 * Run the main loop
 */
bool game_loop(void)
{
        return false;
}

int main(void)
{
        if (!game_init()) {
                return EXIT_FAILURE;
        }
        bool success = game_loop();
        game_destroy();
        return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
