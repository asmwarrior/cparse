#include "parser.h"
#include "context.h"
#include "combine_yacc.h"
#include "pp_lex.h"
#include <stdio.h>

extern int combineparse(ASTNode **proot);

int parseString(ASTNode **proot, const char *str)
{
    int ret;
    YY_BUFFER_STATE buf;
    buf = pp_scan_string(str);
    pppush_buffer_state(buf);
    ret = combineparse(proot);
    pp_flush_buffer(buf);
    pppop_buffer_state();
    return ret;
}

int parseFile(ASTNode **proot, FILE *f)
{
    int ret;
    YY_BUFFER_STATE buf;
    buf = pp_create_buffer(f, YY_BUF_SIZE);
    pppush_buffer_state(buf);
    ret = combineparse(proot);
    pp_flush_buffer(buf);
    pppop_buffer_state();
    return ret;
}
