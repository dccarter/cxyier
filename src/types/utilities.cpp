#include "cxy/types/primitive.hpp"
#include "cxy/types/registry.hpp"

#include <limits>

namespace cxy {
namespace types {

const IntegerType* findBestIntegerType(__uint128_t value, bool isSigned) {
    ::cxy::IntegerKind kind;
    
    if (isSigned) {
        __int128_t signedValue = static_cast<__int128_t>(value);
        if (signedValue >= INT8_MIN && signedValue <= INT8_MAX) {
            kind = ::cxy::IntegerKind::I8;
        } else if (signedValue >= INT16_MIN && signedValue <= INT16_MAX) {
            kind = ::cxy::IntegerKind::I16;
        } else if (signedValue >= INT32_MIN && signedValue <= INT32_MAX) {
            kind = ::cxy::IntegerKind::I32;
        } else if (signedValue >= INT64_MIN && signedValue <= INT64_MAX) {
            kind = ::cxy::IntegerKind::I64;
        } else {
            kind = ::cxy::IntegerKind::I128;
        }
    } else {
        if (value <= UINT8_MAX) {
            kind = ::cxy::IntegerKind::U8;
        } else if (value <= UINT16_MAX) {
            kind = ::cxy::IntegerKind::U16;
        } else if (value <= UINT32_MAX) {
            kind = ::cxy::IntegerKind::U32;
        } else if (value <= UINT64_MAX) {
            kind = ::cxy::IntegerKind::U64;
        } else {
            kind = ::cxy::IntegerKind::U128;
        }
    }
    
    return TypeRegistry::instance().integerType(kind);
}

const FloatType* findBestFloatType(double value) {
    float f32Value = static_cast<float>(value);
    ::cxy::FloatKind kind = (static_cast<double>(f32Value) == value) ? 
                            ::cxy::FloatKind::F32 : ::cxy::FloatKind::F64;
    return TypeRegistry::instance().floatType(kind);
}

const IntegerType* findBestFitIntegerType(__uint128_t value, bool isSigned) {
    return findBestIntegerType(value, isSigned);
}

const FloatType* findBestFitFloatType(double value) {
    return findBestFloatType(value);
}

bool valueCanFitIn(__uint128_t value, bool isSigned, const IntegerType* type) {
    __uint128_t maxVal = type->getMaxValue();
    __uint128_t minVal = type->getMinValue();

    if (isSigned) {
        __int128_t signedValue = static_cast<__int128_t>(value);
        __int128_t signedMin = static_cast<__int128_t>(minVal);
        __int128_t signedMax = static_cast<__int128_t>(maxVal);
        return signedValue >= signedMin && signedValue <= signedMax;
    } else {
        return value >= minVal && value <= maxVal;
    }
}

bool floatCanFitInF32(double value) {
    float f32Value = static_cast<float>(value);
    return static_cast<double>(f32Value) == value;
}

const Type* promoteForBinaryOperation(const Type* left, const Type* right) {
    // Simplified promotion rules - both operands should be same type
    const auto* leftInt = dynamic_cast<const IntegerType*>(left);
    const auto* rightInt = dynamic_cast<const IntegerType*>(right);

    if (leftInt && rightInt) {
        // Integer promotion - choose larger type
        size_t leftBits = leftInt->getBitWidth();
        size_t rightBits = rightInt->getBitWidth();

        if (leftBits > rightBits) return left;
        if (rightBits > leftBits) return right;

        // Same size - prefer signed
        if (leftInt->isSigned() && !rightInt->isSigned()) return left;
        if (!leftInt->isSigned() && rightInt->isSigned()) return right;

        return left; // Same type
    }

    const auto* leftFloat = dynamic_cast<const FloatType*>(left);
    const auto* rightFloat = dynamic_cast<const FloatType*>(right);

    if (leftFloat && rightFloat) {
        // Float promotion - choose larger type
        if (leftFloat->getBitWidth() > rightFloat->getBitWidth()) return left;
        return right;
    }

    // Mixed integer/float - promote to float
    if (leftInt && rightFloat) return right;
    if (leftFloat && rightInt) return left;

    // No promotion possible
    return nullptr;
}

bool canImplicitlyConvert(const Type* from, const Type* to) {
    return from && to && from->isImplicitlyConvertibleTo(to);
}

} // namespace types
} // namespace cxy