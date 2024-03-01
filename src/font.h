#ifndef FONT_H
#define FONT_H

#include "stb_truetype.h"

typedef struct font {
  float scale;
  float size;
  float stride;
  int ascent;
  int baseline;
  int texture_size;
  SDL_Texture* atlas;
  stbtt_fontinfo* info;
  stbtt_packedchar* chars;
} Font;

Font* open_font(SDL_Renderer* renderer, const char* filename, float size);
void close_font(Font* font);
void render_text(SDL_Renderer* renderer, Font* font, float x, float y, const char *text);

#endif /* FONT_H */

