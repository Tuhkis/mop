#ifndef FILE_BROWSER_H
#define FILE_BROWSER_H

#include "SDL2/SDL.h"

#define FILE_BROWSER_MAX_NAME_LEN (256)

typedef struct file_browser {
  char visible;
  SDL_Rect rect;
  char files;
} FileBrowser;

#endif /* FILE_BROWSER_H */

