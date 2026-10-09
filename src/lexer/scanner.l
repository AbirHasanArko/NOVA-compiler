%{
#include "lexer/token.hpp"
#include "lexer/lexer.hpp"
#include <string>
#include <iostream>

using namespace nova;

namespace nova {
    extern Lexer* g_current_lexer;
    extern std::string g_current_filename;
}

static int s_line = 1;
static int s_col = 1;
static int s_tok_line = 1;
static int s_tok_col = 1;
static int s_comment_start_line = 1;
static int s_comment_start_col = 1;

void nova_lexer_reset(const std::string& filename) {
    s_line = 1;
    s_col = 1;
    s_tok_line = 1;
    s_tok_col = 1;
    nova::g_current_filename = filename;
}

#define YY_DECL nova::Token yylex()

#define YY_USER_ACTION \
    s_tok_line = s_line; \
    s_tok_col = s_col; \
    s_col += yyleng;

static Token make_token(TokenKind kind) {
    SourceLocation loc;
    loc.filename = nova::g_current_filename;
    loc.line = s_tok_line;
    loc.column = s_tok_col;
    return Token(kind, std::string(yytext, yyleng), loc);
}

static Token make_error(const std::string& msg) {
    if (nova::g_current_lexer) {
        nova::g_current_lexer->record_error(s_tok_line, s_tok_col, msg);
    }
    SourceLocation loc;
    loc.filename = nova::g_current_filename;
    loc.line = s_tok_line;
    loc.column = s_tok_col;
    return Token(TokenKind::TOKEN_ERROR, std::string(yytext, yyleng), loc);
}
%}

%option noyywrap
%option nounput
%option noinput

%x COMMENT

%%

[ \t\r]+        { /* ignore whitespace within same line */ }
\n              { s_line++; s_col = 1; }

"//"[^\n]*      { /* single-line comment */ }

"/*"            {
                    s_comment_start_line = s_tok_line;
                    s_comment_start_col = s_tok_col;
                    BEGIN(COMMENT);
                }
<COMMENT>"*/"   { BEGIN(INITIAL); }
<COMMENT>\n     { s_line++; s_col = 1; }
<COMMENT>.      { /* skip multiline comment content */ }
<COMMENT><<EOF>> {
                    BEGIN(INITIAL);
                    if (nova::g_current_lexer) {
                        nova::g_current_lexer->record_error(s_comment_start_line, s_comment_start_col, "unterminated multi-line comment");
                    }
                    SourceLocation loc{nova::g_current_filename, s_comment_start_line, s_comment_start_col};
                    return Token(TokenKind::TOKEN_ERROR, "/*", loc);
                }

    /* Keywords */
"ignite"        { return make_token(TokenKind::KW_IGNITE); }
"do"            { return make_token(TokenKind::KW_DO); }
"end"           { return make_token(TokenKind::KW_END); }
"proc"          { return make_token(TokenKind::KW_PROC); }
"ret"           { return make_token(TokenKind::KW_RET); }
"const"         { return make_token(TokenKind::KW_CONST); }
"flux"          { return make_token(TokenKind::KW_FLUX); }
"i32"           { return make_token(TokenKind::KW_I32); }
"f32"           { return make_token(TokenKind::KW_F32); }
"bool"          { return make_token(TokenKind::KW_BOOL); }
"char"          { return make_token(TokenKind::KW_CHAR); }
"str"           { return make_token(TokenKind::KW_STR); }
"void"          { return make_token(TokenKind::KW_VOID); }
"if"            { return make_token(TokenKind::KW_IF); }
"elif"          { return make_token(TokenKind::KW_ELIF); }
"else"          { return make_token(TokenKind::KW_ELSE); }
"route"         { return make_token(TokenKind::KW_ROUTE); }
"case"          { return make_token(TokenKind::KW_CASE); }
"default"       { return make_token(TokenKind::KW_DEFAULT); }
"orbit"         { return make_token(TokenKind::KW_ORBIT); }
"in"            { return make_token(TokenKind::KW_IN); }
"step"          { return make_token(TokenKind::KW_STEP); }
"cruise"        { return make_token(TokenKind::KW_CRUISE); }
"halt"          { return make_token(TokenKind::KW_HALT); }
"skip"          { return make_token(TokenKind::KW_SKIP); }
"transmit"      { return make_token(TokenKind::KW_TRANSMIT); }
"receive"       { return make_token(TokenKind::KW_RECEIVE); }
"verify"        { return make_token(TokenKind::KW_VERIFY); }
"require"       { return make_token(TokenKind::KW_REQUIRE); }
"and"           { return make_token(TokenKind::KW_AND); }
"or"            { return make_token(TokenKind::KW_OR); }
"not"           { return make_token(TokenKind::KW_NOT); }
"true"          { return make_token(TokenKind::KW_TRUE); }
"false"         { return make_token(TokenKind::KW_FALSE); }

    /* Operators & Punctuators */
"**"            { return make_token(TokenKind::OP_POW); }
"+"             { return make_token(TokenKind::OP_ADD); }
"-"             { return make_token(TokenKind::OP_SUB); }
"*"             { return make_token(TokenKind::OP_MUL); }
"/"             { return make_token(TokenKind::OP_DIV); }
"%"             { return make_token(TokenKind::OP_MOD); }
"<="            { return make_token(TokenKind::OP_LE); }
">="            { return make_token(TokenKind::OP_GE); }
"=="            { return make_token(TokenKind::OP_EQ); }
"!="            { return make_token(TokenKind::OP_NEQ); }
"<"             { return make_token(TokenKind::OP_LT); }
">"             { return make_token(TokenKind::OP_GT); }
"="             { return make_token(TokenKind::OP_ASSIGN); }
".."            { return make_token(TokenKind::DOTDOT); }
"->"            { return make_token(TokenKind::ARROW); }
":"             { return make_token(TokenKind::COLON); }
";"             { return make_token(TokenKind::SEMICOLON); }
","             { return make_token(TokenKind::COMMA); }
"."             { return make_token(TokenKind::DOT); }
"("             { return make_token(TokenKind::LPAREN); }
")"             { return make_token(TokenKind::RPAREN); }
"["             { return make_token(TokenKind::LBRACKET); }
"]"             { return make_token(TokenKind::RBRACKET); }

    /* Numeric Literals */
[0-9]+\.[0-9]+  { return make_token(TokenKind::FLOAT_LIT); }
[0-9]+          { return make_token(TokenKind::INT_LIT); }

    /* String Literals */
\"([^"\\\n]|\\.)*\" { return make_token(TokenKind::STR_LIT); }
\"([^"\\\n]|\\.)*   { return make_error("unterminated string literal"); }

    /* Character Literals */
'([^'\\\n]|\\.)'    { return make_token(TokenKind::CHAR_LIT); }
'([^'\\\n]|\\.)*    { return make_error("unterminated or malformed character literal"); }

    /* Identifiers */
[a-zA-Z_][a-zA-Z0-9_]* { return make_token(TokenKind::IDENT); }

    /* Catch-all Invalid Characters */
.               { return make_error(std::string("invalid character: '") + yytext + "'"); }

<<EOF>>         {
                    SourceLocation loc{nova::g_current_filename, s_line, s_col};
                    return Token(TokenKind::TOKEN_EOF, "", loc);
                }

%%
