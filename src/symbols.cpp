#include "cxy/symbols.hpp"
#include "cxy/ast/node.hpp"
#include <algorithm>

namespace cxy {
namespace symbols {

// Symbol implementation
Symbol::Symbol(uint16_t index, InternedString name, const ast::ASTNode* declaration)
    : index_(index), name_(name), declaration_(declaration), lastReference_(nullptr) {}

bool Symbol::operator==(const Symbol& other) const {
    // Symbols are equal if they have the same name and declaration
    return name_ == other.name_ && declaration_ == other.declaration_;
}

// Scope implementation
Scope::Scope(const ast::ASTNode* node, Scope* parent, uint64_t level, ArenaAllocator& arena)
    : node_(node), parent_(parent), children_(ArenaSTLAllocator<std::unique_ptr<Scope>>(arena)), 
      level_(level), nextIndex_(0) {}

Symbol* Scope::defineSymbol(const InternedString& name, const ast::ASTNode* declaration) {
    // Check if symbol already exists in this scope
    auto it = symbolHashTable_.find(name);
    if (it != symbolHashTable_.end()) {
        return nullptr;  // Symbol already exists
    }
    
    // Create new symbol with sequential index
    auto symbol = std::make_unique<Symbol>(nextIndex_++, name, declaration);
    Symbol* result = symbol.get();
    
    // Insert into hash table
    symbolHashTable_[name] = std::move(symbol);
    
    return result;
}

Symbol* Scope::lookupLocal(const InternedString& name) const {
    auto it = symbolHashTable_.find(name);
    if (it != symbolHashTable_.end()) {
        return it->second.get();
    }
    return nullptr;
}

bool Scope::hasSymbol(const InternedString& name) const {
    return symbolHashTable_.find(name) != symbolHashTable_.end();
}

void Scope::iterateSymbols(const std::function<void(const Symbol*)>& callback) const {
    for (const auto& [name, symbol] : symbolHashTable_) {
        callback(symbol.get());
    }
}

// SymbolTable implementation
SymbolTable::SymbolTable(DiagnosticLogger& logger, ArenaAllocator& arena)
    : logger_(logger), arena_(arena) {
    // Create global scope
    globalScope_ = std::make_unique<Scope>(nullptr, nullptr, 0, arena);
    currentScope_ = globalScope_.get();
}

bool SymbolTable::defineSymbol(const InternedString& name, 
                               const ast::ASTNode* declaration,
                               const Location& location) {
    Symbol* symbol = currentScope_->defineSymbol(name, declaration);
    if (!symbol) {
        // Symbol redefinition error
        Symbol* existing = currentScope_->lookupLocal(name);
        if (existing && existing->getDeclaration()) {
            reportRedefinition(name, existing->getDeclaration()->location, location);
        } else {
            reportRedefinition(name, location, location);
        }
        return false;
    }
    return true;
}

const ast::ASTNode* SymbolTable::lookupSymbol(const InternedString& name, 
                                              const Location& location) {
    Symbol* symbol = findSymbolInScopeChain(name);
    if (!symbol) {
        reportUndefinedSymbol(name, location);
        return nullptr;
    }
    
    // Update last reference for usage tracking
    symbol->updateLastReference(nullptr);  // We don't have the reference node here
    
    return symbol->getDeclaration();
}

Scope* SymbolTable::pushScope(const ast::ASTNode* node, const Location& location) {
    auto newScope = std::make_unique<Scope>(node, currentScope_, currentScope_->getLevel() + 1, arena_);
    Scope* scopePtr = newScope.get();
    
    // Add to parent's children list for proper memory management
    currentScope_->children_.push_back(std::move(newScope));
    currentScope_ = scopePtr;
    
    return scopePtr;
}

void SymbolTable::popScope(const Location& location) {
    if (currentScope_ == globalScope_.get()) {
        logger_.warning("Attempted to pop global scope", location);
        return;
    }
    
    Scope* parentScope = currentScope_->getParent();
    if (!parentScope) {
        logger_.error("Internal error: non-global scope has no parent", location);
        return;
    }
    
    // Report unused symbols in the scope being popped
    reportUnusedSymbolsInScope(currentScope_);
    
    // Return to parent scope (child scope remains in parent's children vector)
    currentScope_ = parentScope;
}

void SymbolTable::updateSymbolReference(const InternedString& name, 
                                        const ast::ASTNode* reference,
                                        const Location& location) {
    Symbol* symbol = findSymbolInScopeChain(name);
    if (symbol) {
        symbol->updateLastReference(reference);
    }
}

void SymbolTable::iterateSymbols(const std::function<void(const ast::ASTNode*)>& callback,
                                 bool currentScopeOnly) const {
    if (currentScopeOnly) {
        currentScope_->iterateSymbols([&callback](const Symbol* symbol) {
            callback(symbol->getDeclaration());
        });
    } else {
        // Iterate through all accessible scopes
        Scope* scope = currentScope_;
        while (scope) {
            scope->iterateSymbols([&callback](const Symbol* symbol) {
                callback(symbol->getDeclaration());
            });
            scope = scope->getParent();
        }
    }
}

// Private helper methods
Symbol* SymbolTable::findSymbolInScopeChain(const InternedString& name) const {
    Scope* scope = currentScope_;
    while (scope) {
        Symbol* symbol = scope->lookupLocal(name);
        if (symbol) {
            return symbol;
        }
        scope = scope->getParent();
    }
    return nullptr;
}

void SymbolTable::reportRedefinition(const InternedString& name,
                                     const Location& original,
                                     const Location& duplicate) {
    logger_.error(duplicate, "Redefinition of symbol '{}'", name.toString());
    logger_.info(original, "Previous definition was here");
}

void SymbolTable::reportUndefinedSymbol(const InternedString& name,
                                        const Location& location) {
    logger_.error(location, "Use of undeclared identifier '{}'", name.toString());
    
    // TODO: Implement suggestion engine for similar names
}

void SymbolTable::reportUnusedSymbol(const InternedString& name, 
                                     const ast::ASTNode* declaration,
                                     const Location& location) const {
    if (declaration) {
        const_cast<DiagnosticLogger&>(logger_).warning(declaration->location, "Unused symbol '{}'", name.toString());
    } else {
        const_cast<DiagnosticLogger&>(logger_).warning(location, "Unused symbol '{}'", name.toString());
    }
}

void SymbolTable::reportUnusedSymbolsInScope(Scope* scope) const {
    if (!scope) return;
    
    scope->iterateSymbols([this](const Symbol* symbol) {
        // Only report unused symbols if they have no last reference
        if (!symbol->getLastReference()) {
            // Create a dummy location for the warning since we don't have the reference location
            Location dummyLocation("", Position(0, 0, 0));
            reportUnusedSymbol(symbol->getName(), symbol->getDeclaration(), dummyLocation);
        }
    });
}

} // namespace symbols
} // namespace cxy