#include "SDL2/SDL.h"

#include "keymap.h"

static Keybind keys[] = {
  {NULL, 0, NULL}
};

Keybind* get_keybinds(void) {
  return (Keybind*)(keys);
}

