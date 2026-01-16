#include "cxy/types/kind.hpp"
#include "cxy/types/registry.hpp"
#include "cxy/types/primitive.hpp"
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

void TypeRegistry::clear() {
    integerTypes_.clear();
    floatTypes_.clear();
    boolType_ = nullptr;
    charType_ = nullptr;
    voidType_ = nullptr;
    autoType_ = nullptr;
    arena_.reset();
}

size_t TypeRegistry::getTypeCount() const {
    return integerTypes_.size() + floatTypes_.size() + 
           (boolType_ ? 1 : 0) + (charType_ ? 1 : 0) + 
           (voidType_ ? 1 : 0) + (autoType_ ? 1 : 0);
}

} // namespace cxy