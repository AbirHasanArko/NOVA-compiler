#ifndef NOVA_SEMANTIC_SYMBOL_RESOLVER_HPP
#define NOVA_SEMANTIC_SYMBOL_RESOLVER_HPP

#include "ast/ast_node.hpp"
#include "ast/ast_visitor.hpp"
#include "diagnostics/diagnostic.hpp"
#include "semantic/symbol.hpp"
#include <vector>

namespace nova {

/**
 * AST Visitor pass that builds the lexical scope tree, enforces unique
 * per-scope declarations, checks identifier/routine visibility, and guards
 * immutable (`const` and `orbit` loop index) bindings against mutation.
 */
class SymbolResolver : public ASTVisitor {
public:
    explicit SymbolResolver(DiagnosticEngine& diagnostics);

    bool resolve(ProgramNode& root);

    const SymbolTable& symbol_table() const { return m_symtab; }
    SymbolTable& symbol_table() { return m_symtab; }

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

    void declare_or_report(Symbol sym);
    void check_mutable_storage(const SourceLocation& loc, const std::string& name);
    void visit_stmt_list(const std::vector<StmtPtr>& stmts);
    void visit_indexed_target(IndexAccessNode& idx_node);
};

} // namespace nova

#endif // NOVA_SEMANTIC_SYMBOL_RESOLVER_HPP
