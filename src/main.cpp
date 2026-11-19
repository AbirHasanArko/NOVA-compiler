#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "ast/ast_printer.hpp"
#include "diagnostics/diagnostic.hpp"
#include "semantic/symbol_resolver.hpp"
#include "semantic/type_checker.hpp"
#include "codegen/interpreter.hpp"
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <fstream>

namespace nova {

const std::string VERSION = "0.1.0-alpha";
const std::string EDITION = "Cosmic Telemetry Edition";

void print_version() {
    std::cout << "NOVA Reference Compiler v" << VERSION << " (" << EDITION << ")\n";
    std::cout << "Target: C++17 / Flex & Bison Reference Pipeline\n";
}

void print_help(const std::string& program_name) {
    std::cout << "NOVA Compiler — Cosmic Telemetry & Systems Computing\n";
    std::cout << "Usage: " << program_name << " [options] <source_file.nova>\n\n";
    std::cout << "General Options:\n";
    std::cout << "  -h, --help              Display this help message and exit\n";
    std::cout << "  -v, --version           Display compiler version and exit\n\n";
    std::cout << "Pipeline Inspection Flags:\n";
    std::cout << "  --tokens <file>         Lexical analysis: scan and dump token stream\n";
    std::cout << "  --parse <file>          Syntax analysis: parse and verify grammar\n";
    std::cout << "  --ast <file>            Syntax analysis: parse and dump AST\n";
    std::cout << "  --symbols <file>        Semantic analysis: validate and dump symbol tables\n";
    std::cout << "  --interpret <file>      Execution: evaluate AST and emit program output\n";
    std::cout << "  --contracts <file>      Verification: inspect active contracts & preconditions\n";
    std::cout << "  --baseline-ir <file>    IR: emit baseline Three-Address Code (TAC)\n";
    std::cout << "  --ir <file>             IR: emit contract-aware optimized TAC\n";
    std::cout << "  -o <file>               Specify output artifact destination\n";
}

enum class Mode {
    Compile,
    Tokens,
    Parse,
    AST,
    Symbols,
    Interpret,
    Contracts,
    BaselineIR,
    IR,
    Help,
    Version
};

struct CompilerOptions {
    Mode mode = Mode::Compile;
    std::string input_file;
    std::string output_file;
    bool has_error = false;
    std::string error_message;
};

CompilerOptions parse_args(int argc, char* argv[]) {
    CompilerOptions options;

    if (argc < 2) {
        options.has_error = true;
        options.error_message = "error: no input files or options provided. Use '--help' for usage.";
        return options;
    }

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            options.mode = Mode::Help;
            return options;
        } else if (arg == "-v" || arg == "--version") {
            options.mode = Mode::Version;
            return options;
        } else if (arg == "--tokens") {
            options.mode = Mode::Tokens;
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                options.input_file = argv[++i];
            } else {
                options.has_error = true;
                options.error_message = "error: '--tokens' requires a source file path.";
                return options;
            }
        } else if (arg == "--parse") {
            options.mode = Mode::Parse;
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                options.input_file = argv[++i];
            } else {
                options.has_error = true;
                options.error_message = "error: '--parse' requires a source file path.";
                return options;
            }
        } else if (arg == "--ast") {
            options.mode = Mode::AST;
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                options.input_file = argv[++i];
            } else {
                options.has_error = true;
                options.error_message = "error: '--ast' requires a source file path.";
                return options;
            }
        } else if (arg == "--symbols") {
            options.mode = Mode::Symbols;
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                options.input_file = argv[++i];
            } else {
                options.has_error = true;
                options.error_message = "error: '--symbols' requires a source file path.";
                return options;
            }
        } else if (arg == "--interpret") {
            options.mode = Mode::Interpret;
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                options.input_file = argv[++i];
            } else {
                options.has_error = true;
                options.error_message = "error: '--interpret' requires a source file path.";
                return options;
            }
        } else if (arg == "--contracts") {
            options.mode = Mode::Contracts;
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                options.input_file = argv[++i];
            } else {
                options.has_error = true;
                options.error_message = "error: '--contracts' requires a source file path.";
                return options;
            }
        } else if (arg == "--baseline-ir") {
            options.mode = Mode::BaselineIR;
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                options.input_file = argv[++i];
            } else {
                options.has_error = true;
                options.error_message = "error: '--baseline-ir' requires a source file path.";
                return options;
            }
        } else if (arg == "--ir") {
            options.mode = Mode::IR;
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                options.input_file = argv[++i];
            } else {
                options.has_error = true;
                options.error_message = "error: '--ir' requires a source file path.";
                return options;
            }
        } else if (arg == "-o") {
            if (i + 1 < argc) {
                options.output_file = argv[++i];
            } else {
                options.has_error = true;
                options.error_message = "error: '-o' requires an output file path.";
                return options;
            }
        } else if (!arg.empty() && arg[0] == '-') {
            options.has_error = true;
            options.error_message = "error: unrecognized command line option '" + arg + "'";
            return options;
        } else {
            if (options.input_file.empty()) {
                options.input_file = arg;
            } else {
                options.has_error = true;
                options.error_message = "error: multiple input files are not supported: '" + arg + "'";
                return options;
            }
        }
    }

    if (options.input_file.empty() && options.mode != Mode::Help && options.mode != Mode::Version) {
        options.has_error = true;
        options.error_message = "error: no input file specified.";
    }

    return options;
}

