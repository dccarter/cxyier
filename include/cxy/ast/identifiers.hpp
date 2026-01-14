#pragma once

#include "cxy/ast/node.hpp"
#include "cxy/strings.hpp"
#include <format>

namespace cxy::ast {

/**
 * @brief Simple identifier node.
 *
 * Represents a simple identifier like `name`, `variable`, `function`.
 * Contains a direct reference to the declaration it resolves to.
 */
class IdentifierNode : public ASTNode {
public:
  InternedString name;             ///< Identifier name
  ASTNode *resolvedNode = nullptr; ///< Points to declaration node

  explicit IdentifierNode(InternedString identifier_name, Location loc,
                          ArenaAllocator &arena)
      : ASTNode(astIdentifier, loc, arena), name(identifier_name) {}

  std::format_context::iterator
  toString(std::format_context &ctx) const override {
    return std::format_to(ctx.out(), "Identifier({})", name.view());
  }
};

/**
 * @brief Qualified path node.
 *
 * Represents qualified paths like `Hello.age`, `std.vector.size`,
 * or `MyClass<i32>.method`. Contains segments with optional generic arguments.
 */
class QualifiedPathNode : public ASTNode {
public:
  struct Segment {
    InternedString name;         ///< Segment name
    ArenaVector<ASTNode *> args; ///< Generic arguments (optional)

    explicit Segment(InternedString segment_name, ArenaAllocator &arena)
        : name(segment_name), args(ArenaSTLAllocator<ASTNode *>(arena)) {}
  };

  ArenaVector<Segment> segments;   ///< Path segments
  ASTNode *resolvedNode = nullptr; ///< Points to what this resolves to

  explicit QualifiedPathNode(Location loc, ArenaAllocator &arena)
      : ASTNode(astQualifiedPath, loc, arena),
        segments(ArenaSTLAllocator<Segment>(arena)) {}

  void addSegment(InternedString name) {
    segments.emplace_back(name, *segments.get_allocator().getArena());
  }

  void addSegment(InternedString name, ArenaVector<ASTNode *> &&args) {
    segments.emplace_back(name, *segments.get_allocator().getArena());
    segments.back().args = std::move(args);
    // Add arguments as children
    for (ASTNode *arg : segments.back().args) {
      if (arg) {
        addChild(arg);
      }
    }
  }

  std::format_context::iterator
  toString(std::format_context &ctx) const override {
    auto out = std::format_to(ctx.out(), "QualifiedPath(");
    for (size_t i = 0; i < segments.size(); ++i) {
      if (i > 0)
        out = std::format_to(out, ".");

      out = std::format_to(out, "{}", segments[i].name.view());

      if (!segments[i].args.empty()) {
        out = std::format_to(out, "<");
        for (size_t j = 0; j < segments[i].args.size(); ++j) {
          if (j > 0)
            out = std::format_to(out, ", ");
          out = std::format_to(out, "{}",
                               segments[i].args[j]
                                   ? std::format("{}", *segments[i].args[j])
                                   : "null");
        }
        out = std::format_to(out, ">");
      }
    }
    return std::format_to(out, ")");
  }
};

// Helper functions for creating identifier nodes

/**
 * @brief Create a simple identifier node.
 */
inline IdentifierNode *createIdentifier(InternedString name, Location loc,
                                        ArenaAllocator &arena) {
  return arena.construct<IdentifierNode>(name, loc, arena);
}

/**
 * @brief Create a qualified path node.
 */
inline QualifiedPathNode *createQualifiedPath(Location loc,
                                              ArenaAllocator &arena) {
  return arena.construct<QualifiedPathNode>(loc, arena);
}

} // namespace cxy::ast
