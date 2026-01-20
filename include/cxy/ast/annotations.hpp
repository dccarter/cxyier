#pragma once

#include "cxy/memory/arena_stl.hpp"
#include "cxy/ast/node.hpp"
#include "cxy/strings.hpp"
#include <format>

namespace cxy::ast {

/**
 * @brief AST node representing a single annotation.
 *
 * Annotations have the format:
 * - `name = value
 *
 * Where name is an identifier and value is any expression.
 * Examples: `Hello = 20, `isVector = true
 */
class AnnotationNode : public ASTNode {
public:
  InternedString name; ///< Annotation name
  ASTNode* value = nullptr; ///< Annotation value expression

  /**
   * @brief Construct an annotation node.
   *
   * @param annotationName Name of the annotation
   * @param loc Source location
   * @param arena Arena allocator
   */
  AnnotationNode(InternedString annotationName, Location loc,
                ArenaAllocator &arena)
      : ASTNode(astAnnotation, loc, arena), name(annotationName) {}

  /**
   * @brief Set the value expression for this annotation.
   *
   * @param valueNode Expression node representing the annotation value
   */
  void setValue(ASTNode* valueNode) {
    if (value) removeChild(value);
    value = valueNode;
    if (value) addChild(value);
  }

  /**
   * @brief Check if annotation has a value.
   */
  bool hasValue() const { return value != nullptr; }

  /**
   * @brief Convert to string representation.
   */
  std::format_context::iterator
  toString(std::format_context &ctx) const override {
    auto out = std::format_to(ctx.out(), "Annotation({})", name.view());
    if (hasValue()) {
      out = std::format_to(out, " = {}", *value);
    }
    return out;
  }
};

/**
 * @brief AST node representing a list of annotations.
 *
 * This is used to group multiple annotations together, typically
 * appearing within structs, classes, or other declarations.
 */
class AnnotationListNode : public ASTNode {
public:
  ArenaVector<AnnotationNode *> annotations; ///< List of annotations

  /**
   * @brief Construct an annotation list node.
   *
   * @param loc Source location
   * @param arena Arena allocator
   */
  AnnotationListNode(Location loc, ArenaAllocator &arena)
      : ASTNode(astAnnotationList, loc, arena),
        annotations(ArenaSTLAllocator<AnnotationNode *>(arena)) {}

  /**
   * @brief Add an annotation to the list.
   *
   * @param annotation Annotation node to add
   */
  void addAnnotation(AnnotationNode *annotation) {
    if (annotation) {
      annotations.push_back(annotation);
      addChild(annotation);
    }
  }

  /**
   * @brief Remove an annotation from the list.
   *
   * @param annotation Annotation node to remove
   * @return true if annotation was found and removed
   */
  bool removeAnnotation(AnnotationNode *annotation) {
    auto it = std::find(annotations.begin(), annotations.end(), annotation);
    if (it != annotations.end()) {
      annotations.erase(it);
      removeChild(annotation);
      return true;
    }
    return false;
  }

  /**
   * @brief Get annotation by index.
   *
   * @param index Annotation index
   * @return Annotation node or nullptr if index out of bounds
   */
  AnnotationNode *getAnnotation(size_t index) const {
    return index < annotations.size() ? annotations[index] : nullptr;
  }

  /**
   * @brief Get number of annotations.
   */
  size_t getAnnotationCount() const { return annotations.size(); }

  /**
   * @brief Check if list has any annotations.
   */
  bool hasAnnotations() const { return !annotations.empty(); }

  /**
   * @brief Find annotation by name.
   *
   * @param name Annotation name to search for
   * @return First annotation with matching name, or nullptr if not found
   */
  AnnotationNode *findAnnotation(const InternedString &name) const {
    for (AnnotationNode *annotation : annotations) {
      if (annotation->name == name) {
        return annotation;
      }
    }
    return nullptr;
  }

  /**
   * @brief Convert to string representation.
   */
  std::format_context::iterator
  toString(std::format_context &ctx) const override {
    return std::format_to(ctx.out(), "AnnotationList({} annotations)",
                          annotations.size());
  }
};

// Helper functions for creating annotation nodes

/**
 * @brief Create a simple annotation with name and value.
 *
 * @param name Annotation name
 * @param value Annotation value expression
 * @param loc Source location
 * @param arena Arena allocator
 * @return New annotation node
 */
inline AnnotationNode *createAnnotation(InternedString name, ASTNode* value, 
                                        Location loc, ArenaAllocator &arena) {
  auto* annotation = arena.construct<AnnotationNode>(name, loc, arena);
  annotation->setValue(value);
  return annotation;
}

/**
 * @brief Create an annotation list.
 *
 * @param loc Source location
 * @param arena Arena allocator
 * @return New annotation list node
 */
inline AnnotationListNode *createAnnotationList(Location loc,
                                                ArenaAllocator &arena) {
  return arena.construct<AnnotationListNode>(loc, arena);
}

} // namespace cxy::ast