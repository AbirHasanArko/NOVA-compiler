#include "codegen/interpreter.hpp"
#include <cmath>
#include <iomanip>

namespace nova {

RuntimeValue RuntimeValue::make_void() {
    return RuntimeValue{};
}

RuntimeValue RuntimeValue::make_i32(int32_t v) {
    RuntimeValue rv;
    rv.base = BaseType::I32;
    rv.i32_val = v;
    return rv;
}

RuntimeValue RuntimeValue::make_f32(float v) {
    RuntimeValue rv;
    rv.base = BaseType::F32;
    rv.f32_val = v;
    return rv;
}

RuntimeValue RuntimeValue::make_bool(bool v) {
    RuntimeValue rv;
    rv.base = BaseType::BOOL;
    rv.bool_val = v;
    return rv;
}

RuntimeValue RuntimeValue::make_char(char v) {
    RuntimeValue rv;
    rv.base = BaseType::CHAR;
    rv.char_val = v;
    return rv;
}

RuntimeValue RuntimeValue::make_str(std::string v) {
    RuntimeValue rv;
    rv.base = BaseType::STR;
    rv.str_val = std::move(v);
    return rv;
}

RuntimeValue RuntimeValue::make_default(const TypeSpec& type) {
    if (type.is_array) {
        RuntimeValue rv;
        rv.base = type.base;
        rv.is_array = true;
        TypeSpec elem_spec = TypeSpec::make_primitive(type.base);
        rv.elements.reserve(static_cast<size_t>(type.array_size));
        for (int i = 0; i < type.array_size; ++i) {
            rv.elements.push_back(make_default(elem_spec));
        }
        return rv;
    }
    switch (type.base) {
        case BaseType::I32:  return make_i32(0);
        case BaseType::F32:  return make_f32(0.0f);
        case BaseType::BOOL: return make_bool(false);
        case BaseType::CHAR: return make_char('\0');
        case BaseType::STR:  return make_str("");
        case BaseType::VOID: return make_void();
    }
    return make_void();
}

bool RuntimeValue::equals(const RuntimeValue& other) const {
    if (base != other.base || is_array != other.is_array) {
        return false;
    }
    if (is_array) {
        if (elements.size() != other.elements.size()) return false;
        for (size_t i = 0; i < elements.size(); ++i) {
            if (!elements[i].equals(other.elements[i])) return false;
        }
        return true;
    }
    switch (base) {
        case BaseType::I32:  return i32_val == other.i32_val;
        case BaseType::F32:  return f32_val == other.f32_val;
        case BaseType::BOOL: return bool_val == other.bool_val;
        case BaseType::CHAR: return char_val == other.char_val;
        case BaseType::STR:  return str_val == other.str_val;
        case BaseType::VOID: return true;
    }
    return false;
}

std::string RuntimeValue::to_output_string() const {
    if (is_array) {
        std::ostringstream oss;
        oss << "[";
        for (size_t i = 0; i < elements.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << elements[i].to_output_string();
        }
        oss << "]";
        return oss.str();
    }
    switch (base) {
        case BaseType::I32:
            return std::to_string(i32_val);
        case BaseType::F32: {
            std::ostringstream oss;
            float clean_v = (std::fabs(f32_val) == 0.0f) ? 0.0f : f32_val;
            oss << std::setprecision(6) << clean_v;
            std::string s = oss.str();
            if (s.find('.') == std::string::npos && s.find('e') == std::string::npos &&
                s.find('E') == std::string::npos) {
                s += ".0";
            }
            return s;
        }
        case BaseType::BOOL:
            return bool_val ? "true" : "false";
        case BaseType::CHAR:
            return std::string(1, char_val);
        case BaseType::STR:
            return str_val;
        case BaseType::VOID:
            return "void";
    }
    return "";
}

Interpreter::Interpreter(
    DiagnosticEngine& diagnostics,
    std::ostream& out,
    std::istream& in
) : m_diagnostics(diagnostics), m_out(out), m_in(in) {}

bool Interpreter::execute(ProgramNode& root) {
    m_routines.clear();
    m_frames.clear();
    m_flow = FlowState::NORMAL;
    m_last_value = RuntimeValue::make_void();
    m_return_value = RuntimeValue::make_void();
    m_active_routine.clear();
    m_capture_stream.str("");
    m_capture_stream.clear();
    m_captured_output.clear();

    size_t errors_before = m_diagnostics.error_count();
    root.accept(*this);
    m_captured_output = m_capture_stream.str();
    return (m_flow != FlowState::ABORT) && (m_diagnostics.error_count() == errors_before);
}

void Interpreter::push_frame() {
    m_frames.emplace_back();
    enter_scope();
}

void Interpreter::pop_frame() {
    if (!m_frames.empty()) {
        m_frames.pop_back();
    }
}

void Interpreter::enter_scope() {
    if (m_frames.empty()) {
        m_frames.emplace_back();
    }
    m_frames.back().emplace_back();
}

void Interpreter::exit_scope() {
    if (!m_frames.empty() && !m_frames.back().empty()) {
        m_frames.back().pop_back();
    }
}

void Interpreter::define_var(const std::string& name, RuntimeValue val) {
    if (!m_frames.empty() && !m_frames.back().empty()) {
        m_frames.back().back()[name] = std::move(val);
    }
}

RuntimeValue* Interpreter::find_var(const std::string& name) {
    if (m_frames.empty()) return nullptr;
    auto& scopes = m_frames.back();
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            return &found->second;
        }
    }
    return nullptr;
}

