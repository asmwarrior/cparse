#include "pp.h"
#include "ppcontext.h"
#include "pp_lex.h"
#include "pp_yacc.h"
#include <stdio.h>

extern int ppparse(PP::Context *ctx);

namespace PP {
int parseString(const char *str)
{
    int ret;
    Context ctx;
    YY_BUFFER_STATE buf;
    buf = pp_scan_string(str);
    pppush_buffer_state(buf);
    ret = ppparse(&ctx);
    pp_flush_buffer(buf);
    pppop_buffer_state();
    return ret;
}

int parseFile(FILE *f)
{
    int ret;
    Context ctx;
    YY_BUFFER_STATE buf;
    buf = pp_create_buffer(f, YY_BUF_SIZE);
    pppush_buffer_state(buf);
    ret = ppparse(&ctx);
    pp_flush_buffer(buf);
    pppop_buffer_state();
    return ret;
}

}
