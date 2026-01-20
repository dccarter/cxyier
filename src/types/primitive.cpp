#include "cxy/types/primitive.hpp"
#include "cxy/arena_allocator.hpp"

#include <array>
#include <limits>
#include <cmath>
#include <unordered_map>

namespace cxy {

// =============================================================================
// IntegerType Implementation
// =============================================================================

IntegerType::IntegerType(::cxy::IntegerKind kind, Flags flags) : PrimitiveType(flags), integerKind_(kind) {}

bool IntegerType::equals(const Type* other) const {
    if (!other) return false;
    const auto* intType = dynamic_cast<const IntegerType*>(other);
    return intType && intType->integerKind_ == integerKind_;
}

std::string IntegerType::toString() const {
    switch (integerKind_) {
        case ::cxy::IntegerKind::I8:   return "i8";
        case ::cxy::IntegerKind::I16:  return "i16";
        case ::cxy::IntegerKind::I32:  return "i32";
        case ::cxy::IntegerKind::I64:  return "i64";
        case ::cxy::IntegerKind::I128: return "i128";
        case ::cxy::IntegerKind::U8:   return "u8";
        case ::cxy::IntegerKind::U16:  return "u16";
        case ::cxy::IntegerKind::U32:  return "u32";
        case ::cxy::IntegerKind::U64:  return "u64";
        case ::cxy::IntegerKind::U128: return "u128";
        case ::cxy::IntegerKind::Auto: return "auto_int";
        default: return "unknown_int";
    }
}

size_t IntegerType::hash() const {
    return std::hash<int>{}(static_cast<int>(integerKind_)) ^
           std::hash<int>{}(static_cast<int>(typInteger));
}

bool IntegerType::isAssignableFrom(const Type* other) const {
    return equals(other);
}

bool IntegerType::isImplicitlyConvertibleTo(const Type* other) const {
    // Check for integer-to-integer conversions
    const auto* otherInt = dynamic_cast<const IntegerType*>(other);
    if (otherInt) {
        // C-style implicit conversion rules
        size_t thisBits = getBitWidth();
        size_t otherBits = otherInt->getBitWidth();

        // Can always convert to larger type (widening)
        if (thisBits < otherBits) return true;

        // Unsigned can convert to signed if target is larger
        if (!isSigned() && otherInt->isSigned() && thisBits < otherBits) return true;

        return false;
    }

    // Check for integer-to-float conversions
    const auto* otherFloat = dynamic_cast<const FloatType*>(other);
    if (otherFloat) {
        // All integers can implicitly convert to floats (standard C-style behavior)
        return true;
    }

    return false;
}

bool IntegerType::isExplicitlyConvertibleTo(const Type* other) const {
    // All integer types can be explicitly converted to each other
    return dynamic_cast<const IntegerType*>(other) != nullptr;
}

bool IntegerType::isCompatibleWith(const Type* other) const {
    return isImplicitlyConvertibleTo(other) || isExplicitlyConvertibleTo(other);
}

bool IntegerType::canBeImplicitlyPassedTo(const Type* parameterType) const {
    // Check for integer-to-integer conversions with C-style function call rules
    const auto* paramInt = dynamic_cast<const IntegerType*>(parameterType);
    if (paramInt) {
        // Function calls allow more permissive conversions than assignment
        size_t thisBits = getBitWidth();
        size_t paramBits = paramInt->getBitWidth();

        // Widening is always allowed
        if (thisBits < paramBits) return true;

        // Narrowing is allowed in function calls (with potential warning)
        if (thisBits > paramBits) return true;

        // Same-size signed/unsigned conversions are allowed
        if (thisBits == paramBits) return true;

        return false;
    }

    // Check for integer-to-float conversions
    const auto* paramFloat = dynamic_cast<const FloatType*>(parameterType);
    if (paramFloat) {
        // All integers can be passed to float parameters
        return true;
    }

    // For other types, use default behavior
    return isImplicitlyConvertibleTo(parameterType);
}

size_t IntegerType::getStaticSize() const {
    switch (integerKind_) {
        case ::cxy::IntegerKind::I8:
        case ::cxy::IntegerKind::U8:   return 1;
        case ::cxy::IntegerKind::I16:
        case ::cxy::IntegerKind::U16:  return 2;
        case ::cxy::IntegerKind::I32:
        case ::cxy::IntegerKind::U32:  return 4;
        case ::cxy::IntegerKind::I64:
        case ::cxy::IntegerKind::U64:  return 8;
        case ::cxy::IntegerKind::I128:
        case ::cxy::IntegerKind::U128: return 16;
        case ::cxy::IntegerKind::Auto: return 4; // Default to i32
        default: return 4;
    }
}

size_t IntegerType::getAlignment() const {
    return getStaticSize(); // Simple alignment = size for integers
}

bool IntegerType::isSigned() const {
    switch (integerKind_) {
        case ::cxy::IntegerKind::I8:
        case ::cxy::IntegerKind::I16:
        case ::cxy::IntegerKind::I32:
        case ::cxy::IntegerKind::I64:
        case ::cxy::IntegerKind::I128:
        case ::cxy::IntegerKind::Auto: return true; // Auto defaults to signed
        case ::cxy::IntegerKind::U8:
        case ::cxy::IntegerKind::U16:
        case ::cxy::IntegerKind::U32:
        case ::cxy::IntegerKind::U64:
        case ::cxy::IntegerKind::U128: return false;
        default: return true;
    }
}

size_t IntegerType::getBitWidth() const {
    return getStaticSize() * 8;
}

__uint128_t IntegerType::getMinValue() const {
    if (!isSigned()) return 0;

    switch (integerKind_) {
        case ::cxy::IntegerKind::I8:   return static_cast<__uint128_t>(static_cast<__int128_t>(INT8_MIN));
        case ::cxy::IntegerKind::I16:  return static_cast<__uint128_t>(static_cast<__int128_t>(INT16_MIN));
        case ::cxy::IntegerKind::I32:  return static_cast<__uint128_t>(static_cast<__int128_t>(INT32_MIN));
        case ::cxy::IntegerKind::I64:  return static_cast<__uint128_t>(static_cast<__int128_t>(INT64_MIN));
        case ::cxy::IntegerKind::I128: return static_cast<__uint128_t>(static_cast<__int128_t>(-1) << 127);
        default: return static_cast<__uint128_t>(static_cast<__int128_t>(INT32_MIN));
    }
}

__uint128_t IntegerType::getMaxValue() const {
    switch (integerKind_) {
        case ::cxy::IntegerKind::I8:   return static_cast<__uint128_t>(INT8_MAX);
        case ::cxy::IntegerKind::I16:  return static_cast<__uint128_t>(INT16_MAX);
        case ::cxy::IntegerKind::I32:  return static_cast<__uint128_t>(INT32_MAX);
        case ::cxy::IntegerKind::I64:  return static_cast<__uint128_t>(INT64_MAX);
        case ::cxy::IntegerKind::I128: return static_cast<__uint128_t>((static_cast<__uint128_t>(1) << 127) - 1);
        case ::cxy::IntegerKind::U8:   return static_cast<__uint128_t>(UINT8_MAX);
        case ::cxy::IntegerKind::U16:  return static_cast<__uint128_t>(UINT16_MAX);
        case ::cxy::IntegerKind::U32:  return static_cast<__uint128_t>(UINT32_MAX);
        case ::cxy::IntegerKind::U64:  return static_cast<__uint128_t>(UINT64_MAX);
        case ::cxy::IntegerKind::U128: return ~static_cast<__uint128_t>(0);
        default: return static_cast<__uint128_t>(INT32_MAX);
    }
}




// =============================================================================
// FloatType Implementation
// =============================================================================

FloatType::FloatType(::cxy::FloatKind kind, Flags flags) : PrimitiveType(flags), floatKind_(kind) {}

bool FloatType::equals(const Type* other) const {
    if (!other) return false;
    const auto* floatType = dynamic_cast<const FloatType*>(other);
    return floatType && floatType->floatKind_ == floatKind_;
}

std::string FloatType::toString() const {
    switch (floatKind_) {
        case ::cxy::FloatKind::F32:  return "f32";
        case ::cxy::FloatKind::F64:  return "f64";
        case ::cxy::FloatKind::Auto: return "auto_float";
        default: return "unknown_float";
    }
}

size_t FloatType::hash() const {
    return std::hash<int>{}(static_cast<int>(floatKind_)) ^
           std::hash<int>{}(static_cast<int>(typFloat));
}

bool FloatType::isAssignableFrom(const Type* other) const {
    return equals(other);
}

bool FloatType::isImplicitlyConvertibleTo(const Type* other) const {
    const auto* otherFloat = dynamic_cast<const FloatType*>(other);
    if (!otherFloat) return false;

    // F32 can convert to F64, but not vice versa
    return floatKind_ == ::cxy::FloatKind::F32 && otherFloat->floatKind_ == ::cxy::FloatKind::F64;
}

bool FloatType::isExplicitlyConvertibleTo(const Type* other) const {
    // All float types can be explicitly converted to each other
    return dynamic_cast<const FloatType*>(other) != nullptr;
}

bool FloatType::isCompatibleWith(const Type* other) const {
    return isImplicitlyConvertibleTo(other) || isExplicitlyConvertibleTo(other);
}

size_t FloatType::getStaticSize() const {
    switch (floatKind_) {
        case ::cxy::FloatKind::F32:  return 4;
        case ::cxy::FloatKind::F64:  return 8;
        case ::cxy::FloatKind::Auto: return 8; // Default to f64
        default: return 8;
    }
}

size_t FloatType::getAlignment() const {
    return getStaticSize(); // Simple alignment = size for floats
}

size_t FloatType::getBitWidth() const {
    return getStaticSize() * 8;
}

double FloatType::getMinValue() const {
    switch (floatKind_) {
        case ::cxy::FloatKind::F32:  return -std::numeric_limits<float>::max();
        case ::cxy::FloatKind::F64:  return -std::numeric_limits<double>::max();
        default: return -std::numeric_limits<double>::max();
    }
}

double FloatType::getMaxValue() const {
    switch (floatKind_) {
        case ::cxy::FloatKind::F32:  return std::numeric_limits<float>::max();
        case ::cxy::FloatKind::F64:  return std::numeric_limits<double>::max();
        default: return std::numeric_limits<double>::max();
    }
}

double FloatType::getEpsilon() const {
    switch (floatKind_) {
        case ::cxy::FloatKind::F32:  return std::numeric_limits<float>::epsilon();
        case ::cxy::FloatKind::F64:  return std::numeric_limits<double>::epsilon();
        default: return std::numeric_limits<double>::epsilon();
    }
}




// =============================================================================
// BoolType Implementation
// =============================================================================

BoolType::BoolType(Flags flags) : PrimitiveType(flags) {}

bool BoolType::equals(const Type* other) const {
    return dynamic_cast<const BoolType*>(other) != nullptr;
}

std::string BoolType::toString() const {
    return "bool";
}

size_t BoolType::hash() const {
    return std::hash<int>{}(static_cast<int>(typBool));
}

bool BoolType::isAssignableFrom(const Type* other) const {
    return equals(other);
}

bool BoolType::isImplicitlyConvertibleTo(const Type* other) const {
    // Bool doesn't implicitly convert to anything
    return false;
}

bool BoolType::isExplicitlyConvertibleTo(const Type* other) const {
    // Bool can be explicitly converted to integers
    return dynamic_cast<const IntegerType*>(other) != nullptr;
}

bool BoolType::isCompatibleWith(const Type* other) const {
    return equals(other);
}

size_t BoolType::getStaticSize() const {
    return 1; // 1 byte for bool
}

size_t BoolType::getAlignment() const {
    return 1;
}



// =============================================================================
// CharType Implementation
// =============================================================================

CharType::CharType(Flags flags) : PrimitiveType(flags) {}

bool CharType::equals(const Type* other) const {
    return dynamic_cast<const CharType*>(other) != nullptr;
}

std::string CharType::toString() const {
    return "char";
}

size_t CharType::hash() const {
    return std::hash<int>{}(static_cast<int>(typChar));
}

bool CharType::isAssignableFrom(const Type* other) const {
    return equals(other);
}

bool CharType::isImplicitlyConvertibleTo(const Type* other) const {
    // Char can implicitly convert to integer types
    return dynamic_cast<const IntegerType*>(other) != nullptr;
}

bool CharType::isExplicitlyConvertibleTo(const Type* other) const {
    // Char can be explicitly converted to integers
    return dynamic_cast<const IntegerType*>(other) != nullptr;
}

bool CharType::isCompatibleWith(const Type* other) const {
    return equals(other) || isImplicitlyConvertibleTo(other);
}

size_t CharType::getStaticSize() const {
    return 4; // UTF-32 character (32-bit)
}

size_t CharType::getAlignment() const {
    return 4;
}



// =============================================================================
// VoidType Implementation
// =============================================================================

VoidType::VoidType(Flags flags) : PrimitiveType(flags) {}

bool VoidType::equals(const Type* other) const {
    return dynamic_cast<const VoidType*>(other) != nullptr;
}

std::string VoidType::toString() const {
    return "void";
}

size_t VoidType::hash() const {
    return std::hash<int>{}(static_cast<int>(typVoid));
}

bool VoidType::isAssignableFrom(const Type* other) const {
    return false; // Nothing can be assigned to void
}

bool VoidType::isImplicitlyConvertibleTo(const Type* other) const {
    return false; // Void cannot be converted to anything
}

bool VoidType::isExplicitlyConvertibleTo(const Type* other) const {
    return false; // Void cannot be converted to anything
}

bool VoidType::isCompatibleWith(const Type* other) const {
    return equals(other);
}

size_t VoidType::getStaticSize() const {
    return 0; // Void has no size
}

size_t VoidType::getAlignment() const {
    return 1; // Minimal alignment
}



// =============================================================================
// AutoType Implementation
// =============================================================================

AutoType::AutoType(Flags flags) : PrimitiveType(flags) {}

bool AutoType::equals(const Type* other) const {
    return dynamic_cast<const AutoType*>(other) != nullptr;
}

std::string AutoType::toString() const {
    return "auto";
}

size_t AutoType::hash() const {
    return std::hash<int>{}(static_cast<int>(typAuto));
}

bool AutoType::isAssignableFrom(const Type* other) const {
    return false; // Auto cannot be assigned to until resolved
}

bool AutoType::isImplicitlyConvertibleTo(const Type* other) const {
    return false; // Auto cannot be converted until resolved
}

bool AutoType::isExplicitlyConvertibleTo(const Type* other) const {
    return false; // Auto cannot be converted until resolved
}

bool AutoType::isCompatibleWith(const Type* other) const {
    return equals(other);
}

size_t AutoType::getStaticSize() const {
    return 0; // Auto has no size until resolved
}

size_t AutoType::getAlignment() const {
    return 1; // Minimal alignment
}



// Utility functions moved to separate file to avoid circular dependency

} // namespace cxy
