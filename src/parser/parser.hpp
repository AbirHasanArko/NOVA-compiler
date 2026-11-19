#ifndef NOVA_PARSER_PARSER_HPP
#define NOVA_PARSER_PARSER_HPP

#include "lexer/token.hpp"
#include "ast/ast_node.hpp"
#include <memory>
#include <string>
#include <vector>

namespace nova {

class Parser {
public:
    explicit Parser(std::string filename);
    ~Parser() = default;

    // Non-copyable
    Parser(const Parser&) = delete;
    Parser& operator=(const Parser&) = delete;

    // Run lexical and syntactic analysis on the source file.
    // Returns true if parsing succeeds with zero errors.
    bool parse();

    bool has_error() const { return m_has_error; }
    const std::vector<std::string>& errors() const { return m_errors; }

    // Record a syntax error with exact 1-indexed source coordinates
    void record_error(int line, int col, const std::string& msg);

    // Token stream accessors used by the Bison yylex bridge
    bool has_next_token() const;
    const Token& peek_token() const;
    Token consume_token();
    const Token& consume_token_ref();
    const Token& last_token() const { return m_last_token; }
    const std::vector<Token>& tokens() const { return m_tokens; }

    // AST root ownership
    void set_root(std::unique_ptr<ProgramNode> root) { m_root = std::move(root); }
    ProgramNode* root() const { return m_root.get(); }
    const std::string& filename() const { return m_filename; }

private:
    std::string m_filename;
    std::vector<Token> m_tokens;
    size_t m_pos = 0;
    Token m_last_token{TokenKind::TOKEN_EOF, "", SourceLocation{}};
    bool m_has_error = false;
    std::vector<std::string> m_errors;
    std::unique_ptr<ProgramNode> m_root;
};

} // namespace nova

#endif // NOVA_PARSER_PARSER_HPP

