#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
  /* "Private fields" */
  int _keybind_index;
  /* Behaviour */
  char build_cmd[64];
  /* Layout */
  int margin_x;
  int margin_y;
  int line_offset;
  /* Colours */
} Config;

#endif /* CONFIG_H */