RuntimeValue Interpreter::eval(ExprNode& expr) {
    if (m_flow == FlowState::ABORT) {
        return RuntimeValue::make_void();
    }
    m_last_value = RuntimeValue::make_void();
    expr.accept(*this);
    return m_last_value;
}

void Interpreter::exec_block(const std::vector<StmtPtr>& stmts) {
    for (const auto& stmt : stmts) {
        if (m_flow != FlowState::NORMAL) {
            break;
        }
        if (stmt) {
            stmt->accept(*this);
        }
    }
}

void Interpreter::runtime_error(const SourceLocation& loc, const std::string& msg) {
    m_diagnostics.report_error(loc, "runtime error: " + msg);
    m_flow = FlowState::ABORT;
}

std::string Interpreter::unescape_string(const std::string& raw) {
    std::string inner = raw;
    if (inner.size() >= 2 && inner.front() == '"' && inner.back() == '"') {
        inner = inner.substr(1, inner.size() - 2);
    }
    std::string out;
    out.reserve(inner.size());
    for (size_t i = 0; i < inner.size(); ++i) {
        if (inner[i] == '\\' && i + 1 < inner.size()) {
            char next = inner[++i];
            switch (next) {
                case 'n':  out.push_back('\n'); break;
                case 't':  out.push_back('\t'); break;
                case 'r':  out.push_back('\r'); break;
                case '0':  out.push_back('\0'); break;
                case '\\': out.push_back('\\'); break;
                case '"':  out.push_back('"');  break;
                case '\'': out.push_back('\''); break;
                default:   out.push_back(next); break;
            }
        } else {
            out.push_back(inner[i]);
        }
    }
    return out;
}

char Interpreter::unescape_char(const std::string& raw) {
    std::string inner = raw;
    if (inner.size() >= 2 && inner.front() == '\'' && inner.back() == '\'') {
        inner = inner.substr(1, inner.size() - 2);
    }
    if (inner.empty()) return '\0';
    if (inner[0] == '\\' && inner.size() >= 2) {
        switch (inner[1]) {
            case 'n':  return '\n';
            case 't':  return '\t';
            case 'r':  return '\r';
            case '0':  return '\0';
            case '\\': return '\\';
            case '\'': return '\'';
            case '"':  return '"';
            default:   return inner[1];
        }
    }
    return inner[0];
}

void Interpreter::visit(ProgramNode& node) {
    for (const auto& routine : node.routines) {
        if (routine) {
            m_routines[routine->name] = routine.get();
        }
    }

    push_frame();
    exec_block(node.ignite_body);
    pop_frame();
}

void Interpreter::visit(RoutineDeclNode&) {}

void Interpreter::visit(RequireNode& node) {
    if (!node.condition || m_flow != FlowState::NORMAL) return;
    RuntimeValue cond = eval(*node.condition);
    if (m_flow == FlowState::ABORT) return;
    if (!cond.bool_val) {
        runtime_error(
            node.location,
            "precondition violation in 'require' for routine '" + m_active_routine + "'"
        );
    }
}

