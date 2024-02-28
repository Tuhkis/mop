#ifndef EDITOR_H
#define EDITOR_H

#include "SDL2/SDL.h"

#include "font.h"

/* Hope this isn't exceeded. */
#define EDITOR_BUFFSIZE (4096)
#define EDITOR_BUFF_BUMP (1024)
#define EDITOR_NAME_MAX_LEN (128)

typedef struct editor_t {
  char* text;
  float scroll;
  float target_scroll;
  int caret_pos;
  int line_target;
  int lines;
  int size;
  char name[EDITOR_NAME_MAX_LEN];
} Editor;

char editor_render_line(Editor* editor, int line, int x, int y, SDL_Renderer* renderer, Font* font);
Editor* create_editor(const char* title);
int editor_len_until_prev_line(Editor* editor, int pos);
int editor_len_until_next_line(Editor* editor, int pos);
int editor_newlines_before(Editor* editor, int pos);
void editor_insert_at(Editor* editor, char c, int pos);
void editor_remove_at(Editor* editor, int pos);

#endif /* EDITOR_H */

