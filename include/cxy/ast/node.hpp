#pragma once

#include "cxy/memory/arena_stl.hpp"
#include "cxy/ast/kind.hpp"
#include "cxy/diagnostics.hpp"
#include "cxy/flags.hpp"
#include "cxy/memory.hpp"
#include <any>
#include <format>
#include <string>

// Forward declaration for semantic type system
namespace cxy {
  class Type;
}

namespace cxy::ast {

/**
 * @brief Base class for all AST nodes in the Cxy compiler.
 *
 * This class implements the progressive enhancement design philosophy:
 * - Nodes start simple with just syntax information
 * - Semantic information (types, symbols) added by compiler passes
 * - Memory efficient with arena allocation
 * - Flags track compiler pass state
 *
 * Key design principles:
 * - Raw pointers used with arena allocation (arena owns lifetime)
 * - Parent-child relationships maintained automatically
 * - Progressive enhancement through type and flags fields
 * - Metadata map for pass-specific information
 */
class ASTNode {
public:
  // Core node information (always present)
  NodeKind kind;             ///< Type of AST node
  Location location;         ///< Source location for diagnostics
  ASTNode *parent = nullptr; ///< Parent node (for tree traversal)

  // Arena-allocated children storage
  ArenaVector<ASTNode *> children;

  // Arena-allocated attributes storage
  ArenaVector<ASTNode *> attrs;

  // Progressive enhancement fields (filled by semantic passes)
  const cxy::Type *type = nullptr;  ///< Semantic type information (from type system)
  Flags flags = flgNone; ///< Compiler pass flags

  // Optional metadata for pass-specific information
  ArenaMap<std::string, std::any> metadata;

  /**
   * @brief Construct a new AST node.
   *
   * @param k Node kind
   * @param loc Source location
   * @param arena Arena allocator for children and metadata
   */
  explicit ASTNode(NodeKind k, Location loc, ArenaAllocator &arena)
      : kind(k), location(loc), children(ArenaSTLAllocator<ASTNode *>(arena)),
        attrs(ArenaSTLAllocator<ASTNode *>(arena)),
        metadata(
            ArenaSTLAllocator<std::pair<const std::string, std::any>>(arena)) {}

  virtual ~ASTNode() = default;

  // Disable copy/move since arena manages lifetime
  ASTNode(const ASTNode &) = delete;
  ASTNode &operator=(const ASTNode &) = delete;
  ASTNode(ASTNode &&) = delete;
  ASTNode &operator=(ASTNode &&) = delete;

  /**
   * @brief Add a child node.
   *
   * Automatically sets the parent relationship. The arena manages
   * the lifetime of both parent and child nodes.
   *
   * @param child Child node to add (must be arena-allocated)
   */
  void addChild(ASTNode *child) {
    if (child) {
      child->parent = this;
      children.push_back(child);
    }
  }

  /**
   * @brief Remove a child node.
   *
   * Clears the parent relationship. Note: this doesn't deallocate
   * the child node since arena manages memory.
   *
   * @param child Child node to remove
   * @return true if child was found and removed
   */
  bool removeChild(ASTNode *child) {
    auto it = std::find(children.begin(), children.end(), child);
    if (it != children.end()) {
      (*it)->parent = nullptr;
      children.erase(it);
      return true;
    }
    return false;
  }

  /**
   * @brief Get child at specific index.
   *
   * @param index Child index
   * @return Child node or nullptr if index out of bounds
   */
  ASTNode *getChild(size_t index) const {
    return index < children.size() ? children[index] : nullptr;
  }

  /**
   * @brief Get first child node.
   * @return First child or nullptr if no children
   */
  ASTNode *getFirstChild() const {
    return children.empty() ? nullptr : children.front();
  }

  /**
   * @brief Get last child node.
   * @return Last child or nullptr if no children
   */
  ASTNode *getLastChild() const {
    return children.empty() ? nullptr : children.back();
  }

  /**
   * @brief Get number of children.
   * @return Child count
   */
  size_t getChildCount() const { return children.size(); }

  /**
   * @brief Check if node has children.
   * @return true if node has any children
   */
  bool hasChildren() const { return !children.empty(); }

  // Attribute management methods

  /**
   * @brief Add an attribute node.
   *
   * Attributes are stored separately from children to allow for
   * cleaner separation of syntax structure from metadata.
   *
   * @param attr Attribute node to add (must be arena-allocated)
   */
  void addAttribute(ASTNode *attr) {
    if (attr) {
      attrs.push_back(attr);
    }
  }

