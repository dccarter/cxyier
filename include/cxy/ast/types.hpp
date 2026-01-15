#pragma once

#include "cxy/ast/node.hpp"
#include "cxy/token.hpp"
#include <format>

namespace cxy::ast {

/**
 * @brief Primitive type node.
 *
 * Represents primitive types like i32, f64, bool, string, etc.
 * Used in type annotations and cast expressions.
 */
class PrimitiveTypeNode : public ASTNode {
public:
  TokenKind typeKind; ///< The primitive type token kind (I32, F64, Bool, etc.)

  explicit PrimitiveTypeNode(TokenKind type_kind, Location loc,
                             ArenaAllocator &arena)
      : ASTNode(astPrimitiveType, loc, arena), typeKind(type_kind) {}

  std::format_context::iterator
  toString(std::format_context &ctx) const override {
    return std::format_to(ctx.out(), "Type({})", tokenKindToString(typeKind));
  }
};

/**
 * @brief Create a primitive type node.
 */
inline PrimitiveTypeNode *createPrimitiveType(TokenKind typeKind, Location loc,
                                              ArenaAllocator &arena) {
  return arena.construct<PrimitiveTypeNode>(typeKind, loc, arena);
}

} // namespace cxy::ast
