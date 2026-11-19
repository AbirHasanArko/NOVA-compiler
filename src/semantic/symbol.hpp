#ifndef NOVA_SEMANTIC_SYMBOL_HPP
#define NOVA_SEMANTIC_SYMBOL_HPP

#include "ast/ast_node.hpp"
#include "lexer/token.hpp"
#include <cstddef>
#include <iosfwd>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace nova {

enum class Mutability {
    CONST,
    FLUX
};

enum class SymbolKind {
    VARIABLE,
    PARAMETER,
    ORBIT_INDEX,
    ROUTINE
};

/**
 * Metadata entry for a declared variable, parameter, loop index, or routine.
 */
struct Symbol {
    std::string name;
    TypeSpec type;
    Mutability mutability;
    SymbolKind kind;
    SourceLocation location;
    size_t offset = 0;
    std::vector<TypeSpec> param_types; // Populated for SymbolKind::ROUTINE

    std::string signature_string() const;
};

/**
 * Single lexical scope node inside the program's scope tree.
 */
class Scope {
public:
    Scope(std::string label, int depth, Scope* parent);

    const std::string& label() const { return m_label; }
    int depth() const { return m_depth; }
    Scope* parent() const { return m_parent; }

    // Insert symbol into this scope only. Returns false if name already exists here.
    bool insert(Symbol sym);

    // Non-recursive lookup in this scope only.
    const Symbol* lookup_local(const std::string& name) const;
    Symbol* lookup_local(const std::string& name);

    // Recursive lookup walking enclosing parent scopes.
    const Symbol* lookup(const std::string& name) const;
    Symbol* lookup(const std::string& name);

    Scope* add_child(std::string label);
    const std::vector<Symbol>& symbols() const { return m_symbols; }
    const std::vector<std::unique_ptr<Scope>>& children() const { return m_children; }

private:
    std::string m_label;
    int m_depth;
    Scope* m_parent;
    std::vector<Symbol> m_symbols;
    std::unordered_map<std::string, size_t> m_index_by_name;
    std::vector<std::unique_ptr<Scope>> m_children;
};

/**
 * Tree-backed lexical symbol table supporting scope entry/exit, shadowing,
 * per-routine local offsets, and deterministic formatted dumps.
 */
class SymbolTable {
public:
    SymbolTable();

    void reset();

    Scope* global_scope() const { return m_global.get(); }
    Scope* current_scope() const { return m_current; }

    // Enter a nested lexical scope under the current scope.
    Scope* enter_scope(const std::string& label, bool reset_frame_offset = false);

    // Return to the enclosing parent scope.
    void exit_scope();

    // Declare a symbol in the current scope. Returns false on same-scope duplicate.
    bool declare(Symbol sym);

    // Look up in the current scope only.
    const Symbol* lookup_local(const std::string& name) const;

    // Look up in current and enclosing scopes.
    const Symbol* lookup(const std::string& name) const;

    void dump(std::ostream& out) const;

private:
    struct SavedFrame {
        size_t offset;
        bool restore_on_exit;
    };

    std::unique_ptr<Scope> m_global;
    Scope* m_current = nullptr;
    size_t m_next_offset = 0;
    std::vector<SavedFrame> m_saved_offsets;

    static void dump_scope(std::ostream& out, const Scope& scope, int indent);
};

const char* mutability_to_string(Mutability mut);
const char* symbol_kind_to_string(SymbolKind kind);

} // namespace nova

#endif // NOVA_SEMANTIC_SYMBOL_HPP
