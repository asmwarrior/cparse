%{
#include "ppast.h"
#ifndef YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED
typedef PP::ASTNode *YYSTYPE;
#endif
#include "pp_yacc.h"
%}
%option noyywrap nodefault yylineno

%s SDEFINE

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
auto                {return AUTO;}
break               {return BREAK;}
case                {return CASE;}
char                {return CHAR;}
const               {return CONST;}
continue            {return CONTINUE;}
default             {return DEFAULT;}
do                  {return DO;}
double              {return DOUBLE;}
else                {return ELSE;}
enum                {return ENUM;}
extern              {return EXTERN;}
float               {return FLOAT;}
for                 {return FOR;}
goto                {return GOTO;}
if                  {return IF;}
inline              {return INLINE;}
int                 {return INT;}
long                {return LONG;}
register            {return REGISTER;}
restrict            {return RESTRICT;}
return              {return RETURN;}
short               {return SHORT;}
signed              {return SIGNED;}
sizeof              {return SIZEOF;}
static              {return STATIC;}
struct              {return STRUCT;}
switch              {return SWITCH;}
typedef             {return TYPEDEF;}
union               {return UNION;}
unsigned            {return UNSIGNED;}
void                {return VOID;}
volatile            {return VOLATILE;}
while               {return WHILE;}
_Bool               {return _BOOL;}
_Complex            {return _COMPLEX;}
_Imaginary          {return _IMAGINARY;}

ifdef               {return IFDEF;}
ifndef              {return IFNDEF;}
elif                {return ELIF;}
endif               {return ENDIF;}
include             {return INCLUDE;}
define              {BEGIN(SDEFINE);return DEFINE;}
undef               {return UNDEF;}
line                {return LINE;}
error               {return ERROR;}
pragma              {return PRAGMA;}

{id}                {return ID;}
{pp_number}         {return PP_NUMBER;}
{char_constant}     {return CHAR_CONSTANT;}
{string_literal}    {return STRING_LITERAL;}

<SDEFINE>{id}\(     {BEGIN(0);return ID_FUNC;}

{newline}           {return NEWLINE;}
{ws}
{punct}             {pplval = PP::AST::CreatePunct(yytext); return pplval->type();}
.                   {pplval = PP::AST::CreatePunct(yytext[0]); return pplval->type();}

%%
