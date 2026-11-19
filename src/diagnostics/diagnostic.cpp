#include "diagnostics/diagnostic.hpp"
#include <sstream>

namespace nova {

static const char* severity_label(DiagnosticSeverity sev) {
    switch (sev) {
        case DiagnosticSeverity::ERROR:   return "error";
        case DiagnosticSeverity::WARNING: return "warning";
        case DiagnosticSeverity::NOTE:    return "note";
    }
    return "error";
}

std::string Diagnostic::format() const {
    std::ostringstream oss;
    if (!location.filename.empty()) {
        oss << location.filename << ":";
    }
    oss << location.line << ":" << location.column << ": "
        << severity_label(severity) << ": " << message;
    return oss.str();
}

void DiagnosticEngine::report_error(const SourceLocation& loc, const std::string& message) {
    m_diagnostics.push_back(Diagnostic{DiagnosticSeverity::ERROR, loc, message});
    ++m_error_count;
}

void DiagnosticEngine::report_warning(const SourceLocation& loc, const std::string& message) {
    m_diagnostics.push_back(Diagnostic{DiagnosticSeverity::WARNING, loc, message});
}

void DiagnosticEngine::report_note(const SourceLocation& loc, const std::string& message) {
    m_diagnostics.push_back(Diagnostic{DiagnosticSeverity::NOTE, loc, message});
}

void DiagnosticEngine::print_all(std::ostream& out) const {
    for (const auto& diag : m_diagnostics) {
        out << diag.format() << "\n";
    }
}

void DiagnosticEngine::clear() {
    m_diagnostics.clear();
    m_error_count = 0;
}

} // namespace nova
