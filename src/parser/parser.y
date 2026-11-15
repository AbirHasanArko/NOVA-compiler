%code requires {
    #include "lexer/token.hpp"
    #include "ast/ast_node.hpp"
    #include <memory>
    #include <vector>

    namespace nova {
    class Parser;

    namespace ast_detail {
        struct RoutineList {
            std::vector<std::unique_ptr<RoutineDeclNode>> items;
        };
        struct ParamList {
            std::vector<ParamDecl> items;
        };
        struct RequireList {
            std::vector<std::unique_ptr<RequireNode>> items;
        };
        struct StmtList {
            std::vector<StmtPtr> items;
        };
        struct ExprList {
            std::vector<ExprPtr> items;
        };
        struct BranchList {
            std::vector<DetectNode::Branch> items;
        };
        struct CaseList {
            std::vector<RouteNode::Case> items;
        };
    } // namespace ast_detail
    } // namespace nova
}

%define api.pure full
%define api.prefix {nova_parser_}
%locations
%define parse.error detailed
%parse-param { nova::Parser* parser }
%lex-param   { nova::Parser* parser }

%code {
    #include "parser/parser.hpp"
    #include <stdexcept>
    #include <string>

    int nova_parser_lex(NOVA_PARSER_STYPE* yylval_param, NOVA_PARSER_LTYPE* yylloc_param, nova::Parser* parser);
    void nova_parser_error(NOVA_PARSER_LTYPE* yylloc_param, nova::Parser* parser, const char* msg);

    static nova::SourceLocation make_loc(const nova::Parser* parser, const NOVA_PARSER_LTYPE& loc) {
        return nova::SourceLocation{parser ? parser->filename() : "", loc.first_line, loc.first_column};
    }
}

