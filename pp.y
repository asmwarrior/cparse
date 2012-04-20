%{
#include "ppast.h"
#include "ppcontext.h"
#include <ctype.h>
#include <stdio.h>

using namespace PP;

#ifndef YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED
typedef ASTNode *YYSTYPE;
#endif

extern int pplineno;
extern int pplex();
extern ASTNode *pplval;
void pperror(Context *ctx, const char *);
%}

%parse-param    {Context *ctx}
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
%token DEFINED
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
%token TEXT_LINE
%token NON_DIRECTIVE
%token NODE_LIST
%token CONSTANT_EXPR

%%

pp_file :group  {$$ = $1;}
        |       {$$ = NULL;}
        ;
group   :group_part {$$ = AST::CreateGroup($1);}
        |group group_part {$$ = AST::CreateGroup(static_cast<AST::Group*>($1), $2);}
        ;
group_part  :if_group {$$ = $1;}
            |control_line {$$ = $1;}
            |text_line {$$ = $1;}
            |'#' non_directive {$$ = AST::CreateNonDirective(static_cast<ASTNodeList*>($2));}
            ;
if_group    : ifs_line group endif_line 
{
    $$ = AST::CreateIfGroup(static_cast<ASTConstantExpr*>($1),
            static_cast<ASTGroup*>($2),
            NULL);
}
            | ifs_line endif_line
{
    $$ = AST::CreateIfGroup(static_cast<ASTConstantExpr*>($1),
            NULL,
            NULL);
}
            | ifs_line elif_group endif_line
{
    $$ = AST::CreateIfGroup(static_cast<ASTConstantExpr*>($1),
            static_cast<ASTElIfGroup*>($2),
            NULL,
            NULL);
}
            | ifs_line elif_group group endif_line
{
    $$ = AST::CreateIfGroup(static_cast<ASTConstantExpr*>($1),
            static_cast<ASTElIfGroup*>($2),
            static_cast<ASTGroup*>($3),
            NULL);
}
            | ifs_line else_line endif_line
{
    $$ = AST::CreateIfGroup(static_cast<ASTConstantExpr*>($1),
            NULL,
            NULL);
}
            | ifs_line else_line group endif_line
{
    $$ = AST::CreateIfGroup(static_cast<ASTConstantExpr*>($1),
            NULL,
            static_cast<ASTGroup*>($3));
}
            | ifs_line elif_group else_line endif_line
{
    $$ = AST::CreateIfGroup(static_cast<ASTConstantExpr*>($1),
            static_cast<ASTElIfGroup*>($2),
            NULL,
            NULL);
}
            | ifs_line elif_group group else_line endif_line
{
    $$ = AST::CreateIfGroup(static_cast<ASTConstantExpr*>($1),
            static_cast<ASTElIfGroup*>($2),
            static_cast<ASTGroup*>($3),
            NULL);
}
            | ifs_line elif_group else_line group endif_line
{
    $$ = AST::CreateIfGroup(static_cast<ASTConstantExpr*>($1),
            static_cast<ASTElIfGroup*>($2),
            NULL,
            static_cast<ASTGroup*>($4));
}
            | ifs_line elif_group group else_line group endif_line
{
    $$ = AST::CreateIfGroup(static_cast<ASTConstantExpr*>($1),
            static_cast<ASTElIfGroup*>($2),
            static_cast<ASTGroup*>($3),
            static_cast<ASTGroup*>($5));
}
            ;
elif_group  : group elif_line   {
                  $$ = CreateElIfGroup(static_cast<ASTGroup*>($1), 
                                       static_cast<ASTConstantExpr*>($2));
                                }
            | elif_line         {
                  $$ = CreateElIfGroup(NULL,
                                       static_cast<ASTConstantExpr*>($1));
                                }
             
            | elif_group group elif_line
{
    $$ = CreateElIfGroup(static_cast<ASTElIfGroup*>($1), 
            static_cast<ASTGroup*>($2), 
            static_cast<ASTConstantExpr*>($3));
}
            | elif_group elif_line
{
    $$ = CreateElIfGroup(static_cast<ASTElIfGroup*>($1), 
            NULL,
            static_cast<ASTConstantExpr*>($2));
}
            ;
