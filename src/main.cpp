#include <iostream>
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
    std::cout << "  --ast <file>            Syntax analysis: parse and dump AST\n";
    std::cout << "  --symbols <file>        Semantic analysis: validate and dump symbol tables\n";
    std::cout << "  --contracts <file>      Verification: inspect active contracts & preconditions\n";
    std::cout << "  --baseline-ir <file>    IR: emit baseline Three-Address Code (TAC)\n";
    std::cout << "  --ir <file>             IR: emit contract-aware optimized TAC\n";
    std::cout << "  -o <file>               Specify output artifact destination\n";
}

enum class Mode {
    Compile,
    Tokens,
    AST,
    Symbols,
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
        case nova::Mode::Tokens:
            std::cout << "[NOVA Phase 2 Lexer]: Tokenization will be implemented in Phase 2 for: " << options.input_file << "\n";
            break;
        case nova::Mode::AST:
            std::cout << "[NOVA Phase 4 AST]: AST dump will be implemented in Phase 4 for: " << options.input_file << "\n";
            break;
        case nova::Mode::Symbols:
            std::cout << "[NOVA Phase 6 Symbols]: Symbol resolution will be implemented in Phase 6 for: " << options.input_file << "\n";
            break;
        case nova::Mode::Contracts:
            std::cout << "[NOVA Phase 15 Contracts]: Contract inspector will be implemented in Phase 15 for: " << options.input_file << "\n";
            break;
        case nova::Mode::BaselineIR:
            std::cout << "[NOVA Phase 17 Baseline IR]: Baseline IR will be implemented in Phase 17 for: " << options.input_file << "\n";
            break;
        case nova::Mode::IR:
            std::cout << "[NOVA Phase 16 IR]: Optimized IR will be implemented in Phase 16 for: " << options.input_file << "\n";
            break;
        case nova::Mode::Compile:
            std::cout << "[NOVA Compiler]: Compiling " << options.input_file << " ...\n";
            break;
        default:
            break;
    }

    return 0;
}
