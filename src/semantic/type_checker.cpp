#include "semantic/type_checker.hpp"
#include <sstream>
#include <stdexcept>

namespace nova {

TypeChecker::TypeChecker(DiagnosticEngine& diagnostics)
    : m_diagnostics(diagnostics) {}

bool TypeChecker::check(ProgramNode& root) {
    m_symtab.reset();
    m_in_routine = false;
    m_loop_depth = 0;
    m_current_routine_name.clear();
    m_current_return_type = TypeSpec::make_primitive(BaseType::VOID);

    size_t errors_before = m_diagnostics.error_count();
    root.accept(*this);
    return m_diagnostics.error_count() == errors_before;
}

TypeSpec TypeChecker::evaluate(ExprNode& expr) {
    m_last_type = TypeSpec::unknown();
    expr.accept(*this);
    expr.inferred_type = m_last_type;
    return m_last_type;
}

void TypeChecker::visit_stmt_list(const std::vector<StmtPtr>& stmts) {
    for (const auto& stmt : stmts) {
        if (stmt) {
            stmt->accept(*this);
        }
    }
}

bool TypeChecker::validate_declared_type(
    const SourceLocation& loc,
    const std::string& name,
    const TypeSpec& type
) {
    if (type.base == BaseType::VOID) {
        m_diagnostics.report_error(loc, "cannot declare '" + name + "' with type 'void'");
        return false;
    }
    if (type.is_array && type.array_size <= 0) {
        m_diagnostics.report_error(
            loc,
            "array size for '" + name + "' must be greater than 0, got " + std::to_string(type.array_size)
        );
        return false;
    }
    return true;
}

bool TypeChecker::try_eval_const_i32(const ExprNode* expr, long long& out_val) {
    if (!expr) {
        return false;
    }
    if (const auto* lit = dynamic_cast<const LiteralNode*>(expr)) {
        if (lit->kind != LiteralNode::Kind::INT) {
            return false;
        }
        try {
            out_val = std::stoll(lit->value);
            return true;
        } catch (const std::exception&) {
            return false;
        }
    }
    if (const auto* unary = dynamic_cast<const UnaryExprNode*>(expr)) {
        if (unary->op == UnaryExprNode::Op::NEG) {
            long long inner = 0;
            if (try_eval_const_i32(unary->operand.get(), inner)) {
                out_val = -inner;
                return true;
            }
        }
        return false;
    }
    if (const auto* bin = dynamic_cast<const BinaryExprNode*>(expr)) {
        long long lhs = 0;
        long long rhs = 0;
        if (!try_eval_const_i32(bin->left.get(), lhs) || !try_eval_const_i32(bin->right.get(), rhs)) {
            return false;
        }
        switch (bin->op) {
            case BinaryExprNode::Op::ADD:
                out_val = lhs + rhs;
                return true;
            case BinaryExprNode::Op::SUB:
                out_val = lhs - rhs;
                return true;
            case BinaryExprNode::Op::MUL:
                out_val = lhs * rhs;
                return true;
            case BinaryExprNode::Op::DIV:
                if (rhs == 0) return false;
                out_val = lhs / rhs;
                return true;
            case BinaryExprNode::Op::MOD:
                if (rhs == 0) return false;
                out_val = lhs % rhs;
                return true;
            default:
                return false;
        }
    }
    return false;
}

bool TypeChecker::stmt_list_always_returns(const std::vector<StmtPtr>& stmts) {
    for (const auto& stmt : stmts) {
        if (stmt && stmt_always_returns(stmt.get())) {
            return true;
        }
    }
    return false;
}

bool TypeChecker::stmt_always_returns(const StmtNode* stmt) {
    if (!stmt) {
        return false;
    }
    if (dynamic_cast<const RetNode*>(stmt) != nullptr) {
        return true;
    }
    if (const auto* det = dynamic_cast<const DetectNode*>(stmt)) {
        if (!det->has_fallback) {
            return false;
        }
        for (const auto& branch : det->branches) {
            if (!stmt_list_always_returns(branch.body)) {
                return false;
            }
        }
        return stmt_list_always_returns(det->fallback_body);
    }
    if (const auto* rt = dynamic_cast<const RouteNode*>(stmt)) {
        if (!rt->has_default) {
            return false;
        }
        for (const auto& c : rt->cases) {
            if (!stmt_list_always_returns(c.body)) {
                return false;
            }
        }
        return stmt_list_always_returns(rt->default_body);
    }
    return false;
}

void TypeChecker::visit(ProgramNode& node) {
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
        m_symtab.declare(std::move(rsym));
    }

