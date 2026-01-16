#pragma once

#include "kind.hpp"
#include "cxy/token.hpp"
#include <cstddef>
#include <cstdint>
#include <string>

namespace cxy {

// Forward declaration
class ArenaAllocator;

/**
 * Base class for all primitive types.
 * Primitive types have static size and are not composite.
 */
class PrimitiveType : public Type {
public:
    // Type classification helpers - all primitives return consistent values
    [[nodiscard]] bool isPrimitive() const override { return true; }
    [[nodiscard]] bool isComposite() const override { return false; }
    [[nodiscard]] bool hasStaticSize() const override { return true; }
    [[nodiscard]] bool isDynamicallySized() const override { return false; }
    [[nodiscard]] bool isCallable() const override { return false; }

protected:
    explicit PrimitiveType() = default;
};

/**
 * Integer type implementation with integration to ::cxy::IntegerType enum.
 */
class IntegerType : public PrimitiveType {
private:
    ::cxy::IntegerKind integerKind_;

public:
    // Constructor now public for registry access
    explicit IntegerType(::cxy::IntegerKind kind);
    // Type interface implementation
    [[nodiscard]] TypeKind kind() const override { return typInteger; }
    [[nodiscard]] bool equals(const Type* other) const override;
    [[nodiscard]] std::string toString() const override;
    [[nodiscard]] size_t hash() const override;

    // Type relationship queries
    [[nodiscard]] bool isAssignableFrom(const Type* other) const override;
    [[nodiscard]] bool isImplicitlyConvertibleTo(const Type* other) const override;
    [[nodiscard]] bool isExplicitlyConvertibleTo(const Type* other) const override;
    [[nodiscard]] bool isCompatibleWith(const Type* other) const override;

    // Size and alignment information
    [[nodiscard]] size_t getStaticSize() const override;
    [[nodiscard]] size_t getAlignment() const override;

    // Type classification helpers
    [[nodiscard]] bool isNumeric() const override { return true; }
    [[nodiscard]] bool isIntegral() const override { return true; }
    [[nodiscard]] bool isFloatingPoint() const override { return false; }

    // Integer-specific interface
    [[nodiscard]] ::cxy::IntegerKind getIntegerKind() const { return integerKind_; }
    [[nodiscard]] bool isSigned() const;
    [[nodiscard]] size_t getBitWidth() const;
    [[nodiscard]] __uint128_t getMinValue() const;
    [[nodiscard]] __uint128_t getMaxValue() const;


};

/**
 * Floating-point type implementation with integration to ::cxy::FloatType enum.
 */
class FloatType : public PrimitiveType {
private:
    ::cxy::FloatKind floatKind_;

public:
    // Constructor now public for registry access
    explicit FloatType(::cxy::FloatKind kind);
    // Type interface implementation
    [[nodiscard]] TypeKind kind() const override { return typFloat; }
    [[nodiscard]] bool equals(const Type* other) const override;
    [[nodiscard]] std::string toString() const override;
    [[nodiscard]] size_t hash() const override;

    // Type relationship queries
    [[nodiscard]] bool isAssignableFrom(const Type* other) const override;
    [[nodiscard]] bool isImplicitlyConvertibleTo(const Type* other) const override;
    [[nodiscard]] bool isExplicitlyConvertibleTo(const Type* other) const override;
    [[nodiscard]] bool isCompatibleWith(const Type* other) const override;

    // Size and alignment information
    [[nodiscard]] size_t getStaticSize() const override;
    [[nodiscard]] size_t getAlignment() const override;

    // Type classification helpers
    [[nodiscard]] bool isNumeric() const override { return true; }
    [[nodiscard]] bool isIntegral() const override { return false; }
    [[nodiscard]] bool isFloatingPoint() const override { return true; }

    // Float-specific interface
    [[nodiscard]] ::cxy::FloatKind getFloatKind() const { return floatKind_; }
    [[nodiscard]] size_t getBitWidth() const;
    [[nodiscard]] double getMinValue() const;
    [[nodiscard]] double getMaxValue() const;
    [[nodiscard]] double getEpsilon() const;


};

/**
 * Boolean type implementation.
 */
class BoolType : public PrimitiveType {
private:
public:
    // Constructor now public for registry access
    BoolType() = default;
    // Type interface implementation
    [[nodiscard]] TypeKind kind() const override { return typBool; }
    [[nodiscard]] bool equals(const Type* other) const override;
    [[nodiscard]] std::string toString() const override;
    [[nodiscard]] size_t hash() const override;

    // Type relationship queries
    [[nodiscard]] bool isAssignableFrom(const Type* other) const override;
    [[nodiscard]] bool isImplicitlyConvertibleTo(const Type* other) const override;
    [[nodiscard]] bool isExplicitlyConvertibleTo(const Type* other) const override;
    [[nodiscard]] bool isCompatibleWith(const Type* other) const override;

    // Size and alignment information
    [[nodiscard]] size_t getStaticSize() const override;
    [[nodiscard]] size_t getAlignment() const override;

