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
void pperror(ASTNode **proot, const char *);
%}

%parse-param    {ASTNode **proot}
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
%token LINESTART_HASH   "^#"
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
%token GROUP
%token NON_DIRECTIVE
%token IF_GROUP
%token ELIF_GROUP
%token ELIF_ELEM
%token TOKENS
%token TEXT_LINE
%token NODE_LIST
%token CONSTANT_EXPR

%%

pp_file :group  {$$ = $1; *proot = $$;}
        |       {$$ = NULL; *proot = $$;}
        ;
group   :group_part {$$ = CreateGroup($1);}
        |group group_part {static_cast<ASTGroup*>($1)->append($2); $$ = $1;}
        ;
group_part  :if_group {$$ = $1;}
            |control_line {$$ = $1;}
            |text_line {$$ = $1;}
            |"^#" non_directive {$$ = CreateNonDirective(static_cast<ASTPPTokens*>($2));}
            ;
if_group    : ifs_line group endif_line 
{
    $$ = CreateIfGroup(static_cast<ASTConstantExpr*>($1),
            static_cast<ASTGroup*>($2),
            NULL);
}
            | ifs_line endif_line
{
    $$ = CreateIfGroup(static_cast<ASTConstantExpr*>($1),
            NULL,
            NULL);
}
            | ifs_line elif_group endif_line
{
    $$ = CreateIfGroup(static_cast<ASTConstantExpr*>($1),
            static_cast<ASTElifGroup*>($2),
            NULL,
            NULL);
}
            | ifs_line elif_group group endif_line
{
    $$ = CreateIfGroup(static_cast<ASTConstantExpr*>($1),
            static_cast<ASTElifGroup*>($2),
            static_cast<ASTGroup*>($3),
            NULL);
}
            | ifs_line else_line endif_line
{
    $$ = CreateIfGroup(static_cast<ASTConstantExpr*>($1),
            NULL,
            NULL);
}
            | ifs_line else_line group endif_line
{
    $$ = CreateIfGroup(static_cast<ASTConstantExpr*>($1),
            NULL,
            static_cast<ASTGroup*>($3));
}
            | ifs_line elif_group else_line endif_line
{
    $$ = CreateIfGroup(static_cast<ASTConstantExpr*>($1),
            static_cast<ASTElifGroup*>($2),
            NULL,
            NULL);
}
            | ifs_line elif_group group else_line endif_line
{
    $$ = CreateIfGroup(static_cast<ASTConstantExpr*>($1),
            static_cast<ASTElifGroup*>($2),
            static_cast<ASTGroup*>($3),
            NULL);
}
            | ifs_line elif_group else_line group endif_line
{
    $$ = CreateIfGroup(static_cast<ASTConstantExpr*>($1),
            static_cast<ASTElifGroup*>($2),
            NULL,
            static_cast<ASTGroup*>($4));
}
            | ifs_line elif_group group else_line group endif_line
{
    $$ = CreateIfGroup(static_cast<ASTConstantExpr*>($1),
            static_cast<ASTElifGroup*>($2),
            static_cast<ASTGroup*>($3),
            static_cast<ASTGroup*>($5));
}
            ;
elif_group  : group elif_line   {
                  $$ = CreateElifGroup(static_cast<ASTGroup*>($1),
                                       static_cast<ASTConstantExpr*>($2));
                                }
            | elif_line         {
                  $$ = CreateElifGroup(NULL,
                                       static_cast<ASTConstantExpr*>($1));
                                }
             
            | elif_group group elif_line
{
    $$ = CreateElifGroup(static_cast<ASTElifGroup *>($1),
        static_cast<ASTGroup *>($2),
        static_cast<ASTConstantExpr *>($3));
}
            | elif_group elif_line
{
    $$ = CreateElifGroup(static_cast<ASTElifGroup*>($1),
            NULL,
            static_cast<ASTConstantExpr*>($2));
}
            ;
ifs_line    : "^#" IF constant_expr NEWLINE
{
    $$ = CreateIfExpr(static_cast<ASTConstantExpr*>($3));
}
            | "^#" IFDEF ID NEWLINE
{
    $$ = CreateIfdefExpr(static_cast<ASTPPToken*>($3));
}
            | "^#" IFNDEF ID NEWLINE
{
    $$ = CreateIfndefExpr(static_cast<ASTPPToken*>($3));
}
            ;