bool file_exists(const std::string& path) {
    std::ifstream file(path);
    return file.good();
}

enum class TokenCategory {
    KEYWORD,
    IDENTIFIER,
    LITERAL,
    OPERATOR,
    PUNCTUATOR,
    OTHER
};

TokenCategory classify_token(TokenKind kind) {
    switch (kind) {
        case TokenKind::KW_IGNITE:
        case TokenKind::KW_ROUTINE:
        case TokenKind::KW_RET:
        case TokenKind::KW_CONST:
        case TokenKind::KW_FLUX:
        case TokenKind::KW_DO:
        case TokenKind::KW_END:
        case TokenKind::KW_I32:
        case TokenKind::KW_F32:
        case TokenKind::KW_BOOL:
        case TokenKind::KW_CHAR:
        case TokenKind::KW_STR:
        case TokenKind::KW_VOID:
        case TokenKind::KW_DETECT:
        case TokenKind::KW_REDETECT:
        case TokenKind::KW_FALLBACK:
        case TokenKind::KW_ROUTE:
        case TokenKind::KW_CASE:
        case TokenKind::KW_DEFAULT:
        case TokenKind::KW_ORBIT:
        case TokenKind::KW_IN:
        case TokenKind::KW_STEP:
        case TokenKind::KW_CRUISE:
        case TokenKind::KW_HALT:
        case TokenKind::KW_SKIP:
        case TokenKind::KW_TRANSMIT:
        case TokenKind::KW_RECEIVE:
        case TokenKind::KW_VERIFY:
        case TokenKind::KW_REQUIRE:
        case TokenKind::KW_AND:
        case TokenKind::KW_OR:
        case TokenKind::KW_NOT:
        case TokenKind::KW_TRUE:
        case TokenKind::KW_FALSE:
            return TokenCategory::KEYWORD;

        case TokenKind::IDENT:
            return TokenCategory::IDENTIFIER;

        case TokenKind::INT_LIT:
        case TokenKind::FLOAT_LIT:
        case TokenKind::CHAR_LIT:
        case TokenKind::STR_LIT:
            return TokenCategory::LITERAL;

        case TokenKind::OP_ADD:
        case TokenKind::OP_SUB:
        case TokenKind::OP_MUL:
        case TokenKind::OP_DIV:
        case TokenKind::OP_MOD:
        case TokenKind::OP_POW:
        case TokenKind::OP_LT:
        case TokenKind::OP_LE:
        case TokenKind::OP_GT:
        case TokenKind::OP_GE:
        case TokenKind::OP_EQ:
        case TokenKind::OP_NEQ:
        case TokenKind::OP_ASSIGN:
        case TokenKind::DOTDOT:
        case TokenKind::ARROW:
            return TokenCategory::OPERATOR;

        case TokenKind::COLON:
        case TokenKind::SEMICOLON:
        case TokenKind::COMMA:
        case TokenKind::DOT:
        case TokenKind::LPAREN:
        case TokenKind::RPAREN:
        case TokenKind::LBRACKET:
        case TokenKind::RBRACKET:
            return TokenCategory::PUNCTUATOR;

        default:
            return TokenCategory::OTHER;
    }
}

