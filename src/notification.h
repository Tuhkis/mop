#ifndef NOTIF_H
#define NOTIF_H

#include "SDL2/SDL.h"
#include "linked_list.h"

#define NOTIF_MAX_CHARS (256)
#define NOTIF_MAX_LIFETIME (4.5f)

typedef struct notif {
  char text[NOTIF_MAX_CHARS];
  float lifetime;
} Notif;

typedef struct notif_manager {
  ll_List notifs;
} NotifManager;

Notif* create_notif(const char* text);
void add_notif(NotifManager* notif_manager, Notif* notif);
void process_notifs(NotifManager* notif_manager, float delta_time);

#endif /* NOTIF_H */

