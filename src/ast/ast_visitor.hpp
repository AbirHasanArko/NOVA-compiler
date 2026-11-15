#ifndef NOVA_AST_AST_VISITOR_HPP
#define NOVA_AST_AST_VISITOR_HPP

namespace nova {

class ProgramNode;
class RoutineDeclNode;
class RequireNode;
class ConstDeclNode;
class FluxDeclNode;
class AssignNode;
class ExprStmtNode;
class DetectNode;
class CruiseNode;
class OrbitNode;
class RouteNode;
class TransmitNode;
class ReceiveNode;
class VerifyNode;
class RetNode;
class HaltNode;
class SkipNode;
class LiteralNode;
class IdentifierNode;
class ArrayLiteralNode;
class IndexAccessNode;
class RoutineCallNode;
class UnaryExprNode;
class BinaryExprNode;

/**
 * Abstract visitor interface for traversing the NOVA AST without coupling
 * tree representation to printing, semantic analysis, or execution passes.
 */
class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;

    virtual void visit(ProgramNode& node) = 0;
    virtual void visit(RoutineDeclNode& node) = 0;
    virtual void visit(RequireNode& node) = 0;

    virtual void visit(ConstDeclNode& node) = 0;
    virtual void visit(FluxDeclNode& node) = 0;
    virtual void visit(AssignNode& node) = 0;
    virtual void visit(ExprStmtNode& node) = 0;
    virtual void visit(DetectNode& node) = 0;
    virtual void visit(CruiseNode& node) = 0;
    virtual void visit(OrbitNode& node) = 0;
    virtual void visit(RouteNode& node) = 0;
    virtual void visit(TransmitNode& node) = 0;
    virtual void visit(ReceiveNode& node) = 0;
    virtual void visit(VerifyNode& node) = 0;
    virtual void visit(RetNode& node) = 0;
    virtual void visit(HaltNode& node) = 0;
    virtual void visit(SkipNode& node) = 0;

    virtual void visit(LiteralNode& node) = 0;
    virtual void visit(IdentifierNode& node) = 0;
    virtual void visit(ArrayLiteralNode& node) = 0;
    virtual void visit(IndexAccessNode& node) = 0;
    virtual void visit(RoutineCallNode& node) = 0;
    virtual void visit(UnaryExprNode& node) = 0;
    virtual void visit(BinaryExprNode& node) = 0;
};

} // namespace nova

#endif // NOVA_AST_AST_VISITOR_HPP