  /**
   * @brief Remove an attribute node.
   *
   * @param attr Attribute node to remove
   * @return true if attribute was found and removed
   */
  bool removeAttribute(ASTNode *attr) {
    auto it = std::find(attrs.begin(), attrs.end(), attr);
    if (it != attrs.end()) {
      attrs.erase(it);
      return true;
    }
    return false;
  }

  /**
   * @brief Get attribute at specific index.
   *
   * @param index Attribute index
   * @return Attribute node or nullptr if index out of bounds
   */
  ASTNode *getAttribute(size_t index) const {
    return index < attrs.size() ? attrs[index] : nullptr;
  }

  /**
   * @brief Get number of attributes.
   * @return Attribute count
   */
  size_t getAttributeCount() const { return attrs.size(); }

  /**
   * @brief Check if node has attributes.
   * @return true if node has any attributes
   */
  bool hasAttributes() const { return !attrs.empty(); }

  // Flag management methods

  /**
   * @brief Check if a specific flag is set.
   * @param flag Flag to check
   * @return true if flag is set
   */
  bool hasFlag(Flags flag) const { return (flags & flag) != flgNone; }

  /**
   * @brief Check if any of the specified flags are set.
   * @param mask Flags to check
   * @return true if any flags in mask are set
   */
  bool hasAnyFlag(Flags mask) const { return cxy::hasAnyFlag(flags, mask); }

  /**
   * @brief Check if all of the specified flags are set.
   * @param mask Flags to check
   * @return true if all flags in mask are set
   */
  bool hasAllFlags(Flags mask) const { return cxy::hasAllFlags(flags, mask); }

  /**
   * @brief Set a flag.
   * @param flag Flag to set
   */
  void setFlag(Flags flag) { flags |= flag; }

  /**
   * @brief Clear a flag.
   * @param flag Flag to clear
   */
  void clearFlag(Flags flag) { flags &= ~flag; }

  /**
   * @brief Toggle a flag.
   * @param flag Flag to toggle
   */
  void toggleFlag(Flags flag) { flags ^= flag; }

  /**
   * @brief Set multiple flags at once.
   * @param newFlags Flags to set
   */
  void setFlags(Flags newFlags) { flags |= newFlags; }

  /**
   * @brief Clear all flags.
   */
  void clearAllFlags() { flags = flgNone; }

  // Metadata access methods

  /**
   * @brief Check if metadata key exists.
   * @param key Metadata key
   * @return true if key exists
   */
  bool hasMetadata(const std::string &key) const {
    return metadata.contains(key);
  }

  /**
   * @brief Get metadata value.
   * @tparam T Type to cast metadata to
   * @param key Metadata key
   * @return Pointer to value or nullptr if not found/wrong type
   */
  template <typename T> const T *getMetadata(const std::string &key) const {
    auto it = metadata.find(key);
    if (it != metadata.end()) {
      try {
        return std::any_cast<T>(&it->second);
      } catch (const std::bad_any_cast &) {
        return nullptr;
      }
    }
    return nullptr;
  }

  /**
   * @brief Set metadata value.
   * @tparam T Type of value
   * @param key Metadata key
   * @param value Metadata value
   */
  template <typename T> void setMetadata(const std::string &key, T &&value) {
    metadata[key] = std::forward<T>(value);
  }

  /**
   * @brief Remove metadata entry.
   * @param key Metadata key to remove
   * @return true if key was found and removed
   */
  bool removeMetadata(const std::string &key) {
    return metadata.erase(key) > 0;
  }

  /**
   * @brief Clear all metadata.
   */
  void clearMetadata() { metadata.clear(); }

  /**
   * @brief Convert node to string representation.
   *
   * Override in derived classes for custom formatting.
   * Default implementation shows node kind and memory address.
   *
   * @param ctx Format context to write to
   * @return Iterator to end of output
   */
  virtual std::format_context::iterator
  toString(std::format_context &ctx) const {
    return std::format_to(ctx.out(), "{}@{}", nodeKindToString(kind),
                          static_cast<const void *>(this));
  }
};

} // namespace cxy::ast

// std::format specialization for all ASTNode types
template <> struct std::formatter<cxy::ast::ASTNode> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

  auto format(const cxy::ast::ASTNode &node, format_context &ctx) const {
    return node.toString(ctx);
  }
};
