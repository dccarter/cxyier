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

/**
 * @brief Function declaration node.
 *
 * Represents function declarations including name, parameters, return type, and body.
 */
class FuncDeclarationNode : public DeclarationNode {
public:
  ASTNode* name = nullptr;                    ///< Function name (identifier)
  ArenaVector<ASTNode*> genericParams;        ///< Generic type parameters
  ArenaVector<ASTNode*> parameters;           ///< Function parameters
  ASTNode* returnType = nullptr;              ///< Return type annotation
  ASTNode* body = nullptr;                    ///< Function body (block statement)

  explicit FuncDeclarationNode(Location loc, ArenaAllocator &arena)
      : DeclarationNode(astFuncDeclaration, loc, arena),
        genericParams(ArenaSTLAllocator<ASTNode*>(arena)),
        parameters(ArenaSTLAllocator<ASTNode*>(arena)) {}

  void setName(ASTNode* nameNode) {
    if (name) removeChild(name);
    name = nameNode;
    if (name) addChild(name);
  }

  void addGenericParam(ASTNode* param) {
    if (param) {
      genericParams.push_back(param);
      addChild(param);
    }
  }

  void addParameter(ASTNode* param) {
    if (param) {
      parameters.push_back(param);
      addChild(param);
    }
  }

  void setReturnType(ASTNode* typeNode) {
    if (returnType) removeChild(returnType);
    returnType = typeNode;
    if (returnType) addChild(returnType);
  }

  void setBody(ASTNode* bodyNode) {
    if (body) removeChild(body);
    body = bodyNode;
    if (body) addChild(body);
  }

  std::format_context::iterator toString(std::format_context &ctx) const override {
    auto it = std::format_to(ctx.out(), "FuncDecl(");
    if (name) {
      it = std::format_to(it, "{}", *name);
    } else {
      it = std::format_to(it, "unnamed");
    }
    if (!genericParams.empty()) {
      it = std::format_to(it, ", {} generics", genericParams.size());
    }
    if (!parameters.empty()) {
      it = std::format_to(it, ", {} params", parameters.size());
    }
    if (returnType) {
      it = std::format_to(it, " -> {}", *returnType);
    }
    return std::format_to(it, ")");
  }
};

/**
 * @brief Method declaration node.
 *
 * Represents method declarations that can be overridden and support overloading.
 * Methods have fast lookup capabilities through type objects.
 */
class MethodDeclarationNode : public DeclarationNode {
public:
  ASTNode* name = nullptr;                    ///< Method name
  ArenaVector<ASTNode*> overloads;            ///< Method overloads
  ArenaVector<const cxy::Type*> typeCache;   ///< Type objects for fast lookup

  explicit MethodDeclarationNode(Location loc, ArenaAllocator &arena)
      : DeclarationNode(astMethodDeclaration, loc, arena),
        overloads(ArenaSTLAllocator<ASTNode*>(arena)),
        typeCache(ArenaSTLAllocator<const cxy::Type*>(arena)) {}

  void setName(ASTNode* nameNode) {
    if (name) removeChild(name);
    name = nameNode;
    if (name) addChild(name);
  }

  void addOverload(ASTNode* overload) {
    if (overload) {
      overloads.push_back(overload);
      addChild(overload);
    }
  }

  void addTypeToCache(const cxy::Type* type) {
    if (type) {
      typeCache.push_back(type);
    }
  }

  std::format_context::iterator toString(std::format_context &ctx) const override {
    auto it = std::format_to(ctx.out(), "MethodDecl(");
    if (name) {
      it = std::format_to(it, "{}", *name);
    } else {
      it = std::format_to(it, "unnamed");
    }
    if (!overloads.empty()) {
      it = std::format_to(it, ", {} overloads", overloads.size());
    }
    return std::format_to(it, ")");
  }
};

/**
 * @brief Function parameter declaration node.
 *
 * Represents a single function parameter with name, type, and optional default value.
 */
class FuncParamDeclarationNode : public DeclarationNode {
public:
  ASTNode* name = nullptr;                    ///< Parameter name
  ASTNode* type = nullptr;                    ///< Parameter type
  ASTNode* defaultValue = nullptr;            ///< Optional default value

