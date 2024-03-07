#include "math.h"
#include "stb_sprintf.h"
#include "stdio.h"
#include "string.h"

#include "app.h"
#include "editor.h"
#include "editor_view.h"
#include "font.h"
#include "keymap.h"
#include "render.h"
#include "util.h"

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include "Windows.h"
#endif /* _WIN32 */

int main(int argc, char** argv) {
  App app;
  char ctrl = 0;
  char running = 1;
  char super = 0;
  char shift = 0;
  /* Hope and pray that no one needs a million editor views. */
  EditorView editor_views[MAX_EDITOR_VIEWS];
  EditorView* current_view = &editor_views[0];
  int i;
  SDL_DisplayMode dm;
  SDL_Event event;
  SDL_Rect editor_rect;
  Uint64 prev = 0;

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
  app.win_width = dm.w * 0.6f;
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
  populate_default_config(&app.config, app.scale);
  read_config(&app.config, PROJECT_CONFIG_FILE, app.scale);
  app.editors.len = 0;
  app.editors.first = NULL;
  app.current_editor = 0;

  app.time = 0;

  app.code_font = open_font(app.renderer, app.config.code_font_file_name, app.config.font_size * app.scale);
  app.notif.notifs.first = NULL;
  app.notif.notifs.len = 0;

  init_editor_view(&app, &editor_views[0]);
  editor_views[0].visible = 1;

  if (argc > 1) {
    for (i = 1; i < argc; ++i) {
      char notif[256] = {0};
      FILE* f;
      Editor* editor = create_editor(argv[i]);
      int pos = 0;
      f = fopen(argv[i], "r");
      if (f == NULL) {
        fclose(f);
        printf("Failed to open file \"%s\"", argv[i]);
        return -1;
      }
      for (;;) {
        char c = fgetc(f);
        editor_insert_at(editor, c, pos);
        if (feof(f)) {
          editor_remove_at(editor, pos - 1);
          editor_remove_at(editor, pos - 1);
          break;
        }
        ++pos;
      }
      fclose(f);
      ll_list_add(&app.editors, editor);
      stbsp_snprintf(notif, 256, "Opened %s", argv[i]);
      add_notif(&app.notif, create_notif(notif));
    }
  }

  for (;running;) {
    Uint64 now = SDL_GetPerformanceCounter();
    float delta = (now - prev) / (float) SDL_GetPerformanceFrequency();
    if (delta > 1.0f) delta = 0.0f;
    prev = now;
    app.time += delta;
    process_notifs(&app.notif, delta);
    for (;SDL_PollEvent(&event) != 0;) {
      switch (event.type) {
        case SDL_QUIT: {
          running = 0;
          break;
        }
        case SDL_KEYUP: {
          switch (event.key.keysym.sym) {
            case SDLK_LALT: {
              super = 0;
              break;
            }
            case SDLK_LCTRL: {
              ctrl = 0;
              break;
            }
          }
          break;
        }
        case SDL_KEYDOWN: {
          switch (event.key.keysym.sym) {
            case SDLK_LALT: {
              super = 1;
              break;
            }
            case SDLK_LCTRL: {
              ctrl = 1;
              break;
            }
          }
          keydown_editor_view(current_view, event.key.keysym.sym, ctrl, super, shift);
          break;
        }
        case SDL_TEXTINPUT: {
          if (super) break;
          for (i = 0; i < (int)strlen(event.text.text); ++i) {
            text_input_editor_view(current_view, *event.text.text);
          }
          break;
        }
        case SDL_MOUSEMOTION: {
          app.mouse_x = event.motion.x;
          app.mouse_y = event.motion.y;
          break;
        }
        case SDL_MOUSEWHEEL: {
          scroll_editor_view(current_view, event.wheel.y * 5 * app.scale);
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
        case SDL_MOUSEBUTTONDOWN: {
          if (event.button.button == SDL_BUTTON_LEFT) {
            mouse_button_down_editor_view(&editor_views[0]);
          }
          break;
        }
        default: {
          break;
        }
      }
    }
    editor_rect.x = app.config.margin_x;
    editor_rect.y = app.config.margin_y + app.config.line_offset;
    editor_rect.w = app.win_width - app.config.margin_x * 2;
    editor_rect.h = app.win_height - (app.config.margin_y + app.config.line_offset) * 2;
    SET_COLOR(app.renderer, app.config.bg_color, 255);
    SDL_RenderClear(app.renderer);
    SET_COLOR(app.renderer, app.config.text_color, 255);

    set_editor_view_rect(current_view, &editor_rect);
    render_editor_view(current_view, delta);

    SDL_RenderSetClipRect(app.renderer, NULL);
    draw_notifs(&app);
    SDL_RenderPresent(app.renderer);
    SDL_Delay(1000 / 60);
  }

  for (i = 0; i < (int)(app.editors.len); ++i)
    close_editor(((Editor*)(ll_list_get(app.editors, i))));

  ll_list_free(&app.editors);

  close_font(app.code_font);
  SDL_DestroyWindow(app.win);
  SDL_DestroyRenderer(app.renderer);
  SDL_Quit();
  return 0;
}

