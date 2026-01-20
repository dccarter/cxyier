#pragma once

#include "kind.hpp"
#include "cxy/token.hpp"
#include "cxy/arena_allocator.hpp"
#include "cxy/arena_stl.hpp"
#include "cxy/strings.hpp"
#include <cstddef>
#include <unordered_map>
#include <unordered_set>

// Forward declarations
namespace cxy {
    namespace ast {
        class ASTNode;
    }
    
    class IntegerType;
    class FloatType;
    class BoolType;
    class CharType;
    class VoidType;
    class AutoType;
    class ArrayType;
    class TupleType;
    class UnionType;
    class FunctionType;
    class StructType;
    class PointerType;
    class ReferenceType;
    class ClassType;
}

namespace cxy {

/**
 * Type registry for managing and caching type instances.
 * Uses singleton pattern for global access.
 */
class TypeRegistry {
public:
    // Constructor takes optional arena size (default 1MB)
    explicit TypeRegistry(size_t arenaSize = 1024 * 1024);
    ~TypeRegistry() = default;

    // Singleton access
    static TypeRegistry& instance();

    // Primitive type getters - on-demand allocation with caching
    const IntegerType* integerType(::cxy::IntegerKind kind);
    const FloatType* floatType(::cxy::FloatKind kind);
    const BoolType* boolType();
    const CharType* charType();
    const VoidType* voidType();
    const AutoType* autoType();

    // Legacy getters for backward compatibility
    const IntegerType* getIntegerType(::cxy::IntegerKind kind) { return integerType(kind); }
    const FloatType* getFloatType(::cxy::FloatKind kind) { return floatType(kind); }
    const BoolType* getBoolType() { return boolType(); }
    const CharType* getCharType() { return charType(); }
    const VoidType* getVoidType() { return voidType(); }
    const AutoType* getAutoType() { return autoType(); }

    // Composite type creation
    const ArrayType* getArrayType(const Type* elementType, size_t size);
    const TupleType* getTupleType(const ArenaVector<const Type*>& elementTypes);
    const UnionType* getUnionType(const ArenaVector<const Type*>& variantTypes);
    const FunctionType* getFunctionType(const ArenaVector<const Type*>& parameterTypes, const Type* returnType);
    const StructType* getStructType(const InternedString& name, 
                                    ArenaVector<std::pair<InternedString, const Type*>> fields,
                                    ArenaVector<std::tuple<InternedString, const FunctionType*, const ast::ASTNode*>> methods,
                                    Flags flags, 
                                    const ast::ASTNode* sourceAST);
    const PointerType* getPointerType(const Type* pointeeType);
    const ReferenceType* getReferenceType(const Type* referentType);
    const ClassType* getClassType(const InternedString& name, 
                                  ArenaVector<std::pair<InternedString, const Type*>> fields,
                                  ArenaVector<std::tuple<InternedString, const FunctionType*, const ast::ASTNode*>> methods,
                                  const ClassType* baseClass,
                                  Flags flags, 
                                  const ast::ASTNode* sourceAST);

    // Registry management
    void clear();
    size_t getTypeCount() const;

private:
    // Disable copying and moving
    TypeRegistry(const TypeRegistry&) = delete;
    TypeRegistry& operator=(const TypeRegistry&) = delete;
    TypeRegistry(TypeRegistry&&) = delete;
    TypeRegistry& operator=(TypeRegistry&&) = delete;

    // Arena for all type allocations
    ArenaAllocator arena_;
    
    // Type hash and equality functors for ArenaSet pattern
    template<typename TypeT>
    struct TypeHash {
        size_t operator()(const TypeT* type) const { return type->hash(); }
    };
    
    template<typename TypeT>
    struct TypeEqual {
        bool operator()(const TypeT* a, const TypeT* b) const { return a->equals(b); }
    };
    
    // Arena-allocated type aliases
    template<typename K, typename V>
    using ArenaMap = std::unordered_map<K, V, std::hash<K>, std::equal_to<K>, 
                                        ArenaSTLAllocator<std::pair<const K, V>>>;
    
    template<typename T>
    using ArenaSet = std::unordered_set<T, TypeHash<typename std::remove_pointer<T>::type>, 
                                        TypeEqual<typename std::remove_pointer<T>::type>,
                                        ArenaSTLAllocator<T>>;
    
    // Cached primitive type instances
    ArenaMap<::cxy::IntegerKind, const IntegerType*> integerTypes_;
    ArenaMap<::cxy::FloatKind, const FloatType*> floatTypes_;
    const BoolType* boolType_ = nullptr;
    const CharType* charType_ = nullptr;
    const VoidType* voidType_ = nullptr;
    const AutoType* autoType_ = nullptr;
    
    // Composite type caches using ArenaSet pattern
    ArenaSet<const ArrayType*> arrayTypes_;
    ArenaSet<const TupleType*> tupleTypes_;
    ArenaSet<const UnionType*> unionTypes_;
    ArenaSet<const FunctionType*> functionTypes_;
    ArenaSet<const StructType*> structTypes_;
    ArenaSet<const PointerType*> pointerTypes_;
    ArenaSet<const ReferenceType*> referenceTypes_;
    ArenaSet<const ClassType*> classTypes_;
};

} // namespace cxy