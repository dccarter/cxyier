#pragma once

#include "cxy/arena_allocator.hpp"
#include "cxy/arena_stl.hpp"
#include "cxy/diagnostics.hpp"
#include "cxy/strings.hpp"
#include <unordered_map>
#include <memory>

namespace cxy {
namespace ast {
    class ASTNode;  // Forward declaration
}

namespace symbols {

/**
 * @brief Internal symbol representation within a scope.
 * 
 * This is an implementation detail and should not be exposed in the public API.
 * Clients receive AST nodes directly from lookup operations.
 */
class Symbol {
private:
    uint16_t index_;                    ///< Unique index within the scope
    InternedString name_;               ///< Symbol name (interned for efficiency)
    const ast::ASTNode* declaration_;   ///< Declaration AST node
    const ast::ASTNode* lastReference_; ///< Last reference for usage tracking

public:
    /**
     * @brief Construct a symbol.
     * 
     * @param index Unique index within the scope
     * @param name Interned symbol name
     * @param declaration Declaration AST node
     */
    Symbol(uint16_t index, InternedString name, const ast::ASTNode* declaration);

    // Accessors
    [[nodiscard]] uint16_t getIndex() const { return index_; }
    [[nodiscard]] const InternedString& getName() const { return name_; }
    [[nodiscard]] const ast::ASTNode* getDeclaration() const { return declaration_; }
    [[nodiscard]] const ast::ASTNode* getLastReference() const { return lastReference_; }

    // Reference tracking
    void updateLastReference(const ast::ASTNode* reference) { lastReference_ = reference; }

    // Equality and hashing (for hash table storage)
    [[nodiscard]] bool operator==(const Symbol& other) const;
    [[nodiscard]] bool operator!=(const Symbol& other) const { return !(*this == other); }
};

/**
 * @brief Represents a lexical scope containing symbols.
 * 
 * Scopes are organized in a hierarchical chain reflecting the lexical
 * structure of the source code. Each scope maintains its own symbol table
 * for efficient lookup within the scope.
 */
class Scope {
public:
    /// Hash table type for symbol storage
    using SymbolHashTable = std::unordered_map<InternedString, std::unique_ptr<Symbol>, 
                                          InternedString::Hash, std::equal_to<InternedString>>;

private:
    SymbolHashTable symbolHashTable_;       ///< Hash table of symbols in this scope
    const ast::ASTNode* node_;      ///< AST node that created this scope
    Scope* parent_;                 ///< Parent scope (nullptr for global scope)
    uint64_t level_;                ///< Scope nesting level (0 for global)
    uint16_t nextIndex_;            ///< Next available symbol index
    
public:
    ArenaVector<std::unique_ptr<Scope>> children_;  ///< Child scopes allocated in arena
    /**
     * @brief Construct a scope.
     * 
     * @param node AST node that created this scope
     * @param parent Parent scope (nullptr for global scope)
     * @param level Scope nesting level
     * @param arena Arena allocator for child scope management
     */
    Scope(const ast::ASTNode* node, Scope* parent, uint64_t level, ArenaAllocator& arena);

    /**
     * @brief Destructor.
     */
    ~Scope() = default;

    // Accessors
    [[nodiscard]] const ast::ASTNode* getNode() const { return node_; }
    [[nodiscard]] Scope* getParent() const { return parent_; }
    [[nodiscard]] uint64_t getLevel() const { return level_; }
    [[nodiscard]] size_t getSymbolCount() const { return symbolHashTable_.size(); }

    // Symbol operations
    /**
     * @brief Define a new symbol in this scope.
     * 
     * @param name Symbol name
     * @param declaration Declaration AST node
     * @return Pointer to created symbol, or nullptr if symbol already exists
     */
    Symbol* defineSymbol(const InternedString& name, const ast::ASTNode* declaration);

    /**
     * @brief Lookup a symbol in this scope only.
     * 
     * @param name Symbol name to lookup
     * @return Pointer to symbol if found, nullptr otherwise
     */
    [[nodiscard]] Symbol* lookupLocal(const InternedString& name) const;

    /**
     * @brief Check if a symbol exists in this scope.
     * 
     * @param name Symbol name to check
     * @return true if symbol exists in this scope
     */
    [[nodiscard]] bool hasSymbol(const InternedString& name) const;

