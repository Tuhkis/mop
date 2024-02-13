#ifndef EDITOR_H
#define EDITOR_H

#include "SDL2/SDL.h"

#include "font.h"

/* Hope this isn't exceeded. */
#define BUFFSIZE (200 * 1024)

typedef struct editor_t {
  char* text;
  int caret_pos;
  int scroll;
  int size;
} Editor;

Editor* create_editor(const char* title);
int editor_len_until_prev_line(Editor* editor, int pos);
int editor_len_until_next_line(Editor* editor, int pos);
int editor_newlines_before(Editor* editor, int pos);
void editor_insert_at(Editor* editor, char c, int pos);
void editor_remove_at(Editor* editor, int pos);
void editor_render_line(Editor* editor, int line, int x, int y, SDL_Renderer* renderer, Font* font);

#endif /* EDITOR_H */

