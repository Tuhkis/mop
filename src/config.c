#include "stb_sprintf.h"

#include "config.h"

void populate_default_config(Config* config, float scale) {
  config->font_size = 26;
  config->margin_x = 10 * scale;
  config->margin_y = 8 * scale;
  config->line_offset = 6 * scale;
  config->line_len_suggestor = -1;
  stbsp_snprintf(config->code_font_file_name, 64, "code.ttf");
}

void read_config(Config* config, const char* file, float scale) {
  (void)config;
  (void)scale;
  (void)file;
}

