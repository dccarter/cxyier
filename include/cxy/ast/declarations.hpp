#pragma once

#include "cxy/ast/node.hpp"
#include "cxy/arena_allocator.hpp"
#include "cxy/flags.hpp"
#include "cxy/token.hpp"
#include <format>

namespace cxy::ast {

/**
 * @brief Base class for all declaration nodes.
 *
 * Declarations represent language constructs that introduce new names
 * into the current scope (variables, functions, types, etc.).
 */
class DeclarationNode : public ASTNode {
public:
  explicit DeclarationNode(NodeKind k, Location loc, ArenaAllocator &arena)
      : ASTNode(k, loc, arena) {}
};

/**
 * @brief Variable declaration node.
 *
 * Represents variable declarations with optional type annotations and initializers.
 * Supports multiple variable names for destructuring and compile-time generation.
 * 
 * Examples:
 *   var x: i32 = 42;           // Single variable with type and initializer
 *   var y = getValue();        // Type inference with initializer
 *   var z: String;             // Type annotation without initializer
 *   var a, b, c = getTuple();  // Multiple variables (destructuring)
 *   const PI = 3.14;           // Constant declaration
 */
class VariableDeclarationNode : public DeclarationNode {
public:
  ArenaVector<ASTNode*> names;      ///< Variable names (identifiers or patterns)
  ASTNode* type = nullptr;          ///< Optional type annotation
  ASTNode* initializer = nullptr;   ///< Optional initializer expression

  explicit VariableDeclarationNode(Location loc, ArenaAllocator &arena, bool isConstant = false)
      : DeclarationNode(astVariableDeclaration, loc, arena), 
        names(ArenaSTLAllocator<ASTNode*>(arena)) {
    if (isConstant) {
      flags |= flgConst;
    }
  }

  /**
   * @brief Add a variable name to this declaration.
   * 
   * @param name The identifier or pattern node for the variable name
   */
  void addName(ASTNode* name) {
    if (name) {
      names.push_back(name);
      addChild(name);
    }
  }

  /**
   * @brief Set the type annotation for this declaration.
   * 
   * @param typeNode The type expression node (can be nullptr for type inference)
   */
  void setType(ASTNode* typeNode) {
    if (type) {
      removeChild(type);
    }
    type = typeNode;
    if (type) {
      addChild(type);
    }
  }

  /**
   * @brief Set the initializer expression for this declaration.
   * 
   * @param expr The initializer expression (can be nullptr for uninitialized)
   */
  void setInitializer(ASTNode* expr) {
    if (initializer) {
      removeChild(initializer);
    }
    initializer = expr;
    if (initializer) {
      addChild(initializer);
    }
  }

  bool isConst() const {
    return hasAnyFlag(flgConst);
  }

  std::format_context::iterator toString(std::format_context &ctx) const override {
    auto it = std::format_to(ctx.out(), "{}Decl(", isConst() ? "Const" : "Var");
    
    // Print variable names
    if (names.empty()) {
      it = std::format_to(it, "[]");
    } else {
      it = std::format_to(it, "[");
      for (size_t i = 0; i < names.size(); ++i) {
        if (i > 0) it = std::format_to(it, ", ");
        it = std::format_to(it, "{}", *names[i]);
      }
      it = std::format_to(it, "]");
    }

    // Print type annotation if present
    if (type) {
      it = std::format_to(it, ": {}", *type);
    }

    // Print initializer if present
    if (initializer) {
      it = std::format_to(it, " = {}", *initializer);
    }

    return std::format_to(it, ")");
  }
};

// Declaration creation helpers
inline VariableDeclarationNode *createVariableDeclaration(Location loc, ArenaAllocator &arena, bool isConst = false) {
  return arena.construct<VariableDeclarationNode>(loc, arena, isConst);
}

} // namespace cxy::ast