#include "editor.h"
#include "editor_view.h"
#include "util.h"

void init_editor_view(App* app, EditorView* view) {
  view->app = app;
  view->editor_index = 0;
  view->caret_x = 0;
  view->caret_y = 0;
  view->caret_rect.w = 3 * app->scale;
  view->caret_rect.h = floorf(app->code_font->baseline + 7.5f * app->scale);
  view->editor = NULL;
  view->line_number_width = 8;
  view->visible = 0;
}

void set_editor_view_rect(EditorView* view, SDL_Rect* r) {
  view->area = *r;
}

void render_editor_view(EditorView* view, float delta) {
  int i;
  if (!view->visible) return;
  if (view->editor_index < 0 || view->app->editors.first == NULL)
    view->editor = NULL;
  else
    view->editor = (Editor*)(ll_list_get(view->app->editors, view->editor_index));

  SDL_RenderSetClipRect(view->app->renderer, &view->area);

  if (view->editor != NULL) {
    view->line_number_width += (1 - powf(2, - 40.0f * delta)) *
      (2 * view->app->config.margin_x + (view->app->code_font->stride * (count_digits(view->editor->lines) + 1)) - view->line_number_width);
    int text_start = view->line_number_width + 6 * view->app->scale;
    SDL_Rect r;
    r.y = 0;
    r.w = 2 * view->app->scale;
    r.x = view->line_number_width;
    r.h = view->app->win_height;
    SDL_RenderFillRect(view->app->renderer, &r);
    /* Prevent the user from scrolling too far. */
    if (view->editor->target_scroll < 0) view->editor->target_scroll = 0;
    if (view->editor->target_scroll > view->editor->lines - 1) view->editor->target_scroll = view->editor->lines - 1;
    /* Prevent the user from moving the cursor out of bounds */
    if (view->editor->caret_pos < 0) view->editor->caret_pos = 0;
    if (view->editor->caret_pos > (int)strlen(view->editor->text) - 1) view->editor->caret_pos = (int)strlen(view->editor->text) - 1;

    if (view->app->config.line_len_suggestor > 0) {
      r.x += view->app->code_font->stride * view->app->config.line_len_suggestor + 2 + r.w;
      SET_COLOR(view->app->renderer, view->app->config.text_color, 127);
      SDL_RenderFillRect(view->app->renderer, &r);
      SET_COLOR(view->app->renderer, view->app->config.text_color, 255);
    }

    char title[128] = {0};
    if (view->editor->name[0] != '\0')
      stbsp_snprintf(title, 128, "MOP - %s", view->editor->name);
    else
      stbsp_snprintf(title, 128, "MOP - Unnamed Editor");
    SDL_SetWindowTitle(view->app->win, title);

    view->editor->scroll += (1 - powf(2, - 35.0f * delta)) * (view->editor->target_scroll - view->editor->scroll);
    /* If scroll is near the epsilon value, set it to zero. */
#define epsilon (0.075f)
    if (view->editor->scroll < epsilon)
      view->editor->scroll = 0;
#undef epsilon

    view->caret_x += (1 - powf(2, - 45.0f * delta)) *
      (editor_len_until_prev_line(view->editor, view->editor->caret_pos) * view->app->code_font->stride - view->caret_x);

    /* What in the literal fuck is this? I guess it works... */
    view->caret_y += (1 - powf(2, - 40.0f * delta)) *
      (view->app->config.line_offset + view->app->code_font->baseline * (editor_newlines_before(view->editor, view->editor->caret_pos)
      + 1)
      + view->app->config.line_offset * (editor_newlines_before(view->editor, view->editor->caret_pos)
      + 1) - view->caret_y);

    view->caret_rect.x = floorf(view->caret_x + text_start);
    view->caret_rect.y = floorf(view->caret_y + view->app->config.margin_y
      - (view->editor->scroll * (view->app->code_font->baseline + view->app->config.line_offset))
      + ((view->editor->scroll - floorf(view->editor->scroll))));

    {
      SDL_Rect line_rect = {0};
      line_rect.x = text_start;
      line_rect.y = view->caret_rect.y;
      line_rect.w = view->area.w;
      line_rect.h = view->caret_rect.h;

      SET_COLOR(view->app->renderer, view->app->config.line_highlight_color, 255);
      SDL_RenderFillRect(view->app->renderer, &line_rect);
    }

    SET_COLOR(view->app->renderer, view->app->config.text_color, 255);
    for (i = 0; i < view->area.h / (view->app->config.line_offset + view->app->code_font->baseline) + 2; ++i) {
      int y = view->app->config.margin_y + view->app->code_font->baseline * (i + 1)
        + view->app->config.line_offset * (i + 1) - (view->editor->scroll
        - floorf(view->editor->scroll)) * (view->app->code_font->baseline + view->app->config.line_offset);

      if (editor_render_line(view->editor, i + (int)view->editor->scroll, text_start, y,
        view->app->renderer, view->app->code_font) == 1)
      {
        char line_text[8] = {0};
        stbsp_snprintf(line_text, 6, "%d", i + (int)view->editor->scroll + 1);
        if (editor_newlines_before(view->editor, view->editor->caret_pos) + 1 == i + floorf(view->editor->scroll)) {
          SET_COLOR(view->app->renderer, view->app->config.bright_text_color, 255);
        }
        render_text(view->app->renderer, view->app->code_font,
          view->line_number_width - (5 * view->app->scale) - ((int)strlen(line_text) * view->app->code_font->stride) - view->app->config.margin_x,
          y,
          line_text);
        SET_COLOR(view->app->renderer, view->app->config.text_color, 255);
      }
    }

    SET_COLOR(view->app->renderer, view->app->config.caret_color, 155);
    SDL_RenderFillRect(view->app->renderer, &view->caret_rect);
  } else {
    char* text = "No open editors. Get to it.";
    SDL_SetWindowTitle(view->app->win, "MOP");
    render_text(view->app->renderer, view->app->code_font,
      view->app->win_width * 0.5f - ((int)strlen(text) * view->app->code_font->stride * 0.5f),
      view->app->win_height * 0.5f + (view->app->code_font->baseline * 0.5f),
      text
    );
  }
}

