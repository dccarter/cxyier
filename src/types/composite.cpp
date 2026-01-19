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
        result += parameterTypes_[i]->toString();
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


} // namespace cxy