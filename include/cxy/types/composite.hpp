#pragma once

#include "kind.hpp"
#include "cxy/ast/node.hpp"
#include "cxy/flags.hpp"
#include "cxy/arena_stl.hpp"
#include <string>

namespace cxy {

// Forward declarations
class ArenaAllocator;

/**
 * @brief Base class for all composite types.
 * 
 * Composite types reference their AST declarations for compile-time features
 * and generic instantiation. Unlike primitive types, composite types maintain
 * a connection to their source AST for template instantiation and compile-time
 * property access.
 */
class CompositeType : public Type {
protected:
    const ast::ASTNode* sourceAST_;  ///< Reference to declaration AST
    Flags flags_;                    ///< Type flags (const, public, etc.)

public:
    /**
     * @brief Construct a composite type with AST reference and flags.
     * 
     * @param ast Pointer to the AST node that declares this type
     * @param flags Type flags (default: flgNone)
     */
    explicit CompositeType(const ast::ASTNode* ast, Flags flags = flgNone)
        : sourceAST_(ast), flags_(flags) {}

    /**
     * @brief Virtual destructor for proper inheritance.
     */
    virtual ~CompositeType() = default;

    /**
     * @brief Get the source AST node that declares this type.
     * 
     * @return Pointer to the AST node, or nullptr if not available
     */
    [[nodiscard]] const ast::ASTNode* getSourceAST() const { return sourceAST_; }

    /**
     * @brief Get the type flags.
     * 
     * @return The flags associated with this type
     */
    [[nodiscard]] Flags getFlags() const { return flags_; }

    /**
     * @brief Check if a specific flag is set.
     * 
     * @param flag The flag to check
     * @return true if the flag is set, false otherwise
     */
    [[nodiscard]] bool hasFlag(Flags flag) const { return hasAnyFlag(flags_, flag); }



    // Type classification helpers - composite types override these
    [[nodiscard]] bool isPrimitive() const override { return false; }
    [[nodiscard]] bool isComposite() const override { return true; }

protected:
    /**
     * @brief Protected constructor - only derived classes can create instances.
     * 
     * This constructor is used by derived classes that need to perform
     * additional initialization before setting the AST reference.
     */
    CompositeType() : sourceAST_(nullptr), flags_(flgNone) {}

    /**
     * @brief Set the source AST reference.
     * 
     * This method is provided for derived classes that need to set
     * the AST reference after construction.
     * 
     * @param ast The AST node to reference
     */
    void setSourceAST(const ast::ASTNode* ast) { sourceAST_ = ast; }

    /**
     * @brief Set type flags.
     * 
     * @param flags The flags to set
     */
    void setFlags(Flags flags) { flags_ = flags; }

private:
    // Disable copying - composite types should be unique instances
    CompositeType(const CompositeType&) = delete;
    CompositeType& operator=(const CompositeType&) = delete;

    // Enable moving for efficiency
    CompositeType(CompositeType&&) = default;
    CompositeType& operator=(CompositeType&&) = default;

public:
    // Arena allocation support
    void* operator new(size_t size, ArenaAllocator& arena);
    void operator delete(void* ptr); // Empty - arena manages deallocation
};

/**
 * @brief Array type implementation for fixed and dynamic arrays.
 * 
 * Represents both fixed-size arrays ([10]i32) and dynamic arrays ([]i32).
 * Fixed arrays have compile-time known size, while dynamic arrays are
 * runtime-sized and stored as pointers.
 */
class ArrayType : public CompositeType {
private:
    const Type* elementType_;  ///< Type of array elements
    size_t size_;              ///< Array size (0 for dynamic arrays)

public:
    /**
     * @brief Construct an array type.
     * 
     * @param elementType Type of the array elements
     * @param size Array size (0 for dynamic arrays)
     * @param flags Type flags (default: flgNone)
     */
    ArrayType(const Type* elementType, size_t size, Flags flags = flgNone);

    /**
     * @brief Virtual destructor.
     */
    virtual ~ArrayType() = default;

    /**
     * @brief Get the element type.
     * 
     * @return Pointer to the element type
     */
    [[nodiscard]] const Type* getElementType() const { return elementType_; }

    /**
     * @brief Get the array size.
     * 
     * @return Array size, or 0 for dynamic arrays
     */
    [[nodiscard]] size_t getArraySize() const { return size_; }

    /**
     * @brief Check if this is a dynamic array.
     * 
     * @return true if the array is dynamically sized (size == 0)
     */
    [[nodiscard]] bool isDynamicArray() const { return size_ == 0; }

