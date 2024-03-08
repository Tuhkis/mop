#ifndef SYNTAX_RULE_H
#define SYNTAX_RULE_H

#include "color.h"

typedef struct syntax_rule {
  char* rule;
  Color color;
} SyntaxRule;

static SyntaxRule c_syntax[64];

void register_syntax(void);

#endif /* SYNTAX_RULE_H */

