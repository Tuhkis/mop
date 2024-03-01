#ifndef EDITOR_VIEW_H
#define EDITOR_VIEW_H

#include "app.h"
#include "editor.h"

#define MAX_EDITOR_VIEWS (16)

typedef struct editor_view {
  App* app;
  Editor* editor;
  float caret_x;
  float caret_y;
  int editor_index;
  int line_number_width;
  SDL_Rect area;
  SDL_Rect caret_rect;
} EditorView;

void init_editor_view(App* app, EditorView* view);
void set_editor_view_rect(EditorView* view, SDL_Rect* r);
void render_editor_view(EditorView* view, float delta);
void keydown_editor_view(EditorView* view, SDL_Keycode key, char ctrl, char super, char shift);
void text_input_editor_view(EditorView* view, char text);
void scroll_editor_view(EditorView* view, float scroll);

#endif /* EDITOR_VIEW_H */