elif_line   : "^#" ELIF constant_expr NEWLINE    {$$=$3;}
            ;
else_line   : "^#" ELSE NEWLINE
            ;
endif_line  : "^#" ENDIF NEWLINE
            ;
control_line: "^#" INCLUDE pp_tokens NEWLINE
{
    $$ = CreateInclude(static_cast<ASTPPTokens*>($3));
}
            | "^#" DEFINE ID NEWLINE
{
    $$ = CreateDefine(static_cast<ASTPPToken*>($3),
                               NULL,
                               NULL);
}
            | "^#" DEFINE ID replacement_list NEWLINE
{
    $$ = CreateDefine(static_cast<ASTPPToken*>($3),
                               NULL,
                               static_cast<ASTPPTokens*>($4));
}
            | "^#" DEFINE ID_FUNC  id_list ')' replacement_list NEWLINE
{
    $$ = CreateDefine(static_cast<ASTPPToken*>($3),
                               static_cast<ASTPPTokens*>($4),
                               static_cast<ASTPPTokens*>($6));
}
            | "^#" DEFINE ID_FUNC ')' replacement_list NEWLINE
{
    $$ = CreateDefine(static_cast<ASTPPToken*>($3),
                               static_cast<ASTPPTokens*>(CreatePPTokens()),
                               static_cast<ASTPPTokens*>($5));
}
            | "^#" DEFINE ID_FUNC "..." ')' replacement_list NEWLINE
{
    $$ = CreateDefineVarArgs(static_cast<ASTPPToken*>($3),
                                      static_cast<ASTPPTokens*>(CreatePPTokens()),
                                      static_cast<ASTPPTokens*>($6));
}
            | "^#" DEFINE ID_FUNC id_list ',' "..." ')' replacement_list NEWLINE
{
    $$ = CreateDefineVarArgs(static_cast<ASTPPToken*>($3),
                                      static_cast<ASTPPTokens*>($4),
                                      static_cast<ASTPPTokens*>($8));
}
            | "^#" UNDEF ID NEWLINE  {
                $$=CreateUndef(static_cast<ASTPPToken*>($3));
                                    }
            | "^#" LINE pp_tokens NEWLINE    {
                $$=CreateLine(static_cast<ASTPPTokens*>($3));
                                            }
            | "^#" ERROR NEWLINE     {$$=CreatePPTokens();}
            | "^#" ERROR pp_tokens NEWLINE   {
                $$=CreateError(static_cast<ASTPPTokens*>($3));
                                            }
            | "^#" PRAGMA NEWLINE    {$$=CreatePragma();}
            | "^#" PRAGMA pp_tokens NEWLINE
{
    $$=CreatePragma(static_cast<ASTPPTokens *>($3));
}
            | "^#" NEWLINE           {$$=CreateNonDirective();}
            ;
text_line   : NEWLINE               {$$=CreateTextLine();}
            | pp_tokens NEWLINE
{
    $$ = CreateTextLine(static_cast<ASTPPTokens *>($1));
}
            ;
non_directive   : pp_tokens NEWLINE
{
    $$ = CreateNonDirective(static_cast<ASTPPTokens *>($1));
}
                ;
id_list     : ID                {$$=CreatePPTokens(static_cast<ASTPPToken *>($1));}
            | id_list ',' ID    {static_cast<ASTPPTokens*>($1)->append($3); $$=$1;}
            ;
replacement_list: pp_tokens     {$$=$1;}
                ;
constant_expr   : pp_tokens
{
    $$ = CreateConstantExpr(static_cast<ASTPPTokens*>($1));
}
                ;
pp_tokens   : pp_token          {$$=CreatePPTokens(static_cast<ASTPPToken *>($1));}
            | pp_tokens pp_token{static_cast<ASTPPTokens*>($1)->append($2); $$=$1;}
            ;
pp_token    : ID                {$$=$1;}
            | PP_NUMBER         {$$=$1;}
            | CHAR_CONSTANT     {$$=$1;}
            | STRING_LITERAL    {$$=$1;}
            | DEFINED           {$$=$1;}
            | '#'               {$$=$1;}
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

void pperror(ASTNode **proot, const char *str)
{
    (void)proot;
    fprintf(stderr, "%d:%s\n", pplineno, str);
}