class ConstructDetector : public ASTVisitor {
public:
    struct RoutineInfo {
        std::string signature;
        int line = 0;
        int column = 0;
    };

    std::vector<RoutineInfo> routines;
    size_t routine_calls = 0;
    size_t orbit_loops = 0;
    size_t cruise_loops = 0;
    size_t halt_stmts = 0;
    size_t skip_stmts = 0;
    size_t detect_stmts = 0;
    size_t redetect_branches = 0;
    size_t fallback_branches = 0;
    size_t route_stmts = 0;
    size_t case_clauses = 0;
    size_t default_clauses = 0;
    size_t const_decls = 0;
    size_t flux_decls = 0;
    size_t array_decls = 0;
    size_t assignments = 0;
    size_t require_contracts = 0;
    size_t verify_contracts = 0;
    size_t transmit_stmts = 0;
    size_t receive_stmts = 0;

    void visit(ProgramNode& node) override {
        for (const auto& r : node.routines) {
            if (r) r->accept(*this);
        }
        for (const auto& s : node.ignite_body) {
            if (s) s->accept(*this);
        }
    }

    void visit(RoutineDeclNode& node) override {
        std::ostringstream sig;
        sig << node.name << "(";
        for (size_t i = 0; i < node.params.size(); ++i) {
            if (i > 0) sig << ", ";
            sig << node.params[i].name << ": " << node.params[i].type.to_string();
            if (node.params[i].type.is_array) ++array_decls;
        }
        sig << ") -> " << node.return_type.to_string();
        routines.push_back(RoutineInfo{sig.str(), node.location.line, node.location.column});

        for (const auto& req : node.requires) {
            if (req) req->accept(*this);
        }
        for (const auto& s : node.body) {
            if (s) s->accept(*this);
        }
    }

    void visit(RequireNode& node) override {
        ++require_contracts;
        if (node.condition) node.condition->accept(*this);
    }

    void visit(ConstDeclNode& node) override {
        ++const_decls;
        if (node.type.is_array) ++array_decls;
        if (node.initializer) node.initializer->accept(*this);
    }

    void visit(FluxDeclNode& node) override {
        ++flux_decls;
        if (node.type.is_array) ++array_decls;
        if (node.initializer) node.initializer->accept(*this);
    }

    void visit(AssignNode& node) override {
        ++assignments;
        if (node.target) node.target->accept(*this);
        if (node.value) node.value->accept(*this);
    }

    void visit(ExprStmtNode& node) override {
        if (node.expr) node.expr->accept(*this);
    }

    void visit(DetectNode& node) override {
        ++detect_stmts;
        if (node.branches.size() > 1) {
            redetect_branches += (node.branches.size() - 1);
        }
        for (const auto& b : node.branches) {
            if (b.condition) b.condition->accept(*this);
            for (const auto& s : b.body) {
                if (s) s->accept(*this);
            }
        }
        if (node.has_fallback) {
            ++fallback_branches;
            for (const auto& s : node.fallback_body) {
                if (s) s->accept(*this);
            }
        }
    }

    void visit(CruiseNode& node) override {
        ++cruise_loops;
        if (node.condition) node.condition->accept(*this);
        for (const auto& s : node.body) {
            if (s) s->accept(*this);
        }
    }

