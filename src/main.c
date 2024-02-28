#include "math.h"
#include "stb_sprintf.h"
#include "stdio.h"
#include "string.h"

#include "app.h"
#include "editor.h"
#include "font.h"
#include "keymap.h"
#include "render.h"

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include "Windows.h"
#endif /* _WIN32 */

int main(int argc, char** argv) {
  App app;
  char ctrl = 0;
  char running = 1;
  char super = 0;
  float caret_x = 0.0f;
  float caret_y = 0.0f;
  int i;
  SDL_DisplayMode dm;
  SDL_Event event;
  SDL_Rect caret_rect;
  SDL_Rect editor_rect;
  Uint64 prev = 0;

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

  app.code_font = open_font(app.renderer, app.config.code_font_file_name, app.config.font_size * app.scale);
  app.notif.notifs.first = NULL;
  app.notif.notifs.len = 0;

  caret_rect.w = 3 * app.scale;
  caret_rect.h = app.code_font->baseline + 4;

  if (argc == 2) {
    char notif[256] = {0};
    FILE* f;
    Editor* editor = create_editor(argv[1]);
    int pos = 0;
    f = fopen(argv[1], "r");
    if (f == NULL) {
      fclose(f);
      printf("Failed to open file \"%s\"", argv[1]);
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
    stbsp_snprintf(notif, 256, "Opened %s", argv[1]);
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
          /*Keybind* binds = get_keybinds();
          for (i = 0; i < MAX_KEYBINDS; ++i) {
            if (binds[i].proc == NULL) break;
            if (event.key.keysym.sym == binds[i].key) binds[i].proc(&app);
          }*/
          switch (event.key.keysym.sym) {
            case SDLK_o: {
              if (super) {
                add_notif(&app.notif, create_notif("Open file."));
              }
              break;
            }
            case SDLK_s: {
              if (super) {
                editor_save(editor, &app.notif);
              }
              break;
            }
            case SDLK_b: {
              if (super && app.config.build_cmd[0] != '\0') {
                system(app.config.build_cmd);
                add_notif(&app.notif, create_notif("Build complete"));
              }
              break;
            }
            case SDLK_LALT: {
              super = 1;
              break;
            }
            case SDLK_LCTRL: {
              ctrl = 1;
              break;
            }
          }
          if (editor != NULL)
            keydown_editor(editor, event.key.keysym.sym, ctrl);
          break;
        }
        case SDL_TEXTINPUT: {
          if (super || editor == NULL) break;
          /* FIX: can only deal with ascii */
          for (i = 0; i < (int)strlen(event.text.text); ++i) {
            editor_insert_at(editor, event.text.text[i], editor->caret_pos);
            ++editor->caret_pos;
          }
#define AUTOCLOSE(c, a) if (*event.text.text == (c)) editor_insert_at(editor, (a), editor->caret_pos)
          AUTOCLOSE('(', ')');
          AUTOCLOSE('{', '}');
          AUTOCLOSE('\'', '\'');
          AUTOCLOSE('"', '"');
          AUTOCLOSE('[', ']');
          AUTOCLOSE('`', '`');
#undef AUTOCLOSE
          break;
        }
        case SDL_MOUSEWHEEL: {
          editor->target_scroll -= event.wheel.y * 5 * app.scale;
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
    /* Prevent the user from scrolling too far. */
    if (editor->target_scroll < 0) editor->target_scroll = 0;
    if (editor->target_scroll > editor->lines - 1) editor->target_scroll = editor->lines - 1;

    editor_rect.x = app.config.margin_x;
    editor_rect.y = app.config.margin_y + app.config.line_offset;
    editor_rect.w = app.win_width - app.config.margin_x * 2;
    editor_rect.h = app.win_height - (app.config.margin_y + app.config.line_offset) * 2;
    SDL_SetRenderDrawColor(app.renderer, 20, 20, 20, 255);
    SDL_RenderClear(app.renderer);
    SDL_SetRenderDrawColor(app.renderer, 200, 200, 200, 255);
    SDL_RenderSetClipRect(app.renderer, &editor_rect);
    if (app.editors.first != NULL) {
      SDL_Rect r;
      r.x = app.config.margin_x + app.code_font->stride * 5 - 2;
      r.y = 0;
      r.w = 2 * app.scale + 1;
      r.h = app.win_height;
      SDL_RenderFillRect(app.renderer, &r);

      if (app.config.line_len_suggestor > 0) {
        r.x += app.code_font->stride * app.config.line_len_suggestor + 2 + r.w;
        SDL_SetRenderDrawColor(app.renderer, 200, 200, 200, 127);
        SDL_RenderFillRect(app.renderer, &r);
        SDL_SetRenderDrawColor(app.renderer, 200, 200, 200, 255);
      }

      SDL_SetWindowTitle(app.win, editor->name);

      editor->scroll += (1 - powf(2, - 40.0f * delta)) * (editor->target_scroll - editor->scroll);

      caret_x += (1 - powf(2, - 40.0f * delta)) *
        ((editor_len_until_prev_line(editor, editor->caret_pos) * app.code_font->stride + (app.config.margin_x + (app.code_font->stride * 5))) - caret_x);

      caret_y += (1 - powf(2, - 40.0f * delta)) *
        ((6 + app.config.margin_y + app.code_font->baseline * (editor_newlines_before(editor, editor->caret_pos) + 1 - editor->scroll)
        + app.config.line_offset * (editor_newlines_before(editor, editor->caret_pos) + 1 - editor->scroll)) - caret_y);

      caret_rect.x = caret_x + 4 * app.scale;
      caret_rect.y = caret_y;

      for (i = 0; i < editor_rect.h / (app.config.line_offset + app.code_font->baseline) + 2; ++i) {
        int y = app.config.margin_y + app.code_font->baseline * (i + 1)
          + app.config.line_offset * (i + 1) - (editor->scroll
          - floorf(editor->scroll)) * app.code_font->baseline;

        if (editor_render_line(editor, i + (int)editor->scroll,
          app.config.margin_x + (app.code_font->stride * 5) + (5 * app.scale),
          y,
          app.renderer, app.code_font) == 1)
        {
          char line_text[7] = {0};
          stbsp_snprintf(line_text, 6, "%d", i + (int)editor->scroll);
          render_text(app.renderer, app.code_font, app.config.margin_x,
            y,
            line_text);
        }
      }

      SDL_SetRenderDrawColor(app.renderer, 200, 200, 255, 200);
      SDL_RenderFillRect(app.renderer, &caret_rect);
    } else {
      SDL_SetWindowTitle(app.win, "MOP");
      render_text(app.renderer, app.code_font, app.config.margin_x, app.code_font->baseline + app.config.margin_y, "No open editors.");
    }

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

