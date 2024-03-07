#ifndef APP_H
#define APP_H

#include "SDL2/SDL.h"
#include "linked_list.h"

#include "config.h"
#include "font.h"
#include "notification.h"

typedef struct app {
  Config config;
  float scale;
  float time;
  Font* code_font;
  int current_editor;
  int mouse_x;
  int mouse_y;
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

