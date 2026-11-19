#include "semantic/symbol.hpp"
#include <ostream>
#include <sstream>

namespace nova {

const char* mutability_to_string(Mutability mut) {
    return (mut == Mutability::CONST) ? "CONST" : "FLUX";
}

const char* symbol_kind_to_string(SymbolKind kind) {
    switch (kind) {
        case SymbolKind::VARIABLE:    return "VARIABLE";
        case SymbolKind::PARAMETER:   return "PARAMETER";
        case SymbolKind::ORBIT_INDEX: return "ORBIT_INDEX";
        case SymbolKind::ROUTINE:     return "ROUTINE";
    }
    return "UNKNOWN";
}

std::string Symbol::signature_string() const {
    if (kind != SymbolKind::ROUTINE) {
        return type.to_string();
    }
    std::ostringstream oss;
    oss << "(";
    for (size_t i = 0; i < param_types.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << param_types[i].to_string();
    }
    oss << ") -> " << type.to_string();
    return oss.str();
}

Scope::Scope(std::string label, int depth, Scope* parent)
    : m_label(std::move(label)), m_depth(depth), m_parent(parent) {}

bool Scope::insert(Symbol sym) {
    if (m_index_by_name.find(sym.name) != m_index_by_name.end()) {
        return false;
    }
    size_t idx = m_symbols.size();
    m_index_by_name[sym.name] = idx;
    m_symbols.push_back(std::move(sym));
    return true;
}

const Symbol* Scope::lookup_local(const std::string& name) const {
    auto it = m_index_by_name.find(name);
    if (it == m_index_by_name.end()) {
        return nullptr;
    }
    return &m_symbols[it->second];
}

Symbol* Scope::lookup_local(const std::string& name) {
    auto it = m_index_by_name.find(name);
    if (it == m_index_by_name.end()) {
        return nullptr;
    }
    return &m_symbols[it->second];
}

const Symbol* Scope::lookup(const std::string& name) const {
    for (const Scope* s = this; s != nullptr; s = s->m_parent) {
        if (const Symbol* sym = s->lookup_local(name)) {
            return sym;
        }
    }
    return nullptr;
}

Symbol* Scope::lookup(const std::string& name) {
    for (Scope* s = this; s != nullptr; s = s->m_parent) {
        if (Symbol* sym = s->lookup_local(name)) {
            return sym;
        }
    }
    return nullptr;
}

Scope* Scope::add_child(std::string label) {
    m_children.push_back(std::make_unique<Scope>(std::move(label), m_depth + 1, this));
    return m_children.back().get();
}

SymbolTable::SymbolTable() {
    reset();
}

void SymbolTable::reset() {
    m_global = std::make_unique<Scope>("global", 0, nullptr);
    m_current = m_global.get();
    m_next_offset = 0;
    m_saved_offsets.clear();
}

Scope* SymbolTable::enter_scope(const std::string& label, bool reset_frame_offset) {
    m_saved_offsets.push_back(SavedFrame{m_next_offset, reset_frame_offset});
    if (reset_frame_offset) {
        m_next_offset = 0;
    }
    m_current = m_current->add_child(label);
    return m_current;
}

void SymbolTable::exit_scope() {
    if (m_current && m_current->parent() != nullptr) {
        if (!m_saved_offsets.empty()) {
            SavedFrame frame = m_saved_offsets.back();
            m_saved_offsets.pop_back();
            if (frame.restore_on_exit) {
                m_next_offset = frame.offset;
            }
        }
        m_current = m_current->parent();
    }
}

bool SymbolTable::declare(Symbol sym) {
    if (!m_current || m_current->lookup_local(sym.name) != nullptr) {
        return false;
    }
    if (sym.kind != SymbolKind::ROUTINE) {
        sym.offset = m_next_offset++;
    }
    return m_current->insert(std::move(sym));
}

const Symbol* SymbolTable::lookup_local(const std::string& name) const {
    return m_current ? m_current->lookup_local(name) : nullptr;
}

const Symbol* SymbolTable::lookup(const std::string& name) const {
    return m_current ? m_current->lookup(name) : nullptr;
}

void SymbolTable::dump(std::ostream& out) const {
    if (m_global) {
        dump_scope(out, *m_global, 0);
    }
}

void SymbolTable::dump_scope(std::ostream& out, const Scope& scope, int indent) {
    std::string pad(static_cast<size_t>(indent) * 2, ' ');
    out << pad << "Scope [depth=" << scope.depth() << "] " << scope.label() << ":\n";

    std::string sym_pad(static_cast<size_t>(indent + 1) * 2, ' ');
    for (const auto& sym : scope.symbols()) {
        out << sym_pad << sym.name << ": " << sym.signature_string()
            << " [" << mutability_to_string(sym.mutability)
            << ", " << symbol_kind_to_string(sym.kind);
        if (sym.kind != SymbolKind::ROUTINE) {
            out << ", offset=" << sym.offset;
        }
        out << ", loc=" << sym.location.line << ":" << sym.location.column << "]\n";
    }

    for (const auto& child : scope.children()) {
        dump_scope(out, *child, indent + 1);
    }
}

} // namespace nova