    // Type classification helpers
    [[nodiscard]] bool isNumeric() const override { return false; }
    [[nodiscard]] bool isIntegral() const override { return false; }
    [[nodiscard]] bool isFloatingPoint() const override { return false; }


};

/**
 * Character type implementation.
 */
class CharType : public PrimitiveType {
private:
public:
    // Constructor now public for registry access
    CharType() = default;
    // Type interface implementation
    [[nodiscard]] TypeKind kind() const override { return typChar; }
    [[nodiscard]] bool equals(const Type* other) const override;
    [[nodiscard]] std::string toString() const override;
    [[nodiscard]] size_t hash() const override;

    // Type relationship queries
    [[nodiscard]] bool isAssignableFrom(const Type* other) const override;
    [[nodiscard]] bool isImplicitlyConvertibleTo(const Type* other) const override;
    [[nodiscard]] bool isExplicitlyConvertibleTo(const Type* other) const override;
    [[nodiscard]] bool isCompatibleWith(const Type* other) const override;

    // Size and alignment information
    [[nodiscard]] size_t getStaticSize() const override;
    [[nodiscard]] size_t getAlignment() const override;

    // Type classification helpers
    [[nodiscard]] bool isNumeric() const override { return false; }
    [[nodiscard]] bool isIntegral() const override { return false; }
    [[nodiscard]] bool isFloatingPoint() const override { return false; }


};

/**
 * Void type implementation.
 */
class VoidType : public PrimitiveType {
private:
public:
    // Constructor now public for registry access
    VoidType() = default;
    // Type interface implementation
    [[nodiscard]] TypeKind kind() const override { return typVoid; }
    [[nodiscard]] bool equals(const Type* other) const override;
    [[nodiscard]] std::string toString() const override;
    [[nodiscard]] size_t hash() const override;

    // Type relationship queries
    [[nodiscard]] bool isAssignableFrom(const Type* other) const override;
    [[nodiscard]] bool isImplicitlyConvertibleTo(const Type* other) const override;
    [[nodiscard]] bool isExplicitlyConvertibleTo(const Type* other) const override;
    [[nodiscard]] bool isCompatibleWith(const Type* other) const override;

    // Size and alignment information (void has no size)
    [[nodiscard]] size_t getStaticSize() const override;
    [[nodiscard]] size_t getAlignment() const override;
    [[nodiscard]] bool hasStaticSize() const override { return false; }

    // Type classification helpers
    [[nodiscard]] bool isNumeric() const override { return false; }
    [[nodiscard]] bool isIntegral() const override { return false; }
    [[nodiscard]] bool isFloatingPoint() const override { return false; }


};

/**
 * Auto type implementation for type inference.
 */
class AutoType : public PrimitiveType {
private:
public:
    // Constructor now public for registry access
    AutoType() = default;
    // Type interface implementation
    [[nodiscard]] TypeKind kind() const override { return typAuto; }
    [[nodiscard]] bool equals(const Type* other) const override;
    [[nodiscard]] std::string toString() const override;
    [[nodiscard]] size_t hash() const override;

    // Type relationship queries
    [[nodiscard]] bool isAssignableFrom(const Type* other) const override;
    [[nodiscard]] bool isImplicitlyConvertibleTo(const Type* other) const override;
    [[nodiscard]] bool isExplicitlyConvertibleTo(const Type* other) const override;
    [[nodiscard]] bool isCompatibleWith(const Type* other) const override;

    // Size and alignment information (auto has no static size until resolved)
    [[nodiscard]] size_t getStaticSize() const override;
    [[nodiscard]] size_t getAlignment() const override;
    [[nodiscard]] bool hasStaticSize() const override { return false; }
    [[nodiscard]] bool isDynamicallySized() const override { return true; }

    // Type classification helpers
    [[nodiscard]] bool isNumeric() const override { return false; }
    [[nodiscard]] bool isIntegral() const override { return false; }
    [[nodiscard]] bool isFloatingPoint() const override { return false; }

    // Auto-specific interface
    [[nodiscard]] bool isResolved() const { return false; } // Always false until type inference
    [[nodiscard]] const Type* getResolvedType() const { return nullptr; } // Not resolved yet


};

// Utility functions for type operations
namespace types {

/**
 * Find the best integer type that can hold the given value.
 */
const IntegerType* findBestIntegerType(__uint128_t value, bool isSigned);

/**
 * Find the best float type that can represent the given value.
 */
const FloatType* findBestFloatType(double value);

/**
 * Find best fit integer type (same as findBestIntegerType).
 */
const IntegerType* findBestFitIntegerType(__uint128_t value, bool isSigned);

/**
 * Find best fit float type (same as findBestFloatType).
 */
const FloatType* findBestFitFloatType(double value);

/**
 * Check if a value can fit in the specified integer type.
 */
bool valueCanFitIn(__uint128_t value, bool isSigned, const IntegerType* type);

/**
 * Check if a float value can fit in F32 without loss of precision.
 */
bool floatCanFitInF32(double value);

/**
 * Promote types for binary operations following C-style rules.
 */
const Type* promoteForBinaryOperation(const Type* left, const Type* right);

/**
 * Check if one type can be implicitly converted to another.
 */
bool canImplicitlyConvert(const Type* from, const Type* to);

} // namespace types

} // namespace cxy