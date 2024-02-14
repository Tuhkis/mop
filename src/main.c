#include "stdio.h"
#include "string.h"

#include "app.h"
#include "editor.h"
#include "font.h"
#include "keymap.h"

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include "Windows.h"
#endif /* _WIN32 */

int main(int argc, char** argv) {
  App app;
  char running = 1;
  Uint64 prev = 0;
  int i;
  SDL_DisplayMode dm;
  SDL_Event event;
  SDL_Rect caret_rect;
  SDL_Rect editor_rect;

  (void) argc;
  (void) argv;

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
  SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);
  SDL_StartTextInput();

  SDL_GetDisplayDPI(0, &app.scale, NULL, NULL);
  app.scale /= 96.0f;
  app.code_font = open_font(app.renderer, "code.ttf", 26 * app.scale);
  app.margin_x = 15 * app.scale;
  app.margin_y = 5 * app.scale;
  app.line_offset = 6 * app.scale;
  app.editors.len = 0;
  app.editors.first = NULL;
  app.current_editor = 0;

  app.notif.notifs.first = NULL;
  app.notif.notifs.len = 0;

  if (argc > 1) {
    char notif[256];
    FILE* f;
    Editor* editor = create_editor(argv[1]);
    f = fopen(argv[1], "r");
    if (f == NULL) {
      fclose(f);
      return -1;
    }
    fread(editor->text, editor->size, 1, f);
    fclose(f);
    ll_list_add(&app.editors, editor);
    sprintf(notif, "Open file: %s", argv[1]);
    add_notif(&app.notif, create_notif(notif));
  } else {
    ll_list_add(&app.editors, create_editor("Unnamed"));
  }

  for (;running;) {
    Editor* editor = NULL;
    Uint64 now = SDL_GetPerformanceCounter();
    float delta = (now - prev) / (float) SDL_GetPerformanceFrequency();
    if (delta > 1.0f) delta = 0.0f;
    prev = now;
    if (app.editors.first != NULL)
      editor = (Editor*)(ll_list_get(app.editors, app.current_editor));
    process_notifs(&app.notif, delta);
    for (;SDL_PollEvent(&event) != 0;) {
      switch (event.type) {
        case SDL_QUIT: {
          running = 0;
          break;
        }
        case SDL_KEYDOWN: {
          Keybind* binds = get_keybinds();
          for (i = 0; i < MAX_KEYBINDS; ++i) {
            if (binds[i].proc == NULL) break;
            if (event.key.keysym.sym == binds[i].key) binds[i].proc(&app);
          }
          switch (event.key.keysym.sym) {
            case SDLK_PAGEDOWN: {
              ++editor->scroll;
              break;
            }
            case SDLK_PAGEUP: {
              --editor->scroll;
              if (editor->scroll < 0) editor->scroll = 0;
              break;
            }
            case SDLK_RIGHT: {
              ++editor->caret_pos;
              break;
            }
            case SDLK_LEFT: {
              --editor->caret_pos;
              break;
            }
            case SDLK_UP: {
              int q = editor_len_until_prev_line(editor, editor->caret_pos);
              editor->caret_pos -= q;
              editor->caret_pos -= 2;
              editor->caret_pos -= editor_len_until_prev_line(editor, editor->caret_pos);
              editor->caret_pos += q;
              break;
            }
            case SDLK_DOWN: {
              editor->caret_pos += editor_len_until_next_line(editor, editor->caret_pos) + editor_len_until_prev_line(editor, editor->caret_pos) + 1;
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
          /* FIX: can only deal with ascii */
          if (*event.text.text != *"ö") {
            editor_insert_at(editor, *event.text.text, editor->caret_pos);
            ++editor->caret_pos;
          }
          break;
        }
        case SDL_MOUSEWHEEL: {
          const int s = event.wheel.y * 2 * app.scale;
          editor->scroll -= s;
          if (editor->scroll < 0) editor->scroll = 0;
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
    editor_rect.x = app.margin_x;
    editor_rect.y = app.margin_y + app.line_offset;
    editor_rect.w = app.win_width - app.margin_x * 2;
    editor_rect.h = app.win_height - (app.margin_y + app.line_offset) * 2;
    SDL_SetRenderDrawColor(app.renderer, 20, 20, 20, 255);
    SDL_RenderClear(app.renderer);
    SDL_SetRenderDrawColor(app.renderer, 200, 200, 200, 255);
    SDL_RenderSetClipRect(app.renderer, &editor_rect);
    if (app.editors.first != NULL) {
      // SDL_SetWindowTitle(app.win, editor->title);
      for (i = 0; i < editor_rect.h / (app.line_offset + app.code_font->baseline) + 1; ++i)
        editor_render_line(editor, i + editor->scroll, app.margin_x, app.margin_y + app.code_font->baseline * (i + 1) + app.line_offset * (i + 1), app.renderer, app.code_font);

      /* Create a caret */
      caret_rect.x = editor_len_until_prev_line(editor, editor->caret_pos) * 10.4 * app.scale + app.margin_x;
      caret_rect.y = 4 + app.margin_y + app.code_font->baseline * (editor_newlines_before(editor, editor->caret_pos) + 1 - editor->scroll) + app.line_offset * (editor_newlines_before(editor, editor->caret_pos) + 1 - editor->scroll);
      caret_rect.w = 3 * app.scale;
      caret_rect.h = app.code_font->baseline + 4;
      SDL_SetRenderDrawColor(app.renderer, 200, 200, 255, 200);
      SDL_RenderFillRect(app.renderer, &caret_rect);
    } else {
      SDL_SetWindowTitle(app.win, "MOP");
      render_text(app.renderer, app.code_font, app.margin_x, app.code_font->baseline + app.margin_y, "No open editors.");
    }
    SDL_RenderSetClipRect(app.renderer, NULL);
    draw_notifs(&app);
    SDL_RenderPresent(app.renderer);
    SDL_Delay(1000 / 60);
    (void) delta;
  }

  for (i = 0; i < (int)(app.editors.len); ++i)
    free(((Editor*)(ll_list_get(app.editors, i)))->text);

  ll_list_free(&app.editors);

  close_font(app.code_font);
  SDL_DestroyWindow(app.win);
  SDL_DestroyRenderer(app.renderer);
  SDL_Quit();
  return 0;
}

