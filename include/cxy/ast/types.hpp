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
 * @brief Array type node.
 *
 * Represents array types like [10]i32 or []i32.
 */
class ArrayTypeNode : public ASTNode {
public:
  ASTNode* elementType = nullptr;  ///< Element type
  ASTNode* size = nullptr;         ///< Array size (nullptr for dynamic arrays)

  explicit ArrayTypeNode(Location loc, ArenaAllocator &arena)
      : ASTNode(astArrayType, loc, arena) {}

  void setElementType(ASTNode* type) {
    if (elementType) removeChild(elementType);
    elementType = type;
    if (elementType) addChild(elementType);
  }

  void setSize(ASTNode* sizeNode) {
    if (size) removeChild(size);
    size = sizeNode;
    if (size) addChild(size);
  }

  std::format_context::iterator toString(std::format_context &ctx) const override {
    auto it = std::format_to(ctx.out(), "ArrayType([");
    if (size) {
      it = std::format_to(it, "{}", *size);
    }
    it = std::format_to(it, "]");
    if (elementType) {
      it = std::format_to(it, "{}", *elementType);
    }
    return std::format_to(it, ")");
  }
};

/**
 * @brief Function type node.
 *
 * Represents function types like (i32, string) -> bool.
 */
class FunctionTypeNode : public ASTNode {
public:
  ArenaVector<ASTNode*> params;    ///< Parameter types
  ASTNode* returnType = nullptr;   ///< Return type

  explicit FunctionTypeNode(Location loc, ArenaAllocator &arena)
      : ASTNode(astFunctionType, loc, arena),
        params(ArenaSTLAllocator<ASTNode*>(arena)) {}

  void addParam(ASTNode* param) {
    if (param) {
      params.push_back(param);
      addChild(param);
    }
  }

  void setReturnType(ASTNode* type) {
    if (returnType) removeChild(returnType);
    returnType = type;
    if (returnType) addChild(returnType);
  }

  std::format_context::iterator toString(std::format_context &ctx) const override {
    auto it = std::format_to(ctx.out(), "FunctionType((");
    for (size_t i = 0; i < params.size(); ++i) {
      if (i > 0) it = std::format_to(it, ", ");
      it = std::format_to(it, "{}", *params[i]);
    }
    it = std::format_to(it, ") -> ");
    if (returnType) {
      it = std::format_to(it, "{}", *returnType);
    } else {
      it = std::format_to(it, "void");
    }
    return std::format_to(it, ")");
  }
};

/**
 * @brief Union type node.
 *
 * Represents union types like i32 | string | null.
 */
class UnionTypeNode : public ASTNode {
public:
  ArenaVector<ASTNode*> members;   ///< Union member types

  explicit UnionTypeNode(Location loc, ArenaAllocator &arena)
      : ASTNode(astUnionType, loc, arena),
        members(ArenaSTLAllocator<ASTNode*>(arena)) {}

  void addMember(ASTNode* member) {
    if (member) {
      members.push_back(member);
      addChild(member);
    }
  }

  std::format_context::iterator toString(std::format_context &ctx) const override {
    auto it = std::format_to(ctx.out(), "UnionType(");
    for (size_t i = 0; i < members.size(); ++i) {
      if (i > 0) it = std::format_to(it, " | ");
      it = std::format_to(it, "{}", *members[i]);
    }
    return std::format_to(it, ")");
  }
};

/**
 * @brief Tuple type node.
 *
 * Represents tuple types like (i32, string, bool).
 */
class TupleTypeNode : public ASTNode {
public:
  ArenaVector<ASTNode*> members;   ///< Tuple member types

  explicit TupleTypeNode(Location loc, ArenaAllocator &arena)
      : ASTNode(astTupleType, loc, arena),
        members(ArenaSTLAllocator<ASTNode*>(arena)) {}

  void addMember(ASTNode* member) {
    if (member) {
      members.push_back(member);
      addChild(member);
    }
  }

  std::format_context::iterator toString(std::format_context &ctx) const override {
    auto it = std::format_to(ctx.out(), "TupleType((");
    for (size_t i = 0; i < members.size(); ++i) {
      if (i > 0) it = std::format_to(it, ", ");
      it = std::format_to(it, "{}", *members[i]);
    }
    return std::format_to(it, "))");
  }
};

/**
 * @brief Result type node.
 *
 * Represents result types like !i64 for error-returning functions.
 */
class ResultTypeNode : public ASTNode {
public:
  ASTNode* target = nullptr;       ///< Target type

  explicit ResultTypeNode(Location loc, ArenaAllocator &arena)
      : ASTNode(astResultType, loc, arena) {}

