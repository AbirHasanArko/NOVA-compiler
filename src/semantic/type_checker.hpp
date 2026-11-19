#ifndef NOVA_SEMANTIC_TYPE_CHECKER_HPP
#define NOVA_SEMANTIC_TYPE_CHECKER_HPP

#include "ast/ast_node.hpp"
#include "ast/ast_visitor.hpp"
#include "diagnostics/diagnostic.hpp"
#include "semantic/symbol.hpp"
#include <string>
#include <vector>

namespace nova {

/**
 * Standalone AST Visitor pass that enforces NOVA's strict static type rules:
 * - No implicit promotions or coercions between i32, f32, bool, char, str, void, T[N]
 * - Arithmetic (+, -, *, /, **) on matching numeric primitives (i32 or f32)
 * - Modulo (%) strictly on i32
 * - Relational (<, <=, >, >=) on matching numeric primitives -> bool
 * - Equality (==, !=) on matching non-void types -> bool
 * - Logical (and, or, not) strictly on bool -> bool
 * - Homogeneous array literals [e0, ..., eN-1], non-zero array capacity, i32 indexing,
 *   compile-time constant bounds verification, and mutability checks
 * - Control flow conditions (detect, redetect, cruise), orbit i32 bounds/step,
 *   route selector & case literal matching, contract bool conditions, and routine
 *   call argument & return type checking.
 */
class TypeChecker : public ASTVisitor {
public:
    explicit TypeChecker(DiagnosticEngine& diagnostics);

    bool check(ProgramNode& root);

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
    DiagnosticEngine& m_diagnostics;
    SymbolTable m_symtab;
    TypeSpec m_last_type = TypeSpec::unknown();

    bool m_in_routine = false;
    int m_loop_depth = 0;
    std::string m_current_routine_name;
    TypeSpec m_current_return_type = TypeSpec::make_primitive(BaseType::VOID);

    TypeSpec evaluate(ExprNode& expr);
    void visit_stmt_list(const std::vector<StmtPtr>& stmts);
    bool validate_declared_type(const SourceLocation& loc, const std::string& name, const TypeSpec& type);
    static bool try_eval_const_i32(const ExprNode* expr, long long& out_val);
    static bool stmt_always_returns(const StmtNode* stmt);
    static bool stmt_list_always_returns(const std::vector<StmtPtr>& stmts);
};

} // namespace nova

#endif // NOVA_SEMANTIC_TYPE_CHECKER_HPP