  explicit FuncParamDeclarationNode(Location loc, ArenaAllocator &arena)
      : DeclarationNode(astFuncParamDeclaration, loc, arena) {}

  void setName(ASTNode* nameNode) {
    if (name) removeChild(name);
    name = nameNode;
    if (name) addChild(name);
  }

  void setType(ASTNode* typeNode) {
    if (type) removeChild(type);
    type = typeNode;
    if (type) addChild(type);
  }

  void setDefaultValue(ASTNode* defaultNode) {
    if (defaultValue) removeChild(defaultValue);
    defaultValue = defaultNode;
    if (defaultValue) addChild(defaultValue);
  }

  std::format_context::iterator toString(std::format_context &ctx) const override {
    auto it = std::format_to(ctx.out(), "Param(");
    if (name) {
      it = std::format_to(it, "{}:", *name);
    } else {
      it = std::format_to(it, "unnamed:");
    }
    if (type) {
      it = std::format_to(it, " {}", *type);
    }
    if (defaultValue) {
      it = std::format_to(it, " = {}", *defaultValue);
    }
    return std::format_to(it, ")");
  }
};

/**
 * @brief Type declaration node.
 *
 * Represents type alias declarations.
 */
class TypeDeclarationNode : public DeclarationNode {
public:
  ASTNode* name = nullptr;                    ///< Type alias name
  ASTNode* type = nullptr;                    ///< The type being aliased

  explicit TypeDeclarationNode(Location loc, ArenaAllocator &arena)
      : DeclarationNode(astTypeDeclaration, loc, arena) {}

  void setName(ASTNode* nameNode) {
    if (name) removeChild(name);
    name = nameNode;
    if (name) addChild(name);
  }

  void setType(ASTNode* typeNode) {
    if (type) removeChild(type);
    type = typeNode;
    if (type) addChild(type);
  }

  std::format_context::iterator toString(std::format_context &ctx) const override {
    auto it = std::format_to(ctx.out(), "TypeDecl(");
    if (name) {
      it = std::format_to(it, "{}", *name);
    } else {
      it = std::format_to(it, "unnamed");
    }
    it = std::format_to(it, " = ");
    if (type) {
      it = std::format_to(it, "{}", *type);
    } else {
      it = std::format_to(it, "unspecified");
    }
    return std::format_to(it, ")");
  }
};

/**
 * @brief Enum option declaration node.
 *
 * Represents a single option/variant within an enum declaration.
 */
class EnumOptionDeclarationNode : public DeclarationNode {
public:
  ASTNode* name = nullptr;                    ///< Option name
  ASTNode* value = nullptr;                   ///< Option value/discriminant

  explicit EnumOptionDeclarationNode(Location loc, ArenaAllocator &arena)
      : DeclarationNode(astEnumOptionDeclaration, loc, arena) {}

  void setName(ASTNode* nameNode) {
    if (name) removeChild(name);
    name = nameNode;
    if (name) addChild(name);
  }

  void setValue(ASTNode* valueNode) {
    if (value) removeChild(value);
    value = valueNode;
    if (value) addChild(value);
  }

  std::format_context::iterator toString(std::format_context &ctx) const override {
    auto it = std::format_to(ctx.out(), "EnumOption(");
    if (name) {
      it = std::format_to(it, "{}", *name);
    } else {
      it = std::format_to(it, "unnamed");
    }
    if (value) {
      it = std::format_to(it, " = {}", *value);
    }
    return std::format_to(it, ")");
  }
};

/**
 * @brief Enum declaration node.
 *
 * Represents enum type declarations.
 */
class EnumDeclarationNode : public DeclarationNode {
public:
  ASTNode* name = nullptr;                    ///< Enum name
  ASTNode* base = nullptr;                    ///< Base type for enum backing
  ArenaVector<ASTNode*> options;              ///< Enum options/variants

  explicit EnumDeclarationNode(Location loc, ArenaAllocator &arena)
      : DeclarationNode(astEnumDeclaration, loc, arena),
        options(ArenaSTLAllocator<ASTNode*>(arena)) {}

  void setName(ASTNode* nameNode) {
    if (name) removeChild(name);
    name = nameNode;
    if (name) addChild(name);
  }

