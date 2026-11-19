#ifndef NOVA_CODEGEN_INTERPRETER_HPP
#define NOVA_CODEGEN_INTERPRETER_HPP

#include "ast/ast_node.hpp"
#include "ast/ast_visitor.hpp"
#include "diagnostics/diagnostic.hpp"
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace nova {

/**
 * Tagged runtime value supporting NOVA primitives (i32, f32, bool, char, str, void)
 * and fixed-size arrays T[N].
 */
struct RuntimeValue {
    BaseType base = BaseType::VOID;
    bool is_array = false;
    int32_t i32_val = 0;
    float f32_val = 0.0f;
    bool bool_val = false;
    char char_val = '\0';
    std::string str_val;
    std::vector<RuntimeValue> elements;

    static RuntimeValue make_void();
    static RuntimeValue make_i32(int32_t v);
    static RuntimeValue make_f32(float v);
    static RuntimeValue make_bool(bool v);
    static RuntimeValue make_char(char v);
    static RuntimeValue make_str(std::string v);
    static RuntimeValue make_default(const TypeSpec& type);

    bool equals(const RuntimeValue& other) const;
    std::string to_output_string() const;
};

/**
 * AST-walking execution engine and reference oracle for NOVA programs.
 * Executes routines, preconditions (require), runtime invariants (verify),
 * control flow (detect/redetect/fallback, cruise, orbit, route, halt, skip, ret),
 * and telemetry I/O (transmit, receive), capturing all transmitted output.
 */
class Interpreter : public ASTVisitor {
public:
    Interpreter(
        DiagnosticEngine& diagnostics,
        std::ostream& out = std::cout,
        std::istream& in = std::cin
    );

    bool execute(ProgramNode& root);

    const std::string& captured_output() const { return m_captured_output; }

    void visit(ProgramNode& node) override;
    void visit(RoutineDeclNode& node) override;
    void visit(RequireNode& node) override;

    void visit(ConstDeclNode& node) override;
    void visit(FluxDeclNode& node) override;
    void visit(AssignNode& node) override;
    void visit(ExprStmtNode& node) override;
    void visit(DetectNode& node) override;
    void visit(CruiseNode& node) override;
    void visit(OrbitNode& node) override;
    void visit(RouteNode& node) override;
    void visit(TransmitNode& node) override;
    void visit(ReceiveNode& node) override;
    void visit(VerifyNode& node) override;
    void visit(RetNode& node) override;
    void visit(HaltNode& node) override;
    void visit(SkipNode& node) override;

    void visit(LiteralNode& node) override;
    void visit(IdentifierNode& node) override;
    void visit(ArrayLiteralNode& node) override;
    void visit(IndexAccessNode& node) override;
    void visit(RoutineCallNode& node) override;
    void visit(UnaryExprNode& node) override;
    void visit(BinaryExprNode& node) override;

private:
    enum class FlowState {
        NORMAL,
        HALT,
        SKIP,
        RETURN,
        ABORT
    };

    using ScopeMap = std::unordered_map<std::string, RuntimeValue>;
    using CallFrame = std::vector<ScopeMap>;

    DiagnosticEngine& m_diagnostics;
    std::ostream& m_out;
    std::istream& m_in;
    std::ostringstream m_capture_stream;
    std::string m_captured_output;

    std::unordered_map<std::string, RoutineDeclNode*> m_routines;
    std::vector<CallFrame> m_frames;
    FlowState m_flow = FlowState::NORMAL;
    RuntimeValue m_last_value;
    RuntimeValue m_return_value;
    std::string m_active_routine;

    void push_frame();
    void pop_frame();
    void enter_scope();
    void exit_scope();

    void define_var(const std::string& name, RuntimeValue val);
    RuntimeValue* find_var(const std::string& name);
    RuntimeValue eval(ExprNode& expr);
    void exec_block(const std::vector<StmtPtr>& stmts);
    void runtime_error(const SourceLocation& loc, const std::string& msg);

    static std::string unescape_string(const std::string& raw);
    static char unescape_char(const std::string& raw);
};

} // namespace nova

#endif // NOVA_CODEGEN_INTERPRETER_HPP
