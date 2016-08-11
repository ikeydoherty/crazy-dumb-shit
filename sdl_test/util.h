/*
 * This file is part of crazy-dumb-shit
 *
 * Copyright © 2016 Ikey Doherty
 *
 * sdl_basic is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#pragma once

/**
 * As used in libnica, etc.
 */
#define DEF_AUTOFREE(N, C)                                                     \
  static inline void _autofree_func_##N(void *p) {                             \
    if (p && *(N **)p) {                                                       \
      C(*(N **)p);                                                             \
      (*(void **)p) = NULL;                                                    \
    }                                                                          \
  }

#define autofree(N) __attribute__((cleanup(_autofree_func_##N))) N