    void visit(OrbitNode& node) override {
        ++orbit_loops;
        if (node.start) node.start->accept(*this);
        if (node.end) node.end->accept(*this);
        if (node.step) node.step->accept(*this);
        for (const auto& s : node.body) {
            if (s) s->accept(*this);
        }
    }

    void visit(RouteNode& node) override {
        ++route_stmts;
        if (node.selector) node.selector->accept(*this);
        case_clauses += node.cases.size();
        for (const auto& c : node.cases) {
            for (const auto& s : c.body) {
                if (s) s->accept(*this);
            }
        }
        if (node.has_default) {
            ++default_clauses;
            for (const auto& s : node.default_body) {
                if (s) s->accept(*this);
            }
        }
    }

    void visit(TransmitNode& node) override {
        ++transmit_stmts;
        for (const auto& a : node.args) {
            if (a) a->accept(*this);
        }
    }

    void visit(ReceiveNode&) override { ++receive_stmts; }

    void visit(VerifyNode& node) override {
        ++verify_contracts;
        if (node.condition) node.condition->accept(*this);
    }

    void visit(RetNode& node) override {
        if (node.value) node.value->accept(*this);
    }

    void visit(HaltNode&) override { ++halt_stmts; }
    void visit(SkipNode&) override { ++skip_stmts; }
    void visit(LiteralNode&) override {}
    void visit(IdentifierNode&) override {}

    void visit(ArrayLiteralNode& node) override {
        for (const auto& e : node.elements) {
            if (e) e->accept(*this);
        }
    }

    void visit(IndexAccessNode& node) override {
        if (node.array) node.array->accept(*this);
        if (node.index) node.index->accept(*this);
    }

    void visit(RoutineCallNode& node) override {
        ++routine_calls;
        for (const auto& a : node.args) {
            if (a) a->accept(*this);
        }
    }

    void visit(UnaryExprNode& node) override {
        if (node.operand) node.operand->accept(*this);
    }

    void visit(BinaryExprNode& node) override {
        if (node.left) node.left->accept(*this);
        if (node.right) node.right->accept(*this);
    }
};

