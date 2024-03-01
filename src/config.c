#include "stb_sprintf.h"
#include "string.h"

#include "config.h"

void populate_default_config(Config* config, float scale) {
  config->font_size = 26;
  config->margin_x = 6 * scale;
  config->margin_y = 8 * scale;
  config->line_offset = 6 * scale;
  config->line_len_suggestor = 0;
  stbsp_snprintf(config->code_font_file_name, 64, "code.ttf");
  memset(config->build_cmd, 0, 128);
}

void read_config(Config* config, const char* file, float scale) {
  (void)config;
  (void)scale;
  (void)file;
}