void Interpreter::visit(ConstDeclNode& node) {
    RuntimeValue val = node.initializer ? eval(*node.initializer) : RuntimeValue::make_default(node.type);
    if (m_flow == FlowState::ABORT) return;
    define_var(node.name, std::move(val));
}

void Interpreter::visit(FluxDeclNode& node) {
    RuntimeValue val = node.initializer ? eval(*node.initializer) : RuntimeValue::make_default(node.type);
    if (m_flow == FlowState::ABORT) return;
    define_var(node.name, std::move(val));
}

void Interpreter::visit(AssignNode& node) {
    if (!node.value || !node.target) return;
    RuntimeValue rhs = eval(*node.value);
    if (m_flow == FlowState::ABORT) return;

    if (auto* id = dynamic_cast<IdentifierNode*>(node.target.get())) {
        RuntimeValue* slot = find_var(id->name);
        if (!slot) {
            runtime_error(id->location, "undefined variable '" + id->name + "'");
            return;
        }
        *slot = std::move(rhs);
        return;
    }

    if (auto* idx_node = dynamic_cast<IndexAccessNode*>(node.target.get())) {
        RuntimeValue idx_val = eval(*idx_node->index);
        if (m_flow == FlowState::ABORT) return;

        auto* base_id = dynamic_cast<IdentifierNode*>(idx_node->array.get());
        if (!base_id) {
            runtime_error(idx_node->location, "invalid array assignment target");
            return;
        }
        RuntimeValue* arr_slot = find_var(base_id->name);
        if (!arr_slot || !arr_slot->is_array) {
            runtime_error(base_id->location, "target '" + base_id->name + "' is not an array");
            return;
        }
        int32_t i = idx_val.i32_val;
        if (i < 0 || static_cast<size_t>(i) >= arr_slot->elements.size()) {
            runtime_error(
                idx_node->index->location,
                "array index " + std::to_string(i) + " out of bounds (size " +
                    std::to_string(arr_slot->elements.size()) + ")"
            );
            return;
        }
        arr_slot->elements[static_cast<size_t>(i)] = std::move(rhs);
    }
}

void Interpreter::visit(ExprStmtNode& node) {
    if (node.expr) {
        eval(*node.expr);
    }
}

void Interpreter::visit(DetectNode& node) {
    for (const auto& branch : node.branches) {
        RuntimeValue cond = eval(*branch.condition);
        if (m_flow == FlowState::ABORT) return;
        if (cond.bool_val) {
            enter_scope();
            exec_block(branch.body);
            exit_scope();
            return;
        }
    }
    if (node.has_fallback) {
        enter_scope();
        exec_block(node.fallback_body);
        exit_scope();
    }
}

void Interpreter::visit(CruiseNode& node) {
    while (m_flow == FlowState::NORMAL) {
        RuntimeValue cond = eval(*node.condition);
        if (m_flow == FlowState::ABORT || !cond.bool_val) {
            break;
        }
        enter_scope();
        exec_block(node.body);
        exit_scope();

        if (m_flow == FlowState::HALT) {
            m_flow = FlowState::NORMAL;
            break;
        }
        if (m_flow == FlowState::SKIP) {
            m_flow = FlowState::NORMAL;
        }
    }
}

void Interpreter::visit(OrbitNode& node) {
    RuntimeValue start_v = eval(*node.start);
    if (m_flow == FlowState::ABORT) return;
    RuntimeValue end_v = eval(*node.end);
    if (m_flow == FlowState::ABORT) return;

    int32_t step = 1;
    if (node.step) {
        RuntimeValue step_v = eval(*node.step);
        if (m_flow == FlowState::ABORT) return;
        step = step_v.i32_val;
    }

    if (step == 0) {
        runtime_error(node.location, "orbit step cannot be zero");
        return;
    }

    int32_t cur = start_v.i32_val;
    int32_t limit = end_v.i32_val;

    while (m_flow == FlowState::NORMAL) {
        if ((step > 0 && cur >= limit) || (step < 0 && cur <= limit)) {
            break;
        }

        enter_scope();
        define_var(node.loop_var, RuntimeValue::make_i32(cur));
        exec_block(node.body);
        exit_scope();

        if (m_flow == FlowState::HALT) {
            m_flow = FlowState::NORMAL;
            break;
        }
        if (m_flow == FlowState::SKIP) {
            m_flow = FlowState::NORMAL;
        }

        cur += step;
    }
}