    for (const auto& routine : node.routines) {
        if (routine) {
            routine->accept(*this);
        }
    }

    m_in_routine = false;
    m_loop_depth = 0;
    m_symtab.enter_scope("ignite", true);
    visit_stmt_list(node.ignite_body);
    m_symtab.exit_scope();
}

void TypeChecker::visit(RoutineDeclNode& node) {
    m_in_routine = true;
    int saved_loop_depth = m_loop_depth;
    m_loop_depth = 0;
    m_current_routine_name = node.name;
    m_current_return_type = node.return_type;

    if (node.return_type.is_array && node.return_type.array_size <= 0) {
        m_diagnostics.report_error(
            node.location,
            "return array size for routine '" + node.name + "' must be greater than 0"
        );
    }

    m_symtab.enter_scope("routine:" + node.name, true);
    for (const auto& param : node.params) {
        validate_declared_type(param.location, param.name, param.type);
        Symbol psym;
        psym.name = param.name;
        psym.type = param.type;
        psym.mutability = Mutability::FLUX;
        psym.kind = SymbolKind::PARAMETER;
        psym.location = param.location;
        m_symtab.declare(std::move(psym));
    }

    for (const auto& req : node.requires) {
        if (req) {
            req->accept(*this);
        }
    }

    visit_stmt_list(node.body);

    if (!node.return_type.is_void() && !node.return_type.is_unknown) {
        if (!stmt_list_always_returns(node.body)) {
            m_diagnostics.report_error(
                node.location,
                "non-void routine '" + node.name + "' does not return a value on all control paths"
            );
        }
    }

    m_symtab.exit_scope();
    m_loop_depth = saved_loop_depth;
    m_in_routine = false;
}

void TypeChecker::visit(RequireNode& node) {
    if (!node.condition) return;
    TypeSpec cond_type = evaluate(*node.condition);
    if (!cond_type.is_unknown && !cond_type.is_bool()) {
        m_diagnostics.report_error(
            node.location,
            "precondition in 'require' must be of type 'bool', got '" + cond_type.to_string() + "'"
        );
    }
}

void TypeChecker::visit(ConstDeclNode& node) {
    bool valid_decl = validate_declared_type(node.location, node.name, node.type);
    TypeSpec init_type = TypeSpec::unknown();
    if (node.initializer) {
        init_type = evaluate(*node.initializer);
    }
    if (valid_decl && !init_type.is_unknown && init_type != node.type) {
        m_diagnostics.report_error(
            node.location,
            "type mismatch in declaration of 'const " + node.name + "': expected '" +
                node.type.to_string() + "', got '" + init_type.to_string() + "'"
        );
    }

    Symbol sym;
    sym.name = node.name;
    sym.type = valid_decl ? node.type : TypeSpec::unknown();
    sym.mutability = Mutability::CONST;
    sym.kind = SymbolKind::VARIABLE;
    sym.location = node.location;
    m_symtab.declare(std::move(sym));
}

void TypeChecker::visit(FluxDeclNode& node) {
    bool valid_decl = validate_declared_type(node.location, node.name, node.type);
    if (node.initializer) {
        TypeSpec init_type = evaluate(*node.initializer);
        if (valid_decl && !init_type.is_unknown && init_type != node.type) {
            m_diagnostics.report_error(
                node.location,
                "type mismatch in declaration of 'flux " + node.name + "': expected '" +
                    node.type.to_string() + "', got '" + init_type.to_string() + "'"
            );
        }
    }

    Symbol sym;
    sym.name = node.name;
    sym.type = valid_decl ? node.type : TypeSpec::unknown();
    sym.mutability = Mutability::FLUX;
    sym.kind = SymbolKind::VARIABLE;
    sym.location = node.location;
    m_symtab.declare(std::move(sym));
}

void TypeChecker::visit(AssignNode& node) {
    TypeSpec rhs_type = node.value ? evaluate(*node.value) : TypeSpec::unknown();
    TypeSpec lhs_type = node.target ? evaluate(*node.target) : TypeSpec::unknown();

    if (!lhs_type.is_unknown && !rhs_type.is_unknown && lhs_type != rhs_type) {
        m_diagnostics.report_error(
            node.location,
            "type mismatch in assignment: cannot assign '" + rhs_type.to_string() +
                "' to '" + lhs_type.to_string() + "'"
        );
    }
}

