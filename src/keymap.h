#ifndef KEYMAP_H
#define KEYMAP_H

#include "SDL2/SDL.h"

#include "app.h"

#define MAX_KEYBINDS (64)
#define KEYBIND_DESC_LEN (256)

typedef enum keymod {
  KEYMOD_NONE,
  KEYMOD_ALT,
  KEYMOD_CTRL,
  KEYMOD_CTRL_SHIFT,
} Keymod;

typedef struct keybind {
  char* desc;
  Keymod mod;
  SDL_Keycode key;
  void (*proc)(App*);
} Keybind;

Keybind* get_keybinds(void);

#endif /* KEYMAP_H */

