#ifndef NOVA_DIAGNOSTICS_DIAGNOSTIC_HPP
#define NOVA_DIAGNOSTICS_DIAGNOSTIC_HPP

#include "lexer/token.hpp"
#include <iostream>
#include <string>
#include <vector>

namespace nova {

enum class DiagnosticSeverity {
    ERROR,
    WARNING,
    NOTE
};

struct Diagnostic {
    DiagnosticSeverity severity;
    SourceLocation location;
    std::string message;

    std::string format() const;
};

/**
 * Collects and formats source-location-aware diagnostics across compiler passes.
 */
class DiagnosticEngine {
public:
    DiagnosticEngine() = default;

    void report_error(const SourceLocation& loc, const std::string& message);
    void report_warning(const SourceLocation& loc, const std::string& message);
    void report_note(const SourceLocation& loc, const std::string& message);

    bool has_errors() const { return m_error_count > 0; }
    size_t error_count() const { return m_error_count; }
    const std::vector<Diagnostic>& diagnostics() const { return m_diagnostics; }

    void print_all(std::ostream& out = std::cerr) const;
    void clear();

private:
    std::vector<Diagnostic> m_diagnostics;
    size_t m_error_count = 0;
};

} // namespace nova

#endif // NOVA_DIAGNOSTICS_DIAGNOSTIC_HPP
