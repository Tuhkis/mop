#include "SDL2/SDL.h"
#include "stdlib.h"

#include "font.h"
#include "render.h"

/*
 * Implementation from https://gist.github.com/benob/92ee64d9ffcaa5d3be95edbf4ded55f2
 * Not mine.
 */

void close_font(Font* font) {
  if(font->atlas) SDL_DestroyTexture(font->atlas);
  if(font->info) free(font->info);
  if(font->chars) free(font->chars);
  free(font);
}

Font* open_font_rw(SDL_Renderer* renderer, SDL_RWops* rw, float size) {
  Font* font;
  int i;
  Sint64 file_size = SDL_RWsize(rw);
  static SDL_PixelFormat* format = NULL;
  Uint32* pixels;
  unsigned char* bitmap = NULL;
  unsigned char* buffer = malloc(file_size);

  if (SDL_RWread(rw, buffer, file_size, 1) != 1) return NULL;
  SDL_RWclose(rw);

  font = calloc(sizeof(Font), 1);
  font->info = malloc(sizeof(stbtt_fontinfo));
  font->chars = malloc(sizeof(stbtt_packedchar) * 96);

  if (stbtt_InitFont(font->info, buffer, 0) == 0) {
    free(buffer);
    close_font(font);
    return NULL;
  }

  /* fill bitmap atlas with packed characters */
  font->texture_size = 32;
  for (;;) {
    stbtt_pack_context pack_context;
    bitmap = malloc(font->texture_size * font->texture_size);
    stbtt_PackBegin(&pack_context, bitmap, font->texture_size, font->texture_size, 0, 1, 0);
    stbtt_PackSetOversampling(&pack_context, 1, 1);
    if (!stbtt_PackFontRange(&pack_context, buffer, 0, size, 32, 95, font->chars)) {
      /* too small */
      free(bitmap);
      stbtt_PackEnd(&pack_context);
      font->texture_size *= 2;
    } else {
      stbtt_PackEnd(&pack_context);
      break;
    }
  }

  /* convert bitmap to texture */
  font->atlas = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, font->texture_size, font->texture_size);
  SDL_SetTextureBlendMode(font->atlas, SDL_BLENDMODE_BLEND);

  pixels = malloc(font->texture_size * font->texture_size * sizeof(Uint32));
  if (format == NULL) format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA32);
  for (i = 0; i < font->texture_size * font->texture_size; i++) {
    pixels[i] = SDL_MapRGBA(format, 0xff, 0xff, 0xff, bitmap[i]);
  }
  SDL_UpdateTexture(font->atlas, NULL, pixels, font->texture_size * sizeof(Uint32));
  free(pixels);
  free(bitmap);

  /* setup additional info */
  font->scale = stbtt_ScaleForPixelHeight(font->info, size);
  stbtt_GetFontVMetrics(font->info, &font->ascent, 0, 0);
  font->baseline = (int) (font->ascent * font->scale);
  font->stride = (float)(font->chars['D' - 32].xadvance);

  free(buffer);

  return font;
}

Font* open_font(SDL_Renderer* renderer, const char* filename, float size) {
  SDL_RWops *rw = SDL_RWFromFile(filename, "rb");
  if (rw == NULL) return NULL;
  return open_font_rw(renderer, rw, size);
}

void render_text(SDL_Renderer* renderer, Font* font, float x, float y, const char *text) {
  int i;
  Uint8 r, g, b, a;

  SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);
  SDL_SetTextureColorMod(font->atlas, r, g, b);
  SDL_SetTextureAlphaMod(font->atlas, a);
  for (i = 0; text[i]; i++) {
    if (text[i] == '\t') { /* Draw tabs as little circles */
      stbtt_packedchar* info = &font->chars['n' - 32];
      draw_circle(renderer, x + info->xoff + (info->x1 - info->x0) * 0.4f, y + info->yoff + (info->y1 - info->y0) * 0.4f, 3);
      x += info->xadvance;
    } else if (text[i] >= 32 && text[i] < 127) {
      stbtt_packedchar* info = &font->chars[text[i] - 32];
      SDL_Rect src_rect = {0};
      SDL_Rect dst_rect = {0};

      src_rect.x = info->x0;
      src_rect.y = info->y0;
      src_rect.w = info->x1 - info->x0;
      src_rect.h = info->y1 - info->y0;

      dst_rect.x = x + info->xoff;
      dst_rect.y = y + info->yoff;
      dst_rect.w = info->x1 - info->x0;
      dst_rect.h = info->y1 - info->y0;

      SDL_RenderCopy(renderer, font->atlas, &src_rect, &dst_rect);
      x += info->xadvance;
    } else { /* If the letter isn't ascii, just render a little rectangle. */
      stbtt_packedchar* info = &font->chars['x' - 32];
      SDL_Rect dst_rect = {0};

      dst_rect.x = x + info->xoff;
      dst_rect.y = y + info->yoff;
      dst_rect.w = info->x1 - info->x0;
      dst_rect.h = info->y1 - info->y0;

      SDL_RenderFillRect(renderer, &dst_rect);
      x += info->xadvance;
    }
  }
}

