#include "cxy/types/kind.hpp"
#include "cxy/types/registry.hpp"
#include "cxy/types/primitive.hpp"
#include "cxy/types/composite.hpp"
#include "cxy/memory/arena.hpp"

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
TypeRegistry::TypeRegistry(size_t arenaSize) : 
    arena_(arenaSize),
    integerTypes_(ArenaSTLAllocator<std::pair<const ::cxy::IntegerKind, const IntegerType*>>(arena_)),
    floatTypes_(ArenaSTLAllocator<std::pair<const ::cxy::FloatKind, const FloatType*>>(arena_)),
    arrayTypes_(ArenaSTLAllocator<const ArrayType*>(arena_)),
    tupleTypes_(ArenaSTLAllocator<const TupleType*>(arena_)),
    unionTypes_(ArenaSTLAllocator<const UnionType*>(arena_)),
    functionTypes_(ArenaSTLAllocator<const FunctionType*>(arena_)),
    structTypes_(ArenaSTLAllocator<const StructType*>(arena_)),
    pointerTypes_(ArenaSTLAllocator<const PointerType*>(arena_)),
    referenceTypes_(ArenaSTLAllocator<const ReferenceType*>(arena_)),
    classTypes_(ArenaSTLAllocator<const ClassType*>(arena_)) {}

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
    const IntegerType* type = new(arena_) IntegerType(kind, flgNone);
    integerTypes_[kind] = type;
    return type;
}

const FloatType* TypeRegistry::floatType(::cxy::FloatKind kind) {
    auto it = floatTypes_.find(kind);
    if (it != floatTypes_.end()) {
        return it->second;
    }
    
    // Allocate new type using registry arena
    const FloatType* type = new(arena_) FloatType(kind, flgNone);
    floatTypes_[kind] = type;
    return type;
}

const BoolType* TypeRegistry::boolType() {
    if (!boolType_) {
        boolType_ = new(arena_) BoolType(flgNone);
    }
    return boolType_;
}

const CharType* TypeRegistry::charType() {
    if (!charType_) {
        charType_ = new(arena_) CharType(flgNone);
    }
    return charType_;
}

const VoidType* TypeRegistry::voidType() {
    if (!voidType_) {
        voidType_ = new(arena_) VoidType(flgNone);
    }
    return voidType_;
}

const AutoType* TypeRegistry::autoType() {
    if (!autoType_) {
        autoType_ = new(arena_) AutoType(flgNone);
    }
    return autoType_;
}

const ArrayType* TypeRegistry::getArrayType(const Type* elementType, size_t size) {
    // Create temporary on stack for lookup
    ArrayType lookup(elementType, size, flgNone);
    
    auto it = arrayTypes_.find(&lookup);
    if (it != arrayTypes_.end()) {
        return *it;
    }
    
    // Allocate new array type using registry arena
    const ArrayType* type = new(arena_) ArrayType(elementType, size, flgNone);
    arrayTypes_.insert(type);
    return type;
}

const TupleType* TypeRegistry::getTupleType(const ArenaVector<const Type*>& elementTypes) {
    // Check for empty tuple (not allowed)
    if (elementTypes.empty()) {
        return nullptr;
    }
    
    // Create temporary on stack for lookup
    TupleType lookup(elementTypes);
    
    auto it = tupleTypes_.find(&lookup);
    if (it != tupleTypes_.end()) {
        return *it;
    }
    
    // Create a copy of elementTypes in our arena for permanent storage
    ArenaVector<const Type*> arenaTypes = makeArenaVector<const Type*>(arena_);
    for (const Type* type : elementTypes) {
        arenaTypes.push_back(type);
    }
    
    // Allocate new tuple type using registry arena
    const TupleType* type = new(arena_) TupleType(arenaTypes);
    tupleTypes_.insert(type);
    return type;
}

