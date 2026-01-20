#include "cxy/types/composite.hpp"
#include "cxy/types/primitive.hpp"
#include "cxy/arena_allocator.hpp"
#include <format>
#include <functional>

namespace cxy {

// Arena allocation support for CompositeType
void* CompositeType::operator new(size_t size, ArenaAllocator& arena) {
    return arena.allocate(size, alignof(CompositeType));
}

// Empty delete operator - arena manages deallocation
void CompositeType::operator delete(void* ptr) {
    // Empty - arena manages memory
}

// Platform-specific pointer size
static size_t getPointerSize() {
    return sizeof(void*);
}

// PointerType implementation
PointerType::PointerType(const Type* pointeeType, Flags flags)
    : CompositeType(nullptr, flags), pointeeType_(pointeeType) {
    // Pointers don't need AST reference for basic functionality
}

bool PointerType::equals(const Type* other) const {
    auto otherPointer = dynamic_cast<const PointerType*>(other);
    if (!otherPointer) return false;

    return pointeeType_->equals(otherPointer->pointeeType_);
}

std::string PointerType::toString() const {
    return std::format("*{}", pointeeType_->toString());
}

size_t PointerType::hash() const {
    // Combine pointee type hash with pointer type discriminator
    size_t h1 = pointeeType_->hash();
    size_t h2 = std::hash<int>{}(static_cast<int>(typPointer));
    return h1 ^ (h2 << 1);
}

bool PointerType::isAssignableFrom(const Type* other) const {
    auto otherPointer = dynamic_cast<const PointerType*>(other);
    if (!otherPointer) return false;

    // Exact equality first
    if (pointeeType_->equals(otherPointer->pointeeType_)) return true;
    
    // Pointer inheritance: *Derived can be assigned to *Base
    return pointeeType_->isAssignableFrom(otherPointer->pointeeType_);
}

bool PointerType::isImplicitlyConvertibleTo(const Type* other) const {
    auto otherPointer = dynamic_cast<const PointerType*>(other);
    if (!otherPointer) return false;

    // Exact equality first
    if (pointeeType_->equals(otherPointer->pointeeType_)) return true;
    
    // Pointer inheritance: *Derived → *Base (implicit upcast)
    return pointeeType_->isImplicitlyConvertibleTo(otherPointer->pointeeType_);
}

bool PointerType::isExplicitlyConvertibleTo(const Type* other) const {
    auto otherPointer = dynamic_cast<const PointerType*>(other);
    if (!otherPointer) return false;

    // Exact equality first
    if (pointeeType_->equals(otherPointer->pointeeType_)) return true;
    
    // Pointer inheritance: allow both upcast and downcast with explicit conversion
    return pointeeType_->isExplicitlyConvertibleTo(otherPointer->pointeeType_);
}

bool PointerType::isCompatibleWith(const Type* other) const {
    auto otherPointer = dynamic_cast<const PointerType*>(other);
    if (!otherPointer) return false;

    // Pointers are compatible if pointee types are compatible
    return pointeeType_->isCompatibleWith(otherPointer->pointeeType_);
}

size_t PointerType::getStaticSize() const {
    return getPointerSize();
}

size_t PointerType::getAlignment() const {
    return getPointerSize();
}

// ReferenceType implementation
ReferenceType::ReferenceType(const Type* referentType, Flags flags)
    : CompositeType(nullptr, flags), referentType_(referentType) {
    // References don't need AST reference for basic functionality
}

bool ReferenceType::equals(const Type* other) const {
    auto otherReference = dynamic_cast<const ReferenceType*>(other);
    if (!otherReference) return false;

    return referentType_->equals(otherReference->referentType_);
}

std::string ReferenceType::toString() const {
    return std::format("&{}", referentType_->toString());
}

size_t ReferenceType::hash() const {
    // Combine referent type hash with reference type discriminator
    size_t h1 = referentType_->hash();
    size_t h2 = std::hash<int>{}(static_cast<int>(typReference));
    return h1 ^ (h2 << 1);
}

bool ReferenceType::isAssignableFrom(const Type* other) const {
    auto otherReference = dynamic_cast<const ReferenceType*>(other);
    if (!otherReference) return false;

    // Exact equality first
    if (referentType_->equals(otherReference->referentType_)) return true;
    
    // Reference inheritance: &Derived can be assigned to &Base
    return referentType_->isAssignableFrom(otherReference->referentType_);
}

bool ReferenceType::isImplicitlyConvertibleTo(const Type* other) const {
    auto otherReference = dynamic_cast<const ReferenceType*>(other);
    if (!otherReference) return false;

    // Exact equality first
    if (referentType_->equals(otherReference->referentType_)) return true;
    
    // Reference inheritance: &Derived → &Base (implicit upcast)
    return referentType_->isImplicitlyConvertibleTo(otherReference->referentType_);
}

bool ReferenceType::isExplicitlyConvertibleTo(const Type* other) const {
    auto otherReference = dynamic_cast<const ReferenceType*>(other);
    if (!otherReference) return false;

    // Exact equality first
    if (referentType_->equals(otherReference->referentType_)) return true;
    
    // Reference inheritance: allow both upcast and downcast with explicit conversion
    return referentType_->isExplicitlyConvertibleTo(otherReference->referentType_);
}

bool ReferenceType::isCompatibleWith(const Type* other) const {
    auto otherReference = dynamic_cast<const ReferenceType*>(other);
    if (!otherReference) return false;

    // References are compatible if referent types are compatible
    return referentType_->isCompatibleWith(otherReference->referentType_);
}

size_t ReferenceType::getStaticSize() const {
    return getPointerSize();
}

size_t ReferenceType::getAlignment() const {
    return getPointerSize();
}

// ArrayType implementation
ArrayType::ArrayType(const Type* elementType, size_t size, Flags flags)
    : CompositeType(nullptr, flags), elementType_(elementType), size_(size) {
    // Arrays don't need AST reference for basic functionality
}

bool ArrayType::equals(const Type* other) const {
    auto otherArray = dynamic_cast<const ArrayType*>(other);
    if (!otherArray) return false;

    return elementType_->equals(otherArray->elementType_) &&
           size_ == otherArray->size_;
}

std::string ArrayType::toString() const {
    if (isDynamicArray()) {
        return std::format("[]{}",  elementType_->toString());
    } else {
        return std::format("[{}]{}", size_, elementType_->toString());
    }
}

size_t ArrayType::hash() const {
    // Combine element type hash with size
    size_t h1 = elementType_->hash();
    size_t h2 = std::hash<size_t>{}(size_);
    return h1 ^ (h2 << 1);
}

bool ArrayType::isAssignableFrom(const Type* other) const {
    auto otherArray = dynamic_cast<const ArrayType*>(other);
    if (!otherArray) return false;

    // Arrays are assignable if element types are assignable and sizes match
    return elementType_->isAssignableFrom(otherArray->elementType_) &&
           size_ == otherArray->size_;
}

bool ArrayType::isImplicitlyConvertibleTo(const Type* other) const {
    auto otherArray = dynamic_cast<const ArrayType*>(other);
    if (!otherArray) return false;

    // Fixed arrays can convert to dynamic arrays of same element type
    if (isFixedArray() && otherArray->isDynamicArray()) {
        return elementType_->equals(otherArray->elementType_);
    }

    return false;
}

bool ArrayType::isExplicitlyConvertibleTo(const Type* other) const {
    auto otherArray = dynamic_cast<const ArrayType*>(other);
    if (!otherArray) return false;

    // Arrays can be explicitly converted if element types can be converted
    return elementType_->isExplicitlyConvertibleTo(otherArray->elementType_);
}

bool ArrayType::isCompatibleWith(const Type* other) const {
    auto otherArray = dynamic_cast<const ArrayType*>(other);
    if (!otherArray) return false;

    // Arrays are compatible if element types are compatible
    return elementType_->isCompatibleWith(otherArray->elementType_);
}

size_t ArrayType::getStaticSize() const {
    if (isDynamicArray()) {
        // Dynamic arrays are stored as pointers
        return getPointerSize();
    } else {
        // Fixed arrays: element_size * count
        return elementType_->getStaticSize() * size_;
    }
}

size_t ArrayType::getAlignment() const {
    if (isDynamicArray()) {
        // Dynamic arrays have pointer alignment
        return getPointerSize();
    } else {
        // Fixed arrays have element type alignment
        return elementType_->getAlignment();
    }
}

bool ArrayType::hasStaticSize() const {
    if (isDynamicArray()) {
        return true; // Dynamic arrays have static pointer size
    } else {
        // Fixed arrays have static size if element type does
        return elementType_->hasStaticSize();
    }
}

bool ArrayType::isDynamicallySized() const {
    if (isDynamicArray()) {
        return false; // Dynamic arrays themselves have fixed pointer size
    } else {
        // Fixed arrays are dynamically sized if element type is
        return elementType_->isDynamicallySized();
    }
}



// Helper function for aligning offsets
static size_t alignUp(size_t offset, size_t alignment) {
    return (offset + alignment - 1) & ~(alignment - 1);
}

// TupleType implementation
TupleType::TupleType(const ArenaVector<const Type*>& elementTypes, Flags flags)
    : CompositeType(nullptr, flags), elementTypes_(elementTypes) {
    // Tuples don't need AST reference for basic functionality
    // Require at least one element (no empty tuples)
    if (elementTypes_.empty()) {
        // This should be caught at a higher level, but for safety
        throw std::invalid_argument("TupleType requires at least one element");
    }
}

bool TupleType::equals(const Type* other) const {
    auto otherTuple = dynamic_cast<const TupleType*>(other);
    if (!otherTuple) return false;

    if (elementTypes_.size() != otherTuple->elementTypes_.size()) return false;

    for (size_t i = 0; i < elementTypes_.size(); ++i) {
        if (!elementTypes_[i]->equals(otherTuple->elementTypes_[i])) return false;
    }

    return true;
}

std::string TupleType::toString() const {
    std::string result = "(";
    for (size_t i = 0; i < elementTypes_.size(); ++i) {
        if (i > 0) result += ", ";
        result += elementTypes_[i]->toString();
    }
    result += ")";
    return result;
}

size_t TupleType::hash() const {
    size_t hash = 0;
    for (const Type* type : elementTypes_) {
        hash ^= type->hash() + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    }
    return hash;
}

bool TupleType::isAssignableFrom(const Type* other) const {
    auto otherTuple = dynamic_cast<const TupleType*>(other);
    if (!otherTuple) return false;

    if (elementTypes_.size() != otherTuple->elementTypes_.size()) return false;

    for (size_t i = 0; i < elementTypes_.size(); ++i) {
        if (!elementTypes_[i]->isAssignableFrom(otherTuple->elementTypes_[i])) return false;
    }

    return true;
}

bool TupleType::isImplicitlyConvertibleTo(const Type* other) const {
    auto otherTuple = dynamic_cast<const TupleType*>(other);
    if (!otherTuple) return false;

    if (elementTypes_.size() != otherTuple->elementTypes_.size()) return false;

    for (size_t i = 0; i < elementTypes_.size(); ++i) {
        if (!elementTypes_[i]->isImplicitlyConvertibleTo(otherTuple->elementTypes_[i])) return false;
    }

    return true;
}

bool TupleType::isExplicitlyConvertibleTo(const Type* other) const {
    auto otherTuple = dynamic_cast<const TupleType*>(other);
    if (!otherTuple) return false;

    if (elementTypes_.size() != otherTuple->elementTypes_.size()) return false;

    for (size_t i = 0; i < elementTypes_.size(); ++i) {
        if (!elementTypes_[i]->isExplicitlyConvertibleTo(otherTuple->elementTypes_[i])) return false;
    }

    return true;
}

bool TupleType::isCompatibleWith(const Type* other) const {
    auto otherTuple = dynamic_cast<const TupleType*>(other);
    if (!otherTuple) return false;

    if (elementTypes_.size() != otherTuple->elementTypes_.size()) return false;

    for (size_t i = 0; i < elementTypes_.size(); ++i) {
        if (!elementTypes_[i]->isCompatibleWith(otherTuple->elementTypes_[i])) return false;
    }

    return true;
}

size_t TupleType::getStaticSize() const {
    if (elementTypes_.empty()) return 0;

    size_t totalSize = 0;
    size_t maxAlignment = 1;

    // Calculate size with proper alignment padding
    for (const Type* elementType : elementTypes_) {
        size_t elementSize = elementType->getStaticSize();
        size_t elementAlignment = elementType->getAlignment();

        maxAlignment = std::max(maxAlignment, elementAlignment);
        totalSize = alignUp(totalSize, elementAlignment);
        totalSize += elementSize;
    }

    // Align final size to maximum alignment
    return alignUp(totalSize, maxAlignment);
}

size_t TupleType::getAlignment() const {
    size_t maxAlignment = 1;

    for (const Type* elementType : elementTypes_) {
        maxAlignment = std::max(maxAlignment, elementType->getAlignment());
    }

    return maxAlignment;
}

bool TupleType::hasStaticSize() const {
    for (const Type* elementType : elementTypes_) {
        if (!elementType->hasStaticSize()) return false;
    }
    return true;
}

bool TupleType::isDynamicallySized() const {
    for (const Type* elementType : elementTypes_) {
        if (elementType->isDynamicallySized()) return true;
    }
    return false;
}



// UnionType implementation
UnionType::UnionType(const ArenaVector<const Type*>& variantTypes, Flags flags)
    : CompositeType(nullptr, flags), variantTypes_(variantTypes) {
    // Unions don't need AST reference for basic functionality
    // Require at least one variant (no empty unions)
    if (variantTypes_.empty()) {
        // This should be caught at a higher level, but for safety
        throw std::invalid_argument("UnionType requires at least one variant");
    }
}

bool UnionType::equals(const Type* other) const {
    auto otherUnion = dynamic_cast<const UnionType*>(other);
    if (!otherUnion) return false;

    if (variantTypes_.size() != otherUnion->variantTypes_.size()) return false;

    for (size_t i = 0; i < variantTypes_.size(); ++i) {
        if (!variantTypes_[i]->equals(otherUnion->variantTypes_[i])) return false;
    }

    return true;
}

std::string UnionType::toString() const {
    if (variantTypes_.size() == 1) {
        // Single variant, no pipes needed
        return variantTypes_[0]->toString();
    }

    std::string result;
    for (size_t i = 0; i < variantTypes_.size(); ++i) {
        if (i > 0) result += " | ";
        result += variantTypes_[i]->toString();
    }
    return result;
}

size_t UnionType::hash() const {
    size_t hash = 0;
    for (const Type* type : variantTypes_) {
        hash ^= type->hash() + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    }
    return hash;
}

bool UnionType::isAssignableFrom(const Type* other) const {
    // A union is assignable from any of its variant types
    for (const Type* variantType : variantTypes_) {
        if (variantType->isAssignableFrom(other)) return true;
    }

    // Also check if other is a union that's a subset of this union
    auto otherUnion = dynamic_cast<const UnionType*>(other);
    if (otherUnion) {
        for (const Type* otherVariant : otherUnion->variantTypes_) {
            bool found = false;
            for (const Type* thisVariant : variantTypes_) {
                if (thisVariant->isAssignableFrom(otherVariant)) {
                    found = true;
                    break;
                }
            }
            if (!found) return false;
        }
        return true;
    }

    return false;
}

bool UnionType::isImplicitlyConvertibleTo(const Type* other) const {
    auto otherUnion = dynamic_cast<const UnionType*>(other);
    if (!otherUnion) return false;

    // This union can convert to another union if all variants can convert
    for (const Type* thisVariant : variantTypes_) {
        bool canConvert = false;
        for (const Type* otherVariant : otherUnion->variantTypes_) {
            if (thisVariant->isImplicitlyConvertibleTo(otherVariant)) {
                canConvert = true;
                break;
            }
        }
        if (!canConvert) return false;
    }

    return true;
}

bool UnionType::isExplicitlyConvertibleTo(const Type* other) const {
    auto otherUnion = dynamic_cast<const UnionType*>(other);
    if (!otherUnion) return false;

    // This union can explicitly convert to another union if all variants can convert
    for (const Type* thisVariant : variantTypes_) {
        bool canConvert = false;
        for (const Type* otherVariant : otherUnion->variantTypes_) {
            if (thisVariant->isExplicitlyConvertibleTo(otherVariant)) {
                canConvert = true;
                break;
            }
        }
        if (!canConvert) return false;
    }

    return true;
}

bool UnionType::isCompatibleWith(const Type* other) const {
    auto otherUnion = dynamic_cast<const UnionType*>(other);
    if (!otherUnion) {
        // Check if the other type is compatible with any variant
        for (const Type* variantType : variantTypes_) {
            if (variantType->isCompatibleWith(other)) return true;
        }
        return false;
    }

    // Two unions are compatible if they have overlapping variants
    for (const Type* thisVariant : variantTypes_) {
        for (const Type* otherVariant : otherUnion->variantTypes_) {
            if (thisVariant->isCompatibleWith(otherVariant)) return true;
        }
    }

    return false;
}

size_t UnionType::getStaticSize() const {
    if (variantTypes_.empty()) return 0;

    size_t maxSize = 0;
    for (const Type* variantType : variantTypes_) {
        maxSize = std::max(maxSize, variantType->getStaticSize());
    }

    return maxSize;
}

size_t UnionType::getAlignment() const {
    size_t maxAlignment = 1;

    for (const Type* variantType : variantTypes_) {
        maxAlignment = std::max(maxAlignment, variantType->getAlignment());
    }

    return maxAlignment;
}

bool UnionType::hasStaticSize() const {
    for (const Type* variantType : variantTypes_) {
        if (!variantType->hasStaticSize()) return false;
    }
    return true;
}

bool UnionType::isDynamicallySized() const {
    for (const Type* variantType : variantTypes_) {
        if (variantType->isDynamicallySized()) return true;
    }
    return false;
}



// FunctionType implementation
FunctionType::FunctionType(const ArenaVector<const Type*>& parameterTypes, const Type* returnType, Flags flags)
    : CompositeType(nullptr, flags), parameterTypes_(parameterTypes), returnType_(returnType) {
    // Functions don't need AST reference for basic functionality
    // Return type must not be null
    if (!returnType_) {
        throw std::invalid_argument("FunctionType requires a non-null return type");
    }

    // Validate parameter types are not null
    for (size_t i = 0; i < parameterTypes_.size(); ++i) {
        if (!parameterTypes_[i]) {
            throw std::invalid_argument("FunctionType parameter type at index " + std::to_string(i) + " is null");
        }
    }
}

bool FunctionType::equals(const Type* other) const {
    auto otherFunc = dynamic_cast<const FunctionType*>(other);
    if (!otherFunc) return false;

    // Check return type
    if (!returnType_->equals(otherFunc->returnType_)) return false;

    // Check parameter count and types
    if (parameterTypes_.size() != otherFunc->parameterTypes_.size()) return false;

    for (size_t i = 0; i < parameterTypes_.size(); ++i) {
        if (!parameterTypes_[i]->equals(otherFunc->parameterTypes_[i])) return false;
    }

    return true;
}

std::string FunctionType::toString() const {
    std::string result = "(";
    for (size_t i = 0; i < parameterTypes_.size(); ++i) {
        if (i > 0) result += ", ";
        if (!parameterTypes_[i]) {
            result += "<null>";
        } else {
            result += parameterTypes_[i]->toString();
        }
    }
    result += ") -> ";

    // Add parentheses around function return types
    if (returnType_->kind() == typFunction) {
        result += "(";
        result += returnType_->toString();
        result += ")";
    } else {
        result += returnType_->toString();
    }

    return result;
}

size_t FunctionType::hash() const {
    size_t hash = returnType_->hash();
    for (const Type* type : parameterTypes_) {
        hash ^= type->hash() + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    }
    return hash;
}

bool FunctionType::isAssignableFrom(const Type* other) const {
    auto otherFunc = dynamic_cast<const FunctionType*>(other);
    if (!otherFunc) return false;

    // Function types are assignable if they have identical signatures
    return equals(other);
}

bool FunctionType::isImplicitlyConvertibleTo(const Type* other) const {
    auto otherFunc = dynamic_cast<const FunctionType*>(other);
    if (!otherFunc) return false;

    // For now, function types can only implicitly convert if they're identical
    return equals(other);
}

bool FunctionType::isExplicitlyConvertibleTo(const Type* other) const {
    auto otherFunc = dynamic_cast<const FunctionType*>(other);
    if (!otherFunc) return false;

    // Function types can explicitly convert if signatures are compatible
    // For now, use same logic as implicit conversion
    return equals(other);
}

bool FunctionType::isCompatibleWith(const Type* other) const {
    auto otherFunc = dynamic_cast<const FunctionType*>(other);
    if (!otherFunc) return false;

    // Functions are compatible if they have the same signature
    return equals(other);
}

size_t FunctionType::getStaticSize() const {
    // Function types are pointer-sized (they represent function pointers)
    return getPointerSize();
}

size_t FunctionType::getAlignment() const {
    // Function types have pointer alignment
    return getPointerSize();
}

bool FunctionType::hasStaticSize() const {
    // Function types always have static size (pointer size)
    return true;
}

bool FunctionType::isDynamicallySized() const {
    // Function types are never dynamically sized
    return false;
}

bool FunctionType::canBeCalledWith(const ArenaVector<const Type*>& argumentTypes) const {
    // Quick check: argument count must match parameter count exactly
    if (argumentTypes.size() != parameterTypes_.size()) {
        return false;
    }

    // Check if each argument can be implicitly converted to the corresponding parameter
    for (size_t i = 0; i < argumentTypes.size(); ++i) {
        const Type* argumentType = argumentTypes[i];
        const Type* parameterType = parameterTypes_[i];

        // Exact match is always allowed
        if (argumentType->equals(parameterType)) {
            continue;
        }

        // Check if argument can be passed to parameter (more permissive for function calls)
        if (!argumentType->canBeImplicitlyPassedTo(parameterType)) {
            return false;
        }
    }

    return true;
}

int FunctionType::getConversionDistance(const ArenaVector<const Type*>& argumentTypes) const {
    // Cannot call if argument count doesn't match
    if (argumentTypes.size() != parameterTypes_.size()) {
        return -1;
    }

    int totalDistance = 0;

    for (size_t i = 0; i < argumentTypes.size(); ++i) {
        const Type* argumentType = argumentTypes[i];
        const Type* parameterType = parameterTypes_[i];

        // Exact match has distance 0
        if (argumentType->equals(parameterType)) {
            continue;
        }

        // Check if argument can be passed to parameter (more permissive for function calls)
        if (!argumentType->canBeImplicitlyPassedTo(parameterType)) {
            return -1; // Impossible conversion
        }

        // Calculate conversion distance
        int conversionDistance = 1; // Base cost for any conversion

        // Add specific conversion costs for numeric types
        if (argumentType->isNumeric() && parameterType->isNumeric()) {
            // Integer to integer conversions
            if (argumentType->isIntegral() && parameterType->isIntegral()) {
                auto argInt = argumentType->as<IntegerType>();
                auto paramInt = parameterType->as<IntegerType>();
                if (argInt && paramInt) {
                    // Widening conversions are cheaper than narrowing
                    if (argInt->getBitWidth() < paramInt->getBitWidth()) {
                        conversionDistance = 1; // Widening
                    } else if (argInt->getBitWidth() > paramInt->getBitWidth()) {
                        conversionDistance = 3; // Narrowing (more expensive)
                    } else {
                        conversionDistance = 2; // Same width but different signedness
                    }
                }
            }
            // Float to float conversions
            else if (argumentType->isFloatingPoint() && parameterType->isFloatingPoint()) {
                auto argFloat = argumentType->as<FloatType>();
                auto paramFloat = parameterType->as<FloatType>();
                if (argFloat && paramFloat) {
                    // f32 -> f64 is cheaper than f64 -> f32
                    if (argFloat->getBitWidth() < paramFloat->getBitWidth()) {
                        conversionDistance = 1; // Widening
                    } else {
                        conversionDistance = 3; // Narrowing
                    }
                }
            }
            // Integer to float conversions
            else if (argumentType->isIntegral() && parameterType->isFloatingPoint()) {
                conversionDistance = 2; // Standard numeric conversion
            }
            // Float to integer conversions
            else if (argumentType->isFloatingPoint() && parameterType->isIntegral()) {
                conversionDistance = 4; // More expensive (potential data loss)
            }
        }

        totalDistance += conversionDistance;
    }

    return totalDistance;
}

// =============================================================================
// RecordType Implementation
// =============================================================================

RecordType::RecordType(InternedString name,
                       ArenaVector<FieldType> fields,
                       ArenaVector<MethodType> methods,
                       Flags flags,
                       const ast::ASTNode* sourceAST,
                       ArenaAllocator& arena)
    : CompositeType(sourceAST, flags), name_(std::move(name)), fields_(std::move(fields)), methods_(std::move(methods)), arena_(arena) {}

const Type* RecordType::getFieldType(const InternedString& name) const {
    for (const auto& field : fields_) {
        if (field.name == name) {
            return field.type;
        }
    }
    return nullptr;
}

bool RecordType::hasField(const InternedString& name) const {
    return getFieldType(name) != nullptr;
}

size_t RecordType::getFieldIndex(const InternedString& name) const {
    for (size_t i = 0; i < fields_.size(); ++i) {
        if (fields_[i].name == name) {
            return i;
        }
    }
    return SIZE_MAX;
}

size_t RecordType::getFieldOffset(size_t index) const {
    if (index >= fields_.size()) {
        return SIZE_MAX;
    }

    size_t offset = 0;
    
    // Natural layout: align each field
    for (size_t i = 0; i < index; ++i) {
        const Type* fieldType = fields_[i].type;
        size_t fieldAlignment = fieldType->getAlignment();
        
        // Align offset to field alignment
        offset = alignUp(offset, fieldAlignment);
        offset += fieldType->getStaticSize();
    }
    
    // Align current field
    if (index < fields_.size()) {
        const Type* currentFieldType = fields_[index].type;
        size_t currentFieldAlignment = currentFieldType->getAlignment();
        offset = alignUp(offset, currentFieldAlignment);
    }
    
    return offset;
}

size_t RecordType::getFieldOffset(const InternedString& name) const {
    size_t index = getFieldIndex(name);
    return index != SIZE_MAX ? getFieldOffset(index) : SIZE_MAX;
}

ArenaVector<const RecordType::MethodType*> RecordType::getMethodsByName(const InternedString& name) const {
    ArenaVector<const MethodType*> result{ArenaSTLAllocator<const MethodType*>(arena_)};
    for (const auto& method : methods_) {
        if (method.name == name) {
            result.push_back(&method);
        }
    }
    return result;
}

const RecordType::MethodType* RecordType::getMethod(const InternedString& name, const FunctionType* signature) const {
    for (const auto& method : methods_) {
        if (method.name == name && method.signature->equals(signature)) {
            return &method;
        }
    }
    return nullptr;
}

bool RecordType::hasMethod(const InternedString& name) const {
    for (const auto& method : methods_) {
        if (method.name == name) {
            return true;
        }
    }
    return false;
}

size_t RecordType::getMethodIndex(const InternedString& name, const FunctionType* signature) const {
    for (size_t i = 0; i < methods_.size(); ++i) {
        if (methods_[i].name == name && methods_[i].signature->equals(signature)) {
            return i;
        }
    }
    return SIZE_MAX;
}

size_t RecordType::calculateNaturalSize() const {
    if (fields_.empty()) {
        return 1; // C++ style: empty structs have size 1
    }

    size_t totalSize = 0;
    size_t maxAlignment = 1;

    for (const auto& field : fields_) {
        const Type* fieldType = field.type;
        size_t fieldSize = fieldType->getStaticSize();
        size_t fieldAlignment = fieldType->getAlignment();

        // Track maximum alignment for struct alignment
        maxAlignment = std::max(maxAlignment, fieldAlignment);

        // Align current position to field's alignment
        totalSize = alignUp(totalSize, fieldAlignment);
        totalSize += fieldSize;
    }

    // Align total size to struct's alignment (max field alignment)
    totalSize = alignUp(totalSize, maxAlignment);
    return totalSize;
}

bool RecordType::equals(const Type* other) const {
    const auto* otherRecord = dynamic_cast<const RecordType*>(other);
    if (!otherRecord) return false;

    // Compare name, flags, field count, and method count
    if (name_ != otherRecord->name_ || 
        getFlags() != otherRecord->getFlags() ||
        fields_.size() != otherRecord->fields_.size() ||
        methods_.size() != otherRecord->methods_.size()) {
        return false;
    }

    // Compare each field
    for (size_t i = 0; i < fields_.size(); ++i) {
        const auto& field = fields_[i];
        const auto& otherField = otherRecord->fields_[i];
        
        if (field.name != otherField.name || !field.type->equals(otherField.type)) {
            return false;
        }
    }
    
    // Compare each method
    for (size_t i = 0; i < methods_.size(); ++i) {
        const auto& method = methods_[i];
        const auto& otherMethod = otherRecord->methods_[i];
        
        if (method.name != otherMethod.name || !method.signature->equals(otherMethod.signature)) {
            return false;
        }
    }
    
    return true;
}

std::string RecordType::toString() const {
    std::string result;

    if (!name_.empty()) {
        result = name_.view();
    } else {
        result = getTypeKeyword();
    }

    // Add any type-specific keywords (like "packed" for structs)
    // This is virtual so derived classes can override

    if (fields_.empty() && methods_.empty()) {
        result += " {}";
    } else {
        result += " { ";
        
        // Add fields
        for (size_t i = 0; i < fields_.size(); ++i) {
            if (i > 0) result += ", ";
            result += std::string(fields_[i].name.view()) + ": " + fields_[i].type->toString();
        }
        
        // Add methods
        for (size_t i = 0; i < methods_.size(); ++i) {
            if (i > 0 || !fields_.empty()) result += ", ";
            result += "func " + std::string(methods_[i].name.view()) + methods_[i].signature->toString();
        }
        
        result += " }";
    }
    return result;
}

size_t RecordType::hash() const {
    size_t result = name_.getHash();
    result ^= std::hash<uint64_t>{}(static_cast<uint64_t>(getFlags())) << 1;

    for (const auto& field : fields_) {
        result ^= field.name.getHash() << 2;
        result ^= field.type->hash() << 3;
    }
    
    for (const auto& method : methods_) {
        result ^= method.name.getHash() << 4;
        result ^= method.signature->hash() << 5;
    }
    
    return result;
}

bool RecordType::isAssignableFrom(const Type* other) const {
    // Records are only assignable from identical record types
    return equals(other);
}

bool RecordType::isImplicitlyConvertibleTo(const Type* other) const {
    // Records don't have implicit conversions to other types
    return false;
}

bool RecordType::isExplicitlyConvertibleTo(const Type* other) const {
    // Records don't have explicit conversions to other types
    return false;
}

bool RecordType::isCompatibleWith(const Type* other) const {
    return equals(other);
}

bool RecordType::hasStaticSize() const {
    // All record fields must have static size for the record to have static size
    for (const auto& field : fields_) {
        if (!field.type->hasStaticSize()) {
            return false;
        }
    }
    return true;
}

bool RecordType::isDynamicallySized() const {
    return !hasStaticSize();
}

size_t RecordType::alignUp(size_t size, size_t alignment) const {
    return (size + alignment - 1) & ~(alignment - 1);
}

// StructType Implementation  
// =============================================================================

StructType::StructType(InternedString name,
                       ArenaVector<FieldType> fields,
                       ArenaVector<MethodType> methods,
                       Flags flags,
                       const ast::ASTNode* sourceAST,
                       ArenaAllocator& arena)
    : RecordType(std::move(name), std::move(fields), std::move(methods), flags, sourceAST, arena) {}

size_t StructType::calculatePackedSize() const {
    if (getFieldCount() == 0) {
        return 1; // C++ style: empty structs have size 1
    }
    
    size_t totalSize = 0;
    const auto& fields = getFields();
    for (const auto& field : fields) {
        totalSize += field.type->getStaticSize();
    }
    return totalSize;
}

std::string StructType::toString() const {
    std::string result;

    if (!getName().empty()) {
        result = getName().view();
    } else {
        result = "struct";
    }

    if (isPacked()) {
        result += " packed";
    }

    if (getFieldCount() == 0 && getMethodCount() == 0) {
        result += " {}";
    } else {
        result += " { ";
        
        // Add fields
        const auto& fields = getFields();
        for (size_t i = 0; i < fields.size(); ++i) {
            if (i > 0) result += ", ";
            result += std::string(fields[i].name.view()) + ": " + fields[i].type->toString();
        }
        
        // Add methods
        const auto& methods = getMethods();
        for (size_t i = 0; i < methods.size(); ++i) {
            if (i > 0 || !fields.empty()) result += ", ";
            result += "func " + std::string(methods[i].name.view()) + methods[i].signature->toString();
        }
        
        result += " }";
    }
    return result;
}



size_t StructType::getFieldOffset(size_t index) const {
    if (index >= getFieldCount()) {
        return SIZE_MAX;
    }

    size_t offset = 0;
    const auto& fields = getFields();
    
    if (isPacked()) {
        // Packed layout: no padding between fields
        for (size_t i = 0; i < index; ++i) {
            offset += fields[i].type->getStaticSize();
        }
    } else {
        // Natural layout: align each field
        for (size_t i = 0; i < index; ++i) {
            const Type* fieldType = fields[i].type;
            size_t fieldAlignment = fieldType->getAlignment();
            
            // Align offset to field alignment
            offset = alignUp(offset, fieldAlignment);
            offset += fieldType->getStaticSize();
        }
        
        // Align current field
        if (index < fields.size()) {
            const Type* currentFieldType = fields[index].type;
            size_t currentFieldAlignment = currentFieldType->getAlignment();
            offset = alignUp(offset, currentFieldAlignment);
        }
    }
    
    return offset;
}

size_t StructType::getStaticSize() const {
    if (isPacked()) {
        return calculatePackedSize();
    } else {
        return calculateNaturalSize();
    }
}

size_t StructType::getAlignment() const {
    if (isPacked()) {
        return 1; // Packed structs have no alignment requirements
    }
    
    if (getFieldCount() == 0) {
        return 1;
    }
    
    size_t maxAlignment = 1;
    const auto& fields = getFields();
    for (const auto& field : fields) {
        maxAlignment = std::max(maxAlignment, field.type->getAlignment());
    }
    return maxAlignment;
}

// =============================================================================
// ClassType Implementation
// =============================================================================

ClassType::ClassType(InternedString name,
                     ArenaVector<FieldType> fields,
                     ArenaVector<MethodType> methods,
                     const ClassType* baseClass,
                     Flags flags,
                     const ast::ASTNode* sourceAST,
                     ArenaAllocator& arena)
    : RecordType(std::move(name), std::move(fields), std::move(methods), flags, sourceAST, arena),
      baseClass_(baseClass) {}

bool ClassType::isBaseOf(const ClassType* derived) const {
    if (!derived) return false;
    
    // Check direct inheritance
    if (derived->baseClass_ == this) {
        return true;
    }
    
    // Check indirect inheritance through base class chain
    if (derived->baseClass_) {
        return isBaseOf(derived->baseClass_);
    }
    
    return false;
}

bool ClassType::isDerivedFrom(const ClassType* base) const {
    if (!base) return false;
    return base->isBaseOf(this);
}

const ClassType* ClassType::findCommonBase(const ClassType* other) const {
    if (!other) return nullptr;
    if (this == other) return this;
    
    // Walk up this class's inheritance chain
    const ClassType* myAncestor = this;
    while (myAncestor) {
        // Check if other class derives from this ancestor
        if (other->isDerivedFrom(myAncestor)) {
            return myAncestor;
        }
        myAncestor = myAncestor->baseClass_;
    }
    
    // If no common base found walking up this chain,
    // try walking up other's chain
    const ClassType* otherAncestor = other;
    while (otherAncestor) {
        if (this->isDerivedFrom(otherAncestor)) {
            return otherAncestor;
        }
        otherAncestor = otherAncestor->baseClass_;
    }
    
    return nullptr;
}

bool ClassType::hasVirtualMethods() const {
    // Check if this class has virtual flag set
    if (hasFlag(flgVirtual)) {
        return true;
    }
    
    // Check base class for virtual methods
    if (baseClass_ && baseClass_->hasVirtualMethods()) {
        return true;
    }
    
    return false;
}

bool ClassType::isAbstract() const {
    // Check if this class has abstract flag set
    if (hasFlag(flgAbstract)) {
        return true;
    }
    
    // A class is abstract if it inherits abstract methods that are not overridden
    if (baseClass_ && baseClass_->isAbstract()) {
        // Check if this class provides implementations for all abstract methods
        for (const auto& baseMethod : baseClass_->getMethods()) {
            if (baseClass_->hasFlag(flgAbstract)) {
                // Check if this class overrides this abstract method
                const MethodType* override = getMethod(baseMethod.name, baseMethod.signature);
                if (!override || !hasFlag(flgOverride)) {
                    return true; // Still abstract - unimplemented abstract method
                }
            }
        }
    }
    
    return false;
}

const ClassType::MethodType* ClassType::resolveVirtualMethod(const InternedString& name, const FunctionType* signature) const {
    // First check this class's methods (overrides take priority)
    const MethodType* method = getMethod(name, signature);
    if (method) {
        // If this class has the method, it's either an override or original definition
        return method;
    }
    
    // Then check base class for inherited methods
    if (baseClass_) {
        return baseClass_->resolveVirtualMethod(name, signature);
    }
    
    return nullptr;
}

size_t ClassType::getStaticSize() const {
    // Classes have reference semantics - size is typically pointer size
    // TODO: Consider if this should include vtable pointer size
    return sizeof(void*);
}

size_t ClassType::getAlignment() const {
    // Classes are pointer-aligned
    return alignof(void*);
}

// Field access overrides for inheritance support
const Type* ClassType::getFieldType(const InternedString& name) const {
    // First check this class's fields
    const Type* fieldType = RecordType::getFieldType(name);
    if (fieldType) return fieldType;
    
    // Then check base class
    if (baseClass_) {
        return baseClass_->getFieldType(name);
    }
    
    return nullptr;
}

bool ClassType::hasField(const InternedString& name) const {
    return getFieldType(name) != nullptr;
}

size_t ClassType::getFieldIndex(const InternedString& name) const {
    // For logical type system semantics, only check this class's own fields
    // This maintains the contract that getFieldIndex returns the index within the class
    size_t localIndex = RecordType::getFieldIndex(name);
    if (localIndex != SIZE_MAX) {
        return localIndex;
    }
    
    // Field not found in this class - this is correct behavior for type system
    // Use hasField() or getFieldType() to check inherited fields
    return SIZE_MAX;
}

size_t ClassType::getFlattenedFieldIndex(const InternedString& name) const {
    // Implement flattened field indexing for code generation
    // Layout: [base_fields...] [this_class_fields...]
    
    size_t currentIndex = 0;
    
    // First, count fields from base class
    if (baseClass_) {
        size_t baseIndex = baseClass_->getFlattenedFieldIndex(name);
        if (baseIndex != SIZE_MAX) {
            // Found in base class - return its flattened index
            return baseIndex;
        }
        // Add base class's flattened field count to our running index
        currentIndex += baseClass_->getFlattenedFieldCount();
    }
    
    // Then check this class's own fields
    size_t localIndex = RecordType::getFieldIndex(name);
    if (localIndex != SIZE_MAX) {
        // Found in this class - return flattened index
        return currentIndex + localIndex;
    }
    
    // Not found anywhere
    return SIZE_MAX;
}

// Flattened field layout helpers for code generation
size_t ClassType::getFlattenedFieldCount() const {
    size_t totalFields = 0;
    
    // Count fields from base class
    if (baseClass_) {
        totalFields += baseClass_->getFlattenedFieldCount();
    }
    
    // Add this class's own fields
    totalFields += getFieldCount();
    
    return totalFields;
}

size_t ClassType::getFlattenedFieldOffset(const InternedString& name) const {
    size_t flattenedIndex = getFlattenedFieldIndex(name);
    if (flattenedIndex == SIZE_MAX) {
        return SIZE_MAX;  // Field not found
    }
    
    return getFlattenedFieldOffset(flattenedIndex);
}

size_t ClassType::getFlattenedFieldOffset(size_t flattenedIndex) const {
    if (flattenedIndex >= getFlattenedFieldCount()) {
        return SIZE_MAX;  // Index out of bounds
    }
    
    size_t currentOffset = 0;
    size_t currentIndex = 0;
    
    // Process base class first
    if (baseClass_) {
        size_t baseFieldCount = baseClass_->getFlattenedFieldCount();
        if (flattenedIndex < baseFieldCount) {
            // Field is in base class
            return baseClass_->getFlattenedFieldOffset(flattenedIndex);
        }
        
        // Add base class's size to our running offset
        currentOffset += baseClass_->calculateNaturalSize();
        currentIndex += baseFieldCount;
    }
    
    // Field must be in this class
    size_t localIndex = flattenedIndex - currentIndex;
    const auto& fields = getFields();
    if (localIndex >= fields.size()) {
        return SIZE_MAX;  // Should not happen
    }
    
    // Calculate offset within this class's fields
    size_t localOffset = 0;
    for (size_t i = 0; i < localIndex; ++i) {
        const Type* fieldType = fields[i].type;
        size_t fieldAlignment = fieldType->getAlignment();
        localOffset = alignUp(localOffset, fieldAlignment);
        localOffset += fieldType->getStaticSize();
    }
    
    // Align for the requested field
    const Type* targetFieldType = fields[localIndex].type;
    size_t targetAlignment = targetFieldType->getAlignment();
    localOffset = alignUp(localOffset, targetAlignment);
    
    return currentOffset + localOffset;
}

// Type relationship queries (inheritance-aware overrides)
bool ClassType::isAssignableFrom(const Type* other) const {
    // Check exact type equality first
    if (equals(other)) return true;
    
    // Check if other is a derived class of this base class
    auto otherClass = dynamic_cast<const ClassType*>(other);
    if (!otherClass) return false;
    
    return isBaseOf(otherClass);
}

bool ClassType::isImplicitlyConvertibleTo(const Type* other) const {
    auto otherClass = dynamic_cast<const ClassType*>(other);
    if (!otherClass) return false;
    
    // Check exact type equality first
    if (equals(other)) return true;
    
    // Implicit upcast: derived → base (safe)
    // This class can be implicitly converted to other if other is a base of this
    return otherClass->isBaseOf(this);
}

bool ClassType::isExplicitlyConvertibleTo(const Type* other) const {
    auto otherClass = dynamic_cast<const ClassType*>(other);
    if (!otherClass) return false;
    
    // Check exact type equality first
    if (equals(other)) return true;
    
    // Explicit cast: allow both upcast and downcast
    // Upcast (derived → base): safe
    // Downcast (base → derived): potentially unsafe, but allowed with explicit cast
    return otherClass->isBaseOf(this) || this->isBaseOf(otherClass);
}

} // namespace cxy