void keydown_editor_view(EditorView* view, SDL_Keycode key, char ctrl, char super, char shift) {
  (void)shift;
  if (!view->visible) return;
  switch (key) {
    case SDLK_o: {
      if (ctrl) {
        add_notif(&view->app->notif, create_notif("Open File."));
      }
      break;
    }
    case SDLK_s: {
      if (ctrl) {
        char notif[256] = {0};
        stbsp_snprintf(notif, 256, "Saved %s", view->editor->name);
        editor_save(view->editor, &view->app->notif);
        add_notif(&view->app->notif, create_notif(notif));
      }
      break;
    }
    case SDLK_b: {
      if (!ctrl) break;
      if (view->app->config.build_cmd[0] != '\0') {
        system(view->app->config.build_cmd);
        add_notif(&view->app->notif, create_notif("Build Complete"));
      }
      add_notif(&view->app->notif, create_notif("No Build Command Specified"));
      break;
    }
    case SDLK_w: {
      if (ctrl) {
        char notif[256] = {0};
        stbsp_snprintf(notif, 256, "Closed %s", view->editor->name);
        close_editor(view->editor);
        ll_list_remove_nth(&view->app->editors, view->editor_index);
        add_notif(&view->app->notif, create_notif(notif));
        view->editor_index = view->app->editors.len - 1;
        if (view->app->editors.first != NULL)
          view->editor = (Editor*)(ll_list_get(view->app->editors, view->editor_index));
        else
          view->editor = NULL;
      }
      break;
    }
    case SDLK_0: {
      if (super) {
        view->editor_index = 0;
      }
      break;
    }
    case SDLK_1: {
      if (super) {
        view->editor_index = 1;
      }
      break;
    }
    default: {
      keydown_editor(view->editor, key, ctrl);
      break;
    }
  }
}

void mouse_button_down_editor_view(EditorView* view) {
  int newlines = 0;
  int line = 0;
  int pos_on_line = 0;
  if (view->editor == NULL) return;
  line = roundf(view->editor->scroll)
    + (((view->app->mouse_y - view->app->config.margin_y - 0.25f * (view->app->config.line_offset
    + view->app->code_font->baseline))
    - (view->editor->scroll - floorf(view->editor->scroll))) / (view->app->config.line_offset + view->app->code_font->baseline));

  pos_on_line = (view->app->mouse_x
    - (2 * view->app->config.margin_x + (view->app->code_font->stride * (count_digits(view->editor->lines) + 1)) + 6 * view->app->scale))
    / view->app->code_font->stride + 1;
  view->editor->caret_pos = 0;
  for (;newlines < line;) {
    if (view->editor->text[view->editor->caret_pos] == '\n')
      ++newlines;
    ++view->editor->caret_pos;
  }
  if (pos_on_line < editor_len_until_next_line(view->editor, view->editor->caret_pos))
    view->editor->caret_pos += pos_on_line;
  else
    view->editor->caret_pos += editor_len_until_next_line(view->editor, view->editor->caret_pos);
}

void text_input_editor_view(EditorView* view, char ch) {
  if (view->editor == NULL) return;
  editor_insert_at(view->editor, ch, view->editor->caret_pos);
  ++view->editor->caret_pos;
#define X(c, a) if (ch == (c)) editor_insert_at(view->editor, (a), view->editor->caret_pos);
  EDITOR_AUTOINSERT
#undef X
}

void scroll_editor_view(EditorView* view, float scroll) {
  if (view->editor == NULL) return;
  view->editor->target_scroll -= scroll;
}

