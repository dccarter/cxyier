#pragma once

#include "kind.hpp"
#include "cxy/token.hpp"
#include "cxy/arena_allocator.hpp"
#include <cstddef>
#include <unordered_map>

// Forward declarations for primitive types
namespace cxy {
    class IntegerType;
    class FloatType;
    class BoolType;
    class CharType;
    class VoidType;
    class AutoType;
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

    // Composite type creation (implemented in later phases)
    // const ArrayType* getArrayType(const Type* elementType, size_t size);
    // const TupleType* getTupleType(const std::vector<const Type*>& elementTypes);
    // const StructType* getStructType(const std::string& name, Flags flags = flgNone);
    // const ClassType* getClassType(const std::string& name, Flags flags = flgNone);

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
    
    // Cached type instances
    std::unordered_map<::cxy::IntegerKind, const IntegerType*> integerTypes_;
    std::unordered_map<::cxy::FloatKind, const FloatType*> floatTypes_;
    const BoolType* boolType_ = nullptr;
    const CharType* charType_ = nullptr;
    const VoidType* voidType_ = nullptr;
    const AutoType* autoType_ = nullptr;
};

} // namespace cxy