  void setBase(ASTNode* baseNode) {
    if (base) removeChild(base);
    base = baseNode;
    if (base) addChild(base);
  }

  void addOption(ASTNode* option) {
    if (option) {
      options.push_back(option);
      addChild(option);
    }
  }

  std::format_context::iterator toString(std::format_context &ctx) const override {
    auto it = std::format_to(ctx.out(), "EnumDecl(");
    if (name) {
      it = std::format_to(it, "{}", *name);
    } else {
      it = std::format_to(it, "unnamed");
    }
    if (base) {
      it = std::format_to(it, ": {}", *base);
    }
    return std::format_to(it, ", {} options)", options.size());
  }
};

/**
 * @brief Field declaration node.
 *
 * Represents a field within a struct or class.
 */
class FieldDeclarationNode : public DeclarationNode {
public:
  ASTNode* name = nullptr;                    ///< Field name
  ASTNode* type = nullptr;                    ///< Field type
  ASTNode* defaultValue = nullptr;            ///< Optional default value

  explicit FieldDeclarationNode(Location loc, ArenaAllocator &arena)
      : DeclarationNode(astFieldDeclaration, loc, arena) {}

  void setName(ASTNode* nameNode) {
    if (name) removeChild(name);
    name = nameNode;
    if (name) addChild(name);
  }

  void setType(ASTNode* typeNode) {
    if (type) removeChild(type);
    type = typeNode;
    if (type) addChild(type);
  }

  void setDefaultValue(ASTNode* defaultNode) {
    if (defaultValue) removeChild(defaultValue);
    defaultValue = defaultNode;
    if (defaultValue) addChild(defaultValue);
  }

  std::format_context::iterator toString(std::format_context &ctx) const override {
    auto it = std::format_to(ctx.out(), "Field(");
    if (name) {
      it = std::format_to(it, "{}:", *name);
    } else {
      it = std::format_to(it, "unnamed:");
    }
    if (type) {
      it = std::format_to(it, " {}", *type);
    }
    if (defaultValue) {
      it = std::format_to(it, " = {}", *defaultValue);
    }
    return std::format_to(it, ")");
  }
};

/**
 * @brief Struct declaration node.
 *
 * Represents struct type declarations.
 */
class StructDeclarationNode : public DeclarationNode {
public:
  ASTNode* name = nullptr;                    ///< Struct name
  ArenaVector<ASTNode*> fields;               ///< Struct fields

  explicit StructDeclarationNode(Location loc, ArenaAllocator &arena)
      : DeclarationNode(astStructDeclaration, loc, arena),
        fields(ArenaSTLAllocator<ASTNode*>(arena)) {}

  void setName(ASTNode* nameNode) {
    if (name) removeChild(name);
    name = nameNode;
    if (name) addChild(name);
  }

  void addField(ASTNode* field) {
    if (field) {
      fields.push_back(field);
      addChild(field);
    }
  }

  std::format_context::iterator toString(std::format_context &ctx) const override {
    auto it = std::format_to(ctx.out(), "StructDecl(");
    if (name) {
      it = std::format_to(it, "{}", *name);
    } else {
      it = std::format_to(it, "unnamed");
    }
    return std::format_to(it, ", {} fields)", fields.size());
  }
};

/**
 * @brief Class declaration node.
 *
 * Represents class type declarations.
 */
class ClassDeclarationNode : public DeclarationNode {
public:
  ASTNode* name = nullptr;                    ///< Class name
  ASTNode* base = nullptr;                    ///< Base class
  ArenaVector<ASTNode*> members;              ///< Class members (fields and methods)
  ArenaVector<ASTNode*> annotations;          ///< Class annotations

  explicit ClassDeclarationNode(Location loc, ArenaAllocator &arena)
      : DeclarationNode(astClassDeclaration, loc, arena),
        members(ArenaSTLAllocator<ASTNode*>(arena)),
        annotations(ArenaSTLAllocator<ASTNode*>(arena)) {}

  void setName(ASTNode* nameNode) {
    if (name) removeChild(name);
    name = nameNode;
    if (name) addChild(name);
  }

  void setBase(ASTNode* baseNode) {
    if (base) removeChild(base);
    base = baseNode;
    if (base) addChild(base);
  }