void format_analysis_report(
    std::ostream& report,
    const std::string& input_file,
    const std::vector<Token>& tokens,
    ProgramNode& root,
    const SymbolTable& symtab,
    const std::string& execution_output
) {
    size_t total_tokens = 0;
    size_t kw_count = 0;
    size_t ident_count = 0;
    size_t lit_count = 0;
    size_t op_count = 0;
    size_t punct_count = 0;
    std::map<std::string, size_t> keyword_freq;

    for (const auto& tok : tokens) {
        if (tok.kind == TokenKind::TOKEN_EOF) continue;
        ++total_tokens;
        switch (classify_token(tok.kind)) {
            case TokenCategory::KEYWORD:
                ++kw_count;
                ++keyword_freq[tok.lexeme];
                break;
            case TokenCategory::IDENTIFIER:
                ++ident_count;
                break;
            case TokenCategory::LITERAL:
                ++lit_count;
                break;
            case TokenCategory::OPERATOR:
                ++op_count;
                break;
            case TokenCategory::PUNCTUATOR:
                ++punct_count;
                break;
            default:
                break;
        }
    }

    ConstructDetector detector;
    root.accept(detector);

    report << "======================================================================\n";
    report << "              NOVA COMPILER — ANALYSIS & EXECUTION REPORT             \n";
    report << "======================================================================\n";
    report << "Source File : " << input_file << "\n\n";

    report << "----------------------------------------------------------------------\n";
    report << "1. PROGRAM EXECUTION OUTPUT (transmit Results)\n";
    report << "----------------------------------------------------------------------\n";
    report << execution_output << "\n";

    report << "----------------------------------------------------------------------\n";
    report << "2. SYNTACTIC & AST CONSTRUCT DETECTION SUMMARY\n";
    report << "----------------------------------------------------------------------\n";
    report << "Functions / Routines Detected : " << detector.routines.size()
           << " (Total Calls: " << detector.routine_calls << ")\n";
    for (size_t i = 0; i < detector.routines.size(); ++i) {
        report << "  [Function #" << (i + 1) << "] routine " << detector.routines[i].signature
               << " (line " << detector.routines[i].line << ", col " << detector.routines[i].column << ")\n";
    }
    report << "Loops Detected                : " << (detector.orbit_loops + detector.cruise_loops)
           << " (orbit: " << detector.orbit_loops
           << ", cruise: " << detector.cruise_loops
           << ", skip: " << detector.skip_stmts
           << ", halt: " << detector.halt_stmts << ")\n";
    report << "Conditional Branches Detected : " << (detector.detect_stmts + detector.route_stmts)
           << " (detect: " << detector.detect_stmts
           << ", redetect: " << detector.redetect_branches
           << ", fallback: " << detector.fallback_branches
           << ", route: " << detector.route_stmts
           << ", case: " << detector.case_clauses
           << ", default: " << detector.default_clauses << ")\n";
    report << "State Declarations Detected   : " << (detector.const_decls + detector.flux_decls)
           << " (const: " << detector.const_decls
           << ", flux: " << detector.flux_decls
           << ", arrays T[N]: " << detector.array_decls
           << ", assignments: " << detector.assignments << ")\n";
    report << "Contracts & I/O Detected      : "
           << "require: " << detector.require_contracts
           << ", verify: " << detector.verify_contracts
           << ", transmit: " << detector.transmit_stmts
           << ", receive: " << detector.receive_stmts << "\n\n";

    report << "----------------------------------------------------------------------\n";
    report << "3. LEXICAL TOKEN COUNTS & KEYWORD FREQUENCY\n";
    report << "----------------------------------------------------------------------\n";
    report << "Total Tokens Scanned          : " << total_tokens << "\n";
    report << "  - Keywords                  : " << kw_count << " (" << keyword_freq.size() << " unique)\n";
    report << "  - Identifiers               : " << ident_count << "\n";
    report << "  - Literals                  : " << lit_count << "\n";
    report << "  - Operators                 : " << op_count << "\n";
    report << "  - Punctuators / Delimiters  : " << punct_count << "\n\n";

    report << "Keyword Breakdown:\n";
    for (const auto& kv : keyword_freq) {
        report << "  " << std::left << std::setw(14) << kv.first << " : " << kv.second << "\n";
    }
    report << "\n";

    report << "----------------------------------------------------------------------\n";
    report << "4. LEXICAL SYMBOL TABLE\n";
    report << "----------------------------------------------------------------------\n";
    symtab.dump(report);
    report << "\n";

    report << "----------------------------------------------------------------------\n";
    report << "5. COMPLETE TOKEN STREAM (" << total_tokens << " tokens)\n";
    report << "----------------------------------------------------------------------\n";
    for (const auto& tok : tokens) {
        if (tok.kind == TokenKind::TOKEN_EOF) continue;
        report << "[" << tok.location.line << ":" << tok.location.column << "] "
               << std::left << std::setw(15) << token_kind_to_string(tok.kind)
               << " \"" << tok.lexeme << "\"\n";
    }
    report << "======================================================================\n";
}

} // namespace nova

