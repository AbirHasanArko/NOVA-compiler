#ifndef NOVA_AST_AST_NODE_HPP
#define NOVA_AST_AST_NODE_HPP

#include "ast/ast_visitor.hpp"
#include "lexer/token.hpp"
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace nova {

enum class BaseType {
    I32,
    F32,
    BOOL,
    CHAR,
    STR,
    VOID
};

/**
 * Represents a static NOVA type annotation, supporting primitives and fixed-size arrays T[N].
 */
struct TypeSpec {
    BaseType base = BaseType::VOID;
    bool is_array = false;
    int array_size = 0;

    static const char* base_to_string(BaseType b) {
        switch (b) {
            case BaseType::I32:  return "i32";
            case BaseType::F32:  return "f32";
            case BaseType::BOOL: return "bool";
            case BaseType::CHAR: return "char";
            case BaseType::STR:  return "str";
            case BaseType::VOID: return "void";
        }
        return "unknown";
    }

    std::string to_string() const {
        std::string s = base_to_string(base);
        if (is_array) {
            s += "[" + std::to_string(array_size) + "]";
        }
        return s;
    }
};

/**
 * Base class for all NOVA Abstract Syntax Tree nodes.
 */
class ASTNode {
public:
    explicit ASTNode(SourceLocation loc) : location(std::move(loc)) {}
    virtual ~ASTNode() = default;

    ASTNode(const ASTNode&) = delete;
    ASTNode& operator=(const ASTNode&) = delete;

    virtual void accept(ASTVisitor& visitor) = 0;

    SourceLocation location;
};

class ExprNode : public ASTNode {
public:
    using ASTNode::ASTNode;
};

class StmtNode : public ASTNode {
public:
    using ASTNode::ASTNode;
};

using ExprPtr = std::unique_ptr<ExprNode>;
using StmtPtr = std::unique_ptr<StmtNode>;

// ============================================================================
// Expression Nodes
// ============================================================================

class LiteralNode : public ExprNode {
public:
    enum class Kind { INT, FLOAT, BOOL, CHAR, STR };

    LiteralNode(Kind k, std::string val, SourceLocation loc)
        : ExprNode(std::move(loc)), kind(k), value(std::move(val)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    static const char* kind_to_string(Kind k) {
        switch (k) {
            case Kind::INT:   return "INT";
            case Kind::FLOAT: return "FLOAT";
            case Kind::BOOL:  return "BOOL";
            case Kind::CHAR:  return "CHAR";
            case Kind::STR:   return "STR";
        }
        return "UNKNOWN";
    }

    Kind kind;
    std::string value;
};

class IdentifierNode : public ExprNode {
public:
    IdentifierNode(std::string n, SourceLocation loc)
        : ExprNode(std::move(loc)), name(std::move(n)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    std::string name;
};

class ArrayLiteralNode : public ExprNode {
public:
    ArrayLiteralNode(std::vector<ExprPtr> elems, SourceLocation loc)
        : ExprNode(std::move(loc)), elements(std::move(elems)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    std::vector<ExprPtr> elements;
};

class IndexAccessNode : public ExprNode {
public:
    IndexAccessNode(ExprPtr arr, ExprPtr idx, SourceLocation loc)
        : ExprNode(std::move(loc)), array(std::move(arr)), index(std::move(idx)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    ExprPtr array;
    ExprPtr index;
};

class RoutineCallNode : public ExprNode {
public:
    RoutineCallNode(std::string name, std::vector<ExprPtr> arguments, SourceLocation loc)
        : ExprNode(std::move(loc)), callee(std::move(name)), args(std::move(arguments)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    std::string callee;
    std::vector<ExprPtr> args;
};

class UnaryExprNode : public ExprNode {
public:
    enum class Op { NEG, NOT };

    UnaryExprNode(Op o, ExprPtr expr, SourceLocation loc)
        : ExprNode(std::move(loc)), op(o), operand(std::move(expr)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    static const char* op_to_string(Op o) {
        switch (o) {
            case Op::NEG: return "-";
            case Op::NOT: return "not";
        }
        return "?";
    }

    Op op;
    ExprPtr operand;
};

class BinaryExprNode : public ExprNode {
public:
    enum class Op {
        POW, MUL, DIV, MOD, ADD, SUB,
        LT, LE, GT, GE, EQ, NEQ,
        AND, OR
    };

    BinaryExprNode(Op o, ExprPtr lhs, ExprPtr rhs, SourceLocation loc)
        : ExprNode(std::move(loc)), op(o), left(std::move(lhs)), right(std::move(rhs)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    static const char* op_to_string(Op o) {
        switch (o) {
            case Op::POW: return "**";
            case Op::MUL: return "*";
            case Op::DIV: return "/";
            case Op::MOD: return "%";
            case Op::ADD: return "+";
            case Op::SUB: return "-";
            case Op::LT:  return "<";
            case Op::LE:  return "<=";
            case Op::GT:  return ">";
            case Op::GE:  return ">=";
            case Op::EQ:  return "==";
            case Op::NEQ: return "!=";
            case Op::AND: return "and";
            case Op::OR:  return "or";
        }
        return "?";
    }

    Op op;
    ExprPtr left;
    ExprPtr right;
};

// ============================================================================
// Statement Nodes
// ============================================================================

class ConstDeclNode : public StmtNode {
public:
    ConstDeclNode(std::string n, TypeSpec t, ExprPtr init, SourceLocation loc)
        : StmtNode(std::move(loc)), name(std::move(n)), type(t), initializer(std::move(init)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    std::string name;
    TypeSpec type;
    ExprPtr initializer;
};

class FluxDeclNode : public StmtNode {
public:
    FluxDeclNode(std::string n, TypeSpec t, ExprPtr init, SourceLocation loc)
        : StmtNode(std::move(loc)), name(std::move(n)), type(t), initializer(std::move(init)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    std::string name;
    TypeSpec type;
    ExprPtr initializer; // nullptr if omitted
};

class AssignNode : public StmtNode {
public:
    AssignNode(ExprPtr tgt, ExprPtr val, SourceLocation loc)
        : StmtNode(std::move(loc)), target(std::move(tgt)), value(std::move(val)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    ExprPtr target;
    ExprPtr value;
};

class ExprStmtNode : public StmtNode {
public:
    ExprStmtNode(ExprPtr e, SourceLocation loc)
        : StmtNode(std::move(loc)), expr(std::move(e)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    ExprPtr expr;
};

class DetectNode : public StmtNode {
public:
    struct Branch {
        ExprPtr condition;
        std::vector<StmtPtr> body;
        SourceLocation location;
    };

    DetectNode(std::vector<Branch> br, std::vector<StmtPtr> fb, bool has_fb, SourceLocation loc)
        : StmtNode(std::move(loc)), branches(std::move(br)), fallback_body(std::move(fb)), has_fallback(has_fb) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    std::vector<Branch> branches;
    std::vector<StmtPtr> fallback_body;
    bool has_fallback = false;
};

class CruiseNode : public StmtNode {
public:
    CruiseNode(ExprPtr cond, std::vector<StmtPtr> b, SourceLocation loc)
        : StmtNode(std::move(loc)), condition(std::move(cond)), body(std::move(b)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    ExprPtr condition;
    std::vector<StmtPtr> body;
};

class OrbitNode : public StmtNode {
public:
    OrbitNode(std::string var, ExprPtr s, ExprPtr e, ExprPtr st, std::vector<StmtPtr> b, SourceLocation loc)
        : StmtNode(std::move(loc)),
          loop_var(std::move(var)),
          start(std::move(s)),
          end(std::move(e)),
          step(std::move(st)),
          body(std::move(b)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    std::string loop_var;
    ExprPtr start;
    ExprPtr end;
    ExprPtr step; // nullptr if step omitted
    std::vector<StmtPtr> body;
};

class RouteNode : public StmtNode {
public:
    struct Case {
        std::unique_ptr<LiteralNode> value;
        std::vector<StmtPtr> body;
        SourceLocation location;
    };

    RouteNode(ExprPtr sel, std::vector<Case> c, std::vector<StmtPtr> def_body, bool has_def, SourceLocation loc)
        : StmtNode(std::move(loc)),
          selector(std::move(sel)),
          cases(std::move(c)),
          default_body(std::move(def_body)),
          has_default(has_def) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    ExprPtr selector;
    std::vector<Case> cases;
    std::vector<StmtPtr> default_body;
    bool has_default = false;
};

class TransmitNode : public StmtNode {
public:
    TransmitNode(std::vector<ExprPtr> arguments, SourceLocation loc)
        : StmtNode(std::move(loc)), args(std::move(arguments)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    std::vector<ExprPtr> args;
};

class ReceiveNode : public StmtNode {
public:
    ReceiveNode(std::string tgt, SourceLocation loc)
        : StmtNode(std::move(loc)), target(std::move(tgt)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    std::string target;
};

class VerifyNode : public StmtNode {
public:
    VerifyNode(ExprPtr cond, SourceLocation loc)
        : StmtNode(std::move(loc)), condition(std::move(cond)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    ExprPtr condition;
};

class RetNode : public StmtNode {
public:
    RetNode(ExprPtr val, SourceLocation loc)
        : StmtNode(std::move(loc)), value(std::move(val)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    ExprPtr value; // nullptr for bare ret;
};

class HaltNode : public StmtNode {
public:
    explicit HaltNode(SourceLocation loc) : StmtNode(std::move(loc)) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class SkipNode : public StmtNode {
public:
    explicit SkipNode(SourceLocation loc) : StmtNode(std::move(loc)) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

// ============================================================================
// Program & Routine Nodes
// ============================================================================

class RequireNode : public ASTNode {
public:
    RequireNode(ExprPtr cond, SourceLocation loc)
        : ASTNode(std::move(loc)), condition(std::move(cond)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    ExprPtr condition;
};

struct ParamDecl {
    std::string name;
    TypeSpec type;
    SourceLocation location;
};

class RoutineDeclNode : public ASTNode {
public:
    RoutineDeclNode(std::string n,
                    std::vector<ParamDecl> p,
                    TypeSpec ret,
                    std::vector<std::unique_ptr<RequireNode>> reqs,
                    std::vector<StmtPtr> b,
                    SourceLocation loc)
        : ASTNode(std::move(loc)),
          name(std::move(n)),
          params(std::move(p)),
          return_type(ret),
          requires(std::move(reqs)),
          body(std::move(b)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    std::string name;
    std::vector<ParamDecl> params;
    TypeSpec return_type;
    std::vector<std::unique_ptr<RequireNode>> requires;
    std::vector<StmtPtr> body;
};

class ProgramNode : public ASTNode {
public:
    ProgramNode(std::vector<std::unique_ptr<RoutineDeclNode>> r,
                std::vector<StmtPtr> ignite_stmts,
                SourceLocation ignite_loc,
                SourceLocation prog_loc)
        : ASTNode(std::move(prog_loc)),
          routines(std::move(r)),
          ignite_body(std::move(ignite_stmts)),
          ignite_location(std::move(ignite_loc)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    std::vector<std::unique_ptr<RoutineDeclNode>> routines;
    std::vector<StmtPtr> ignite_body;
    SourceLocation ignite_location;
};

} // namespace nova

#endif // NOVA_AST_AST_NODE_HPP
