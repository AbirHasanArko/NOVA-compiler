#include "parser/parser.hpp"
#include "parser/parser.tab.hpp"
#include "lexer/lexer.hpp"

namespace nova {

Parser::Parser(std::string filename)
    : m_filename(std::move(filename)) {}

void Parser::record_error(int line, int col, const std::string& msg) {
    m_has_error = true;
    std::string err = m_filename + ":" + std::to_string(line) + ":" + std::to_string(col) + ": syntax error: " + msg;
    m_errors.push_back(std::move(err));
}

bool Parser::has_next_token() const {
    return m_pos < m_tokens.size();
}

const Token& Parser::peek_token() const {
    if (m_pos < m_tokens.size()) {
        return m_tokens[m_pos];
    }
    return m_last_token;
}

Token Parser::consume_token() {
    if (m_pos < m_tokens.size()) {
        m_last_token = m_tokens[m_pos++];
        return m_last_token;
    }
    return m_last_token;
}

const Token& Parser::consume_token_ref() {
    if (m_pos < m_tokens.size()) {
        const Token& ref = m_tokens[m_pos++];
        m_last_token = ref;
        return ref;
    }
    return m_last_token;
}

bool Parser::parse() {
    m_tokens.clear();
    m_pos = 0;
    m_has_error = false;
    m_errors.clear();
    m_root.reset();

    Lexer lexer(m_filename);
    m_tokens = lexer.tokenize();

    if (lexer.has_error()) {
        m_has_error = true;
        for (const auto& err : lexer.errors()) {
            m_errors.push_back(err);
        }
        return false;
    }

    int status = nova_parser_parse(this);
    return (status == 0) && !m_has_error;
}

} // namespace nova

static int map_token_kind(nova::TokenKind kind) {
    using K = nova::TokenKind;
    switch (kind) {
        case K::TOKEN_EOF:   return 0; // YYEOF / NOVA_PARSER_EOF
        case K::TOKEN_ERROR: return TOKEN_INVALID;

        case K::KW_IGNITE:   return KW_IGNITE;
        case K::KW_ROUTINE:  return KW_ROUTINE;
        case K::KW_RET:      return KW_RET;
        case K::KW_CONST:    return KW_CONST;
        case K::KW_FLUX:     return KW_FLUX;
        case K::KW_DO:       return KW_DO;
        case K::KW_END:      return KW_END;

        case K::KW_I32:      return KW_I32;
        case K::KW_F32:      return KW_F32;
        case K::KW_BOOL:     return KW_BOOL;
        case K::KW_CHAR:     return KW_CHAR;
        case K::KW_STR:      return KW_STR;
        case K::KW_VOID:     return KW_VOID;

        case K::KW_DETECT:   return KW_DETECT;
        case K::KW_REDETECT: return KW_REDETECT;
        case K::KW_FALLBACK: return KW_FALLBACK;
        case K::KW_ROUTE:    return KW_ROUTE;
        case K::KW_CASE:     return KW_CASE;
        case K::KW_DEFAULT:  return KW_DEFAULT;

        case K::KW_ORBIT:    return KW_ORBIT;
        case K::KW_IN:       return KW_IN;
        case K::KW_STEP:     return KW_STEP;
        case K::KW_CRUISE:   return KW_CRUISE;
        case K::KW_HALT:     return KW_HALT;
        case K::KW_SKIP:     return KW_SKIP;

        case K::KW_TRANSMIT: return KW_TRANSMIT;
        case K::KW_RECEIVE:  return KW_RECEIVE;
        case K::KW_VERIFY:   return KW_VERIFY;
        case K::KW_REQUIRE:  return KW_REQUIRE;

        case K::KW_AND:      return KW_AND;
        case K::KW_OR:       return KW_OR;
        case K::KW_NOT:      return KW_NOT;
        case K::KW_TRUE:     return KW_TRUE;
        case K::KW_FALSE:    return KW_FALSE;

        case K::IDENT:       return IDENT;
        case K::INT_LIT:     return INT_LIT;
        case K::FLOAT_LIT:   return FLOAT_LIT;
        case K::CHAR_LIT:    return CHAR_LIT;
        case K::STR_LIT:     return STR_LIT;

        case K::OP_ADD:      return OP_ADD;
        case K::OP_SUB:      return OP_SUB;
        case K::OP_MUL:      return OP_MUL;
        case K::OP_DIV:      return OP_DIV;
        case K::OP_MOD:      return OP_MOD;
        case K::OP_POW:      return OP_POW;

        case K::OP_LT:       return OP_LT;
        case K::OP_LE:       return OP_LE;
        case K::OP_GT:       return OP_GT;
        case K::OP_GE:       return OP_GE;
        case K::OP_EQ:       return OP_EQ;
        case K::OP_NEQ:      return OP_NEQ;
        case K::OP_ASSIGN:   return OP_ASSIGN;

        case K::DOTDOT:      return DOTDOT;
        case K::ARROW:       return ARROW;
        case K::COLON:       return COLON;
        case K::SEMICOLON:   return SEMICOLON;
        case K::COMMA:       return COMMA;
        case K::DOT:         return DOT;
        case K::LPAREN:      return LPAREN;
        case K::RPAREN:      return RPAREN;
        case K::LBRACKET:    return LBRACKET;
        case K::RBRACKET:    return RBRACKET;
        default:             return TOKEN_INVALID;
    }
}

int nova_parser_lex(NOVA_PARSER_STYPE* yylval_param, NOVA_PARSER_LTYPE* yylloc_param, nova::Parser* parser) {
    if (!parser) {
        return 0;
    }

    const nova::Token& tok = parser->consume_token_ref();
    if (yylval_param) {
        yylval_param->token = &tok;
    }
    if (yylloc_param) {
        yylloc_param->first_line = tok.location.line;
        yylloc_param->first_column = tok.location.column;
        yylloc_param->last_line = tok.location.line;
        yylloc_param->last_column = tok.location.column + static_cast<int>(tok.lexeme.size());
    }

    return map_token_kind(tok.kind);
}