int main(int argc, char* argv[]) {
    nova::CompilerOptions options = nova::parse_args(argc, argv);

    if (options.has_error) {
        std::cerr << options.error_message << "\n";
        return 1;
    }

    if (options.mode == nova::Mode::Help) {
        nova::print_help(argv[0]);
        return 0;
    }

    if (options.mode == nova::Mode::Version) {
        nova::print_version();
        return 0;
    }

    // Verify input file existence
    if (!nova::file_exists(options.input_file)) {
        std::cerr << "error: cannot open input file '" << options.input_file << "'\n";
        return 1;
    }

    switch (options.mode) {
        case nova::Mode::Tokens: {
            nova::Lexer lexer(options.input_file);
            lexer.print_tokens(std::cout);
            if (lexer.has_error()) {
                return 1;
            }
            return 0;
        }
        case nova::Mode::Parse: {
            nova::Parser parser(options.input_file);
            if (!parser.parse()) {
                for (const auto& err : parser.errors()) {
                    std::cerr << err << "\n";
                }
                return 1;
            }
            std::cout << "[NOVA Parser]: Syntax check passed for '" << options.input_file << "'.\n";
            return 0;
        }
        case nova::Mode::AST: {
            nova::Parser parser(options.input_file);
            if (!parser.parse()) {
                for (const auto& err : parser.errors()) {
                    std::cerr << err << "\n";
                }
                return 1;
            }
            if (parser.root()) {
                nova::ASTPrinter printer(std::cout);
                parser.root()->accept(printer);
            }
            return 0;
        }
        case nova::Mode::Symbols: {
            nova::Parser parser(options.input_file);
            if (!parser.parse()) {
                for (const auto& err : parser.errors()) {
                    std::cerr << err << "\n";
                }
                return 1;
            }
            nova::DiagnosticEngine diagnostics;
            nova::SymbolResolver resolver(diagnostics);
            nova::TypeChecker type_checker(diagnostics);
            if (parser.root()) {
                resolver.resolve(*parser.root());
                type_checker.check(*parser.root());
            }
            if (diagnostics.has_errors()) {
                diagnostics.print_all(std::cerr);
                return 1;
            }
            resolver.symbol_table().dump(std::cout);
            return 0;
        }
        case nova::Mode::Contracts:
            std::cout << "[NOVA Phase 15 Contracts]: Contract inspector will be implemented in Phase 15 for: " << options.input_file << "\n";
            break;
        case nova::Mode::BaselineIR:
            std::cout << "[NOVA Phase 17 Baseline IR]: Baseline IR will be implemented in Phase 17 for: " << options.input_file << "\n";
            break;
        case nova::Mode::IR:
            std::cout << "[NOVA Phase 16 IR]: Optimized IR will be implemented in Phase 16 for: " << options.input_file << "\n";
            break;
        case nova::Mode::Interpret:
        case nova::Mode::Compile: {
            nova::Parser parser(options.input_file);
            if (!parser.parse()) {
                for (const auto& err : parser.errors()) {
                    std::cerr << err << "\n";
                }
                return 1;
            }
            nova::DiagnosticEngine diagnostics;
            nova::SymbolResolver resolver(diagnostics);
            nova::TypeChecker type_checker(diagnostics);
            if (parser.root()) {
                resolver.resolve(*parser.root());
                type_checker.check(*parser.root());
            }
            if (diagnostics.has_errors()) {
                diagnostics.print_all(std::cerr);
                return 1;
            }

            if (options.mode == nova::Mode::Compile) {
                std::cout << "[NOVA Compiler]: Compiling " << options.input_file << " ...\n";
            }

            nova::Interpreter interpreter(diagnostics, std::cout, std::cin);
            if (parser.root() && !interpreter.execute(*parser.root())) {
                diagnostics.print_all(std::cerr);
                return 1;
            }

            std::string out_path = options.output_file.empty() ? "output.txt" : options.output_file;
            std::ofstream out_file(out_path);
            if (!out_file.is_open()) {
                std::cerr << "error: cannot open output file '" << out_path << "' for writing\n";
                return 1;
            }
            if (options.mode == nova::Mode::Compile && parser.root()) {
                nova::format_analysis_report(
                    out_file,
                    options.input_file,
                    parser.tokens(),
                    *parser.root(),
                    resolver.symbol_table(),
                    interpreter.captured_output()
                );
            } else {
                out_file << interpreter.captured_output();
            }
            out_file.close();
            return 0;
        }
        default:
            break;
    }

    return 0;
}
