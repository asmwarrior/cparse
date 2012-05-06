%{
#include "pp_lex.h"
#include "ast.h"
#include "astexpr.h"
#include "lexer.h"
#include "context.h"
#include <ctype.h>
#include <stdio.h>

//#ifndef YYSTYPE_IS_DECLARED
//#define YYSTYPE_IS_DECLARED
#define YYSTYPE ASTNode *
//#endif

extern int combinelex(Context *ctx);
void combineerror(Context *ctx, const char *);
%}
%lex-param      {Context *ctx}
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
%token LINESTART_HASH   "^#"
%token HASH_HASH        "##"
%token COLON_COLON      "::"

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
%token UNKNOWN

/* fake tokens introduced by parser */
%token GROUP
%token NON_DIRECTIVE
%token IF_GROUP
%token ELIF_GROUP
%token ELIF_ELEM
%token TOKENS
%token TEXT_LINE
%token NODE_LIST
%token PLACE_MARKER

/* language dialects */
%token CPP_FILE
%token CPP_EXPRESSION

/* fake tokens used by #if's expression */
%token NUM

%%
top     :CPP_FILE pp_file   {$$=$2; ctx->root = $$;}
        |CPP_EXPRESSION pp_expr     {$$=$2; ctx->root = $$;}
        ;
pp_file :group  {$$ = $1;}
        |       {$$ = NULL;}
        ;
group   :group_part {$$ = CreateGroup($1);}
        |group group_part {static_cast<ASTGroup*>($1)->appendPart($2); $$ = $1;}
        ;
group_part  :if_group {$$ = $1;}
            |control_line {$$ = $1;}
            |text_line {$$ = $1;}
            |"^#" non_directive {$$ = CreateNonDirective(static_cast<ASTPPTokens*>($2));}
            ;
if_group    : ifs_line group endif_line 
{
    $$ = CreateIfGroup(static_cast<ASTPPTokens*>($1),
            static_cast<ASTGroup*>($2),
            NULL);
}
            | ifs_line endif_line
{
    $$ = CreateIfGroup(static_cast<ASTPPTokens*>($1),
            NULL,
            NULL);
}
            | ifs_line elif_group endif_line
{
    $$ = CreateIfGroup(static_cast<ASTPPTokens*>($1),
            static_cast<ASTElifGroup*>($2),
            NULL,
            NULL);
}
            | ifs_line elif_group group endif_line
{
    $$ = CreateIfGroup(static_cast<ASTPPTokens*>($1),
            static_cast<ASTElifGroup*>($2),
            static_cast<ASTGroup*>($3),
            NULL);
}
            | ifs_line else_line endif_line
{
    $$ = CreateIfGroup(static_cast<ASTPPTokens*>($1),
            NULL,
            NULL);
}
            | ifs_line group else_line endif_line
{
    $$ = CreateIfGroup(static_cast<ASTPPTokens*>($1),
            static_cast<ASTGroup*>($2),
            NULL);
}
            | ifs_line else_line group endif_line
{
    $$ = CreateIfGroup(static_cast<ASTPPTokens*>($1),
            NULL,
            static_cast<ASTGroup*>($3));
}
            | ifs_line group else_line group endif_line
{
    $$ = CreateIfGroup(static_cast<ASTPPTokens*>($1),
            static_cast<ASTGroup*>($2),
            static_cast<ASTGroup*>($4));
}
            | ifs_line elif_group else_line endif_line
{
    $$ = CreateIfGroup(static_cast<ASTPPTokens*>($1),
            static_cast<ASTElifGroup*>($2),
            NULL,
            NULL);
}
            | ifs_line elif_group group else_line endif_line
{
    $$ = CreateIfGroup(static_cast<ASTPPTokens*>($1),
            static_cast<ASTElifGroup*>($2),
            static_cast<ASTGroup*>($3),
            NULL);
}
            | ifs_line elif_group else_line group endif_line
{
    $$ = CreateIfGroup(static_cast<ASTPPTokens*>($1),
            static_cast<ASTElifGroup*>($2),
            NULL,
            static_cast<ASTGroup*>($4));
}
            | ifs_line elif_group group else_line group endif_line
{
    $$ = CreateIfGroup(static_cast<ASTPPTokens*>($1),
            static_cast<ASTElifGroup*>($2),
            static_cast<ASTGroup*>($3),
            static_cast<ASTGroup*>($5));
}
            ;