void Interpreter::visit(RouteNode& node) {
    RuntimeValue sel = eval(*node.selector);
    if (m_flow == FlowState::ABORT) return;

    for (const auto& c : node.cases) {
        RuntimeValue case_v = eval(*c.value);
        if (m_flow == FlowState::ABORT) return;
        if (sel.equals(case_v)) {
            enter_scope();
            exec_block(c.body);
            exit_scope();
            return;
        }
    }

    if (node.has_default) {
        enter_scope();
        exec_block(node.default_body);
        exit_scope();
    }
}

void Interpreter::visit(TransmitNode& node) {
    std::ostringstream line;
    for (const auto& arg : node.args) {
        if (!arg) continue;
        RuntimeValue val = eval(*arg);
        if (m_flow == FlowState::ABORT) return;
        line << val.to_output_string();
    }
    line << "\n";
    std::string text = line.str();
    m_out << text;
    m_capture_stream << text;
}

void Interpreter::visit(ReceiveNode& node) {
    RuntimeValue* slot = find_var(node.target);
    if (!slot) {
        runtime_error(node.location, "undefined variable '" + node.target + "' in receive");
        return;
    }

    std::string token;
    if (!(m_in >> token)) {
        // Non-interactive / EOF fallback preserves existing value without blocking.
        return;
    }

    try {
        switch (slot->base) {
            case BaseType::I32:
                slot->i32_val = static_cast<int32_t>(std::stoi(token));
                break;
            case BaseType::F32:
                slot->f32_val = std::stof(token);
                break;
            case BaseType::BOOL:
                if (token == "true" || token == "1") {
                    slot->bool_val = true;
                } else if (token == "false" || token == "0") {
                    slot->bool_val = false;
                } else {
                    runtime_error(node.location, "invalid boolean input '" + token + "' in receive");
                }
                break;
            case BaseType::CHAR:
                slot->char_val = token.empty() ? '\0' : token[0];
                break;
            case BaseType::STR:
                slot->str_val = token;
                break;
            case BaseType::VOID:
                break;
        }
    } catch (const std::exception&) {
        runtime_error(node.location, "failed to parse input '" + token + "' in receive");
    }
}

void Interpreter::visit(VerifyNode& node) {
    if (!node.condition) return;
    RuntimeValue cond = eval(*node.condition);
    if (m_flow == FlowState::ABORT) return;
    if (!cond.bool_val) {
        runtime_error(node.location, "invariant violation in 'verify'");
    }
}

void Interpreter::visit(RetNode& node) {
    if (node.value) {
        m_return_value = eval(*node.value);
        if (m_flow == FlowState::ABORT) return;
    } else {
        m_return_value = RuntimeValue::make_void();
    }
    m_flow = FlowState::RETURN;
}

void Interpreter::visit(HaltNode&) {
    m_flow = FlowState::HALT;
}

void Interpreter::visit(SkipNode&) {
    m_flow = FlowState::SKIP;
}

void Interpreter::visit(LiteralNode& node) {
    switch (node.kind) {
        case LiteralNode::Kind::INT:
            m_last_value = RuntimeValue::make_i32(static_cast<int32_t>(std::stoi(node.value)));
            break;
        case LiteralNode::Kind::FLOAT:
            m_last_value = RuntimeValue::make_f32(std::stof(node.value));
            break;
        case LiteralNode::Kind::BOOL:
            m_last_value = RuntimeValue::make_bool(node.value == "true");
            break;
        case LiteralNode::Kind::CHAR:
            m_last_value = RuntimeValue::make_char(unescape_char(node.value));
            break;
        case LiteralNode::Kind::STR:
            m_last_value = RuntimeValue::make_str(unescape_string(node.value));
            break;
    }
}

void Interpreter::visit(IdentifierNode& node) {
    RuntimeValue* slot = find_var(node.name);
    if (!slot) {
        runtime_error(node.location, "undefined variable '" + node.name + "'");
        return;
    }
    m_last_value = *slot;
}