%union {
    const nova::Token* token;
    nova::ProgramNode* program_node;
    nova::RoutineDeclNode* routine_node;
    nova::RequireNode* require_node;
    nova::StmtNode* stmt_node;
    nova::ExprNode* expr_node;
    nova::LiteralNode* literal_node;
    nova::ParamDecl* param_val;
    nova::TypeSpec* type_spec;
    nova::BaseType base_type;
    nova::ast_detail::RoutineList* routine_list;
    nova::ast_detail::ParamList* param_list;
    nova::ast_detail::RequireList* require_list;
    nova::ast_detail::StmtList* stmt_list;
    nova::ast_detail::ExprList* expr_list;
    nova::ast_detail::BranchList* branch_list;
    nova::ast_detail::CaseList* case_list;
    nova::RouteNode::Case* route_case;
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
%token <token> IDENT       "identifier"
%token <token> INT_LIT     "integer literal"
%token <token> FLOAT_LIT   "float literal"
%token <token> CHAR_LIT    "character literal"
%token <token> STR_LIT     "string literal"

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

/* Semantic types for nonterminals */
%type <program_node> program
%type <routine_list> routine_decl_list
%type <stmt_list>    ignite_block stmt_list opt_fallback opt_default
%type <routine_node> routine_decl
%type <param_list>   opt_param_list param_list
%type <param_val>    param
%type <type_spec>    type opt_return_type
%type <base_type>    base_type
%type <require_list> opt_require_list
%type <require_node> require_stmt
%type <stmt_node>    stmt declaration_stmt assignment_stmt expr_stmt
%type <stmt_node>    transmit_stmt receive_stmt verify_stmt ret_stmt halt_stmt skip_stmt
%type <stmt_node>    detect_stmt cruise_stmt orbit_stmt route_stmt
%type <branch_list>  redetect_list
%type <case_list>    case_list
%type <route_case>   case_clause
%type <expr_node>    expr opt_step postfix_expr routine_call primary_expr array_literal
%type <expr_list>    opt_expr_list expr_list opt_arg_list
%type <literal_node> literal

/* Destructors to prevent memory leaks during syntax error recovery */
%destructor { delete $$; } <program_node>
%destructor { delete $$; } <routine_node>
%destructor { delete $$; } <require_node>
%destructor { delete $$; } <stmt_node>
%destructor { delete $$; } <expr_node>
%destructor { delete $$; } <literal_node>
%destructor { delete $$; } <param_val>
%destructor { delete $$; } <type_spec>
%destructor { delete $$; } <routine_list>
%destructor { delete $$; } <param_list>
%destructor { delete $$; } <require_list>
%destructor { delete $$; } <stmt_list>
%destructor { delete $$; } <expr_list>
%destructor { delete $$; } <branch_list>
%destructor { delete $$; } <case_list>
%destructor { delete $$; } <route_case>

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
    : routine_decl_list ignite_block {
        auto root = std::make_unique<nova::ProgramNode>(
            std::move($1->items),
            std::move($2->items),
            make_loc(parser, @2),
            make_loc(parser, @1)
        );
        delete $1;
        delete $2;
        if (parser) {
            parser->set_root(std::move(root));
        }
        $$ = nullptr;
    }
    | ignite_block {
        std::vector<std::unique_ptr<nova::RoutineDeclNode>> empty_routines;
        auto root = std::make_unique<nova::ProgramNode>(
            std::move(empty_routines),
            std::move($1->items),
            make_loc(parser, @1),
            make_loc(parser, @1)
        );
        delete $1;
        if (parser) {
            parser->set_root(std::move(root));
        }
        $$ = nullptr;
    }
    ;

routine_decl_list
    : routine_decl_list routine_decl {
        $1->items.emplace_back($2);
        $$ = $1;
    }
    | routine_decl {
        auto* list = new nova::ast_detail::RoutineList();
        list->items.emplace_back($1);
        $$ = list;
    }
    ;

ignite_block
    : KW_IGNITE KW_DO stmt_list KW_END {
        $$ = $3;
    }
    ;

routine_decl
    : KW_ROUTINE IDENT LPAREN opt_param_list RPAREN opt_return_type KW_DO opt_require_list stmt_list KW_END {
        $$ = new nova::RoutineDeclNode(
            $2->lexeme,
            std::move($4->items),
            *$6,
            std::move($8->items),
            std::move($9->items),
            make_loc(parser, @1)
        );
        delete $4;
        delete $6;
        delete $8;
        delete $9;
    }
    ;

opt_param_list
    : param_list { $$ = $1; }
    | %empty { $$ = new nova::ast_detail::ParamList(); }
    ;

param_list
    : param_list COMMA param {
        $1->items.push_back(std::move(*$3));
        delete $3;
        $$ = $1;
    }
    | param {
        auto* list = new nova::ast_detail::ParamList();
        list->items.push_back(std::move(*$1));
        delete $1;
        $$ = list;
    }
    ;

param
    : IDENT COLON type {
        $$ = new nova::ParamDecl{$1->lexeme, *$3, make_loc(parser, @1)};
        delete $3;
    }
    ;

opt_return_type
    : ARROW type { $$ = $2; }
    | %empty { $$ = new nova::TypeSpec{nova::BaseType::VOID, false, 0}; }
    ;

opt_require_list
    : opt_require_list require_stmt {
        $1->items.emplace_back($2);
        $$ = $1;
    }
    | %empty { $$ = new nova::ast_detail::RequireList(); }
    ;

require_stmt
    : KW_REQUIRE expr SEMICOLON {
        $$ = new nova::RequireNode(nova::ExprPtr($2), make_loc(parser, @1));
    }
    ;

type
    : base_type {
        $$ = new nova::TypeSpec{$1, false, 0};
    }
    | base_type LBRACKET INT_LIT RBRACKET {
        int size = 0;
        try {
            size = std::stoi($3->lexeme);
        } catch (const std::exception&) {
            if (parser) {
                parser->record_error(@3.first_line, @3.first_column, "array size literal out of range");
            }
        }
        $$ = new nova::TypeSpec{$1, true, size};
    }
    ;

base_type
    : KW_I32  { $$ = nova::BaseType::I32; }
    | KW_F32  { $$ = nova::BaseType::F32; }
    | KW_BOOL { $$ = nova::BaseType::BOOL; }
    | KW_CHAR { $$ = nova::BaseType::CHAR; }
    | KW_STR  { $$ = nova::BaseType::STR; }
    | KW_VOID { $$ = nova::BaseType::VOID; }
    ;

stmt_list
    : stmt_list stmt {
        if ($2) {
            $1->items.emplace_back($2);
        }
        $$ = $1;
    }
    | %empty { $$ = new nova::ast_detail::StmtList(); }
    ;

stmt
    : declaration_stmt SEMICOLON { $$ = $1; }
    | assignment_stmt SEMICOLON  { $$ = $1; }
    | expr_stmt SEMICOLON        { $$ = $1; }
    | detect_stmt                { $$ = $1; }
    | cruise_stmt                { $$ = $1; }
    | orbit_stmt                 { $$ = $1; }
    | route_stmt                 { $$ = $1; }
    | transmit_stmt SEMICOLON    { $$ = $1; }
    | receive_stmt SEMICOLON     { $$ = $1; }
    | verify_stmt SEMICOLON      { $$ = $1; }
    | ret_stmt SEMICOLON         { $$ = $1; }
    | halt_stmt SEMICOLON        { $$ = $1; }
    | skip_stmt SEMICOLON        { $$ = $1; }
    | error SEMICOLON {
        yyerrok;
        $$ = nullptr;
    }
    ;

declaration_stmt
    : KW_CONST IDENT COLON type OP_ASSIGN expr {
        $$ = new nova::ConstDeclNode($2->lexeme, *$4, nova::ExprPtr($6), make_loc(parser, @1));
        delete $4;
    }
    | KW_FLUX IDENT COLON type {
        $$ = new nova::FluxDeclNode($2->lexeme, *$4, nullptr, make_loc(parser, @1));
        delete $4;
    }
    | KW_FLUX IDENT COLON type OP_ASSIGN expr {
        $$ = new nova::FluxDeclNode($2->lexeme, *$4, nova::ExprPtr($6), make_loc(parser, @1));
        delete $4;
    }
    ;

assignment_stmt
    : IDENT OP_ASSIGN expr {
        auto target = std::make_unique<nova::IdentifierNode>($1->lexeme, make_loc(parser, @1));
        $$ = new nova::AssignNode(std::move(target), nova::ExprPtr($3), make_loc(parser, @1));
    }
    | postfix_expr LBRACKET expr RBRACKET OP_ASSIGN expr {
        auto target = std::make_unique<nova::IndexAccessNode>(
            nova::ExprPtr($1), nova::ExprPtr($3), make_loc(parser, @1));
        $$ = new nova::AssignNode(std::move(target), nova::ExprPtr($6), make_loc(parser, @1));
    }
    ;

expr_stmt
    : expr {
        $$ = new nova::ExprStmtNode(nova::ExprPtr($1), make_loc(parser, @1));
    }
    ;

transmit_stmt
    : KW_TRANSMIT LPAREN expr_list RPAREN {
        $$ = new nova::TransmitNode(std::move($3->items), make_loc(parser, @1));
        delete $3;
    }
    ;

receive_stmt
    : KW_RECEIVE LPAREN IDENT RPAREN {
        $$ = new nova::ReceiveNode($3->lexeme, make_loc(parser, @1));
    }
    ;

verify_stmt
    : KW_VERIFY expr {
        $$ = new nova::VerifyNode(nova::ExprPtr($2), make_loc(parser, @1));
    }
    ;

ret_stmt
    : KW_RET expr {
        $$ = new nova::RetNode(nova::ExprPtr($2), make_loc(parser, @1));
    }
    | KW_RET {
        $$ = new nova::RetNode(nullptr, make_loc(parser, @1));
    }
    ;

halt_stmt
    : KW_HALT {
        $$ = new nova::HaltNode(make_loc(parser, @1));
    }
    ;

skip_stmt
    : KW_SKIP {
        $$ = new nova::SkipNode(make_loc(parser, @1));
    }
    ;

detect_stmt
    : KW_DETECT expr KW_DO stmt_list redetect_list opt_fallback KW_END {
        std::vector<nova::DetectNode::Branch> branches;
        branches.push_back(nova::DetectNode::Branch{
            nova::ExprPtr($2), std::move($4->items), make_loc(parser, @1)
        });
        for (auto& br : $5->items) {
            branches.push_back(std::move(br));
        }
        bool has_fallback = ($6 != nullptr);
        std::vector<nova::StmtPtr> fallback_body;
        if (has_fallback) {
            fallback_body = std::move($6->items);
        }
        $$ = new nova::DetectNode(
            std::move(branches), std::move(fallback_body), has_fallback, make_loc(parser, @1));
        delete $4;
        delete $5;
        delete $6;
    }
    ;

redetect_list
    : redetect_list KW_REDETECT expr KW_DO stmt_list {
        $1->items.push_back(nova::DetectNode::Branch{
            nova::ExprPtr($3), std::move($5->items), make_loc(parser, @2)
        });
        delete $5;
        $$ = $1;
    }
    | %empty {
        $$ = new nova::ast_detail::BranchList();
    }
    ;

opt_fallback
    : KW_FALLBACK KW_DO stmt_list {
        $$ = $3;
    }
    | %empty {
        $$ = nullptr;
    }
    ;

cruise_stmt
    : KW_CRUISE expr KW_DO stmt_list KW_END {
        $$ = new nova::CruiseNode(nova::ExprPtr($2), std::move($4->items), make_loc(parser, @1));
        delete $4;
    }
    ;

orbit_stmt
    : KW_ORBIT IDENT KW_IN expr DOTDOT expr opt_step KW_DO stmt_list KW_END {
        $$ = new nova::OrbitNode(
            $2->lexeme,
            nova::ExprPtr($4),
            nova::ExprPtr($6),
            nova::ExprPtr($7),
            std::move($9->items),
            make_loc(parser, @1)
        );
        delete $9;
    }
    ;

opt_step
    : KW_STEP expr { $$ = $2; }
    | %empty       { $$ = nullptr; }
    ;

route_stmt
    : KW_ROUTE expr KW_DO case_list opt_default KW_END {
        bool has_default = ($5 != nullptr);
        std::vector<nova::StmtPtr> default_body;
        if (has_default) {
            default_body = std::move($5->items);
        }
        $$ = new nova::RouteNode(
            nova::ExprPtr($2),
            std::move($4->items),
            std::move(default_body),
            has_default,
            make_loc(parser, @1)
        );
        delete $4;
        delete $5;
    }
    ;

case_list
    : case_list case_clause {
        $1->items.push_back(std::move(*$2));
        delete $2;
        $$ = $1;
    }
    | case_clause {
        auto* list = new nova::ast_detail::CaseList();
        list->items.push_back(std::move(*$1));
        delete $1;
        $$ = list;
    }
    ;

case_clause
    : KW_CASE literal COLON stmt_list {
        $$ = new nova::RouteNode::Case{
            std::unique_ptr<nova::LiteralNode>($2),
            std::move($4->items),
            make_loc(parser, @1)
        };
        delete $4;
    }
    ;

opt_default
    : KW_DEFAULT COLON stmt_list { $$ = $3; }
    | %empty                   { $$ = nullptr; }
    ;

expr
    : postfix_expr { $$ = $1; }
    | expr OP_POW expr {
        $$ = new nova::BinaryExprNode(nova::BinaryExprNode::Op::POW, nova::ExprPtr($1), nova::ExprPtr($3), make_loc(parser, @2));
    }
    | OP_SUB expr %prec UMINUS {
        $$ = new nova::UnaryExprNode(nova::UnaryExprNode::Op::NEG, nova::ExprPtr($2), make_loc(parser, @1));
    }
    | KW_NOT expr {
        $$ = new nova::UnaryExprNode(nova::UnaryExprNode::Op::NOT, nova::ExprPtr($2), make_loc(parser, @1));
    }
    | expr OP_MUL expr {
        $$ = new nova::BinaryExprNode(nova::BinaryExprNode::Op::MUL, nova::ExprPtr($1), nova::ExprPtr($3), make_loc(parser, @2));
    }
    | expr OP_DIV expr {
        $$ = new nova::BinaryExprNode(nova::BinaryExprNode::Op::DIV, nova::ExprPtr($1), nova::ExprPtr($3), make_loc(parser, @2));
    }
    | expr OP_MOD expr {
        $$ = new nova::BinaryExprNode(nova::BinaryExprNode::Op::MOD, nova::ExprPtr($1), nova::ExprPtr($3), make_loc(parser, @2));
    }
    | expr OP_ADD expr {
        $$ = new nova::BinaryExprNode(nova::BinaryExprNode::Op::ADD, nova::ExprPtr($1), nova::ExprPtr($3), make_loc(parser, @2));
    }
    | expr OP_SUB expr {
        $$ = new nova::BinaryExprNode(nova::BinaryExprNode::Op::SUB, nova::ExprPtr($1), nova::ExprPtr($3), make_loc(parser, @2));
    }
    | expr OP_LT expr {
        $$ = new nova::BinaryExprNode(nova::BinaryExprNode::Op::LT, nova::ExprPtr($1), nova::ExprPtr($3), make_loc(parser, @2));
    }
    | expr OP_LE expr {
        $$ = new nova::BinaryExprNode(nova::BinaryExprNode::Op::LE, nova::ExprPtr($1), nova::ExprPtr($3), make_loc(parser, @2));
    }
    | expr OP_GT expr {
        $$ = new nova::BinaryExprNode(nova::BinaryExprNode::Op::GT, nova::ExprPtr($1), nova::ExprPtr($3), make_loc(parser, @2));
    }
    | expr OP_GE expr {
        $$ = new nova::BinaryExprNode(nova::BinaryExprNode::Op::GE, nova::ExprPtr($1), nova::ExprPtr($3), make_loc(parser, @2));
    }
    | expr OP_EQ expr {
        $$ = new nova::BinaryExprNode(nova::BinaryExprNode::Op::EQ, nova::ExprPtr($1), nova::ExprPtr($3), make_loc(parser, @2));
    }
    | expr OP_NEQ expr {
        $$ = new nova::BinaryExprNode(nova::BinaryExprNode::Op::NEQ, nova::ExprPtr($1), nova::ExprPtr($3), make_loc(parser, @2));
    }
    | expr KW_AND expr {
        $$ = new nova::BinaryExprNode(nova::BinaryExprNode::Op::AND, nova::ExprPtr($1), nova::ExprPtr($3), make_loc(parser, @2));
    }
    | expr KW_OR expr {
        $$ = new nova::BinaryExprNode(nova::BinaryExprNode::Op::OR, nova::ExprPtr($1), nova::ExprPtr($3), make_loc(parser, @2));
    }
    ;

postfix_expr
    : primary_expr { $$ = $1; }
    | postfix_expr LBRACKET expr RBRACKET {
        $$ = new nova::IndexAccessNode(nova::ExprPtr($1), nova::ExprPtr($3), make_loc(parser, @1));
    }
    | routine_call { $$ = $1; }
    ;

routine_call
    : IDENT LPAREN opt_arg_list RPAREN {
        $$ = new nova::RoutineCallNode($1->lexeme, std::move($3->items), make_loc(parser, @1));
        delete $3;
    }
    ;

primary_expr
    : IDENT {
        $$ = new nova::IdentifierNode($1->lexeme, make_loc(parser, @1));
    }
    | literal {
        $$ = $1;
    }
    | array_literal {
        $$ = $1;
    }
    | LPAREN expr RPAREN {
        $$ = $2;
    }
    ;

array_literal
    : LBRACKET opt_expr_list RBRACKET {
        $$ = new nova::ArrayLiteralNode(std::move($2->items), make_loc(parser, @1));
        delete $2;
    }
    ;

opt_expr_list
    : expr_list { $$ = $1; }
    | %empty    { $$ = new nova::ast_detail::ExprList(); }
    ;

expr_list
    : expr_list COMMA expr {
        $1->items.emplace_back($3);
        $$ = $1;
    }
    | expr {
        auto* list = new nova::ast_detail::ExprList();
        list->items.emplace_back($1);
        $$ = list;
    }
    ;

opt_arg_list
    : expr_list { $$ = $1; }
    | %empty    { $$ = new nova::ast_detail::ExprList(); }
    ;

literal
    : INT_LIT {
        $$ = new nova::LiteralNode(nova::LiteralNode::Kind::INT, $1->lexeme, make_loc(parser, @1));
    }
    | FLOAT_LIT {
        $$ = new nova::LiteralNode(nova::LiteralNode::Kind::FLOAT, $1->lexeme, make_loc(parser, @1));
    }
    | CHAR_LIT {
        $$ = new nova::LiteralNode(nova::LiteralNode::Kind::CHAR, $1->lexeme, make_loc(parser, @1));
    }
    | STR_LIT {
        $$ = new nova::LiteralNode(nova::LiteralNode::Kind::STR, $1->lexeme, make_loc(parser, @1));
    }
    | KW_TRUE {
        $$ = new nova::LiteralNode(nova::LiteralNode::Kind::BOOL, "true", make_loc(parser, @1));
    }
    | KW_FALSE {
        $$ = new nova::LiteralNode(nova::LiteralNode::Kind::BOOL, "false", make_loc(parser, @1));
    }
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