elif_group  : group elif_line   {
                  $$ = CreateElifGroup(static_cast<ASTGroup*>($1),
                                       static_cast<ASTPPTokens*>($2));
                                }
            | elif_line         {
                  $$ = CreateElifGroup(NULL,
                                       static_cast<ASTPPTokens*>($1));
                                }
             
            | elif_group group elif_line
{
    $$ = CreateElifGroup(static_cast<ASTElifGroup *>($1),
        static_cast<ASTGroup *>($2),
        static_cast<ASTPPTokens *>($3));
}
            | elif_group elif_line
{
    $$ = CreateElifGroup(static_cast<ASTElifGroup*>($1),
            NULL,
            static_cast<ASTPPTokens*>($2));
}
            ;
ifs_line    : "^#" IF pp_constant_expr NEWLINE
{
    $$ = CreateIfExpr(static_cast<ASTPPTokens*>($3));
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
elif_line   : "^#" ELIF pp_constant_expr NEWLINE    {$$=$3;}
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
            | id_list ','
{
    static_cast<ASTPPTokens*>($1)->append(CreatePlaceMarker());
    $$=$1;
}
            ;
replacement_list: pp_tokens     {$$=$1;}
                ;
pp_constant_expr: pp_tokens
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
            | "::"              {$$=$1;}
            ;

pp_expr     :  pp_cond_expr {$$ = $1;}
            ;
pp_cond_expr: pp_rel_or_expr {$$ = $1;}
            | pp_rel_or_expr '?' pp_expr ':' pp_cond_expr
{
    $$ = CreateTernaryExpr(ASTTernaryExpr::Conditional, static_cast<ASTExpr*>($1),
    static_cast<ASTExpr*>($3), static_cast<ASTExpr*>($5));
}
            ;
pp_rel_or_expr  : pp_rel_and_expr {$$ = $1;}
                | pp_rel_or_expr "||" pp_rel_and_expr
{
    $$ = CreateBinaryExpr(ASTBinaryExpr::LogicalOr, static_cast<ASTExpr*>($1),
        static_cast<ASTExpr*>($3));
}
                ;
pp_rel_and_expr : pp_or_expr {$$ = $1;}
                | pp_rel_and_expr "&&" pp_or_expr
{
    $$ = CreateBinaryExpr(ASTBinaryExpr::LogicalAnd, static_cast<ASTExpr*>($1),
        static_cast<ASTExpr*>($3));
}
                ;
pp_or_expr      : pp_xor_expr {$$ = $1;}
                | pp_or_expr '|' pp_xor_expr
{
    $$ = CreateBinaryExpr(ASTBinaryExpr::BitwiseOr, static_cast<ASTExpr*>($1),
        static_cast<ASTExpr*>($3));
}
                ;
pp_xor_expr     : pp_and_expr {$$ = $1;}
                | pp_xor_expr '^' pp_and_expr
{
    $$ = CreateBinaryExpr(ASTBinaryExpr::BitwiseXor, static_cast<ASTExpr*>($1),
        static_cast<ASTExpr*>($3));
}
                ;
pp_and_expr     : pp_eq_expr {$$ = $1;}
                | pp_and_expr '&' pp_eq_expr
{
    $$ = CreateBinaryExpr(ASTBinaryExpr::BitwiseAnd, static_cast<ASTExpr*>($1),
        static_cast<ASTExpr*>($3));
}
                ;
pp_eq_expr      : pp_rel_expr {$$ = $1;}
                | pp_eq_expr "==" pp_rel_expr
{
    $$ = CreateBinaryExpr(ASTBinaryExpr::RelaEqual, static_cast<ASTExpr*>($1),
        static_cast<ASTExpr*>($3));
}
                | pp_eq_expr "!=" pp_rel_expr
{
    $$ = CreateBinaryExpr(ASTBinaryExpr::RelaInequal, static_cast<ASTExpr*>($1),
        static_cast<ASTExpr*>($3));
}
                ;
pp_rel_expr     : pp_shift_expr {$$ = $1;}
                | pp_rel_expr '<' pp_shift_expr
{
    $$ = CreateBinaryExpr(ASTBinaryExpr::RelaLess, static_cast<ASTExpr*>($1),
        static_cast<ASTExpr*>($3));
}
                | pp_rel_expr '>' pp_shift_expr
{
    $$ = CreateBinaryExpr(ASTBinaryExpr::RelaGreater, static_cast<ASTExpr*>($1),
        static_cast<ASTExpr*>($3));
}
                | pp_rel_expr "<=" pp_shift_expr
{
    $$ = CreateBinaryExpr(ASTBinaryExpr::RelaLessEqual, static_cast<ASTExpr*>($1),
        static_cast<ASTExpr*>($3));
}
                | pp_rel_expr ">=" pp_shift_expr
{
    $$ = CreateBinaryExpr(ASTBinaryExpr::RelaGreaterEqual, static_cast<ASTExpr*>($1),
        static_cast<ASTExpr*>($3));
}
                ;
pp_shift_expr   : pp_add_expr {$$ = $1;}
                | pp_shift_expr "<<" pp_add_expr
{
    $$ = CreateBinaryExpr(ASTBinaryExpr::BitwiseLShift, static_cast<ASTExpr*>($1),
        static_cast<ASTExpr*>($3));
}
                | pp_shift_expr ">>" pp_add_expr
{
    $$ = CreateBinaryExpr(ASTBinaryExpr::BitwiseRShift, static_cast<ASTExpr*>($1),
        static_cast<ASTExpr*>($3));
}
                ;
pp_add_expr     : pp_mult_expr {$$ = $1;}
                | pp_add_expr '+' pp_mult_expr
{
    $$ = CreateBinaryExpr(ASTBinaryExpr::AriPlus, static_cast<ASTExpr*>($1),
        static_cast<ASTExpr*>($3));
}
                | pp_add_expr '-' pp_mult_expr
{
    $$ = CreateBinaryExpr(ASTBinaryExpr::AriMinus, static_cast<ASTExpr*>($1),
        static_cast<ASTExpr*>($3));
}
                ;
pp_mult_expr    : pp_unary_expr {$$ = $1;}
                | pp_mult_expr '*' pp_unary_expr
{
    $$ = CreateBinaryExpr(ASTBinaryExpr::AriMult, static_cast<ASTExpr*>($1),
        static_cast<ASTExpr*>($3));
}
                | pp_mult_expr '/' pp_unary_expr
{
    $$ = CreateBinaryExpr(ASTBinaryExpr::AriDiv, static_cast<ASTExpr*>($1),
        static_cast<ASTExpr*>($3));
}
                | pp_mult_expr '%' pp_unary_expr
{
    $$ = CreateBinaryExpr(ASTBinaryExpr::AriMod, static_cast<ASTExpr*>($1),
        static_cast<ASTExpr*>($3));
}
                ;
pp_unary_expr   : pp_primary_expr {$$ = $1;}
                | '-' pp_unary_expr
{
    $$ = CreateUnaryExpr(ASTUnaryExpr::AriReverse, static_cast<ASTExpr*>($2));
}
                | '+' pp_unary_expr {$$ = $2;}
                | '~' pp_unary_expr
{
    $$ = CreateUnaryExpr(ASTUnaryExpr::BitwiseReverse, static_cast<ASTExpr*>($2));
}
                | '!' pp_unary_expr
{
    $$ = CreateUnaryExpr(ASTUnaryExpr::LogicalNot, static_cast<ASTExpr*>($2));
}
                ;
pp_primary_expr : NUM {$$ = $1;}
                | '(' pp_expr ')' {$$ = $2;}
                ;
%%

int combinelex(Context *ctx)
{
    if (!ctx->lexer)
        return 0;
    if (ctx->parseStart) {
        ctx->parseStart = false;
        switch (ctx->langDialect) {
        case Context::None:
            break;
        case Context::PP:
            return CPP_FILE;
        case Context::PPExpression:
            return CPP_EXPRESSION;
        }
    }
    Token tok = ctx->lexer->lex();
    combinelval = (YYSTYPE)tok.value;
    return tok.type;
}

void combineerror(Context *ctx, const char *str)
{
    (void)ctx;
    fprintf(stderr, "%d:%s\n", pplineno, str);
}
