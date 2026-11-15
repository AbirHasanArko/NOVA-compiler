#include "ast/ast_printer.hpp"

namespace nova {

void ASTPrinter::print_indent() const {
    for (int i = 0; i < m_indent; ++i) {
        m_out << "  ";
    }
}

void ASTPrinter::print_stmt_list(const std::vector<StmtPtr>& stmts) {
    m_indent++;
    for (const auto& stmt : stmts) {
        if (stmt) {
            stmt->accept(*this);
        }
    }
    m_indent--;
}

void ASTPrinter::visit(ProgramNode& node) {
    print_indent();
    m_out << "Program\n";
    m_indent++;

    for (const auto& routine : node.routines) {
        if (routine) {
            routine->accept(*this);
        }
    }

    print_indent();
    m_out << "IgniteBlock\n";
    print_stmt_list(node.ignite_body);

    m_indent--;
}

void ASTPrinter::visit(RoutineDeclNode& node) {
    print_indent();
    m_out << "RoutineDecl " << node.name << "(";
    for (size_t i = 0; i < node.params.size(); ++i) {
        if (i > 0) m_out << ", ";
        m_out << node.params[i].name << ": " << node.params[i].type.to_string();
    }
    m_out << ") -> " << node.return_type.to_string() << "\n";

    m_indent++;
    for (const auto& req : node.requires) {
        if (req) {
            req->accept(*this);
        }
    }
    print_indent();
    m_out << "Body\n";
    print_stmt_list(node.body);
    m_indent--;
}

void ASTPrinter::visit(RequireNode& node) {
    print_indent();
    m_out << "Require\n";
    m_indent++;
    if (node.condition) node.condition->accept(*this);
    m_indent--;
}

void ASTPrinter::visit(ConstDeclNode& node) {
    print_indent();
    m_out << "ConstDecl " << node.name << ": " << node.type.to_string() << "\n";
    m_indent++;
    if (node.initializer) node.initializer->accept(*this);
    m_indent--;
}

void ASTPrinter::visit(FluxDeclNode& node) {
    print_indent();
    m_out << "FluxDecl " << node.name << ": " << node.type.to_string() << "\n";
    if (node.initializer) {
        m_indent++;
        node.initializer->accept(*this);
        m_indent--;
    }
}

void ASTPrinter::visit(AssignNode& node) {
    print_indent();
    m_out << "Assign\n";
    m_indent++;
    if (node.target) node.target->accept(*this);
    if (node.value) node.value->accept(*this);
    m_indent--;
}

void ASTPrinter::visit(ExprStmtNode& node) {
    print_indent();
    m_out << "ExprStmt\n";
    m_indent++;
    if (node.expr) node.expr->accept(*this);
    m_indent--;
}

void ASTPrinter::visit(DetectNode& node) {
    for (size_t i = 0; i < node.branches.size(); ++i) {
        print_indent();
        m_out << (i == 0 ? "Detect" : "Redetect") << "\n";
        m_indent++;
        if (node.branches[i].condition) {
            node.branches[i].condition->accept(*this);
        }
        print_indent();
        m_out << "Then\n";
        print_stmt_list(node.branches[i].body);
        m_indent--;
    }

    if (node.has_fallback) {
        print_indent();
        m_out << "Fallback\n";
        print_stmt_list(node.fallback_body);
    }
}

void ASTPrinter::visit(CruiseNode& node) {
    print_indent();
    m_out << "Cruise\n";
    m_indent++;
    if (node.condition) node.condition->accept(*this);
    print_indent();
    m_out << "Body\n";
    print_stmt_list(node.body);
    m_indent--;
}

void ASTPrinter::visit(OrbitNode& node) {
    print_indent();
    m_out << "Orbit " << node.loop_var << "\n";
    m_indent++;
    print_indent();
    m_out << "Range\n";
    m_indent++;
    if (node.start) node.start->accept(*this);
    if (node.end) node.end->accept(*this);
    m_indent--;
    if (node.step) {
        print_indent();
        m_out << "Step\n";
        m_indent++;
        node.step->accept(*this);
        m_indent--;
    }
    print_indent();
    m_out << "Body\n";
    print_stmt_list(node.body);
    m_indent--;
}

void ASTPrinter::visit(RouteNode& node) {
    print_indent();
    m_out << "Route\n";
    m_indent++;
    if (node.selector) node.selector->accept(*this);
    for (const auto& c : node.cases) {
        print_indent();
        m_out << "Case\n";
        m_indent++;
        if (c.value) c.value->accept(*this);
        print_stmt_list(c.body);
        m_indent--;
    }
    if (node.has_default) {
        print_indent();
        m_out << "Default\n";
        print_stmt_list(node.default_body);
    }
    m_indent--;
}

void ASTPrinter::visit(TransmitNode& node) {
    print_indent();
    m_out << "Transmit\n";
    m_indent++;
    for (const auto& arg : node.args) {
        if (arg) arg->accept(*this);
    }
    m_indent--;
}

void ASTPrinter::visit(ReceiveNode& node) {
    print_indent();
    m_out << "Receive " << node.target << "\n";
}

void ASTPrinter::visit(VerifyNode& node) {
    print_indent();
    m_out << "Verify\n";
    m_indent++;
    if (node.condition) node.condition->accept(*this);
    m_indent--;
}

void ASTPrinter::visit(RetNode& node) {
    print_indent();
    m_out << "Ret\n";
    if (node.value) {
        m_indent++;
        node.value->accept(*this);
        m_indent--;
    }
}

void ASTPrinter::visit(HaltNode& /*node*/) {
    print_indent();
    m_out << "Halt\n";
}

void ASTPrinter::visit(SkipNode& /*node*/) {
    print_indent();
    m_out << "Skip\n";
}

void ASTPrinter::visit(LiteralNode& node) {
    print_indent();
    m_out << "Literal (" << LiteralNode::kind_to_string(node.kind) << ") " << node.value << "\n";
}

void ASTPrinter::visit(IdentifierNode& node) {
    print_indent();
    m_out << "Identifier " << node.name << "\n";
}

void ASTPrinter::visit(ArrayLiteralNode& node) {
    print_indent();
    m_out << "ArrayLiteral\n";
    m_indent++;
    for (const auto& elem : node.elements) {
        if (elem) elem->accept(*this);
    }
    m_indent--;
}

void ASTPrinter::visit(IndexAccessNode& node) {
    print_indent();
    m_out << "IndexAccess\n";
    m_indent++;
    if (node.array) node.array->accept(*this);
    if (node.index) node.index->accept(*this);
    m_indent--;
}

void ASTPrinter::visit(RoutineCallNode& node) {
    print_indent();
    m_out << "RoutineCall " << node.callee << "\n";
    m_indent++;
    for (const auto& arg : node.args) {
        if (arg) arg->accept(*this);
    }
    m_indent--;
}

void ASTPrinter::visit(UnaryExprNode& node) {
    print_indent();
    m_out << "UnaryExpr (" << UnaryExprNode::op_to_string(node.op) << ")\n";
    m_indent++;
    if (node.operand) node.operand->accept(*this);
    m_indent--;
}

void ASTPrinter::visit(BinaryExprNode& node) {
    print_indent();
    m_out << "BinaryExpr (" << BinaryExprNode::op_to_string(node.op) << ")\n";
    m_indent++;
    if (node.left) node.left->accept(*this);
    if (node.right) node.right->accept(*this);
    m_indent--;
}

} // namespace nova