void TypeChecker::visit(ExprStmtNode& node) {
    if (node.expr) {
        evaluate(*node.expr);
    }
}

void TypeChecker::visit(DetectNode& node) {
    for (size_t i = 0; i < node.branches.size(); ++i) {
        if (node.branches[i].condition) {
            TypeSpec cond_type = evaluate(*node.branches[i].condition);
            if (!cond_type.is_unknown && !cond_type.is_bool()) {
                std::string kw = (i == 0) ? "detect" : "redetect";
                m_diagnostics.report_error(
                    node.branches[i].location,
                    "condition in '" + kw + "' must be of type 'bool', got '" + cond_type.to_string() + "'"
                );
            }
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

void TypeChecker::visit(CruiseNode& node) {
    if (node.condition) {
        TypeSpec cond_type = evaluate(*node.condition);
        if (!cond_type.is_unknown && !cond_type.is_bool()) {
            m_diagnostics.report_error(
                node.location,
                "condition in 'cruise' must be of type 'bool', got '" + cond_type.to_string() + "'"
            );
        }
    }
    m_symtab.enter_scope("cruise");
    ++m_loop_depth;
    visit_stmt_list(node.body);
    --m_loop_depth;
    m_symtab.exit_scope();
}

void TypeChecker::visit(OrbitNode& node) {
    if (node.start) {
        TypeSpec start_type = evaluate(*node.start);
        if (!start_type.is_unknown && !start_type.is_i32()) {
            m_diagnostics.report_error(
                node.start->location,
                "orbit range start must be of type 'i32', got '" + start_type.to_string() + "'"
            );
        }
    }
    if (node.end) {
        TypeSpec end_type = evaluate(*node.end);
        if (!end_type.is_unknown && !end_type.is_i32()) {
            m_diagnostics.report_error(
                node.end->location,
                "orbit range end must be of type 'i32', got '" + end_type.to_string() + "'"
            );
        }
    }
    if (node.step) {
        TypeSpec step_type = evaluate(*node.step);
        if (!step_type.is_unknown && !step_type.is_i32()) {
            m_diagnostics.report_error(
                node.step->location,
                "orbit step must be of type 'i32', got '" + step_type.to_string() + "'"
            );
        } else if (step_type.is_i32()) {
            long long const_step = 1;
            if (try_eval_const_i32(node.step.get(), const_step) && const_step == 0) {
                m_diagnostics.report_error(node.step->location, "orbit step cannot be zero");
            }
        }
    }

    m_symtab.enter_scope("orbit");
    Symbol loop_sym;
    loop_sym.name = node.loop_var;
    loop_sym.type = TypeSpec::make_primitive(BaseType::I32);
    loop_sym.mutability = Mutability::CONST;
    loop_sym.kind = SymbolKind::ORBIT_INDEX;
    loop_sym.location = node.location;
    m_symtab.declare(std::move(loop_sym));

    ++m_loop_depth;
    visit_stmt_list(node.body);
    --m_loop_depth;
    m_symtab.exit_scope();
}

void TypeChecker::visit(RouteNode& node) {
    TypeSpec sel_type = node.selector ? evaluate(*node.selector) : TypeSpec::unknown();
    if (!sel_type.is_unknown) {
        bool valid_selector = !sel_type.is_array &&
            (sel_type.base == BaseType::I32 ||
             sel_type.base == BaseType::CHAR ||
             sel_type.base == BaseType::BOOL);
        if (!valid_selector) {
            m_diagnostics.report_error(
                node.location,
                "route selector must be of type 'i32', 'char', or 'bool', got '" + sel_type.to_string() + "'"
            );
            sel_type = TypeSpec::unknown();
        }
    }

    std::unordered_map<std::string, SourceLocation> seen_cases;
    for (const auto& c : node.cases) {
        if (c.value) {
            TypeSpec case_type = evaluate(*c.value);
            if (!sel_type.is_unknown && !case_type.is_unknown && case_type != sel_type) {
                m_diagnostics.report_error(
                    c.location,
                    "type mismatch in route case: selector is '" + sel_type.to_string() +
                        "', got '" + case_type.to_string() + "'"
                );
            }
            auto it = seen_cases.find(c.value->value);
            if (it != seen_cases.end()) {
                std::ostringstream oss;
                oss << "duplicate case value '" << c.value->value
                    << "' in 'route' statement (previously defined at "
                    << it->second.line << ":" << it->second.column << ")";
                m_diagnostics.report_error(c.location, oss.str());
            } else {
                seen_cases.emplace(c.value->value, c.location);
            }
        }
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

void TypeChecker::visit(TransmitNode& node) {
    for (const auto& arg : node.args) {
        if (!arg) continue;
        TypeSpec t = evaluate(*arg);
        if (!t.is_unknown && t.is_void()) {
            m_diagnostics.report_error(arg->location, "cannot transmit 'void' expression");
        }
    }
}

void TypeChecker::visit(ReceiveNode& node) {
    const Symbol* sym = m_symtab.lookup(node.target);
    if (sym && sym->kind != SymbolKind::ROUTINE && !sym->type.is_unknown) {
        if (sym->type.is_array || sym->type.is_void()) {
            m_diagnostics.report_error(
                node.location,
                "cannot receive input into non-primitive variable '" + node.target +
                    "' of type '" + sym->type.to_string() + "'"
            );
        }
    }
}

void TypeChecker::visit(VerifyNode& node) {
    if (!node.condition) return;
    TypeSpec cond_type = evaluate(*node.condition);
    if (!cond_type.is_unknown && !cond_type.is_bool()) {
        m_diagnostics.report_error(
            node.location,
            "condition in 'verify' must be of type 'bool', got '" + cond_type.to_string() + "'"
        );
    }
}

void TypeChecker::visit(RetNode& node) {
    TypeSpec val_type = node.value ? evaluate(*node.value) : TypeSpec::make_primitive(BaseType::VOID);
    if (!m_in_routine) {
        m_diagnostics.report_error(node.location, "'ret' statement is only allowed inside a routine");
        return;
    }

    if (m_current_return_type.is_void()) {
        if (node.value != nullptr) {
            m_diagnostics.report_error(
                node.location,
                "void routine '" + m_current_routine_name + "' cannot return a value"
            );
        }
        return;
    }

    if (node.value == nullptr) {
        m_diagnostics.report_error(
            node.location,
            "non-void routine '" + m_current_routine_name + "' must return a value of type '" +
                m_current_return_type.to_string() + "'"
        );
        return;
    }

    if (!val_type.is_unknown && val_type != m_current_return_type) {
        m_diagnostics.report_error(
            node.location,
            "return type mismatch in routine '" + m_current_routine_name + "': expected '" +
                m_current_return_type.to_string() + "', got '" + val_type.to_string() + "'"
        );
    }
}

void TypeChecker::visit(HaltNode& node) {
    if (m_loop_depth <= 0) {
        m_diagnostics.report_error(
            node.location,
            "'halt' statement is only allowed inside a 'cruise' or 'orbit' loop"
        );
    }
}

void TypeChecker::visit(SkipNode& node) {
    if (m_loop_depth <= 0) {
        m_diagnostics.report_error(
            node.location,
            "'skip' statement is only allowed inside a 'cruise' or 'orbit' loop"
        );
    }
}

void TypeChecker::visit(LiteralNode& node) {
    switch (node.kind) {
        case LiteralNode::Kind::INT:
            m_last_type = TypeSpec::make_primitive(BaseType::I32);
            break;
        case LiteralNode::Kind::FLOAT:
            m_last_type = TypeSpec::make_primitive(BaseType::F32);
            break;
        case LiteralNode::Kind::BOOL:
            m_last_type = TypeSpec::make_primitive(BaseType::BOOL);
            break;
        case LiteralNode::Kind::CHAR:
            m_last_type = TypeSpec::make_primitive(BaseType::CHAR);
            break;
        case LiteralNode::Kind::STR:
            m_last_type = TypeSpec::make_primitive(BaseType::STR);
            break;
    }
}

void TypeChecker::visit(IdentifierNode& node) {
    const Symbol* sym = m_symtab.lookup(node.name);
    if (!sym || sym->kind == SymbolKind::ROUTINE) {
        m_last_type = TypeSpec::unknown();
        return;
    }
    m_last_type = sym->type;
}

void TypeChecker::visit(ArrayLiteralNode& node) {
    if (node.elements.empty()) {
        m_diagnostics.report_error(node.location, "array literal cannot be empty");
        m_last_type = TypeSpec::unknown();
        return;
    }

    TypeSpec elem_type = TypeSpec::unknown();
    bool has_error = false;

    for (size_t i = 0; i < node.elements.size(); ++i) {
        if (!node.elements[i]) continue;
        TypeSpec cur = evaluate(*node.elements[i]);
        if (cur.is_unknown) {
            has_error = true;
            continue;
        }
        if (cur.is_array || cur.is_void()) {
            m_diagnostics.report_error(
                node.elements[i]->location,
                "array literal elements must be primitive values, got '" + cur.to_string() + "'"
            );
            has_error = true;
            continue;
        }
        if (elem_type.is_unknown) {
            elem_type = cur;
        } else if (cur != elem_type) {
            m_diagnostics.report_error(
                node.elements[i]->location,
                "inconsistent element types in array literal: expected '" +
                    elem_type.to_string() + "', got '" + cur.to_string() + "'"
            );
            has_error = true;
        }
    }

    if (has_error || elem_type.is_unknown) {
        m_last_type = TypeSpec::unknown();
        return;
    }
    m_last_type = TypeSpec::make_array(elem_type.base, static_cast<int>(node.elements.size()));
}

void TypeChecker::visit(IndexAccessNode& node) {
    TypeSpec arr_type = node.array ? evaluate(*node.array) : TypeSpec::unknown();
    TypeSpec idx_type = node.index ? evaluate(*node.index) : TypeSpec::unknown();

    if (!idx_type.is_unknown && !idx_type.is_i32()) {
        m_diagnostics.report_error(
            node.index ? node.index->location : node.location,
            "array index must be of type 'i32', got '" + idx_type.to_string() + "'"
        );
    }

    if (arr_type.is_unknown) {
        m_last_type = TypeSpec::unknown();
        return;
    }

    if (!arr_type.is_array) {
        m_diagnostics.report_error(
            node.location,
            "cannot index non-array type '" + arr_type.to_string() + "'"
        );
        m_last_type = TypeSpec::unknown();
        return;
    }

    if (idx_type.is_i32() && node.index) {
        long long const_idx = 0;
        if (try_eval_const_i32(node.index.get(), const_idx)) {
            if (const_idx < 0 || const_idx >= static_cast<long long>(arr_type.array_size)) {
                std::ostringstream oss;
                oss << "array index " << const_idx << " out of bounds for type '"
                    << arr_type.to_string() << "' (valid range: 0.." << (arr_type.array_size - 1) << ")";
                m_diagnostics.report_error(node.index->location, oss.str());
            }
        }
    }

    m_last_type = TypeSpec::make_primitive(arr_type.base);
}

void TypeChecker::visit(RoutineCallNode& node) {
    std::vector<TypeSpec> arg_types;
    arg_types.reserve(node.args.size());
    for (const auto& arg : node.args) {
        arg_types.push_back(arg ? evaluate(*arg) : TypeSpec::unknown());
    }

    const Symbol* sym = m_symtab.lookup(node.callee);
    if (!sym || sym->kind != SymbolKind::ROUTINE) {
        m_last_type = TypeSpec::unknown();
        return;
    }

    if (node.args.size() != sym->param_types.size()) {
        std::ostringstream oss;
        oss << "routine '" << node.callee << "' expects " << sym->param_types.size()
            << " argument(s), got " << node.args.size();
        m_diagnostics.report_error(node.location, oss.str());
    } else {
        for (size_t i = 0; i < node.args.size(); ++i) {
            if (!arg_types[i].is_unknown && arg_types[i] != sym->param_types[i]) {
                std::ostringstream oss;
                oss << "type mismatch in argument " << (i + 1) << " of call to '"
                    << node.callee << "': expected '" << sym->param_types[i].to_string()
                    << "', got '" << arg_types[i].to_string() << "'";
                m_diagnostics.report_error(node.args[i]->location, oss.str());
            }
        }
    }

    m_last_type = sym->type;
}

void TypeChecker::visit(UnaryExprNode& node) {
    TypeSpec op_type = node.operand ? evaluate(*node.operand) : TypeSpec::unknown();
    if (op_type.is_unknown) {
        m_last_type = TypeSpec::unknown();
        return;
    }

    switch (node.op) {
        case UnaryExprNode::Op::NEG:
            if (!op_type.is_numeric()) {
                m_diagnostics.report_error(
                    node.location,
                    "unary '-' requires numeric operand ('i32' or 'f32'), got '" + op_type.to_string() + "'"
                );
                m_last_type = TypeSpec::unknown();
                return;
            }
            m_last_type = op_type;
            return;

        case UnaryExprNode::Op::NOT:
            if (!op_type.is_bool()) {
                m_diagnostics.report_error(
                    node.location,
                    "logical 'not' requires 'bool' operand, got '" + op_type.to_string() + "'"
                );
                m_last_type = TypeSpec::unknown();
                return;
            }
            m_last_type = TypeSpec::make_primitive(BaseType::BOOL);
            return;
    }
}

void TypeChecker::visit(BinaryExprNode& node) {
    TypeSpec lhs = node.left ? evaluate(*node.left) : TypeSpec::unknown();
    TypeSpec rhs = node.right ? evaluate(*node.right) : TypeSpec::unknown();

    if (lhs.is_unknown || rhs.is_unknown) {
        m_last_type = TypeSpec::unknown();
        return;
    }

    const char* op_str = BinaryExprNode::op_to_string(node.op);
    switch (node.op) {
        case BinaryExprNode::Op::ADD:
        case BinaryExprNode::Op::SUB:
        case BinaryExprNode::Op::MUL:
        case BinaryExprNode::Op::DIV:
        case BinaryExprNode::Op::POW:
            if (!lhs.is_numeric() || !rhs.is_numeric() || lhs != rhs) {
                m_diagnostics.report_error(
                    node.location,
                    std::string("type mismatch in binary '") + op_str +
                        "': invalid operands '" + lhs.to_string() + "' and '" + rhs.to_string() +
                        "' (expected matching 'i32' or 'f32')"
                );
                m_last_type = TypeSpec::unknown();
                return;
            }
            m_last_type = lhs;
            return;

        case BinaryExprNode::Op::MOD:
            if (!lhs.is_i32() || !rhs.is_i32()) {
                m_diagnostics.report_error(
                    node.location,
                    "type mismatch in binary '%': invalid operands '" + lhs.to_string() +
                        "' and '" + rhs.to_string() + "' (modulo requires 'i32')"
                );
                m_last_type = TypeSpec::unknown();
                return;
            }
            m_last_type = TypeSpec::make_primitive(BaseType::I32);
            return;

        case BinaryExprNode::Op::LT:
        case BinaryExprNode::Op::LE:
        case BinaryExprNode::Op::GT:
        case BinaryExprNode::Op::GE:
            if (!lhs.is_numeric() || !rhs.is_numeric() || lhs != rhs) {
                m_diagnostics.report_error(
                    node.location,
                    std::string("type mismatch in relational '") + op_str +
                        "': invalid operands '" + lhs.to_string() + "' and '" + rhs.to_string() +
                        "' (expected matching 'i32' or 'f32')"
                );
                m_last_type = TypeSpec::unknown();
                return;
            }
            m_last_type = TypeSpec::make_primitive(BaseType::BOOL);
            return;

        case BinaryExprNode::Op::EQ:
        case BinaryExprNode::Op::NEQ:
            if (lhs != rhs || lhs.is_void()) {
                m_diagnostics.report_error(
                    node.location,
                    std::string("type mismatch in equality '") + op_str +
                        "': cannot compare '" + lhs.to_string() + "' and '" + rhs.to_string() + "'"
                );
                m_last_type = TypeSpec::unknown();
                return;
            }
            m_last_type = TypeSpec::make_primitive(BaseType::BOOL);
            return;

        case BinaryExprNode::Op::AND:
        case BinaryExprNode::Op::OR:
            if (!lhs.is_bool() || !rhs.is_bool()) {
                m_diagnostics.report_error(
                    node.location,
                    std::string("type mismatch in logical '") + op_str +
                        "': invalid operands '" + lhs.to_string() + "' and '" + rhs.to_string() +
                        "' (expected 'bool')"
                );
                m_last_type = TypeSpec::unknown();
                return;
            }
            m_last_type = TypeSpec::make_primitive(BaseType::BOOL);
            return;
    }
}

} // namespace nova
