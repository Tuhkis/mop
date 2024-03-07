#ifndef COLOR_H
#define COLOR_H

#include "SDL2/SDL.h"

#define SET_COLOR(renderer, color, alpha) \
  SDL_SetRenderDrawColor((renderer), color.r, color.g, color.b, (alpha))

typedef struct color {
  unsigned char r;
  unsigned char g;
  unsigned char b;
} __attribute__((packed)) Color;

#endif /* COLOR_H */

