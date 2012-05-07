%{
#include "ast.h"
#define YY_DECL int pplex(ASTNode *&pplval)
#include "combine_yacc.h"
#include <ctype.h>
#include <QDebug>

static void comment();

%}
%option noyywrap nodefault yylineno
%option header-file="pp_lex.h"

%x XLINE_START XDIRECTIVE XDEFINE

digit       [0-9]
nondigit    [_a-zA-Z]
octdigit    [0-7]
hexdigit    [0-9a-fA-F]
hexquad     {hexdigit}{hexdigit}{hexdigit}{hexdigit}
ucn         \\u{hexquad}|\\U{hexquad}{hexquad}
id_nondigit {nondigit}|{ucn}
id          {id_nondigit}({id_nondigit}|{digit})*

h_char      [^\n>]
h_char_seq  {h_char}+
q_char      [^\n\"]
q_char_seq  {q_char}+
header_name <{h_char_seq}>|\"{q_char_seq}\"

sign        [\+-]
pp_number_tail  {digit}|{id_nondigit}|e{sign}|E{sign}|p{sign}|P{sign}|\.
pp_number   ({digit}|\.{digit}){pp_number_tail}*

c_char      [^\n'\\]|{escape_seq}
c_char_seq  {c_char}+
escape_seq  {simple_escape_seq}|{oct_escape_seq}|{hex_escape_seq}|{ucn}
simple_escape_seq   \\'|\\\"|\\\?|\\\\|\\a|\\b|\\f|\\n|\\r|\\t|\\v
oct_escape_seq      \\{octdigit}|\\{octdigit}{octdigit}|\\{octdigit}{octdigit}{octdigit}
hex_escape_seq      \\x{hexdigit}{hex_escape_seq_tail}*
hex_escape_seq_tail {hexdigit}
char_constant       '{c_char_seq}'|L'{c_char_seq}'

string_literal      \"{s_char_seq}?\"|L\"{s_char_seq}?\"
s_char_seq          {s_char}*
s_char              [^\"\\\n]|{escape_seq}
punct               [\[\]\(\)\{\}\.&\*\+-~\/%\^\|<>\?:;=,#!]|->|\+\+|--|<<|>>|<=|>=|==|::|!=|\|\||&&|\*=|\/=|%=|\+=|-=|<<=|>>=|&=|\^=|\|=|##|<:|:>|<%|%>|%:|%:%:|\.\.\.
ws                  [ \t]
newline             \n

%%
<*>^\/\*                        {comment();BEGIN(XLINE_START);}
<*>\/\*                         {comment();}
<*>\/\/.*\n                     {return NEWLINE;}

<*>^.                           {yyless(0);BEGIN(XLINE_START);}
<XLINE_START>#                  {BEGIN(XDIRECTIVE);return LINESTART_HASH;}
<XLINE_START>{ws}    
<XLINE_START>.|\n               {yyless(0);BEGIN(0);}
<XDIRECTIVE>if               {return IF;}
<XDIRECTIVE>ifdef            {return IFDEF;}
<XDIRECTIVE>ifndef           {return IFNDEF;}
<XDIRECTIVE>elif             {return ELIF;}
<XDIRECTIVE>else             {return ELSE;}
<XDIRECTIVE>endif            {return ENDIF;}
<XDIRECTIVE>include          {return INCLUDE;}
<XDIRECTIVE>define           {BEGIN(XDEFINE);return DEFINE;}
<XDIRECTIVE>defined          {pplval = CreateOp(yytext); return DEFINED;}
<XDIRECTIVE>undef            {return UNDEF;}
<XDIRECTIVE>line             {return LINE;}
<XDIRECTIVE>error            {BEGIN(0);return ERROR;}
<XDIRECTIVE>pragma           {return PRAGMA;}

<XDEFINE>{id}\(     {
    QString str(yytext);
    str.chop(1);
    pplval = CreateID(str);
    BEGIN(XDIRECTIVE);
    return ID_FUNC;
}
<XDEFINE>{ws}
<XDEFINE>.          {yyless(0);BEGIN(0);}
<XDEFINE>{id}       {BEGIN(XDIRECTIVE); pplval = CreateID(yytext); return ID;}
<INITIAL,XLINE_START,XDIRECTIVE>{id}             {pplval = CreateID(yytext); return ID;}
<*>{pp_number}      {pplval = CreatePPNumber(yytext); return PP_NUMBER;}
<*>{char_constant}  {
    pplval = CreateCharConstant(yytext);
    return CHAR_CONSTANT;
                    }
<*>{string_literal} {
    pplval = CreateStringLiteral(yytext);
    return STRING_LITERAL;
                    }
<*>{newline}        {BEGIN(XLINE_START);return NEWLINE;}
<*>{ws}
<*>{punct}          {
    pplval = CreateOp(yytext);
    return static_cast<ASTPPToken*>(pplval)->ppTokenType();
}
<*>.                {
    if (isgraph(yytext[0])) {
        pplval = CreateOp(yytext[0]);
        return static_cast<ASTPPToken*>(pplval)->ppTokenType();
    }
}

%%
void comment()
{
    char c, c1;

loop:
    while (((c = yyinput()) != '*') && (c != 0))
        ;

    if (((c1 = yyinput()) != '/') && (c != 0)) {
        unput(c1);
        goto loop;
    }

}