  void setTarget(ASTNode* type) {
    if (target) removeChild(target);
    target = type;
    if (target) addChild(target);
  }

  std::format_context::iterator toString(std::format_context &ctx) const override {
    auto it = std::format_to(ctx.out(), "ResultType(!");
    if (target) {
      it = std::format_to(it, "{}", *target);
    }
    return std::format_to(it, ")");
  }
};

/**
 * @brief Optional type node.
 *
 * Represents optional types like ?i32.
 */
class OptionalTypeNode : public ASTNode {
public:
  ASTNode* target = nullptr;       ///< Target type

  explicit OptionalTypeNode(Location loc, ArenaAllocator &arena)
      : ASTNode(astOptionalType, loc, arena) {}

  void setTarget(ASTNode* type) {
    if (target) removeChild(target);
    target = type;
    if (target) addChild(target);
  }

  std::format_context::iterator toString(std::format_context &ctx) const override {
    auto it = std::format_to(ctx.out(), "OptionalType(?");
    if (target) {
      it = std::format_to(it, "{}", *target);
    }
    return std::format_to(it, ")");
  }
};

/**
 * @brief Reference type node.
 *
 * Represents reference types like &i32.
 */
class ReferenceTypeNode : public ASTNode {
public:
  ASTNode* target = nullptr;       ///< Target type

  explicit ReferenceTypeNode(Location loc, ArenaAllocator &arena)
      : ASTNode(astReferenceType, loc, arena) {}

  void setTarget(ASTNode* type) {
    if (target) removeChild(target);
    target = type;
    if (target) addChild(target);
  }

  std::format_context::iterator toString(std::format_context &ctx) const override {
    auto it = std::format_to(ctx.out(), "ReferenceType(&");
    if (target) {
      it = std::format_to(it, "{}", *target);
    }
    return std::format_to(it, ")");
  }
};

/**
 * @brief Pointer type node.
 *
 * Represents pointer types like *i32.
 */
class PointerTypeNode : public ASTNode {
public:
  ASTNode* target = nullptr;       ///< Target type

  explicit PointerTypeNode(Location loc, ArenaAllocator &arena)
      : ASTNode(astPointerType, loc, arena) {}

  void setTarget(ASTNode* type) {
    if (target) removeChild(target);
    target = type;
    if (target) addChild(target);
  }

  std::format_context::iterator toString(std::format_context &ctx) const override {
    auto it = std::format_to(ctx.out(), "PointerType(*");
    if (target) {
      it = std::format_to(it, "{}", *target);
    }
    return std::format_to(it, ")");
  }
};

/**
 * @brief Create a primitive type node.
 */
inline PrimitiveTypeNode *createPrimitiveType(TokenKind typeKind, Location loc,
                                              ArenaAllocator &arena) {
  return arena.construct<PrimitiveTypeNode>(typeKind, loc, arena);
}

/**
 * @brief Create an array type node.
 */
inline ArrayTypeNode *createArrayType(Location loc, ArenaAllocator &arena) {
  return arena.construct<ArrayTypeNode>(loc, arena);
}

/**
 * @brief Create a function type node.
 */
inline FunctionTypeNode *createFunctionType(Location loc, ArenaAllocator &arena) {
  return arena.construct<FunctionTypeNode>(loc, arena);
}

/**
 * @brief Create a union type node.
 */
inline UnionTypeNode *createUnionType(Location loc, ArenaAllocator &arena) {
  return arena.construct<UnionTypeNode>(loc, arena);
}

/**
 * @brief Create a tuple type node.
 */
inline TupleTypeNode *createTupleType(Location loc, ArenaAllocator &arena) {
  return arena.construct<TupleTypeNode>(loc, arena);
}

/**
 * @brief Create a result type node.
 */
inline ResultTypeNode *createResultType(Location loc, ArenaAllocator &arena) {
  return arena.construct<ResultTypeNode>(loc, arena);
}

/**
 * @brief Create an optional type node.
 */
inline OptionalTypeNode *createOptionalType(Location loc, ArenaAllocator &arena) {
  return arena.construct<OptionalTypeNode>(loc, arena);
}

/**
 * @brief Create a reference type node.
 */
inline ReferenceTypeNode *createReferenceType(Location loc, ArenaAllocator &arena) {
  return arena.construct<ReferenceTypeNode>(loc, arena);
}

/**
 * @brief Create a pointer type node.
 */
inline PointerTypeNode *createPointerType(Location loc, ArenaAllocator &arena) {
  return arena.construct<PointerTypeNode>(loc, arena);
}

} // namespace cxy::ast
