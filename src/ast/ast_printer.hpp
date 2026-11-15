#ifndef NOVA_AST_AST_PRINTER_HPP
#define NOVA_AST_AST_PRINTER_HPP

#include "ast/ast_node.hpp"
#include "ast/ast_visitor.hpp"
#include <iostream>
#include <vector>

namespace nova {

/**
 * Formats and prints a NOVA AST in deterministic indented tree form for '--ast'.
 */
class ASTPrinter : public ASTVisitor {
public:
    explicit ASTPrinter(std::ostream& out = std::cout) : m_out(out) {}

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
    std::ostream& m_out;
    int m_indent = 0;

    void print_indent() const;
    void print_stmt_list(const std::vector<StmtPtr>& stmts);
};

} // namespace nova

#endif // NOVA_AST_AST_PRINTER_HPP
