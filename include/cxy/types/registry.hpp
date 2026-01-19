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
    class ArrayType;
    class TupleType;
    class UnionType;
    class FunctionType;
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
    
    // Future composite types (implemented in later phases)
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
    
    // Array type cache - key is (elementType*, size)
    struct ArrayTypeKeyHash {
        size_t operator()(const std::pair<const Type*, size_t>& key) const {
            size_t h1 = std::hash<const void*>{}(key.first);
            size_t h2 = std::hash<size_t>{}(key.second);
            return h1 ^ (h2 << 1);
        }
    };
    
    std::unordered_map<std::pair<const Type*, size_t>, const ArrayType*, ArrayTypeKeyHash> arrayTypes_;
    
    // Tuple type cache - key is vector of element types
    struct TupleTypeKeyHash {
        size_t operator()(const ArenaVector<const Type*>& key) const {
            size_t hash = 0;
            for (const Type* type : key) {
                hash ^= std::hash<const void*>{}(type) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            }
            return hash;
        }
    };
    
    struct TupleTypeKeyEqual {
        bool operator()(const ArenaVector<const Type*>& a, const ArenaVector<const Type*>& b) const {
            if (a.size() != b.size()) return false;
            for (size_t i = 0; i < a.size(); ++i) {
                if (a[i] != b[i]) return false;
            }
            return true;
        }
    };
    
    std::unordered_map<ArenaVector<const Type*>, const TupleType*, TupleTypeKeyHash, TupleTypeKeyEqual> tupleTypes_;
    
    // Union type cache - key is vector of variant types (reusing TupleType's hash and equal functions)
    std::unordered_map<ArenaVector<const Type*>, const UnionType*, TupleTypeKeyHash, TupleTypeKeyEqual> unionTypes_;
    
    // Function type cache - key is (parameter types, return type)
    struct FunctionTypeKey {
        ArenaVector<const Type*> parameterTypes;
        const Type* returnType;
        
        explicit FunctionTypeKey(ArenaAllocator& arena) 
            : parameterTypes(ArenaSTLAllocator<const Type*>(arena)), returnType(nullptr) {}
        
        bool operator==(const FunctionTypeKey& other) const {
            if (returnType != other.returnType) return false;
            if (parameterTypes.size() != other.parameterTypes.size()) return false;
            for (size_t i = 0; i < parameterTypes.size(); ++i) {
                if (parameterTypes[i] != other.parameterTypes[i]) return false;
            }
            return true;
        }
    };
    
    struct FunctionTypeKeyHash {
        size_t operator()(const FunctionTypeKey& key) const {
            size_t hash = std::hash<const void*>{}(key.returnType);
            for (const Type* type : key.parameterTypes) {
                hash ^= std::hash<const void*>{}(type) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            }
            return hash;
        }
    };
    
    std::unordered_map<FunctionTypeKey, const FunctionType*, FunctionTypeKeyHash> functionTypes_;
};

} // namespace cxy