    /**
     * @brief Check if this is a fixed-size array.
     * 
     * @return true if the array has a fixed size (size > 0)
     */
    [[nodiscard]] bool isFixedArray() const { return size_ > 0; }

    // Type interface implementation
    [[nodiscard]] TypeKind kind() const override { return typArray; }
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
    [[nodiscard]] bool hasStaticSize() const override;
    [[nodiscard]] bool isDynamicallySized() const override;

    // Type classification helpers
    [[nodiscard]] bool isCallable() const override { return false; }
    [[nodiscard]] bool isNumeric() const override { return false; }
    [[nodiscard]] bool isIntegral() const override { return false; }
    [[nodiscard]] bool isFloatingPoint() const override { return false; }



private:
    // Disable copying and moving - array types should be unique instances
    ArrayType(const ArrayType&) = delete;
    ArrayType& operator=(const ArrayType&) = delete;
    ArrayType(ArrayType&&) = delete;
    ArrayType& operator=(ArrayType&&) = delete;
};

/**
 * @brief Tuple type implementation for heterogeneous element collections.
 * 
 * Represents tuple types like (i32, string, bool) with compile-time known
 * element types and count. Unlike arrays, tuples can contain different
 * element types and provide compile-time element access.
 */
class TupleType : public CompositeType {
private:
    ArenaVector<const Type*> elementTypes_;  ///< Types of tuple elements

public:
    /**
     * @brief Construct a tuple type.
     * 
     * @param elementTypes Vector of element types (must have at least 1 element)
     * @param flags Type flags (default: flgNone)
     */
    TupleType(const ArenaVector<const Type*>& elementTypes, Flags flags = flgNone);

    /**
     * @brief Virtual destructor.
     */
    virtual ~TupleType() = default;

    /**
     * @brief Get the element types.
     * 
     * @return Reference to the vector of element types
     */
    [[nodiscard]] const ArenaVector<const Type*>& getElementTypes() const { return elementTypes_; }

    /**
     * @brief Get the number of elements.
     * 
     * @return Element count
     */
    [[nodiscard]] size_t getElementCount() const { return elementTypes_.size(); }

    /**
     * @brief Get the type of a specific element.
     * 
     * @param index Element index (0-based)
     * @return Pointer to the element type, or nullptr if index is out of bounds
     */
    [[nodiscard]] const Type* getElementType(size_t index) const {
        if (index >= elementTypes_.size()) return nullptr;
        return elementTypes_[index];
    }

    // Type interface implementation
    [[nodiscard]] TypeKind kind() const override { return typTuple; }
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
    [[nodiscard]] bool hasStaticSize() const override;
    [[nodiscard]] bool isDynamicallySized() const override;

    // Type classification helpers
    [[nodiscard]] bool isCallable() const override { return false; }
    [[nodiscard]] bool isNumeric() const override { return false; }
    [[nodiscard]] bool isIntegral() const override { return false; }
    [[nodiscard]] bool isFloatingPoint() const override { return false; }



private:
    // Disable copying and moving - tuple types should be unique instances
    TupleType(const TupleType&) = delete;
    TupleType& operator=(const TupleType&) = delete;
    TupleType(TupleType&&) = delete;
    TupleType& operator=(TupleType&&) = delete;
};

/**
 * @brief Union type implementation for variant types.
 * 
 * Represents union types like (i32 | f64 | bool) that can hold exactly
 * one value of any of the specified variant types at runtime. The size
 * is the maximum of all variant sizes, and alignment is the maximum
 * of all variant alignments.
 */
class UnionType : public CompositeType {
private:
    ArenaVector<const Type*> variantTypes_;  ///< Types of union variants

public:
    /**
     * @brief Construct a union type.
     * 
     * @param variantTypes Vector of variant types (must have at least 1 variant)
     * @param flags Type flags (default: flgNone)
     */
    UnionType(const ArenaVector<const Type*>& variantTypes, Flags flags = flgNone);

    /**
     * @brief Virtual destructor.
     */
    virtual ~UnionType() = default;

    /**
     * @brief Get the variant types.
     * 
     * @return Reference to the vector of variant types
     */
    [[nodiscard]] const ArenaVector<const Type*>& getVariantTypes() const { return variantTypes_; }

    /**
     * @brief Get the number of variants.
     * 
     * @return Variant count
     */
    [[nodiscard]] size_t getVariantCount() const { return variantTypes_.size(); }

    /**
     * @brief Get the type of a specific variant.
     * 
     * @param index Variant index (0-based)
     * @return Pointer to the variant type, or nullptr if index is out of bounds
     */
    [[nodiscard]] const Type* getVariantType(size_t index) const {
        if (index >= variantTypes_.size()) return nullptr;
        return variantTypes_[index];
    }