void Interpreter::visit(ArrayLiteralNode& node) {
    RuntimeValue arr;
    arr.is_array = true;
    arr.elements.reserve(node.elements.size());
    for (const auto& elem : node.elements) {
        RuntimeValue v = eval(*elem);
        if (m_flow == FlowState::ABORT) return;
        arr.base = v.base;
        arr.elements.push_back(std::move(v));
    }
    m_last_value = std::move(arr);
}

void Interpreter::visit(IndexAccessNode& node) {
    RuntimeValue arr = eval(*node.array);
    if (m_flow == FlowState::ABORT) return;
    RuntimeValue idx = eval(*node.index);
    if (m_flow == FlowState::ABORT) return;

    int32_t i = idx.i32_val;
    if (i < 0 || static_cast<size_t>(i) >= arr.elements.size()) {
        runtime_error(
            node.index->location,
            "array index " + std::to_string(i) + " out of bounds (size " +
                std::to_string(arr.elements.size()) + ")"
        );
        return;
    }
    m_last_value = arr.elements[static_cast<size_t>(i)];
}

void Interpreter::visit(RoutineCallNode& node) {
    auto it = m_routines.find(node.callee);
    if (it == m_routines.end() || !it->second) {
        runtime_error(node.location, "undefined routine '" + node.callee + "'");
        return;
    }
    RoutineDeclNode* decl = it->second;

    std::vector<RuntimeValue> arg_vals;
    arg_vals.reserve(node.args.size());
    for (const auto& arg : node.args) {
        RuntimeValue v = eval(*arg);
        if (m_flow == FlowState::ABORT) return;
        arg_vals.push_back(std::move(v));
    }

    std::string prev_routine = m_active_routine;
    m_active_routine = decl->name;
    push_frame();

    for (size_t i = 0; i < decl->params.size() && i < arg_vals.size(); ++i) {
        define_var(decl->params[i].name, std::move(arg_vals[i]));
    }

    for (const auto& req : decl->requires) {
        if (m_flow != FlowState::NORMAL) break;
        if (req) req->accept(*this);
    }

    if (m_flow == FlowState::NORMAL) {
        exec_block(decl->body);
    }

    RuntimeValue result = RuntimeValue::make_void();
    if (m_flow == FlowState::RETURN) {
        result = std::move(m_return_value);
        m_return_value = RuntimeValue::make_void();
        m_flow = FlowState::NORMAL;
    }

    pop_frame();
    m_active_routine = std::move(prev_routine);
    m_last_value = std::move(result);
}

void Interpreter::visit(UnaryExprNode& node) {
    RuntimeValue op = eval(*node.operand);
    if (m_flow == FlowState::ABORT) return;

    switch (node.op) {
        case UnaryExprNode::Op::NEG:
            if (op.base == BaseType::I32) {
                m_last_value = RuntimeValue::make_i32(-op.i32_val);
            } else {
                m_last_value = RuntimeValue::make_f32(-op.f32_val);
            }
            return;
        case UnaryExprNode::Op::NOT:
            m_last_value = RuntimeValue::make_bool(!op.bool_val);
            return;
    }
}

