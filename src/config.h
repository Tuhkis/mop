#ifndef CONFIG_H
#define CONFIG_H

#define CONFIG_FILE ((char*)"config")
#define PROJECT_CONFIG_FILE ((char*)".mop_config")

typedef struct config {
  /* "Private fields" */
  int _keybind_index;
  /* Behaviour */
  char build_cmd[128];
  /* Layout */
  char code_font_file_name[128];
  int margin_x;
  int margin_y;
  int line_offset;
  int font_size;
  int line_len_suggestor;
  /* Colours */
} Config;

void populate_default_config(Config* config, float scale);
void read_config(Config* config, const char* file, float scale);

#endif /* CONFIG_H */