  void addMember(ASTNode* member) {
    if (member) {
      members.push_back(member);
      addChild(member);
    }
  }

  void addAnnotation(ASTNode* annotation) {
    if (annotation) {
      annotations.push_back(annotation);
      addChild(annotation);
    }
  }

  std::format_context::iterator toString(std::format_context &ctx) const override {
    auto it = std::format_to(ctx.out(), "ClassDecl(");
    if (name) {
      it = std::format_to(it, "{}", *name);
    } else {
      it = std::format_to(it, "unnamed");
    }
    if (base) {
      it = std::format_to(it, " : {}", *base);
    }
    return std::format_to(it, ", {} members)", members.size());
  }
};

/**
 * @brief External declaration node.
 *
 * Represents external/foreign function interface declarations.
 */
class ExternDeclarationNode : public DeclarationNode {
public:
  ASTNode* declaration = nullptr;             ///< The external declaration

  explicit ExternDeclarationNode(Location loc, ArenaAllocator &arena)
      : DeclarationNode(astExternDeclaration, loc, arena) {}

  void setDeclaration(ASTNode* declNode) {
    if (declaration) removeChild(declaration);
    declaration = declNode;
    if (declaration) addChild(declaration);
  }

  std::format_context::iterator toString(std::format_context &ctx) const override {
    auto it = std::format_to(ctx.out(), "ExternDecl(");
    if (declaration) {
      it = std::format_to(it, "{}", *declaration);
    } else {
      it = std::format_to(it, "empty");
    }
    return std::format_to(it, ")");
  }
};

/**
 * @brief Module declaration node.
 *
 * Represents module declarations.
 */
class ModuleDeclarationNode : public DeclarationNode {
public:
  ASTNode* name = nullptr;                    ///< Module name
  ArenaVector<ASTNode*> topLevel;             ///< Top-level declarations
  ArenaVector<ASTNode*> mainContent;          ///< Main content declarations

  explicit ModuleDeclarationNode(Location loc, ArenaAllocator &arena)
      : DeclarationNode(astModuleDeclaration, loc, arena),
        topLevel(ArenaSTLAllocator<ASTNode*>(arena)),
        mainContent(ArenaSTLAllocator<ASTNode*>(arena)) {}

  void setName(ASTNode* nameNode) {
    if (name) removeChild(name);
    name = nameNode;
    if (name) addChild(name);
  }

  void addTopLevel(ASTNode* decl) {
    if (decl) {
      topLevel.push_back(decl);
      addChild(decl);
    }
  }

  void addMainContent(ASTNode* decl) {
    if (decl) {
      mainContent.push_back(decl);
      addChild(decl);
    }
  }

  std::format_context::iterator toString(std::format_context &ctx) const override {
    auto it = std::format_to(ctx.out(), "ModuleDecl(");
    if (name) {
      it = std::format_to(it, "{}", *name);
    } else {
      it = std::format_to(it, "unnamed");
    }
    return std::format_to(it, ", {} top, {} main)", topLevel.size(), mainContent.size());
  }
};

/**
 * @brief Import declaration node.
 *
 * Represents import statements.
 */
class ImportDeclarationNode : public DeclarationNode {
public:
  ASTNode* path = nullptr;                    ///< Import path
  ASTNode* name = nullptr;                    ///< Module name
  ArenaVector<ASTNode*> entities;             ///< Imported entities
  ASTNode* alias = nullptr;                   ///< Module alias

  explicit ImportDeclarationNode(Location loc, ArenaAllocator &arena)
      : DeclarationNode(astImportDeclaration, loc, arena),
        entities(ArenaSTLAllocator<ASTNode*>(arena)) {}

  void setPath(ASTNode* pathNode) {
    if (path) removeChild(path);
    path = pathNode;
    if (path) addChild(path);
  }

  void setName(ASTNode* nameNode) {
    if (name) removeChild(name);
    name = nameNode;
    if (name) addChild(name);
  }

  void setAlias(ASTNode* aliasNode) {
    if (alias) removeChild(alias);
    alias = aliasNode;
    if (alias) addChild(alias);
  }

  void addEntity(ASTNode* entity) {
    if (entity) {
      entities.push_back(entity);
      addChild(entity);
    }
  }

