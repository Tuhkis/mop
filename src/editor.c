#include "stdlib.h"
#include "string.h"

#include "editor.h"

Editor* create_editor(const char* title) {
  (void) title;
  Editor* ret = malloc(sizeof(Editor));
  ret->text = malloc(BUFFSIZE);
  ret->size = BUFFSIZE;
  memset(ret->text, 0, BUFFSIZE - 1);
  ret->scroll = 0;

  return ret;
}

void editor_render_line(Editor* editor, int line, int x, int y, SDL_Renderer* renderer, Font* font) {
  size_t begin = 0;
  int newlines_encountered = 0;
  int line_end = 0;

  for (;newlines_encountered < line;) {
    if (editor->text[begin] == '\n') ++newlines_encountered;
    ++begin;
  }
  line_end = begin;
  while (editor->text[line_end] != '\n') {
    if (editor->text[line_end] == '\0') break;
    ++line_end;
  }
  if (begin > BUFFSIZE || line_end > BUFFSIZE) return;
  editor->text[line_end] = '\0';

  render_text(renderer, font, x, y, editor->text + begin);
  editor->text[line_end] = '\n';
}

int editor_newlines_before(Editor* editor, int pos) {
  int c = 0;
  int i;
  for (i = 0; i < pos; ++i)
    if (editor->text[i] == '\n')
      ++c;
  return c - 1;
}

int editor_len_until_prev_line(Editor* editor, int pos) {
  int c = 0;
  while (editor->text[pos - 1 - c] != '\n' && editor->text != editor->text + (pos - c))
    ++c;
  return c;
}

int editor_len_until_next_line(Editor* editor, int pos) {
  int c = 0;
  while (editor->text[pos + c] != '\n')
    ++c;
  return c;
}

void editor_insert_at(Editor* editor, char c, int pos) {
  int i;
  char temp = editor->text[pos];
  /* TODO: Fix. */
  if (pos > editor->size - 1) {
    editor->text = realloc(editor->text, editor->size + 512);
    editor->size += 512;
  }
  for (i = editor->size - 1; i >= pos + 1; --i) {
    editor->text[i + 1] = editor->text[i];
  }
  editor->text[i] = c;
  editor->text[i + 1] = temp;
}

void editor_remove_at(Editor* editor, int pos) {
  int i;
  /* Copy next element value to current element */
  for (i = pos; i < editor->size - 1; ++i)
  {
    editor->text[i] = editor->text[i + 1];
  }
}

