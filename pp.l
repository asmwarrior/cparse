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

%x XLINE_START XCONTROL_LINE XDEFINE

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
punct               [\[\]\(\)\{\}\.&\*\+-~\/%\^\|<>\?:;=,#!]|->|\+\+|--|<<|>>|<=|>=|==|::|!=|&&|\*=|\/=|%=|\+=|-=|<<=|>>=|&=|\^=|\|=|##|<:|:>|<%|%>|%:|%:%:|\.\.\.
ws                  [ \t]
newline             \n

%%
<*>^\/\*                        {comment();BEGIN(XLINE_START);}
<*>\/\*                         {comment();}
<*>\/\/.*\n 			

<*>^.                           {yyless(0);BEGIN(XLINE_START);}
<XLINE_START>#                  {BEGIN(XCONTROL_LINE);return LINESTART_HASH;}
<XLINE_START>{ws}    
<XLINE_START>.|\n               {yyless(0);BEGIN(0);}
<XCONTROL_LINE>if               {return IF;}
<XCONTROL_LINE>ifdef            {return IFDEF;}
<XCONTROL_LINE>ifndef           {return IFNDEF;}
<XCONTROL_LINE>elif             {return ELIF;}
<XCONTROL_LINE>else             {return ELSE;}
<XCONTROL_LINE>endif            {return ENDIF;}
<XCONTROL_LINE>include          {return INCLUDE;}
<XCONTROL_LINE>define           {BEGIN(XDEFINE);return DEFINE;}
<XCONTROL_LINE>defined          {pplval = CreateOp(yytext); return DEFINED;}
<XCONTROL_LINE>undef            {return UNDEF;}
<XCONTROL_LINE>line             {return LINE;}
<XCONTROL_LINE>error            {return ERROR;}
<XCONTROL_LINE>pragma           {return PRAGMA;}

<XDEFINE>{id}\(     {
    QString str(yytext);
    str.chop(1);
    pplval = CreateID(str);
    BEGIN(XCONTROL_LINE);
    return ID_FUNC;
}
<XDEFINE>{ws}
<XDEFINE>.          {yyless(0);BEGIN(XCONTROL_LINE);}
<*>{id}             {pplval = CreateID(yytext); return ID;}
<*>{pp_number}      {pplval = CreatePPNumber(yytext); return PP_NUMBER;}
<*>{char_constant}  {
    pplval = CreateCharConstant(yytext);
    return CHAR_CONSTANT;
                    }
<*>{string_literal} {
    pplval = CreateStringLiteral(yytext);
    return STRING_LITERAL;
                    }
<*>{newline}        {BEGIN(0);return NEWLINE;}
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