  std::format_context::iterator toString(std::format_context &ctx) const override {
    auto it = std::format_to(ctx.out(), "ImportDecl(");
    if (path) {
      it = std::format_to(it, "{}", *path);
    } else {
      it = std::format_to(it, "unspecified");
    }
    if (!entities.empty()) {
      it = std::format_to(it, ", {} entities", entities.size());
    }
    return std::format_to(it, ")");
  }
};

/**
 * @brief Type parameter declaration node.
 *
 * Represents a type parameter in generic declarations.
 */
class TypeParameterDeclarationNode : public DeclarationNode {
public:
  ASTNode* name = nullptr;                    ///< Type parameter name
  ASTNode* defaultValue = nullptr;            ///< Default type
  ASTNode* constraint = nullptr;              ///< Type constraint

  explicit TypeParameterDeclarationNode(Location loc, ArenaAllocator &arena)
      : DeclarationNode(astTypeParameterDeclaration, loc, arena) {}

  void setName(ASTNode* nameNode) {
    if (name) removeChild(name);
    name = nameNode;
    if (name) addChild(name);
  }

  void setDefaultValue(ASTNode* defaultNode) {
    if (defaultValue) removeChild(defaultValue);
    defaultValue = defaultNode;
    if (defaultValue) addChild(defaultValue);
  }

  void setConstraint(ASTNode* constraintNode) {
    if (constraint) removeChild(constraint);
    constraint = constraintNode;
    if (constraint) addChild(constraint);
  }

  std::format_context::iterator toString(std::format_context &ctx) const override {
    auto it = std::format_to(ctx.out(), "TypeParam(");
    if (name) {
      it = std::format_to(it, "{}", *name);
    } else {
      it = std::format_to(it, "unnamed");
    }
    if (constraint) {
      it = std::format_to(it, ": {}", *constraint);
    }
    if (defaultValue) {
      it = std::format_to(it, " = {}", *defaultValue);
    }
    return std::format_to(it, ")");
  }
};

/**
 * @brief Generic declaration node.
 *
 * Represents generic/template declarations.
 */
class GenericDeclarationNode : public DeclarationNode {
public:
  ArenaVector<ASTNode*> parameters;           ///< Type parameters
  ASTNode* decl = nullptr;                    ///< Declaration being made generic

  explicit GenericDeclarationNode(Location loc, ArenaAllocator &arena)
      : DeclarationNode(astGenericDeclaration, loc, arena),
        parameters(ArenaSTLAllocator<ASTNode*>(arena)) {}

  void addParameter(ASTNode* param) {
    if (param) {
      parameters.push_back(param);
      addChild(param);
    }
  }

  void setDeclaration(ASTNode* declNode) {
    if (decl) removeChild(decl);
    decl = declNode;
    if (decl) addChild(decl);
  }

  std::format_context::iterator toString(std::format_context &ctx) const override {
    return std::format_to(ctx.out(), "GenericDecl({} params)", parameters.size());
  }
};

/**
 * @brief Test declaration node.
 *
 * Represents test declarations.
 */
class TestDeclarationNode : public DeclarationNode {
public:
  ASTNode* name = nullptr;                    ///< Test name
  ASTNode* body = nullptr;                    ///< Test body

  explicit TestDeclarationNode(Location loc, ArenaAllocator &arena)
      : DeclarationNode(astTestDeclaration, loc, arena) {}

  void setName(ASTNode* nameNode) {
    if (name) removeChild(name);
    name = nameNode;
    if (name) addChild(name);
  }

  void setBody(ASTNode* bodyNode) {
    if (body) removeChild(body);
    body = bodyNode;
    if (body) addChild(body);
  }

  std::format_context::iterator toString(std::format_context &ctx) const override {
    auto it = std::format_to(ctx.out(), "TestDecl(");
    if (name) {
      it = std::format_to(it, "{}", *name);
    } else {
      it = std::format_to(it, "unnamed");
    }
    return std::format_to(it, ")");
  }
};

/**
 * @brief Macro declaration node.
 *
 * Represents macro definitions.
 */
class MacroDeclarationNode : public DeclarationNode {
public:
  ASTNode* name = nullptr;                    ///< Macro name
  ArenaVector<ASTNode*> parameters;           ///< Macro parameters
  ASTNode* body = nullptr;                    ///< Macro body

