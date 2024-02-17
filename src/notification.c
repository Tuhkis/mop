#include "stb_sprintf.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "app.h"
#include "font.h"
#include "notification.h"

Notif* create_notif(const char* title) {
  Notif* ret;

  ret = malloc(sizeof(Notif));
  ret->lifetime = 0;
  stbsp_snprintf(ret->text, NOTIF_MAX_CHARS, "%s", title);

  return ret;
}

void add_notif(NotifManager* notif_manager, Notif* notif) {
  ll_list_add(&notif_manager->notifs, notif);
}

void draw_notif(Notif* notif, App* app, int i) {
  SDL_Rect r;
  int alpha = (int)((NOTIF_MAX_LIFETIME - notif->lifetime) / NOTIF_MAX_LIFETIME * 1000.0f);
  if (alpha > 255) alpha = 255;

  r.w = 10.4 * app->scale * strlen(notif->text) + 8 * app->scale;
  r.h = 32 * app->scale;
  r.x = app->win_width - r.w - app->margin_x;
  r.y = app->win_height - r.h * (i + 1) - app->margin_y - 5 * app->scale * i;

  SDL_SetRenderDrawColor(app->renderer, 200, 200, 200, alpha);
  SDL_RenderFillRect(app->renderer, &r);
  SDL_SetRenderDrawColor(app->renderer, 20, 20, 20, 255);
  render_text(app->renderer, app->code_font, r.x + 4 * app->scale, r.y + 4 * app->scale + app->code_font->baseline, notif->text);
  /* render_text(app->renderer, app->icon_font, r.x + r.w - 22 * app->scale, r.y + 7 * app->scale + app->icon_font->baseline, "c");
  */
}

void draw_notifs(App* app) {
  int i;
  for (i = 0; i < (int)(app->notif.notifs.len); ++i) {
    draw_notif(ll_list_get(app->notif.notifs, i), app, i);
  }
}

void process_notifs(NotifManager* notif_manager, float delta_time) {
  int i;
  for (i = 0; i < (int)(notif_manager->notifs.len); ++i) {
    Notif* n = ll_list_get(notif_manager->notifs, i);
    n->lifetime += delta_time;
  }
  if (notif_manager->notifs.first != NULL) {
    Notif* n = ll_list_get(notif_manager->notifs, 0);
    if (n->lifetime > NOTIF_MAX_LIFETIME) {
      ll_list_remove_nth(&notif_manager->notifs, 0);
    }
  }
}

