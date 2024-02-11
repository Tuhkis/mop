#include "stdio.h"
#include "string.h"

#include "app.h"
#include "editor.h"
#include "font.h"

int main(int argc, char** argv) {
  App app;
  char running = 1;
  SDL_DisplayMode dm;
  SDL_Event event;
  SDL_Rect caret_rect;

  (void)argc;
  (void)argv;

#ifdef _WIN32
  {
    HINSTANCE lib = LoadLibrary("user32.dll");
    int (*SetProcessDPIAware)(void) = (void*) GetProcAddress(lib, "SetProcessDPIAware");
    if (SetProcessDPIAware() == 0) { printf("DPI stuff went wrong (?)\n"); }
  }
#endif /* _WIN32 */

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
    printf((char*)"Could not init SDL2.\nError: %s\n", (char*)SDL_GetError());
    return -1;
  }

  SDL_GetCurrentDisplayMode(0, &dm);
  app.win_width = dm.w * 0.5f;
  app.win_height = dm.h * 0.7f,
  app.win = SDL_CreateWindow((char*)"MOP",
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    app.win_width, app.win_height,
    SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_HIDDEN);
  if (!app.win) {
    printf((char*)"Could not create window.\nError: %s\n", (char*)SDL_GetError());
    SDL_Quit();
    return -1;
  }
  SDL_ShowWindow(app.win);

  app.renderer = SDL_CreateRenderer(app.win, -1,
    SDL_RENDERER_ACCELERATED);
  if (!app.renderer) {
    printf((char*)"Could not create renderer.\nError: %s\n", (char*)SDL_GetError());
    SDL_Quit();
    return -1;
  }
  SDL_StartTextInput();

  SDL_GetDisplayDPI(0, &app.scale, NULL, NULL);
  app.scale /= 96.0f;
  app.code_font = open_font(app.renderer, "code.ttf", 26 * app.scale);
  app.margin_x = 15 * app.scale;
  app.margin_y = 10 * app.scale;
  app.line_offset = 5 * app.scale;
  app.editors.len = 0;
  app.editors.first = NULL;
  app.current_editor = 0;

  if (argc > 1) {
    FILE* f;
    Editor* editor = create_editor(argv[1]);
    f = fopen(argv[1], "r");
    if (f == NULL) {
      fclose(f);
      return -1;
    }
    fread(editor->text, BUFFSIZE, 1, f);
    fclose(f);
    ll_list_add(&app.editors, editor);
  }

  for (;running;) {
    Editor* editor = NULL;
    if (app.editors.first != NULL)
      editor = (Editor*)(ll_list_get(app.editors, app.current_editor));
    for (;SDL_PollEvent(&event) != 0;) {
      switch (event.type) {
        case SDL_QUIT: {
          running = 0;
          break;
        }
        case SDL_KEYDOWN: {
          switch (event.key.keysym.sym) {
            case SDLK_RIGHT: {
              ++editor->caret_pos;
              break;
            }
            case SDLK_LEFT: {
              --editor->caret_pos;
              break;
            }
            case SDLK_UP: {
              break;
            }
            case SDLK_DOWN: {
              break;
            }
            case SDLK_BACKSPACE: {
              if (editor->text[editor->caret_pos - 1] == '\0') break;
              editor_remove_at(editor, editor->caret_pos - 1);
              --editor->caret_pos;
              break;
            }
            case SDLK_RETURN: {
              ++editor->caret_pos;
              editor_insert_at(editor, '\n', editor->caret_pos - 1);
              break;
            }
            default: {
              break;
            }
          }
          if (editor->caret_pos < 0) editor->caret_pos = 0;
          break;
        }
        case SDL_TEXTINPUT: {
          editor_insert_at(editor, *event.text.text, editor->caret_pos);
          ++editor->caret_pos;
          break;
        }
        case SDL_WINDOWEVENT: {
          switch (event.window.event) {
            case SDL_WINDOWEVENT_SIZE_CHANGED: /* fallthrough */
            case SDL_WINDOWEVENT_RESIZED: {
              app.win_width = event.window.data1;
              app.win_height = event.window.data2;
              break;
            }
            default: {
              break;
            }
          }
          break;
        }
        default: {
          break;
        }
      }
    }
    SDL_Delay(1000 / 30);
    SDL_SetRenderDrawColor(app.renderer, 20, 20, 20, 255);
    SDL_RenderClear(app.renderer);
    SDL_SetRenderDrawColor(app.renderer, 200, 200, 200, 255);
    if (app.editors.first != NULL) {
      SDL_SetWindowTitle(app.win, editor->title);
      int i;

      for (i = 0; i < 32; ++i)
        editor_render_line(editor, i, app.margin_x, app.margin_y + app.code_font->baseline * (i + 1) + app.line_offset * (i + 1), app.renderer, app.code_font);

      /* Create a caret */
      caret_rect.x = editor_len_until_prev_line(editor, editor->caret_pos) * 10.4 * app.scale + app.margin_x;
      caret_rect.y = 4 + app.margin_y + app.code_font->baseline * (editor_newlines_before(editor, editor->caret_pos) + 1) + app.line_offset * (editor_newlines_before(editor, editor->caret_pos) + 1);
      caret_rect.w = 3 * app.scale;
      caret_rect.h = app.code_font->baseline + 4;
      SDL_SetRenderDrawColor(app.renderer, 200, 200, 255, 200);
      SDL_RenderFillRect(app.renderer, &caret_rect);
    } else {
      SDL_SetWindowTitle(app.win, "MOP");
      render_text(app.renderer, app.code_font, app.margin_x, app.code_font->baseline + app.margin_y, "No open editors.");
    }
    SDL_RenderPresent(app.renderer);
  }

  free(((Editor*)(ll_list_get(app.editors, 0)))->text);

  ll_list_free(&app.editors);

  close_font(app.code_font);
  SDL_DestroyWindow(app.win);
  SDL_DestroyRenderer(app.renderer);
  SDL_Quit();
  return 0;
}

