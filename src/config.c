#include "stb_sprintf.h"
#include "string.h"

#include "config.h"

void populate_default_config(Config* config, float scale) {
  config->font_size = 26 * scale;
  config->margin_x = 6 * scale;
  config->margin_y = 8 * scale;
  config->line_offset = 6 * scale;
  config->line_len_suggestor = -1;
  stbsp_snprintf(config->code_font_file_name, 64, "code.ttf");
  memset(config->build_cmd, 0, 128);

  config->bg_color = (Color) {20, 20, 20};
  config->text_color = (Color) {200, 200, 200};
  config->bright_text_color = (Color) {255, 255, 255};
  config->line_highlight_color = (Color) {45, 45, 45};
  config->caret_color = (Color) {200, 210, 255};
}

void read_config(Config* config, const char* file, float scale) {
  (void)config;
  (void)file;
  (void)scale;
}