const UnionType* TypeRegistry::getUnionType(const ArenaVector<const Type*>& variantTypes) {
    // Check for empty union (not allowed)
    if (variantTypes.empty()) {
        return nullptr;
    }
    
    // Create temporary on stack for lookup
    UnionType lookup(variantTypes);
    
    auto it = unionTypes_.find(&lookup);
    if (it != unionTypes_.end()) {
        return *it;
    }
    
    // Create a copy of variantTypes in our arena for permanent storage
    ArenaVector<const Type*> arenaTypes = makeArenaVector<const Type*>(arena_);
    for (const Type* type : variantTypes) {
        arenaTypes.push_back(type);
    }
    
    // Allocate new union type using registry arena
    const UnionType* type = new(arena_) UnionType(arenaTypes);
    unionTypes_.insert(type);
    return type;
}

const FunctionType* TypeRegistry::getFunctionType(const ArenaVector<const Type*>& parameterTypes, const Type* returnType) {
    // Check for null return type (not allowed)
    if (!returnType) {
        return nullptr;
    }

    // Create temporary on stack for lookup
    FunctionType lookup(parameterTypes, returnType, flgNone);

    auto it = functionTypes_.find(&lookup);
    if (it != functionTypes_.end()) {
        return *it;
    }

    // Create a copy of parameterTypes in our arena for permanent storage
    ArenaVector<const Type*> arenaParams = makeArenaVector<const Type*>(arena_);
    for (const Type* type : parameterTypes) {
        arenaParams.push_back(type);
    }

    // Allocate new function type using registry arena
    const FunctionType* type = new(arena_) FunctionType(arenaParams, returnType, flgNone);
    functionTypes_.insert(type);
    return type;
}

const StructType* TypeRegistry::getStructType(const InternedString& name, 
                                               ArenaVector<std::pair<InternedString, const Type*>> fields,
                                               ArenaVector<std::tuple<InternedString, const FunctionType*, const ast::ASTNode*>> methods,
                                               Flags flags, 
                                               const ast::ASTNode* sourceAST) {
    // Use temporary arena for lookup to avoid polluting permanent arena
    ArenaAllocator tempArena(1024);
    
    // Convert pairs to FieldType vector using TEMPORARY arena
    ArenaVector<StructType::FieldType> lookupFields{ArenaSTLAllocator<StructType::FieldType>(tempArena)};
    for (const auto& field : fields) {
        lookupFields.emplace_back(field.first, field.second);
    }
    
    // Convert methods to MethodType vector using TEMPORARY arena
    ArenaVector<StructType::MethodType> lookupMethods{ArenaSTLAllocator<StructType::MethodType>(tempArena)};
    for (const auto& method : methods) {
        lookupMethods.emplace_back(std::get<0>(method), std::get<1>(method), std::get<2>(method));
    }
    
    // Create temporary on stack for lookup (uses tempArena internally)
    StructType lookup(name, std::move(lookupFields), std::move(lookupMethods), flags, sourceAST, tempArena);
    
    auto it = structTypes_.find(&lookup);
    if (it != structTypes_.end()) {
        return *it;  // tempArena automatically freed when function exits
    }
    
    // Create permanent copy using PERMANENT arena
    ArenaVector<StructType::FieldType> permanentFields{ArenaSTLAllocator<StructType::FieldType>(arena_)};
    for (const auto& field : fields) {
        permanentFields.emplace_back(field.first, field.second);
    }
    
    ArenaVector<StructType::MethodType> permanentMethods{ArenaSTLAllocator<StructType::MethodType>(arena_)};
    for (const auto& method : methods) {
        permanentMethods.emplace_back(std::get<0>(method), std::get<1>(method), std::get<2>(method));
    }
    
    // Create new struct type
    auto* type = new(arena_) StructType(name, std::move(permanentFields), std::move(permanentMethods), flags, sourceAST, arena_);
    structTypes_.insert(type);
    return type;
    // tempArena destructor frees all temporary allocations
}

const PointerType* TypeRegistry::getPointerType(const Type* pointeeType) {
    if (!pointeeType) {
        return nullptr;
    }

    // If pointeeType is a reference, return pointer to the referent type
    if (auto refType = pointeeType->as<ReferenceType>()) {
        return getPointerType(refType->getReferentType());
    }

    // Create temporary on stack for lookup
    PointerType lookup(pointeeType, flgNone);
    
    auto it = pointerTypes_.find(&lookup);
    if (it != pointerTypes_.end()) {
        return *it;
    }
    
    // Allocate new pointer type using registry arena
    const PointerType* type = new(arena_) PointerType(pointeeType, flgNone);
    pointerTypes_.insert(type);
    return type;
}

