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
class PathSegmentNode : public ASTNode {
public:
  InternedString name;         ///< Segment name
  ArenaVector<ASTNode *> args; ///< Generic arguments (optional)
  ASTNode *resolvedNode = nullptr; ///< Points to what this resolves to

  explicit PathSegmentNode(InternedString segment_name, Location loc, ArenaAllocator &arena)
      : ASTNode(astPathSegment, loc, arena), name(segment_name),
        args(ArenaSTLAllocator<ASTNode *>(arena)) {}

  void addGenericArg(ASTNode *arg) {
    if (arg) {
      args.push_back(arg);
      addChild(arg);
    }
  }

  std::format_context::iterator toString(std::format_context &ctx) const override {
    auto out = std::format_to(ctx.out(), "PathSegment({})", name.view());
    if (!args.empty()) {
      out = std::format_to(out, "<");
      for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0) out = std::format_to(out, ", ");
        if (args[i]) {
          out = std::format_to(out, "{}", *args[i]);
        } else {
          out = std::format_to(out, "null");
        }
      }
      out = std::format_to(out, ">");
    }
    return std::format_to(out, ")");
  }
};

class QualifiedPathNode : public ASTNode {
public:
  ArenaVector<PathSegmentNode *> segments; ///< Path segments

  explicit QualifiedPathNode(Location loc, ArenaAllocator &arena)
      : ASTNode(astQualifiedPath, loc, arena),
        segments(ArenaSTLAllocator<PathSegmentNode *>(arena)) {}

  void addSegment(PathSegmentNode *segment) {
    if (segment) {
      segments.push_back(segment);
      addChild(segment);
    }
  }

  void addSegment(InternedString name, Location loc, ArenaAllocator &arena) {
    auto *segment = arena.construct<PathSegmentNode>(name, loc, arena);
    addSegment(segment);
  }

  void addSegment(InternedString name, Location loc, ArenaVector<ASTNode *> &&args, ArenaAllocator &arena) {
    auto *segment = arena.construct<PathSegmentNode>(name, loc, arena);
    for (auto *arg : args) {
      segment->addGenericArg(arg);
    }
    addSegment(segment);
  }

  std::format_context::iterator
  toString(std::format_context &ctx) const override {
    auto out = std::format_to(ctx.out(), "QualifiedPath(");
    for (size_t i = 0; i < segments.size(); ++i) {
      if (i > 0) out = std::format_to(out, ".");
      if (segments[i]) {
        out = segments[i]->toString(ctx);
      } else {
        out = std::format_to(out, "null");
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
inline PathSegmentNode *createPathSegment(InternedString name, Location loc,
                                          ArenaAllocator &arena) {
  return arena.construct<PathSegmentNode>(name, loc, arena);
}

inline QualifiedPathNode *createQualifiedPath(Location loc,
                                              ArenaAllocator &arena) {
  return arena.construct<QualifiedPathNode>(loc, arena);
}

} // namespace cxy::ast
