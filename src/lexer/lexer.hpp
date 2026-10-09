#ifndef NOVA_LEXER_LEXER_HPP
#define NOVA_LEXER_LEXER_HPP

#include "lexer/token.hpp"
#include <string>
#include <vector>
#include <memory>

namespace nova {

class Lexer {
public:
    explicit Lexer(std::string filename);
    ~Lexer();

    // Non-copyable
    Lexer(const Lexer&) = delete;
    Lexer& operator=(const Lexer&) = delete;

    // Retrieve all tokens until EOF
    std::vector<Token> tokenize();

    // Check if lexical errors were encountered
    bool has_error() const { return m_has_error; }

    // Retrieve list of error messages
    const std::vector<std::string>& errors() const { return m_errors; }

    // Print formatted token stream to an output stream
    void print_tokens(std::ostream& out = std::cout);

    // Internal callback from Flex scanner
    void record_error(int line, int col, const std::string& msg);

private:
    std::string m_filename;
    bool m_has_error = false;
    std::vector<std::string> m_errors;
    void* m_file_handle = nullptr;
};

} // namespace nova

#endif // NOVA_LEXER_LEXER_HPP
