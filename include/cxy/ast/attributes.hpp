#pragma once

#include "cxy/arena_stl.hpp"
#include "cxy/ast/node.hpp"
#include "cxy/strings.hpp"
#include <format>

namespace cxy::ast {

// Forward declarations
class FieldExpressionNode;

/**
 * @brief AST node representing a single attribute.
 *
 * Attributes have the format:
 * - @Name
 * - @Name(value1, value2, ...)
 * - @Name(name1: value1, name2: value2, ...)
 *
 * Where values are always literals and named parameters use field expressions.
 */
class AttributeNode : public ASTNode {
public:
  InternedString name; ///< Attribute name
  ArenaVector<ASTNode *>
      args; ///< Arguments (either literals or field expressions)

  /**
   * @brief Construct an attribute node.
   *
   * @param attributeName Name of the attribute
   * @param loc Source location
   * @param arena Arena allocator
   */
  AttributeNode(InternedString attributeName, Location loc,
                ArenaAllocator &arena)
      : ASTNode(astAttribute, loc, arena), name(attributeName),
        args(ArenaSTLAllocator<ASTNode *>(arena)) {}

  /**
   * @brief Add an argument (literal or field expression).
   *
   * @param arg Argument node to add
   */
  void addArg(ASTNode *arg) {
    if (arg) {
      args.push_back(arg);
      addChild(arg);
    }
  }

  /**
   * @brief Check if attribute has any arguments.
   */
  bool hasParameters() const { return !args.empty(); }

  /**
   * @brief Get number of arguments.
   */
  size_t getArgCount() const { return args.size(); }

  /**
   * @brief Convert to string representation.
   */
  std::format_context::iterator
  toString(std::format_context &ctx) const override {
    auto out = std::format_to(ctx.out(), "Attribute({})", name.view());
    if (hasParameters()) {
      out = std::format_to(out, " with {} args", args.size());
    }
    return out;
  }
};

/**
 * @brief AST node representing a list of attributes.
 *
 * This is used to group multiple attributes together, typically
 * appearing before declarations or expressions.
 */
class AttributeListNode : public ASTNode {
public:
  ArenaVector<AttributeNode *> attributes; ///< List of attributes

  /**
   * @brief Construct an attribute list node.
   *
   * @param loc Source location
   * @param arena Arena allocator
   */
  AttributeListNode(Location loc, ArenaAllocator &arena)
      : ASTNode(astAttributeList, loc, arena),
        attributes(ArenaSTLAllocator<AttributeNode *>(arena)) {}

  /**
   * @brief Add an attribute to the list.
   *
   * @param attr Attribute node to add
   */
  void addAttribute(AttributeNode *attr) {
    if (attr) {
      attributes.push_back(attr);
      addChild(attr);
    }
  }

  /**
   * @brief Remove an attribute from the list.
   *
   * @param attr Attribute node to remove
   * @return true if attribute was found and removed
   */
  bool removeAttribute(AttributeNode *attr) {
    auto it = std::find(attributes.begin(), attributes.end(), attr);
    if (it != attributes.end()) {
      attributes.erase(it);
      removeChild(attr);
      return true;
    }
    return false;
  }

  /**
   * @brief Get attribute by index.
   *
   * @param index Attribute index
   * @return Attribute node or nullptr if index out of bounds
   */
  AttributeNode *getAttribute(size_t index) const {
    return index < attributes.size() ? attributes[index] : nullptr;
  }

  /**
   * @brief Get number of attributes.
   */
  size_t getAttributeCount() const { return attributes.size(); }

  /**
   * @brief Check if list has any attributes.
   */
  bool hasAttributes() const { return !attributes.empty(); }

  /**
   * @brief Find attribute by name.
   *
   * @param name Attribute name to search for
   * @return First attribute with matching name, or nullptr if not found
   */
  AttributeNode *findAttribute(const InternedString &name) const {
    for (AttributeNode *attr : attributes) {
      if (attr->name == name) {
        return attr;
      }
    }
    return nullptr;
  }

  /**
   * @brief Convert to string representation.
   */
  std::format_context::iterator
  toString(std::format_context &ctx) const override {
    return std::format_to(ctx.out(), "AttributeList({} attributes)",
                          attributes.size());
  }
};

// Helper functions for creating attribute nodes

/**
 * @brief Create a simple attribute with just a name.
 *
 * @param name Attribute name
 * @param loc Source location
 * @param arena Arena allocator
 * @return New attribute node
 */
inline AttributeNode *createAttribute(InternedString name, Location loc,
                                      ArenaAllocator &arena) {
  return arena.construct<AttributeNode>(name, loc, arena);
}

/**
 * @brief Create an attribute list.
 *
 * @param loc Source location
 * @param arena Arena allocator
 * @return New attribute list node
 */
inline AttributeListNode *createAttributeList(Location loc,
                                              ArenaAllocator &arena) {
  return arena.construct<AttributeListNode>(loc, arena);
}

} // namespace cxy::ast
