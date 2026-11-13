%code requires {
    namespace nova { class Parser; }
}

%define api.pure full
%define api.prefix {nova_parser_}
%locations
%define parse.error detailed
%parse-param { nova::Parser* parser }
%lex-param   { nova::Parser* parser }

%code {
    #include "parser/parser.hpp"
    #include <string>

    int nova_parser_lex(NOVA_PARSER_STYPE* yylval_param, NOVA_PARSER_LTYPE* yylloc_param, nova::Parser* parser);
    void nova_parser_error(NOVA_PARSER_LTYPE* yylloc_param, nova::Parser* parser, const char* msg);
}

/* Keywords */
%token KW_IGNITE   "ignite"
%token KW_ROUTINE  "routine"
%token KW_RET      "ret"
%token KW_CONST    "const"
%token KW_FLUX     "flux"
%token KW_DO       "do"
%token KW_END      "end"

%token KW_I32      "i32"
%token KW_F32      "f32"
%token KW_BOOL     "bool"
%token KW_CHAR     "char"
%token KW_STR      "str"
%token KW_VOID     "void"

%token KW_DETECT   "detect"
%token KW_REDETECT "redetect"
%token KW_FALLBACK "fallback"
%token KW_ROUTE    "route"
%token KW_CASE     "case"
%token KW_DEFAULT  "default"

%token KW_ORBIT    "orbit"
%token KW_IN       "in"
%token KW_STEP     "step"
%token KW_CRUISE   "cruise"
%token KW_HALT     "halt"
%token KW_SKIP     "skip"

%token KW_TRANSMIT "transmit"
%token KW_RECEIVE  "receive"
%token KW_VERIFY   "verify"
%token KW_REQUIRE  "require"

%token KW_AND      "and"
%token KW_OR       "or"
%token KW_NOT      "not"
%token KW_TRUE     "true"
%token KW_FALSE    "false"

/* Identifiers & Literals */
%token IDENT       "identifier"
%token INT_LIT     "integer literal"
%token FLOAT_LIT   "float literal"
%token CHAR_LIT    "character literal"
%token STR_LIT     "string literal"

/* Operators & Punctuators */
%token OP_ADD      "+"
%token OP_SUB      "-"
%token OP_MUL      "*"
%token OP_DIV      "/"
%token OP_MOD      "%"
%token OP_POW      "**"

%token OP_LT       "<"
%token OP_LE       "<="
%token OP_GT       ">"
%token OP_GE       ">="
%token OP_EQ       "=="
%token OP_NEQ      "!="
%token OP_ASSIGN   "="

%token DOTDOT      ".."
%token ARROW       "->"
%token COLON       ":"
%token SEMICOLON   ";"
%token COMMA       ","
%token DOT         "."
%token LPAREN      "("
%token RPAREN      ")"
%token LBRACKET    "["
%token RBRACKET    "]"

%token TOKEN_INVALID "invalid token"

/* Operator Precedence (lowest to highest) */
%left       KW_OR
%left       KW_AND
%left       OP_EQ OP_NEQ
%left       OP_LT OP_LE OP_GT OP_GE
%left       OP_ADD OP_SUB
%left       OP_MUL OP_DIV OP_MOD
%precedence UMINUS KW_NOT
%right      OP_POW

%start program

%%

program
    : routine_decl_list ignite_block
    | ignite_block
    ;

routine_decl_list
    : routine_decl_list routine_decl
    | routine_decl
    ;

ignite_block
    : KW_IGNITE KW_DO stmt_list KW_END
    ;

routine_decl
    : KW_ROUTINE IDENT LPAREN opt_param_list RPAREN opt_return_type KW_DO opt_require_list stmt_list KW_END
    ;

opt_param_list
    : param_list
    | %empty
    ;

param_list
    : param_list COMMA param
    | param
    ;

param
    : IDENT COLON type
    ;

opt_return_type
    : ARROW type
    | %empty
    ;

opt_require_list
    : opt_require_list require_stmt
    | %empty
    ;

require_stmt
    : KW_REQUIRE expr SEMICOLON
    ;

type
    : base_type
    | base_type LBRACKET INT_LIT RBRACKET
    ;

base_type
    : KW_I32
    | KW_F32
    | KW_BOOL
    | KW_CHAR
    | KW_STR
    | KW_VOID
    ;

