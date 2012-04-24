%{
#include "ppast.h"
#ifndef YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED
typedef PP::ASTNode *YYSTYPE;
#endif
#include "pp_yacc.h"
%}
%option noyywrap nodefault yylineno
%option header-file="pp_lex.h"

%s SDEFINE
%x XLINESTART

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
punct               [\[\]\(\)\{\}\.&\*\+-~\/%\^\|<>\?:;=,#]|->|\+\+|--|<<|>>|<=|>=|==|!=|&&|\*=|\/=|%=|\+=|-=|<<=|>>=|&=|\^=|\|=|##|<:|:>|<%|%>|%:|%:%:|\.\.\.
ws                  [ \t]
newline             \n

%%
    BEGIN(XLINESTART);
<XLINESTART>#       {BEGIN(0);return LINESTART_HASH;}
<XLINESTART>.|\n    {yyless(0);BEGIN(0);}
if                  {return IF;}
ifdef               {return IFDEF;}
ifndef              {return IFNDEF;}
elif                {return ELIF;}
else                {return ELSE;}
endif               {return ENDIF;}
include             {return INCLUDE;}
define              {BEGIN(SDEFINE);return DEFINE;}
defined             {pplval = PP::AST::CreateOp(yytext); return DEFINED;}
undef               {return UNDEF;}
line                {return LINE;}
error               {return ERROR;}
pragma              {return PRAGMA;}

{id}                {pplval = PP::AST::CreateID(yytext); return ID;}
{pp_number}         {pplval = PP::AST::CreatePPNumber(yytext); return PP_NUMBER;}
{char_constant}     {
    pplval = PP::AST::CreateCharConstant(yytext);
    return CHAR_CONSTANT;
                    }
{string_literal}    {
    pplval = PP::AST::CreateStringLiteral(yytext);
    return STRING_LITERAL;
                    }
<SDEFINE>{id}\(     {
    QString str(yytext);
    str.chop(1);
    pplval = PP::AST::CreateID(str);
    BEGIN(0);
    return ID_FUNC;
}
{newline}           {BEGIN(XLINESTART);return NEWLINE;}
{ws}
{punct} {
    pplval = PP::AST::CreateOp(yytext);
    return static_cast<PP::ASTToken*>(pplval)->tokenType();
}
.   {
    pplval = PP::AST::CreateOp(yytext[0]);
    return static_cast<PP::ASTToken*>(pplval)->tokenType();
}

%%
