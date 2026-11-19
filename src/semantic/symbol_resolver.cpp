#include "semantic/symbol_resolver.hpp"
#include <sstream>

namespace nova {

SymbolResolver::SymbolResolver(DiagnosticEngine& diagnostics)
    : m_diagnostics(diagnostics) {}

bool SymbolResolver::resolve(ProgramNode& root) {
    m_symtab.reset();
    size_t errors_before = m_diagnostics.error_count();
    root.accept(*this);
    return m_diagnostics.error_count() == errors_before;
}

void SymbolResolver::declare_or_report(Symbol sym) {
    if (const Symbol* prev = m_symtab.lookup_local(sym.name)) {
        std::ostringstream oss;
        oss << "redeclaration of '" << sym.name
            << "' in the same scope (previously declared at "
            << prev->location.line << ":" << prev->location.column << ")";
        m_diagnostics.report_error(sym.location, oss.str());
        return;
    }
    m_symtab.declare(std::move(sym));
}

void SymbolResolver::check_mutable_storage(const SourceLocation& loc, const std::string& name) {
    const Symbol* sym = m_symtab.lookup(name);
    if (!sym) {
        m_diagnostics.report_error(loc, "undeclared identifier '" + name + "'");
    } else if (sym->kind == SymbolKind::ROUTINE) {
        m_diagnostics.report_error(loc, "cannot assign to routine '" + name + "'");
    } else if (sym->mutability == Mutability::CONST) {
        m_diagnostics.report_error(loc, "cannot assign to immutable constant '" + name + "'");
    }
}

void SymbolResolver::visit_stmt_list(const std::vector<StmtPtr>& stmts) {
    for (const auto& stmt : stmts) {
        if (stmt) {
            stmt->accept(*this);
        }
    }
}

void SymbolResolver::visit(ProgramNode& node) {
    // Pass 1: register top-level routine signatures so routines may reference one another.
    for (const auto& routine : node.routines) {
        if (!routine) continue;
        std::vector<TypeSpec> ptypes;
        ptypes.reserve(routine->params.size());
        for (const auto& p : routine->params) {
            ptypes.push_back(p.type);
        }
        Symbol rsym;
        rsym.name = routine->name;
        rsym.type = routine->return_type;
        rsym.mutability = Mutability::CONST;
        rsym.kind = SymbolKind::ROUTINE;
        rsym.location = routine->location;
        rsym.param_types = std::move(ptypes);
        declare_or_report(std::move(rsym));
    }

    // Pass 2: resolve routine bodies and preconditions.
    for (const auto& routine : node.routines) {
        if (routine) {
            routine->accept(*this);
        }
    }

    // Pass 3: resolve the root ignite execution capsule.
    m_symtab.enter_scope("ignite", true);
    visit_stmt_list(node.ignite_body);
    m_symtab.exit_scope();
}

void SymbolResolver::visit(RoutineDeclNode& node) {
    m_symtab.enter_scope("routine:" + node.name, true);

    for (const auto& param : node.params) {
        Symbol psym;
        psym.name = param.name;
        psym.type = param.type;
        psym.mutability = Mutability::FLUX;
        psym.kind = SymbolKind::PARAMETER;
        psym.location = param.location;
        declare_or_report(std::move(psym));
    }

    for (const auto& req : node.requires) {
        if (req) {
            req->accept(*this);
        }
    }

    visit_stmt_list(node.body);
    m_symtab.exit_scope();
}

void SymbolResolver::visit(RequireNode& node) {
    if (node.condition) {
        node.condition->accept(*this);
    }
}

void SymbolResolver::visit(ConstDeclNode& node) {
    // Evaluate initializer before introducing binding so RHS cannot self-reference.
    if (node.initializer) {
        node.initializer->accept(*this);
    }
    Symbol sym;
    sym.name = node.name;
    sym.type = node.type;
    sym.mutability = Mutability::CONST;
    sym.kind = SymbolKind::VARIABLE;
    sym.location = node.location;
    declare_or_report(std::move(sym));
}

void SymbolResolver::visit(FluxDeclNode& node) {
    if (node.initializer) {
        node.initializer->accept(*this);
    }
    Symbol sym;
    sym.name = node.name;
    sym.type = node.type;
    sym.mutability = Mutability::FLUX;
    sym.kind = SymbolKind::VARIABLE;
    sym.location = node.location;
    declare_or_report(std::move(sym));
}

void SymbolResolver::visit_indexed_target(IndexAccessNode& idx_node) {
    if (idx_node.index) {
        idx_node.index->accept(*this);
    }

    ExprNode* base = idx_node.array.get();
    while (auto* nested = dynamic_cast<IndexAccessNode*>(base)) {
        if (nested->index) {
            nested->index->accept(*this);
        }
        base = nested->array.get();
    }

    if (auto* id = dynamic_cast<IdentifierNode*>(base)) {
        check_mutable_storage(id->location, id->name);
        return;
    }

    if (base) {
        base->accept(*this);
    }
    m_diagnostics.report_error(
        idx_node.location,
        "assignment target must be a mutable variable or array element"
    );
}

void SymbolResolver::visit(AssignNode& node) {
    if (node.value) {
        node.value->accept(*this);
    }
    if (!node.target) {
        return;
    }

    if (auto* id = dynamic_cast<IdentifierNode*>(node.target.get())) {
        check_mutable_storage(id->location, id->name);
        return;
    }

    if (auto* idx = dynamic_cast<IndexAccessNode*>(node.target.get())) {
        visit_indexed_target(*idx);
        return;
    }

    node.target->accept(*this);
    m_diagnostics.report_error(node.location, "invalid assignment target");
}

void SymbolResolver::visit(ExprStmtNode& node) {
    if (node.expr) {
        node.expr->accept(*this);
    }
}

void SymbolResolver::visit(DetectNode& node) {
    for (size_t i = 0; i < node.branches.size(); ++i) {
        if (node.branches[i].condition) {
            node.branches[i].condition->accept(*this);
        }
        m_symtab.enter_scope(i == 0 ? "detect" : "redetect");
        visit_stmt_list(node.branches[i].body);
        m_symtab.exit_scope();
    }
    if (node.has_fallback) {
        m_symtab.enter_scope("fallback");
        visit_stmt_list(node.fallback_body);
        m_symtab.exit_scope();
    }
}

void SymbolResolver::visit(CruiseNode& node) {
    if (node.condition) {
        node.condition->accept(*this);
    }
    m_symtab.enter_scope("cruise");
    visit_stmt_list(node.body);
    m_symtab.exit_scope();
}

void SymbolResolver::visit(OrbitNode& node) {
    if (node.start) node.start->accept(*this);
    if (node.end)   node.end->accept(*this);
    if (node.step)  node.step->accept(*this);

    m_symtab.enter_scope("orbit");
    Symbol loop_sym;
    loop_sym.name = node.loop_var;
    loop_sym.type = TypeSpec{BaseType::I32, false, 0};
    loop_sym.mutability = Mutability::CONST;
    loop_sym.kind = SymbolKind::ORBIT_INDEX;
    loop_sym.location = node.location;
    declare_or_report(std::move(loop_sym));

    visit_stmt_list(node.body);
    m_symtab.exit_scope();
}

void SymbolResolver::visit(RouteNode& node) {
    if (node.selector) {
        node.selector->accept(*this);
    }
    for (const auto& c : node.cases) {
        m_symtab.enter_scope("case");
        visit_stmt_list(c.body);
        m_symtab.exit_scope();
    }
    if (node.has_default) {
        m_symtab.enter_scope("default");
        visit_stmt_list(node.default_body);
        m_symtab.exit_scope();
    }
}

void SymbolResolver::visit(TransmitNode& node) {
    for (const auto& arg : node.args) {
        if (arg) {
            arg->accept(*this);
        }
    }
}

void SymbolResolver::visit(ReceiveNode& node) {
    check_mutable_storage(node.location, node.target);
}

void SymbolResolver::visit(VerifyNode& node) {
    if (node.condition) {
        node.condition->accept(*this);
    }
}

void SymbolResolver::visit(RetNode& node) {
    if (node.value) {
        node.value->accept(*this);
    }
}

void SymbolResolver::visit(HaltNode&) {}
void SymbolResolver::visit(SkipNode&) {}
void SymbolResolver::visit(LiteralNode&) {}

void SymbolResolver::visit(IdentifierNode& node) {
    const Symbol* sym = m_symtab.lookup(node.name);
    if (!sym) {
        m_diagnostics.report_error(node.location, "undeclared identifier '" + node.name + "'");
    } else if (sym->kind == SymbolKind::ROUTINE) {
        m_diagnostics.report_error(
            node.location,
            "routine '" + node.name + "' must be called rather than used as a variable"
        );
    }
}

void SymbolResolver::visit(ArrayLiteralNode& node) {
    for (const auto& elem : node.elements) {
        if (elem) {
            elem->accept(*this);
        }
    }
}

void SymbolResolver::visit(IndexAccessNode& node) {
    if (node.array) node.array->accept(*this);
    if (node.index) node.index->accept(*this);
}

void SymbolResolver::visit(RoutineCallNode& node) {
    const Symbol* sym = m_symtab.lookup(node.callee);
    if (!sym) {
        m_diagnostics.report_error(node.location, "undeclared routine '" + node.callee + "'");
    } else if (sym->kind != SymbolKind::ROUTINE) {
        m_diagnostics.report_error(node.location, "'" + node.callee + "' is not a routine");
    }
    for (const auto& arg : node.args) {
        if (arg) {
            arg->accept(*this);
        }
    }
}

void SymbolResolver::visit(UnaryExprNode& node) {
    if (node.operand) {
        node.operand->accept(*this);
    }
}

void SymbolResolver::visit(BinaryExprNode& node) {
    if (node.left)  node.left->accept(*this);
    if (node.right) node.right->accept(*this);
}

} // namespace nova