void Interpreter::visit(BinaryExprNode& node) {
    // Short-circuit logical operators
    if (node.op == BinaryExprNode::Op::AND) {
        RuntimeValue lhs = eval(*node.left);
        if (m_flow == FlowState::ABORT) return;
        if (!lhs.bool_val) {
            m_last_value = RuntimeValue::make_bool(false);
            return;
        }
        RuntimeValue rhs = eval(*node.right);
        if (m_flow == FlowState::ABORT) return;
        m_last_value = RuntimeValue::make_bool(rhs.bool_val);
        return;
    }
    if (node.op == BinaryExprNode::Op::OR) {
        RuntimeValue lhs = eval(*node.left);
        if (m_flow == FlowState::ABORT) return;
        if (lhs.bool_val) {
            m_last_value = RuntimeValue::make_bool(true);
            return;
        }
        RuntimeValue rhs = eval(*node.right);
        if (m_flow == FlowState::ABORT) return;
        m_last_value = RuntimeValue::make_bool(rhs.bool_val);
        return;
    }

    RuntimeValue lhs = eval(*node.left);
    if (m_flow == FlowState::ABORT) return;
    RuntimeValue rhs = eval(*node.right);
    if (m_flow == FlowState::ABORT) return;

    switch (node.op) {
        case BinaryExprNode::Op::ADD:
            if (lhs.base == BaseType::I32) {
                m_last_value = RuntimeValue::make_i32(lhs.i32_val + rhs.i32_val);
            } else {
                m_last_value = RuntimeValue::make_f32(lhs.f32_val + rhs.f32_val);
            }
            return;

        case BinaryExprNode::Op::SUB:
            if (lhs.base == BaseType::I32) {
                m_last_value = RuntimeValue::make_i32(lhs.i32_val - rhs.i32_val);
            } else {
                m_last_value = RuntimeValue::make_f32(lhs.f32_val - rhs.f32_val);
            }
            return;

        case BinaryExprNode::Op::MUL:
            if (lhs.base == BaseType::I32) {
                m_last_value = RuntimeValue::make_i32(lhs.i32_val * rhs.i32_val);
            } else {
                m_last_value = RuntimeValue::make_f32(lhs.f32_val * rhs.f32_val);
            }
            return;

        case BinaryExprNode::Op::DIV:
            if (lhs.base == BaseType::I32) {
                if (rhs.i32_val == 0) {
                    runtime_error(node.location, "division by zero");
                    return;
                }
                m_last_value = RuntimeValue::make_i32(lhs.i32_val / rhs.i32_val);
            } else {
                if (rhs.f32_val == 0.0f) {
                    runtime_error(node.location, "division by zero");
                    return;
                }
                m_last_value = RuntimeValue::make_f32(lhs.f32_val / rhs.f32_val);
            }
            return;

        case BinaryExprNode::Op::MOD:
            if (rhs.i32_val == 0) {
                runtime_error(node.location, "modulo by zero");
                return;
            }
            m_last_value = RuntimeValue::make_i32(lhs.i32_val % rhs.i32_val);
            return;

        case BinaryExprNode::Op::POW:
            if (lhs.base == BaseType::I32) {
                if (rhs.i32_val < 0) {
                    m_last_value = RuntimeValue::make_i32(0);
                } else {
                    int32_t res = 1;
                    int32_t base = lhs.i32_val;
                    int32_t exp = rhs.i32_val;
                    while (exp > 0) {
                        if (exp & 1) res *= base;
                        base *= base;
                        exp >>= 1;
                    }
                    m_last_value = RuntimeValue::make_i32(res);
                }
            } else {
                m_last_value = RuntimeValue::make_f32(std::pow(lhs.f32_val, rhs.f32_val));
            }
            return;

        case BinaryExprNode::Op::LT:
            m_last_value = RuntimeValue::make_bool(
                lhs.base == BaseType::I32 ? (lhs.i32_val < rhs.i32_val) : (lhs.f32_val < rhs.f32_val)
            );
            return;

        case BinaryExprNode::Op::LE:
            m_last_value = RuntimeValue::make_bool(
                lhs.base == BaseType::I32 ? (lhs.i32_val <= rhs.i32_val) : (lhs.f32_val <= rhs.f32_val)
            );
            return;

        case BinaryExprNode::Op::GT:
            m_last_value = RuntimeValue::make_bool(
                lhs.base == BaseType::I32 ? (lhs.i32_val > rhs.i32_val) : (lhs.f32_val > rhs.f32_val)
            );
            return;

        case BinaryExprNode::Op::GE:
            m_last_value = RuntimeValue::make_bool(
                lhs.base == BaseType::I32 ? (lhs.i32_val >= rhs.i32_val) : (lhs.f32_val >= rhs.f32_val)
            );
            return;

        case BinaryExprNode::Op::EQ:
            m_last_value = RuntimeValue::make_bool(lhs.equals(rhs));
            return;

        case BinaryExprNode::Op::NEQ:
            m_last_value = RuntimeValue::make_bool(!lhs.equals(rhs));
            return;

        case BinaryExprNode::Op::AND:
        case BinaryExprNode::Op::OR:
            return;
    }
}

} // namespace nova
