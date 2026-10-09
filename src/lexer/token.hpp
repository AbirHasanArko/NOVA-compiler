#ifndef NOVA_LEXER_TOKEN_HPP
#define NOVA_LEXER_TOKEN_HPP

#include <string>
#include <iostream>

namespace nova {

enum class TokenKind {
    // End of input & errors
    TOKEN_EOF,
    TOKEN_ERROR,

    // Execution Capsule & Procedures
    KW_IGNITE,
    KW_DO,
    KW_END,
    KW_PROC,
    KW_RET,

    // State Bindings
    KW_CONST,
    KW_FLUX,

    // Primitive Types
    KW_I32,
    KW_F32,
    KW_BOOL,
    KW_CHAR,
    KW_STR,
    KW_VOID,

    // Control Flow
    KW_IF,
    KW_ELIF,
    KW_ELSE,
    KW_ROUTE,
    KW_CASE,
    KW_DEFAULT,

    // Cycles & Control
    KW_ORBIT,
    KW_IN,
    KW_STEP,
    KW_WHILE,
    KW_HALT,
    KW_SKIP,

    // Telemetry I/O
    KW_TRANSMIT,
    KW_RECEIVE,

    // Contracts
    KW_VERIFY,
    KW_REQUIRE,

    // Logical & Booleans
    KW_AND,
    KW_OR,
    KW_NOT,
    KW_TRUE,
    KW_FALSE,

    // Literals & Identifiers
    IDENT,
    INT_LIT,
    FLOAT_LIT,
    CHAR_LIT,
    STR_LIT,

    // Operators
    OP_ADD,      // +
    OP_SUB,      // -
    OP_MUL,      // *
    OP_DIV,      // /
    OP_MOD,      // %
    OP_POW,      // **
    OP_LT,       // <
    OP_LE,       // <=
    OP_GT,       // >
    OP_GE,       // >=
    OP_EQ,       // ==
    OP_NEQ,      // !=
    OP_ASSIGN,   // =

    // Delimiters
    DOTDOT,      // ..
    ARROW,       // ->
    COLON,       // :
    SEMICOLON,   // ;
    COMMA,       // ,
    DOT,         // .
    LPAREN,      // (
    RPAREN,      // )
    LBRACKET,    // [
    RBRACKET     // ]
};

struct SourceLocation {
    std::string filename;
    int line = 1;
    int column = 1;
};

struct Token {
    TokenKind kind;
    std::string lexeme;
    SourceLocation location;

    Token(TokenKind k, std::string lex, SourceLocation loc)
        : kind(k), lexeme(std::move(lex)), location(std::move(loc)) {}
};

inline const char* token_kind_to_string(TokenKind kind) {
    switch (kind) {
        case TokenKind::TOKEN_EOF:     return "EOF";
        case TokenKind::TOKEN_ERROR:   return "ERROR";
        case TokenKind::KW_IGNITE:     return "KW_IGNITE";
        case TokenKind::KW_DO:         return "KW_DO";
        case TokenKind::KW_END:        return "KW_END";
        case TokenKind::KW_PROC:       return "KW_PROC";
        case TokenKind::KW_RET:        return "KW_RET";
        case TokenKind::KW_CONST:      return "KW_CONST";
        case TokenKind::KW_FLUX:       return "KW_FLUX";
        case TokenKind::KW_I32:        return "KW_I32";
        case TokenKind::KW_F32:        return "KW_F32";
        case TokenKind::KW_BOOL:       return "KW_BOOL";
        case TokenKind::KW_CHAR:       return "KW_CHAR";
        case TokenKind::KW_STR:        return "KW_STR";
        case TokenKind::KW_VOID:       return "KW_VOID";
        case TokenKind::KW_IF:         return "KW_IF";
        case TokenKind::KW_ELIF:       return "KW_ELIF";
        case TokenKind::KW_ELSE:       return "KW_ELSE";
        case TokenKind::KW_ROUTE:      return "KW_ROUTE";
        case TokenKind::KW_CASE:       return "KW_CASE";
        case TokenKind::KW_DEFAULT:    return "KW_DEFAULT";
        case TokenKind::KW_ORBIT:      return "KW_ORBIT";
        case TokenKind::KW_IN:         return "KW_IN";
        case TokenKind::KW_STEP:       return "KW_STEP";
        case TokenKind::KW_WHILE:      return "KW_WHILE";
        case TokenKind::KW_HALT:       return "KW_HALT";
        case TokenKind::KW_SKIP:       return "KW_SKIP";
        case TokenKind::KW_TRANSMIT:   return "KW_TRANSMIT";
        case TokenKind::KW_RECEIVE:    return "KW_RECEIVE";
        case TokenKind::KW_VERIFY:     return "KW_VERIFY";
        case TokenKind::KW_REQUIRE:    return "KW_REQUIRE";
        case TokenKind::KW_AND:        return "KW_AND";
        case TokenKind::KW_OR:         return "KW_OR";
        case TokenKind::KW_NOT:        return "KW_NOT";
        case TokenKind::KW_TRUE:       return "KW_TRUE";
        case TokenKind::KW_FALSE:      return "KW_FALSE";
        case TokenKind::IDENT:         return "IDENT";
        case TokenKind::INT_LIT:       return "INT_LIT";
        case TokenKind::FLOAT_LIT:     return "FLOAT_LIT";
        case TokenKind::CHAR_LIT:      return "CHAR_LIT";
        case TokenKind::STR_LIT:       return "STR_LIT";
        case TokenKind::OP_ADD:        return "OP_ADD";
        case TokenKind::OP_SUB:        return "OP_SUB";
        case TokenKind::OP_MUL:        return "OP_MUL";
        case TokenKind::OP_DIV:        return "OP_DIV";
        case TokenKind::OP_MOD:        return "OP_MOD";
        case TokenKind::OP_POW:        return "OP_POW";
        case TokenKind::OP_LT:         return "OP_LT";
        case TokenKind::OP_LE:         return "OP_LE";
        case TokenKind::OP_GT:         return "OP_GT";
        case TokenKind::OP_GE:         return "OP_GE";
        case TokenKind::OP_EQ:         return "OP_EQ";
        case TokenKind::OP_NEQ:        return "OP_NEQ";
        case TokenKind::OP_ASSIGN:     return "OP_ASSIGN";
        case TokenKind::DOTDOT:        return "DOTDOT";
        case TokenKind::ARROW:         return "ARROW";
        case TokenKind::COLON:         return "COLON";
        case TokenKind::SEMICOLON:     return "SEMICOLON";
        case TokenKind::COMMA:         return "COMMA";
        case TokenKind::DOT:           return "DOT";
        case TokenKind::LPAREN:        return "LPAREN";
        case TokenKind::RPAREN:        return "RPAREN";
        case TokenKind::LBRACKET:      return "LBRACKET";
        case TokenKind::RBRACKET:      return "RBRACKET";
        default:                       return "UNKNOWN";
    }
}

} // namespace nova

#endif // NOVA_LEXER_TOKEN_HPP