  explicit MacroDeclarationNode(Location loc, ArenaAllocator &arena)
      : DeclarationNode(astMacroDeclaration, loc, arena),
        parameters(ArenaSTLAllocator<ASTNode*>(arena)) {}

  void setName(ASTNode* nameNode) {
    if (name) removeChild(name);
    name = nameNode;
    if (name) addChild(name);
  }

  void addParameter(ASTNode* param) {
    if (param) {
      parameters.push_back(param);
      addChild(param);
    }
  }

  void setBody(ASTNode* bodyNode) {
    if (body) removeChild(body);
    body = bodyNode;
    if (body) addChild(body);
  }

  std::format_context::iterator toString(std::format_context &ctx) const override {
    auto it = std::format_to(ctx.out(), "MacroDecl(");
    if (name) {
      it = std::format_to(it, "{}", *name);
    } else {
      it = std::format_to(it, "unnamed");
    }
    return std::format_to(it, ", {} params)", parameters.size());
  }
};

// Declaration creation helpers
inline VariableDeclarationNode *createVariableDeclaration(Location loc, ArenaAllocator &arena, bool isConst = false) {
  return arena.construct<VariableDeclarationNode>(loc, arena, isConst);
}

inline FuncDeclarationNode *createFuncDeclaration(Location loc, ArenaAllocator &arena) {
  return arena.construct<FuncDeclarationNode>(loc, arena);
}

inline FuncParamDeclarationNode *createFuncParamDeclaration(Location loc, ArenaAllocator &arena) {
  return arena.construct<FuncParamDeclarationNode>(loc, arena);
}

inline MethodDeclarationNode *createMethodDeclaration(Location loc, ArenaAllocator &arena) {
  return arena.construct<MethodDeclarationNode>(loc, arena);
}

inline TypeDeclarationNode *createTypeDeclaration(Location loc, ArenaAllocator &arena) {
  return arena.construct<TypeDeclarationNode>(loc, arena);
}

inline EnumOptionDeclarationNode *createEnumOptionDeclaration(Location loc, ArenaAllocator &arena) {
  return arena.construct<EnumOptionDeclarationNode>(loc, arena);
}

inline EnumDeclarationNode *createEnumDeclaration(Location loc, ArenaAllocator &arena) {
  return arena.construct<EnumDeclarationNode>(loc, arena);
}

inline FieldDeclarationNode *createFieldDeclaration(Location loc, ArenaAllocator &arena) {
  return arena.construct<FieldDeclarationNode>(loc, arena);
}

inline StructDeclarationNode *createStructDeclaration(Location loc, ArenaAllocator &arena) {
  return arena.construct<StructDeclarationNode>(loc, arena);
}

inline ClassDeclarationNode *createClassDeclaration(Location loc, ArenaAllocator &arena) {
  return arena.construct<ClassDeclarationNode>(loc, arena);
}

inline ExternDeclarationNode *createExternDeclaration(Location loc, ArenaAllocator &arena) {
  return arena.construct<ExternDeclarationNode>(loc, arena);
}

inline ModuleDeclarationNode *createModuleDeclaration(Location loc, ArenaAllocator &arena) {
  return arena.construct<ModuleDeclarationNode>(loc, arena);
}

inline ImportDeclarationNode *createImportDeclaration(Location loc, ArenaAllocator &arena) {
  return arena.construct<ImportDeclarationNode>(loc, arena);
}

inline TypeParameterDeclarationNode *createTypeParameterDeclaration(Location loc, ArenaAllocator &arena) {
  return arena.construct<TypeParameterDeclarationNode>(loc, arena);
}

inline GenericDeclarationNode *createGenericDeclaration(Location loc, ArenaAllocator &arena) {
  return arena.construct<GenericDeclarationNode>(loc, arena);
}

inline TestDeclarationNode *createTestDeclaration(Location loc, ArenaAllocator &arena) {
  return arena.construct<TestDeclarationNode>(loc, arena);
}

inline MacroDeclarationNode *createMacroDeclaration(Location loc, ArenaAllocator &arena) {
  return arena.construct<MacroDeclarationNode>(loc, arena);
}

} // namespace cxy::ast