const ReferenceType* TypeRegistry::getReferenceType(const Type* referentType) {
    if (!referentType) {
        return nullptr;
    }

    // References to pointers are not allowed - pointers are mutable objects
    // and references must be bound to immutable targets
    if (referentType->kind() == typPointer) {
        return nullptr;
    }

    // Create temporary on stack for lookup
    ReferenceType lookup(referentType, flgNone);
    
    auto it = referenceTypes_.find(&lookup);
    if (it != referenceTypes_.end()) {
        return *it;
    }
    
    // Allocate new reference type using registry arena
    const ReferenceType* type = new(arena_) ReferenceType(referentType, flgNone);
    referenceTypes_.insert(type);
    return type;
}

const ClassType* TypeRegistry::getClassType(const InternedString& name, 
                                             ArenaVector<std::pair<InternedString, const Type*>> fields,
                                             ArenaVector<std::tuple<InternedString, const FunctionType*, const ast::ASTNode*>> methods,
                                             const ClassType* baseClass,
                                             Flags flags, 
                                             const ast::ASTNode* sourceAST) {
    // Use temporary arena for lookup to avoid polluting permanent arena
    ArenaAllocator tempArena(1024);
    
    // Convert pairs to FieldType vector using TEMPORARY arena
    ArenaVector<ClassType::FieldType> lookupFields{ArenaSTLAllocator<ClassType::FieldType>(tempArena)};
    for (const auto& field : fields) {
        lookupFields.emplace_back(field.first, field.second);
    }
    
    // Convert methods to MethodType vector using TEMPORARY arena
    ArenaVector<ClassType::MethodType> lookupMethods{ArenaSTLAllocator<ClassType::MethodType>(tempArena)};
    for (const auto& method : methods) {
        lookupMethods.emplace_back(std::get<0>(method), std::get<1>(method), std::get<2>(method));
    }
    
    // Base class doesn't need temporary conversion - just use directly
    
    // Create temporary on stack for lookup (uses tempArena internally)
    ClassType lookup(name, std::move(lookupFields), std::move(lookupMethods), baseClass, flags, sourceAST, tempArena);
    
    auto it = classTypes_.find(&lookup);
    if (it != classTypes_.end()) {
        return *it;  // tempArena automatically freed when function exits
    }
    
    // Create permanent copy using PERMANENT arena
    ArenaVector<ClassType::FieldType> permanentFields{ArenaSTLAllocator<ClassType::FieldType>(arena_)};
    for (const auto& field : fields) {
        permanentFields.emplace_back(field.first, field.second);
    }
    
    ArenaVector<ClassType::MethodType> permanentMethods{ArenaSTLAllocator<ClassType::MethodType>(arena_)};
    for (const auto& method : methods) {
        permanentMethods.emplace_back(std::get<0>(method), std::get<1>(method), std::get<2>(method));
    }
    
    // Base class doesn't need copying - just use directly
    
    // Create new class type
    auto* type = new(arena_) ClassType(name, std::move(permanentFields), std::move(permanentMethods), baseClass, flags, sourceAST, arena_);
    classTypes_.insert(type);
    return type;
    // tempArena destructor frees all temporary allocations
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
    structTypes_.clear();
    pointerTypes_.clear();
    referenceTypes_.clear();
    classTypes_.clear();
}

size_t TypeRegistry::getTypeCount() const {
    return integerTypes_.size() + floatTypes_.size() + 
           (boolType_ ? 1 : 0) + (charType_ ? 1 : 0) + 
           (voidType_ ? 1 : 0) + (autoType_ ? 1 : 0) +
           arrayTypes_.size() + tupleTypes_.size() + unionTypes_.size() + 
           functionTypes_.size() + structTypes_.size() + 
           pointerTypes_.size() + referenceTypes_.size() + classTypes_.size();
}

// Default implementation for canBeImplicitlyPassedTo
bool Type::canBeImplicitlyPassedTo(const Type* parameterType) const {
    // Default behavior: same as assignment conversion
    return isImplicitlyConvertibleTo(parameterType);
}

} // namespace cxy