    /**
     * @brief Iterate over all symbols in this scope.
     * 
     * @param callback Function to call for each symbol
     */
    void iterateSymbols(const std::function<void(const Symbol*)>& callback) const;

    // Disable copying and moving
    Scope(const Scope&) = delete;
    Scope& operator=(const Scope&) = delete;
    Scope(Scope&&) = delete;
    Scope& operator=(Scope&&) = delete;
};

/**
 * @brief Symbol table environment managing scopes and symbol lookup.
 * 
 * The SymbolTable provides a scope-based hierarchical symbol management system
 * with efficient name resolution and comprehensive error reporting through
 * integration with the diagnostic system.
 */
class SymbolTable {
private:
    DiagnosticLogger& logger_;      ///< Logger for error reporting
    ArenaAllocator& arena_;         ///< Arena for memory allocation
    std::unique_ptr<Scope> globalScope_;  ///< Global scope (root)
    Scope* currentScope_;           ///< Currently active scope

public:
    /**
     * @brief Construct a symbol table.
     * 
     * @param logger Diagnostic logger for error reporting
     * @param arena Arena allocator for memory management
     */
    SymbolTable(DiagnosticLogger& logger, ArenaAllocator& arena);

    /**
     * @brief Destructor.
     */
    ~SymbolTable() = default;

    // Core symbol operations
    /**
     * @brief Define a symbol in the current scope.
     * 
     * @param name Symbol name
     * @param declaration Declaration AST node
     * @param location Source location for error reporting
     * @return true if symbol was successfully defined, false on error
     */
    [[nodiscard]] bool defineSymbol(const InternedString& name, 
                                   const ast::ASTNode* declaration,
                                   const Location& location);

    /**
     * @brief Lookup a symbol in the scope chain.
     * 
     * Searches from the current scope up through the parent chain.
     * 
     * @param name Symbol name to lookup
     * @param location Source location for error reporting
     * @return Declaration AST node if found, nullptr if not found
     */
    [[nodiscard]] const ast::ASTNode* lookupSymbol(const InternedString& name, 
                                                  const Location& location);

    // Scope management
    /**
     * @brief Push a new scope as child of current scope.
     * 
     * @param node AST node that creates this scope
     * @param location Source location for error reporting
     * @return Pointer to the newly created scope
     */
    Scope* pushScope(const ast::ASTNode* node, const Location& location);

    /**
     * @brief Pop the current scope, returning to parent.
     * 
     * @param location Source location for error reporting
     */
    void popScope(const Location& location);

    // Scope queries
    [[nodiscard]] Scope* getCurrentScope() const { return currentScope_; }
    [[nodiscard]] Scope* getGlobalScope() const { return globalScope_.get(); }
    [[nodiscard]] uint64_t getCurrentScopeLevel() const { 
        return currentScope_ ? currentScope_->getLevel() : 0; 
    }

    // Symbol reference tracking
    /**
     * @brief Update symbol reference for usage tracking.
     * 
     * @param name Symbol name
     * @param reference Reference AST node
     * @param location Source location for error reporting
     */
    void updateSymbolReference(const InternedString& name, 
                              const ast::ASTNode* reference,
                              const Location& location);

    // Symbol iteration
    /**
     * @brief Iterate over symbols in accessible scopes.
     * 
     * @param callback Function to call for each symbol's declaration
     * @param currentScopeOnly If true, only iterate current scope
     */
    void iterateSymbols(const std::function<void(const ast::ASTNode*)>& callback,
                       bool currentScopeOnly = false) const;

    // Disable copying and moving
    SymbolTable(const SymbolTable&) = delete;
    SymbolTable& operator=(const SymbolTable&) = delete;
    SymbolTable(SymbolTable&&) = delete;
    SymbolTable& operator=(SymbolTable&&) = delete;

private:
    // Error reporting helpers
    void reportRedefinition(const InternedString& name,
                           const Location& original,
                           const Location& duplicate);

    void reportUndefinedSymbol(const InternedString& name,
                              const Location& location);

    void reportUnusedSymbol(const InternedString& name, 
                           const ast::ASTNode* declaration,
                           const Location& location) const;

    // Symbol lookup helpers
    Symbol* findSymbolInScopeChain(const InternedString& name) const;

    // Scope management helpers
    void reportUnusedSymbolsInScope(Scope* scope) const;
};

} // namespace symbols
} // namespace cxy