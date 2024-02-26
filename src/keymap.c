#include "SDL2/SDL.h"

#include "keymap.h"

static int key_index = 0;

static Keybind keys[] = {
  {NULL, KEYMOD_NONE, 0, NULL}
};

Keybind* get_keybinds(void) {
  return (Keybind*)(keys);
}

void add_keybind(Keybind bind, App* app) {
  if (key_index > MAX_KEYBINDS) {
    add_notif(&app->notif, create_notif("Could not add new keybind; maximum exceeded."));
    return;
  }
  keys[key_index] = bind;
  ++key_index;
}

