#include "ctype.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "editor.h"

Editor* create_editor(const char* title) {
  int i;
  Editor* ret = malloc(sizeof(Editor));
  (void) title;
  ret->text = malloc(BUFFSIZE);
  ret->size = BUFFSIZE;
  for (i = 0; i < BUFFSIZE; ++i) ret->text[i] = '\0';
  ret->scroll = 0;
  ret->caret_pos = 0;

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

void keydown_editor(Editor* editor, SDL_Keycode key, char ctrl) {
  (void) ctrl;
  switch (key) {
    case SDLK_PAGEDOWN: {
      ++editor->scroll;
      break;
    }
    case SDLK_PAGEUP: {
      --editor->scroll;
      if (editor->scroll < 0) editor->scroll = 0;
      break;
    }
    case SDLK_RIGHT: {
      if (ctrl) {
        if (editor->text[editor->caret_pos] == '\n') {
          ++editor->caret_pos;
          break;
        }
        if (editor->text[editor->caret_pos] == ' ') {
          for (;editor->text[editor->caret_pos] == ' ';) ++editor->caret_pos;
          break;
        }
        if (!isalnum(editor->text[editor->caret_pos])) {
          ++editor->caret_pos;
          break;
        }
        for (;isalnum(editor->text[editor->caret_pos]);)
          ++editor->caret_pos;

        break;
      }
      ++editor->caret_pos;
      break;
    }
    case SDLK_LEFT: {
      if (ctrl) {
        if (editor->text[editor->caret_pos - 1] == '\n') {
          --editor->caret_pos;
          break;
        }
        if (editor->text[editor->caret_pos - 1] == ' ') {
          for (;editor->text[editor->caret_pos - 1] == ' ';) --editor->caret_pos;
          break;
        }
        if (!isalnum(editor->text[editor->caret_pos - 1])) {
          --editor->caret_pos;
          break;
        }
        for (;isalnum(editor->text[editor->caret_pos - 1]);)
          --editor->caret_pos;
        break;
      }
      --editor->caret_pos;
      break;
    }
    case SDLK_UP: {
      int target = editor->caret_pos;
      int newlines = 0;
      int q = editor_len_until_prev_line(editor, target);
      target -= q + 1;
      target -= editor_len_until_prev_line(editor, target);
      target += q;
      for (;(editor->caret_pos != target) && (newlines != 2);) {
        if (editor->text[editor->caret_pos - 1] == '\n') {
          ++newlines;
        }
        --editor->caret_pos;
      }
      if (newlines < 1) {
        editor->caret_pos -= editor_len_until_prev_line(editor, editor->caret_pos) + 1;
      }
      break;
    }
    case SDLK_DOWN: {
      /* false = 0, true = 1 */
      int newlines = editor->text[editor->caret_pos] == '\n';
      int target = editor->caret_pos + editor_len_until_next_line(editor, editor->caret_pos) + editor_len_until_prev_line(editor, editor->caret_pos) + 1;
      for (;(editor->caret_pos != target) && (newlines != 2);) {
        if (editor->text[editor->caret_pos + 1] == '\n') {
          ++newlines;
        }
        ++editor->caret_pos;
      }
      break;
    }
    case SDLK_BACKSPACE: {
      if (editor->text[editor->caret_pos - 1] == '\0') break;
      if (ctrl) {
        if (editor->text[editor->caret_pos - 1] == '\n') {
          editor_remove_at(editor, editor->caret_pos - 1);
          --editor->caret_pos;
          break;
        }
        if (editor->text[editor->caret_pos - 1] == ' ') {
          for (;editor->text[editor->caret_pos - 1] == ' ';) {
            editor_remove_at(editor, editor->caret_pos - 1);
            --editor->caret_pos;
          }
          break;
        }
        if (!isalnum(editor->text[editor->caret_pos - 1])) {
          editor_remove_at(editor, editor->caret_pos - 1);
          --editor->caret_pos;
          break;
        }
        for (;isalnum(editor->text[editor->caret_pos - 1]);) {
          editor_remove_at(editor, editor->caret_pos - 1);
          --editor->caret_pos;
        }
        break;
      }
      editor_remove_at(editor, editor->caret_pos - 1);
      --editor->caret_pos;
      break;
    }
    case SDLK_RETURN: {
      if (ctrl) {
        editor->caret_pos += editor_len_until_next_line(editor, editor->caret_pos) + 1;
        editor_insert_at(editor, '\n', editor->caret_pos - 1);
        break;
      }
      ++editor->caret_pos;
      editor_insert_at(editor, '\n', editor->caret_pos - 1);
      break;
    }
    case SDLK_v: {
      if (!ctrl) break;
      char* clipboard_text = SDL_GetClipboardText();
      int text_len = strlen(clipboard_text);
      int i;
      for (i = 0; i < text_len; ++i) {
        ++editor->caret_pos;
        editor_insert_at(editor, clipboard_text[i], editor->caret_pos - 1);
      }
      break;
    }
    /* Terminal controls */
    case SDLK_a: {
      if (!ctrl) break;
      editor->caret_pos -= editor_len_until_prev_line(editor, editor->caret_pos);
      break;
    }
    case SDLK_e: {
      if (!ctrl) break;
      editor->caret_pos += editor_len_until_next_line(editor, editor->caret_pos);
      break;
    }
    case SDLK_d: {
      if (!ctrl) break;
      editor_remove_at(editor, editor->caret_pos);
      break;
    }
    default: {
      break;
    }
  }
  if (editor->caret_pos < 0) editor->caret_pos = 0;
}

