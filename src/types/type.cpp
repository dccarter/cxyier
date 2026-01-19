#include "cxy/types/kind.hpp"
#include "cxy/types/registry.hpp"
#include "cxy/types/primitive.hpp"
#include "cxy/types/composite.hpp"
#include "cxy/arena_allocator.hpp"

#include <cassert>

namespace cxy {

const char* typeKindToString(TypeKind kind) {
    switch (kind) {
#define STRINGIFY_TYPE_KIND(name) case typ##name: return #name;
        CXY_TYPES(STRINGIFY_TYPE_KIND)
#undef STRINGIFY_TYPE_KIND
        default:
            return "Unknown";
    }
}

void* Type::operator new(size_t size, ArenaAllocator& arena) {
    return arena.allocate(size, alignof(Type));
}

bool operator==(const Type& lhs, const Type& rhs) {
    return lhs.equals(&rhs);
}

bool operator!=(const Type& lhs, const Type& rhs) {
    return !lhs.equals(&rhs);
}

// TypeRegistry implementation
TypeRegistry::TypeRegistry(size_t arenaSize) : arena_(arenaSize) {}

TypeRegistry& TypeRegistry::instance() {
    static TypeRegistry instance_;
    return instance_;
}

const IntegerType* TypeRegistry::integerType(::cxy::IntegerKind kind) {
    auto it = integerTypes_.find(kind);
    if (it != integerTypes_.end()) {
        return it->second;
    }
    
    // Allocate new type using registry arena
    const IntegerType* type = new(arena_) IntegerType(kind);
    integerTypes_[kind] = type;
    return type;
}

const FloatType* TypeRegistry::floatType(::cxy::FloatKind kind) {
    auto it = floatTypes_.find(kind);
    if (it != floatTypes_.end()) {
        return it->second;
    }
    
    // Allocate new type using registry arena
    const FloatType* type = new(arena_) FloatType(kind);
    floatTypes_[kind] = type;
    return type;
}

const BoolType* TypeRegistry::boolType() {
    if (!boolType_) {
        boolType_ = new(arena_) BoolType();
    }
    return boolType_;
}

const CharType* TypeRegistry::charType() {
    if (!charType_) {
        charType_ = new(arena_) CharType();
    }
    return charType_;
}

const VoidType* TypeRegistry::voidType() {
    if (!voidType_) {
        voidType_ = new(arena_) VoidType();
    }
    return voidType_;
}

const AutoType* TypeRegistry::autoType() {
    if (!autoType_) {
        autoType_ = new(arena_) AutoType();
    }
    return autoType_;
}

const ArrayType* TypeRegistry::getArrayType(const Type* elementType, size_t size) {
    auto key = std::make_pair(elementType, size);
    auto it = arrayTypes_.find(key);
    if (it != arrayTypes_.end()) {
        return it->second;
    }
    
    // Allocate new array type using registry arena
    const ArrayType* type = new(arena_) ArrayType(elementType, size);
    arrayTypes_[key] = type;
    return type;
}

const TupleType* TypeRegistry::getTupleType(const ArenaVector<const Type*>& elementTypes) {
    // Check for empty tuple (not allowed)
    if (elementTypes.empty()) {
        return nullptr;
    }
    
    auto it = tupleTypes_.find(elementTypes);
    if (it != tupleTypes_.end()) {
        return it->second;
    }
    
    // Create a copy of elementTypes in our arena for the key
    ArenaVector<const Type*> keyTypes = makeArenaVector<const Type*>(arena_);
    for (const Type* type : elementTypes) {
        keyTypes.push_back(type);
    }
    
    // Allocate new tuple type using registry arena
    const TupleType* type = new(arena_) TupleType(keyTypes);
    tupleTypes_[keyTypes] = type;
    return type;
}

const UnionType* TypeRegistry::getUnionType(const ArenaVector<const Type*>& variantTypes) {
    // Check for empty union (not allowed)
    if (variantTypes.empty()) {
        return nullptr;
    }
    
    auto it = unionTypes_.find(variantTypes);
    if (it != unionTypes_.end()) {
        return it->second;
    }
    
    // Create a copy of variantTypes in our arena for the key
    ArenaVector<const Type*> keyTypes = makeArenaVector<const Type*>(arena_);
    for (const Type* type : variantTypes) {
        keyTypes.push_back(type);
    }
    
    // Allocate new union type using registry arena
    const UnionType* type = new(arena_) UnionType(keyTypes);
    unionTypes_[keyTypes] = type;
    return type;
}

const FunctionType* TypeRegistry::getFunctionType(const ArenaVector<const Type*>& parameterTypes, const Type* returnType) {
    // Check for null return type (not allowed)
    if (!returnType) {
        return nullptr;
    }
    
    FunctionTypeKey key(arena_);
    key.returnType = returnType;
    for (const Type* type : parameterTypes) {
        key.parameterTypes.push_back(type);
    }
    
    auto it = functionTypes_.find(key);
    if (it != functionTypes_.end()) {
        return it->second;
    }
    
    // Allocate new function type using registry arena
    const FunctionType* type = new(arena_) FunctionType(key.parameterTypes, returnType);
    functionTypes_[key] = type;
    return type;
}

void TypeRegistry::clear() {
    integerTypes_.clear();
    floatTypes_.clear();
    boolType_ = nullptr;
    charType_ = nullptr;
    voidType_ = nullptr;
    autoType_ = nullptr;
    arrayTypes_.clear();
    tupleTypes_.clear();
    unionTypes_.clear();
    functionTypes_.clear();
}

size_t TypeRegistry::getTypeCount() const {
    return integerTypes_.size() + floatTypes_.size() + 
           (boolType_ ? 1 : 0) + (charType_ ? 1 : 0) + 
           (voidType_ ? 1 : 0) + (autoType_ ? 1 : 0) +
           arrayTypes_.size() + tupleTypes_.size() + unionTypes_.size() + functionTypes_.size();
}

// Default implementation for canBeImplicitlyPassedTo
bool Type::canBeImplicitlyPassedTo(const Type* parameterType) const {
    // Default behavior: same as assignment conversion
    return isImplicitlyConvertibleTo(parameterType);
}

} // namespace cxy