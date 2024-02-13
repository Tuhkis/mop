#ifndef KEYMAP_H
#define KEYMAP_H

#include "SDL2/SDL.h"

#include "app.h"

#define MAX_KEYBINDS (64)
#define KEYBIND_DESC_LEN (256)

typedef struct keybind_t {
  char* desc;
  SDL_Keycode key;
  void (*proc)(App*);
} Keybind;

Keybind* get_keybinds(void);

#endif /* KEYMAP_H */

