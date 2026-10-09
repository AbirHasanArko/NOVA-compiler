#include "lexer/lexer.hpp"
#include <cstdio>
#include <iomanip>
#include <iostream>

// External Flex prototypes and variables
extern nova::Token yylex();
extern FILE* yyin;
extern void nova_lexer_reset(const std::string& filename);

namespace nova {

Lexer* g_current_lexer = nullptr;
std::string g_current_filename = "";

Lexer::Lexer(std::string filename)
    : m_filename(std::move(filename)) {
}

Lexer::~Lexer() {
    if (m_file_handle) {
        std::fclose(static_cast<FILE*>(m_file_handle));
        m_file_handle = nullptr;
    }
}

void Lexer::record_error(int line, int col, const std::string& msg) {
    m_has_error = true;
    std::string err = m_filename + ":" + std::to_string(line) + ":" + std::to_string(col) + ": lexical error: " + msg;
    m_errors.push_back(std::move(err));
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    FILE* file = std::fopen(m_filename.c_str(), "r");
    if (!file) {
        record_error(1, 1, "cannot open source file '" + m_filename + "'");
        return tokens;
    }

    m_file_handle = file;
    yyin = file;
    g_current_lexer = this;
    nova_lexer_reset(m_filename);

    while (true) {
        Token tok = yylex();
        tokens.push_back(tok);
        if (tok.kind == TokenKind::TOKEN_EOF) {
            break;
        }
    }

    std::fclose(file);
    m_file_handle = nullptr;
    yyin = nullptr;
    g_current_lexer = nullptr;

    return tokens;
}

void Lexer::print_tokens(std::ostream& out) {
    std::vector<Token> tokens = tokenize();

    for (const auto& tok : tokens) {
        out << "Line " << std::setw(3) << std::left << tok.location.line
            << ":" << std::setw(3) << std::left << tok.location.column
            << " " << std::setw(15) << std::left << token_kind_to_string(tok.kind);

        if (tok.kind != TokenKind::TOKEN_EOF) {
            out << " '" << tok.lexeme << "'";
        }
        out << "\n";
    }

    if (m_has_error) {
        for (const auto& err : m_errors) {
            std::cerr << err << "\n";
        }
    }
}

} // namespace nova
