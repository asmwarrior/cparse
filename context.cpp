#include "context.h"

Context::Context()
    : root(NULL),
      lexer(NULL),
      includeDepth(0)
{
}

