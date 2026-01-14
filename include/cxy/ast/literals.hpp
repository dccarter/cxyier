#pragma once

#include "cxy/ast/node.hpp"
#include "cxy/strings.hpp"
#include <format>
#include <string>

namespace cxy::ast {

/**
 * @brief Base class for all literal nodes.
 *
 * Literals represent compile-time constant values in the source code.
 * They carry their value directly in the AST node for easy access
 * during compilation phases.
 */
class LiteralNode : public ASTNode {
public:
  explicit LiteralNode(NodeKind k, Location loc, ArenaAllocator &arena)
      : ASTNode(k, loc, arena) {}
};

/**
 * @brief Boolean literal node (true/false).
 *
 * Represents boolean literals like `true` and `false`.
 */
class BoolLiteralNode : public LiteralNode {
public:
  bool value;

  explicit BoolLiteralNode(bool val, Location loc, ArenaAllocator &arena)
      : LiteralNode(astBool, loc, arena), value(val) {}

  std::format_context::iterator
  toString(std::format_context &ctx) const override {
    return std::format_to(ctx.out(), "Bool({})", value ? "true" : "false");
  }
};

/**
 * @brief Integer literal node.
 *
 * Represents integer literals like `42`, `0xFF`, `0b1010`, etc.
 * Uses 128-bit signed integer to handle all integer sizes including large
 * literals.
 */
class IntLiteralNode : public LiteralNode {
public:
  __int128 value;

  explicit IntLiteralNode(__int128 val, Location loc, ArenaAllocator &arena)
      : LiteralNode(astInt, loc, arena), value(val) {}

  std::format_context::iterator
  toString(std::format_context &ctx) const override {
    return std::format_to(ctx.out(), "Int({})", static_cast<long long>(value));
  }
};

/**
 * @brief Floating-point literal node.
 *
 * Represents floating-point literals like `3.14`, `1.0e10`, etc.
 * Uses double precision for maximum range and precision.
 */
class FloatLiteralNode : public LiteralNode {
public:
  double value;

  explicit FloatLiteralNode(double val, Location loc, ArenaAllocator &arena)
      : LiteralNode(astFloat, loc, arena), value(val) {}

  std::format_context::iterator
  toString(std::format_context &ctx) const override {
    return std::format_to(ctx.out(), "Float({})", value);
  }
};

/**
 * @brief String literal node.
 *
 * Represents string literals like `"hello"`, `"world\n"`, etc.
 * Uses interned strings for memory efficiency and fast comparisons.
 */
class StringLiteralNode : public LiteralNode {
public:
  InternedString value;

  explicit StringLiteralNode(InternedString val, Location loc,
                             ArenaAllocator &arena)
      : LiteralNode(astString, loc, arena), value(val) {}

  std::format_context::iterator
  toString(std::format_context &ctx) const override {
    return std::format_to(ctx.out(), "String(\"{}\")", value.view());
  }
};

/**
 * @brief Character literal node.
 *
 * Represents character literals like `'a'`, `'\n'`, `'\xFF'`, etc.
 * Uses 32-bit unsigned integer to support Unicode code points.
 */
class CharLiteralNode : public LiteralNode {
public:
  uint32_t value; // Unicode code point

  explicit CharLiteralNode(uint32_t val, Location loc, ArenaAllocator &arena)
      : LiteralNode(astChar, loc, arena), value(val) {}

  std::format_context::iterator
  toString(std::format_context &ctx) const override {
    if (value >= 32 && value <= 126) {
      return std::format_to(ctx.out(), "Char('{}')", static_cast<char>(value));
    } else {
      return std::format_to(ctx.out(), "Char('\\u{{{:04x}}}')", value);
    }
  }
};

/**
 * @brief Null literal node.
 *
 * Represents null pointer literals like `null` or `nullptr`.
 * No value needed - the existence of the node represents null.
 */
class NullLiteralNode : public LiteralNode {
public:
  explicit NullLiteralNode(Location loc, ArenaAllocator &arena)
      : LiteralNode(astNull, loc, arena) {}

  std::format_context::iterator
  toString(std::format_context &ctx) const override {
    return std::format_to(ctx.out(), "Null()");
  }
};

// Helper functions for creating literal nodes

/**
 * @brief Create a boolean literal node.
 */
inline BoolLiteralNode *createBoolLiteral(bool value, Location loc,
                                          ArenaAllocator &arena) {
  return arena.construct<BoolLiteralNode>(value, loc, arena);
}

/**
 * @brief Create an integer literal node.
 */
inline IntLiteralNode *createIntLiteral(__int128 value, Location loc,
                                        ArenaAllocator &arena) {
  return arena.construct<IntLiteralNode>(value, loc, arena);
}

/**
 * @brief Create a floating-point literal node.
 */
inline FloatLiteralNode *createFloatLiteral(double value, Location loc,
                                            ArenaAllocator &arena) {
  return arena.construct<FloatLiteralNode>(value, loc, arena);
}

/**
 * @brief Create a string literal node.
 */
inline StringLiteralNode *
createStringLiteral(InternedString value, Location loc, ArenaAllocator &arena) {
  return arena.construct<StringLiteralNode>(value, loc, arena);
}

/**
 * @brief Create a character literal node.
 */
inline CharLiteralNode *createCharLiteral(uint32_t value, Location loc,
                                          ArenaAllocator &arena) {
  return arena.construct<CharLiteralNode>(value, loc, arena);
}

/**
 * @brief Create a null literal node.
 */
inline NullLiteralNode *createNullLiteral(Location loc, ArenaAllocator &arena) {
  return arena.construct<NullLiteralNode>(loc, arena);
}

} // namespace cxy::ast
