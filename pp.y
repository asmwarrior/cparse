%{
#include "ppast.h"
#include <ctype.h>
#include <stdio.h>

#ifndef YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED
typedef PP::ASTNode *YYSTYPE;
#endif

extern int pplineno;
extern int pplex();
extern PP::ASTNode *pplval;
void pperror(const char *);
%}

%token AUTO
%token BREAK
%token CASE
%token CHAR
%token CONST
%token CONTINUE
%token DEFAULT
%token DO
%token DOUBLE
%token ELSE
%token ENUM
%token EXTERN
%token FLOAT
%token FOR
%token GOTO
%token IF
%token INLINE
%token INT
%token LONG
%token REGISTER
%token RESTRICT
%token RETURN
%token SHORT
%token SIGNED
%token SIZEOF
%token STATIC
%token STRUCT
%token SWITCH
%token TYPEDEF
%token UNION
%token UNSIGNED
%token VOID
%token VOLATILE
%token WHILE
%token _BOOL
%token _COMPLEX
%token _IMAGINARY

%token MEMBER_REF       "->"
%token PLUS_PLUS        "++"
%token MINUS_MINUS      "--"
%token LSHIFT           "<<"
%token RSHIFT           ">>"
%token LE               "<="
%token GE               ">="
%token EQ               "=="
%token NE               "!="
%token AND_AND          "&&"
%token OR_OR            "||"
%token DOT_DOT_DOT      "..."
%token MULT_ASSIGN      "*="
%token DIV_ASSIGN       "/="
%token MOD_ASSIGN       "%="
%token PLUS_ASSIGN      "+="
%token MINUS_ASSIGN     "-="
%token LSHIFT_ASSIGN    "<<="
%token RSHIFT_ASSIGN    ">>="
%token AND_ASSIGN       "&="
%token XOR_ASSIGN       "^="
%token OR_ASSIGN        "|="
%token HASH_HASH        "##"

%token IFDEF
%token IFNDEF
%token ELIF
%token ENDIF
%token INCLUDE
%token DEFINE
%token UNDEF
%token LINE
%token ERROR
%token PRAGMA

%token ID
%token PP_NUMBER
%token CHAR_CONSTANT
%token STRING_LITERAL
%token HEADER_NAME
%token NEWLINE 10

/* fake tokens introduced by lexer */
%token ID_FUNC

/* fake tokens introduced by parser */
%token IF_GROUP
%token CONTROL_LINE
%token TEXT_LINE
%token NON_DIRECTIVE
%token NODE_LIST
%token CL_INCLUDE
%token CL_DEFINE
%token CL_UNDEF
%token CL_LINE
%token CL_ERROR
%token CL_PRAGMA

%%

pp_file :group
        |
        ;
group   :group_part
        |group group_part
        ;
group_part  :if_group
            |control_line
            |text_line
            |'#' non_directive
            ;
if_group    : ifs_line group endif_line
            | ifs_line endif_line
            | ifs_line elif_group endif_line
            | ifs_line elif_group group endif_line
            | ifs_line else_line endif_line
            | ifs_line else_line group endif_line
            | ifs_line elif_group else_line endif_line
            | ifs_line elif_group group else_line endif_line
            | ifs_line elif_group else_line group endif_line
            | ifs_line elif_group group else_line group endif_line
            ;
elif_group  : group elif_line
            | elif_line
            | elif_group group elif_line
            | elif_group elif_line
            ;
ifs_line    : '#' IF constant_expr NEWLINE
            | '#' IFDEF ID NEWLINE
            | '#' IFNDEF ID NEWLINE
            ;
elif_line   : '#' ELIF constant_expr NEWLINE
            ;
else_line   : '#' ELSE NEWLINE
            ;
endif_line  : '#' ENDIF NEWLINE
            ;
control_line: '#' INCLUDE pp_tokens NEWLINE
            | '#' DEFINE ID NEWLINE                     {printf("MacroDef:%s\n",$3->spellName());}
            | '#' DEFINE ID replacement_list NEWLINE
            | '#' DEFINE ID_FUNC  id_list ')' replacement_list NEWLINE
            | '#' DEFINE ID_FUNC ')' replacement_list NEWLINE
            | '#' DEFINE ID_FUNC "..." ')' replacement_list NEWLINE
            | '#' DEFINE ID_FUNC id_list ',' "..." ')' replacement_list NEWLINE
            | '#' UNDEF ID NEWLINE
            | '#' LINE pp_tokens NEWLINE
            | '#' ERROR NEWLINE
            | '#' ERROR pp_tokens NEWLINE
            | '#' PRAGMA NEWLINE
            | '#' PRAGMA pp_tokens NEWLINE
            | '#' NEWLINE
            ;
text_line   : NEWLINE               {$$=PP::AST::CreateNodeList()};
            | pp_tokens NEWLINE     {$$=$1;}
            ;
non_directive   : pp_tokens NEWLINE {$$=$1;}
                ;
id_list     : ID                {$$=PP::AST::CreateNodeList($1);}
            | id_list ',' ID    {static_cast<PP::ASTNodeList*>($1)->append($2); $$=$1;}
            ;
replacement_list: pp_tokens     {$$=$1;}
                ;
constant_expr   : pp_tokens     {$$=$1;}
                ;
pp_tokens   : pp_token          {$$=PP::AST::CreateNodeList($1);}
            | pp_tokens pp_token{static_cast<PP::ASTNodeList*>($1)->append($2); $$=$1;}
            ;
pp_token    : ID                {$$=$1;}
            | PP_NUMBER         {$$=$1;}
            | CHAR_CONSTANT     {$$=$1;}
            | STRING_LITERAL    {$$=$1;}
            | '['               {$$=$1;}
            | ']'               {$$=$1;}
            | '('               {$$=$1;}
            | ')'               {$$=$1;}
            | '{'               {$$=$1;}
            | '}'               {$$=$1;}
            | '.'               {$$=$1;}
            | "->"              {$$=$1;}
            | "++"              {$$=$1;}
            | "--"              {$$=$1;}
            | '&'               {$$=$1;}
            | '*'               {$$=$1;}
            | '+'               {$$=$1;}
            | '-'               {$$=$1;}
            | '~'               {$$=$1;}
            | '!'               {$$=$1;}
            | '/'               {$$=$1;}
            | '%'               {$$=$1;}
            | "<<"              {$$=$1;}
            | ">>"              {$$=$1;}
            | '<'               {$$=$1;}
            | '>'               {$$=$1;}
            | "<="              {$$=$1;}
            | ">="              {$$=$1;}
            | "=="              {$$=$1;}
            | "!="              {$$=$1;}
            | '^'               {$$=$1;}
            | '|'               {$$=$1;}
            | "&&"              {$$=$1;}
            | "||"              {$$=$1;}
            | '?'               {$$=$1;}
            | ':'               {$$=$1;}
            | ';'               {$$=$1;}
            | "..."             {$$=$1;}
            | '='               {$$=$1;}
            | "*="              {$$=$1;}
            | "/="              {$$=$1;}
            | "%="              {$$=$1;}
            | "+="              {$$=$1;}
            | "-="              {$$=$1;}
            | "<<="             {$$=$1;}
            | ">>="             {$$=$1;}
            | "&="              {$$=$1;}
            | "^="              {$$=$1;}
            | "|="              {$$=$1;}
            | ','               {$$=$1;}
            | "##"              {$$=$1;}
            ;

%%

void pperror(const char *str)
{
    fprintf(stderr, "%d:%s\n", pplineno, str);
}
