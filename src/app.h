#ifndef APP_H
#define APP_H

#include "SDL2/SDL.h"
#include "linked_list.h"

#include "font.h"
#include "notification.h"

typedef struct app_t {
  float scale;
  Font* code_font;
  int current_editor;
  int line_offset;
  int margin_x;
  int margin_y;
  int win_height;
  int win_width;
  ll_List editors;
  NotifManager notif;
  SDL_Renderer* renderer;
  SDL_Window* win;
} App;

void draw_notif(Notif* notif, App* app, int i);
void draw_notifs(App* app);

#endif /* APP_H */