stmt_list
    : stmt_list stmt
    | %empty
    ;

stmt
    : declaration_stmt SEMICOLON
    | assignment_stmt SEMICOLON
    | expr_stmt SEMICOLON
    | detect_stmt
    | cruise_stmt
    | orbit_stmt
    | route_stmt
    | transmit_stmt SEMICOLON
    | receive_stmt SEMICOLON
    | verify_stmt SEMICOLON
    | ret_stmt SEMICOLON
    | halt_stmt SEMICOLON
    | skip_stmt SEMICOLON
    | error SEMICOLON { yyerrok; }
    ;

declaration_stmt
    : KW_CONST IDENT COLON type OP_ASSIGN expr
    | KW_FLUX IDENT COLON type
    | KW_FLUX IDENT COLON type OP_ASSIGN expr
    ;

assignment_stmt
    : IDENT OP_ASSIGN expr
    | postfix_expr LBRACKET expr RBRACKET OP_ASSIGN expr
    ;

expr_stmt
    : expr
    ;

transmit_stmt
    : KW_TRANSMIT LPAREN expr_list RPAREN
    ;

receive_stmt
    : KW_RECEIVE LPAREN IDENT RPAREN
    ;

verify_stmt
    : KW_VERIFY expr
    ;

ret_stmt
    : KW_RET expr
    | KW_RET
    ;

halt_stmt
    : KW_HALT
    ;

skip_stmt
    : KW_SKIP
    ;

detect_stmt
    : KW_DETECT expr KW_DO stmt_list redetect_list opt_fallback KW_END
    ;

redetect_list
    : redetect_list KW_REDETECT expr KW_DO stmt_list
    | %empty
    ;

opt_fallback
    : KW_FALLBACK KW_DO stmt_list
    | %empty
    ;

cruise_stmt
    : KW_CRUISE expr KW_DO stmt_list KW_END
    ;

orbit_stmt
    : KW_ORBIT IDENT KW_IN expr DOTDOT expr opt_step KW_DO stmt_list KW_END
    ;

opt_step
    : KW_STEP expr
    | %empty
    ;

route_stmt
    : KW_ROUTE expr KW_DO case_list opt_default KW_END
    ;

case_list
    : case_list case_clause
    | case_clause
    ;

case_clause
    : KW_CASE literal COLON stmt_list
    ;

opt_default
    : KW_DEFAULT COLON stmt_list
    | %empty
    ;

expr
    : postfix_expr
    | expr OP_POW expr
    | OP_SUB expr %prec UMINUS
    | KW_NOT expr
    | expr OP_MUL expr
    | expr OP_DIV expr
    | expr OP_MOD expr
    | expr OP_ADD expr
    | expr OP_SUB expr
    | expr OP_LT expr
    | expr OP_LE expr
    | expr OP_GT expr
    | expr OP_GE expr
    | expr OP_EQ expr
    | expr OP_NEQ expr
    | expr KW_AND expr
    | expr KW_OR expr
    ;

postfix_expr
    : primary_expr
    | postfix_expr LBRACKET expr RBRACKET
    | routine_call
    ;

routine_call
    : IDENT LPAREN opt_arg_list RPAREN
    ;

primary_expr
    : IDENT
    | literal
    | array_literal
    | LPAREN expr RPAREN
    ;

array_literal
    : LBRACKET opt_expr_list RBRACKET
    ;

opt_expr_list
    : expr_list
    | %empty
    ;

expr_list
    : expr_list COMMA expr
    | expr
    ;

opt_arg_list
    : expr_list
    | %empty
    ;

literal
    : INT_LIT
    | FLOAT_LIT
    | CHAR_LIT
    | STR_LIT
    | KW_TRUE
    | KW_FALSE
    ;

%%

void nova_parser_error(NOVA_PARSER_LTYPE* yylloc_param, nova::Parser* parser, const char* msg) {
    if (!parser) return;
    int line = yylloc_param ? yylloc_param->first_line : 1;
    int col = yylloc_param ? yylloc_param->first_column : 1;
    std::string detail = msg ? msg : "syntax error";
    const std::string prefix = "syntax error, ";
    if (detail.rfind(prefix, 0) == 0) {
        detail = detail.substr(prefix.size());
    } else if (detail == "syntax error") {
        detail = "unexpected token";
    }
    parser->record_error(line, col, detail);
}
