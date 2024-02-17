#include "SDL2/SDL.h"
#include "stdlib.h"

void draw_circle(SDL_Renderer *renderer, int x, int y, int radius) {
  int w, h;
  Uint8 r, g, b, a;
  SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);
  for (w = 0; w < radius * 2; w++) {
    for (h = 0; h < radius * 2; h++) {
      int dx = radius - w; /* horizontal offset */
      int dy = radius - h; /* vertical offset */
      if ((dx * dx + dy * dy) <= ((radius - 1) * (radius - 1))) {
        SDL_RenderDrawPoint(renderer, x + dx, y + dy);
      } else if (
        (dx * dx + dy * dy) <= ((radius) * (radius)) &&
        (dx * dx + dy * dy) >= ((radius - 1) * (radius - 1))
      ) {
        SDL_SetRenderDrawColor(renderer, r, g, b, a * 0.75f);
        SDL_RenderDrawPoint(renderer, x + dx, y + dy);
        SDL_SetRenderDrawColor(renderer, r, g, b, a);
      }
    }
  }
}