    // Type interface implementation
    [[nodiscard]] TypeKind kind() const override { return typUnion; }
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
    [[nodiscard]] bool hasStaticSize() const override;
    [[nodiscard]] bool isDynamicallySized() const override;

    // Type classification helpers
    [[nodiscard]] bool isCallable() const override { return false; }
    [[nodiscard]] bool isNumeric() const override { return false; }
    [[nodiscard]] bool isIntegral() const override { return false; }
    [[nodiscard]] bool isFloatingPoint() const override { return false; }



private:
    // Disable copying and moving - union types should be unique instances
    UnionType(const UnionType&) = delete;
    UnionType& operator=(const UnionType&) = delete;
    UnionType(UnionType&&) = delete;
    UnionType& operator=(UnionType&&) = delete;
};

/**
 * @brief Function type implementation for function signatures.
 * 
 * Represents function types like (i32, f64) -> bool that specify
 * parameter types and return type. Function types are pointer-sized
 * and represent function pointers or callable objects.
 */
class FunctionType : public CompositeType {
private:
    ArenaVector<const Type*> parameterTypes_;  ///< Types of function parameters
    const Type* returnType_;                   ///< Return type (never null, use VoidType for void)

public:
    /**
     * @brief Construct a function type.
     * 
     * @param parameterTypes Vector of parameter types (can be empty for no parameters)
     * @param returnType Return type (must not be null, use VoidType for void functions)
     * @param flags Type flags (default: flgNone)
     */
    FunctionType(const ArenaVector<const Type*>& parameterTypes, const Type* returnType, Flags flags = flgNone);

    /**
     * @brief Virtual destructor.
     */
    virtual ~FunctionType() = default;

    /**
     * @brief Get the parameter types.
     * 
     * @return Reference to the vector of parameter types
     */
    [[nodiscard]] const ArenaVector<const Type*>& getParameterTypes() const { return parameterTypes_; }

    /**
     * @brief Get the number of parameters.
     * 
     * @return Parameter count
     */
    [[nodiscard]] size_t getParameterCount() const { return parameterTypes_.size(); }

    /**
     * @brief Get the type of a specific parameter.
     * 
     * @param index Parameter index (0-based)
     * @return Pointer to the parameter type, or nullptr if index is out of bounds
     */
    [[nodiscard]] const Type* getParameterType(size_t index) const {
        if (index >= parameterTypes_.size()) return nullptr;
        return parameterTypes_[index];
    }

    /**
     * @brief Get the return type.
     * 
     * @return Pointer to the return type (never null)
     */
    [[nodiscard]] const Type* getReturnType() const { return returnType_; }

    // Type interface implementation
    [[nodiscard]] TypeKind kind() const override { return typFunction; }
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
    [[nodiscard]] bool hasStaticSize() const override;
    [[nodiscard]] bool isDynamicallySized() const override;

    // Type classification helpers
    [[nodiscard]] bool isCallable() const override { return true; }
    [[nodiscard]] bool isNumeric() const override { return false; }
    [[nodiscard]] bool isIntegral() const override { return false; }
    [[nodiscard]] bool isFloatingPoint() const override { return false; }

    /**
     * @brief Check if this function can be called with the given argument types.
     * 
     * This method checks if the function can be invoked with the provided argument
     * types, considering implicit conversions (e.g., i8 -> i32). This is essential
     * for function overload resolution and call validation.
     * 
     * @param argumentTypes Vector of argument types being passed to the function
     * @return true if the function can be called with these arguments, false otherwise
     */
    [[nodiscard]] bool canBeCalledWith(const ArenaVector<const Type*>& argumentTypes) const;

    /**
     * @brief Calculate the conversion distance for calling with given arguments.
     * 
     * Returns a measure of how "far" the implicit conversions are from the
     * provided arguments to the function parameters. Lower values indicate
     * better matches. Returns -1 if the call is not possible.
     * 
     * This is useful for overload resolution where multiple functions could
     * be called with the same arguments.
     * 
     * @param argumentTypes Vector of argument types being passed to the function
     * @return Conversion distance (0 = exact match, higher = more conversions needed, -1 = impossible)
     */
    [[nodiscard]] int getConversionDistance(const ArenaVector<const Type*>& argumentTypes) const;




private:
    // Disable copying and moving - function types should be unique instances
    FunctionType(const FunctionType&) = delete;
    FunctionType& operator=(const FunctionType&) = delete;
    FunctionType(FunctionType&&) = delete;
    FunctionType& operator=(FunctionType&&) = delete;
};

} // namespace cxy