ifs_line    : '#' IF constant_expr NEWLINE      {$$ = CreateIf(static_cast<ASTConstantExpr*>($3));}
            | '#' IFDEF ID NEWLINE              {$$ = CreateIfdef(static_cast<ASTToken*>($3));}
            | '#' IFNDEF ID NEWLINE             {$$ = CreateIfndef(static_cast<ASTToken*>($3));}
            ;
elif_line   : '#' ELIF constant_expr NEWLINE    {$$=$3;}
            ;
else_line   : '#' ELSE NEWLINE
            ;
endif_line  : '#' ENDIF NEWLINE
            ;
control_line: '#' INCLUDE pp_tokens NEWLINE
{
    $$ = AST::CreateInclude(static_cast<ASTNodeList*>($3));
}
            | '#' DEFINE ID NEWLINE                   
{
    $$ = AST::CreateDefine(static_cast<ASTToken*>($3),
                               NULL,
                               NULL);
}
            | '#' DEFINE ID replacement_list NEWLINE
{
    $$ = AST::CreateDefine(static_cast<ASTToken*>($3),
                               NULL,
                               static_cast<ASTNodeList*>($4));
}
            | '#' DEFINE ID_FUNC  id_list ')' replacement_list NEWLINE
{
    $$ = AST::CreateDefine(static_cast<ASTToken*>($3),
                               static_cast<ASTNodeList*>($4),
                               static_cast<ASTNodeList*>($6));
}
            | '#' DEFINE ID_FUNC ')' replacement_list NEWLINE
{
    $$ = AST::CreateDefine(static_cast<ASTToken*>($3),
                               static_cast<ASTNodeList*>(AST::CreateNodeList()),
                               static_cast<ASTNodeList*>($5));
}
            | '#' DEFINE ID_FUNC "..." ')' replacement_list NEWLINE
{
    $$ = AST::CreateDefineVarArgs(static_cast<ASTToken*>($3),
                                      static_cast<ASTNodeList*>(AST::CreateNodeList()),
                                      static_cast<ASTNodeList*>($6));
}
            | '#' DEFINE ID_FUNC id_list ',' "..." ')' replacement_list NEWLINE
{
    $$ = AST::CreateDefineVarArgs(static_cast<ASTToken*>($3),
                                      static_cast<ASTNodeList*>($4),
                                      static_cast<ASTNodeList*>($8));
}
            | '#' UNDEF ID NEWLINE  {
                $$=AST::CreateUndef(static_cast<ASTToken*>($3));
                                    }
            | '#' LINE pp_tokens NEWLINE    {
                $$=AST::CreateLine(static_cast<ASTNodeList*>($3));
                                            }
            | '#' ERROR NEWLINE     {$$=AST::CreateNodeList();}
            | '#' ERROR pp_tokens NEWLINE   {
                $$=AST::CreateError(static_cast<ASTNodeList*>($3));
                                            }
            | '#' PRAGMA NEWLINE    {$$=AST::CreateEmpty("Empty pragma line");}
            | '#' PRAGMA pp_tokens NEWLINE  {}
            | '#' NEWLINE           {$$=AST::CreateEmpty("Empty control line");}
            ;
text_line   : NEWLINE               {$$=AST::CreateEmpty("Empty text line");}
            | pp_tokens NEWLINE     {$$=$1;}
            ;
non_directive   : pp_tokens NEWLINE {$$=$1;}
                ;
id_list     : ID                {$$=AST::CreateNodeList($1);}
            | id_list ',' ID    {static_cast<ASTNodeList*>($1)->append($2); $$=$1;}
            ;
replacement_list: pp_tokens     {$$=$1;}
                ;
constant_expr   : pp_tokens     {$$=AST::CreateConstantExpr(static_cast<ASTNodeList*>($1));}
                ;
pp_tokens   : pp_token          {$$=AST::CreateNodeList($1);}
            | pp_tokens pp_token{static_cast<ASTNodeList*>($1)->append($2); $$=$1;}
            ;
pp_token    : ID                {$$=$1;}
            | PP_NUMBER         {$$=$1;}
            | CHAR_CONSTANT     {$$=$1;}
            | STRING_LITERAL    {$$=$1;}
            | DEFINED           {$$=$1;}
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

void pperror(Context *ctx, const char *str)
{
    fprintf(stderr, "%d:%s\n", pplineno, str);
}
