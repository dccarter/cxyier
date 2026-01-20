#pragma once

#include "kind.hpp"
#include "cxy/ast/node.hpp"
#include "cxy/flags.hpp"
#include "cxy/memory/arena_stl.hpp"
#include "cxy/strings.hpp"
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
        : Type(flags), sourceAST_(ast), flags_(flags) {}

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
    CompositeType() : Type(flgNone), sourceAST_(nullptr), flags_(flgNone) {}

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
 * @brief Pointer type implementation for pointer types.
 * 
 * Represents pointer types (*T) that can be null and are reassignable.
 * Pointers require explicit dereferencing and can point to any type.
 */
class PointerType : public CompositeType {
private:
    const Type* pointeeType_;  ///< Type being pointed to

public:
    /**
     * @brief Construct a pointer type.
     * 
     * @param pointeeType Type being pointed to
     * @param flags Type flags (default: flgNone)
     */
    PointerType(const Type* pointeeType, Flags flags = flgNone);

    /**
     * @brief Virtual destructor.
     */
    virtual ~PointerType() = default;

    /**
     * @brief Get the pointee type.
     * 
     * @return Pointer to the type being pointed to
     */
    [[nodiscard]] const Type* getPointeeType() const { return pointeeType_; }

    // Type interface implementation
    [[nodiscard]] TypeKind kind() const override { return typPointer; }
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
    [[nodiscard]] bool hasStaticSize() const override { return true; }
    [[nodiscard]] bool isDynamicallySized() const override { return false; }

    // Type classification helpers
    [[nodiscard]] bool isCallable() const override { return false; }
    [[nodiscard]] bool isNumeric() const override { return false; }
    [[nodiscard]] bool isIntegral() const override { return false; }
    [[nodiscard]] bool isFloatingPoint() const override { return false; }

private:
    // Disable copying and moving - pointer types should be unique instances
    PointerType(const PointerType&) = delete;
    PointerType& operator=(const PointerType&) = delete;
    PointerType(PointerType&&) = delete;
    PointerType& operator=(PointerType&&) = delete;
};

/**
 * @brief Reference type implementation for reference types.
 * 
 * Represents reference types (&T) that cannot be null and are not reassignable
 * after initialization. References provide automatic dereferencing.
 */
class ReferenceType : public CompositeType {
private:
    const Type* referentType_;  ///< Type being referenced

public:
    /**
     * @brief Construct a reference type.
     * 
     * @param referentType Type being referenced
     * @param flags Type flags (default: flgNone)
     */
    ReferenceType(const Type* referentType, Flags flags = flgNone);

    /**
     * @brief Virtual destructor.
     */
    virtual ~ReferenceType() = default;

    /**
     * @brief Get the referent type.
     * 
     * @return Pointer to the type being referenced
     */
    [[nodiscard]] const Type* getReferentType() const { return referentType_; }

    // Type interface implementation
    [[nodiscard]] TypeKind kind() const override { return typReference; }
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
    [[nodiscard]] bool hasStaticSize() const override { return true; }
    [[nodiscard]] bool isDynamicallySized() const override { return false; }

    // Type classification helpers
    [[nodiscard]] bool isCallable() const override { return false; }
    [[nodiscard]] bool isNumeric() const override { return false; }
    [[nodiscard]] bool isIntegral() const override { return false; }
    [[nodiscard]] bool isFloatingPoint() const override { return false; }

private:
    // Disable copying and moving - reference types should be unique instances
    ReferenceType(const ReferenceType&) = delete;
    ReferenceType& operator=(const ReferenceType&) = delete;
    ReferenceType(ReferenceType&&) = delete;
    ReferenceType& operator=(ReferenceType&&) = delete;
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

/**
 * @brief Base class for types with fields and methods (structs and classes).
 * 
 * Provides common functionality for named types that contain fields and methods.
 * Handles layout, method resolution, and basic type operations.
 */
class RecordType : public CompositeType {
public:
    /**
     * @brief Represents a named field in a record type.
     */
    struct FieldType {
        InternedString name;  ///< Field name (interned)
        const Type* type;     ///< Field type (non-null)
        
        FieldType(const InternedString& fieldName, const Type* fieldType)
            : name(fieldName), type(fieldType) {}
    };

    /**
     * @brief Represents a method in a record type.
     * 
     * Methods are functions with a receiver type as the first parameter.
     * Method qualifiers (const, static) are encoded in the function type flags.
     */
    struct MethodType {
        InternedString name;                      ///< Method name (interned)
        const FunctionType* signature;           ///< Function signature (receiver + params -> return)
        const ast::ASTNode* declaration;         ///< AST node for method implementation
        
        MethodType(const InternedString& methodName, const FunctionType* methodSignature, const ast::ASTNode* methodDecl)
            : name(methodName), signature(methodSignature), declaration(methodDecl) {}
    };

protected:
    InternedString name_;                   ///< Record name (empty for anonymous)
    ArenaVector<FieldType> fields_;         ///< Field list (declaration order preserved)
    ArenaVector<MethodType> methods_;       ///< Method list (declaration order preserved)
    ArenaAllocator& arena_;                 ///< Arena allocator reference for method lookups

    /**
     * @brief Protected constructor for record types.
     * 
     * @param name Record name (can be empty for anonymous records)
     * @param fields Vector of field definitions (moved into record)
     * @param methods Vector of method definitions (moved into record)
     * @param flags Type flags
     * @param sourceAST AST node that declares this record (optional)
     * @param arena Arena allocator for memory management
     */
    RecordType(InternedString name,
               ArenaVector<FieldType> fields,
               ArenaVector<MethodType> methods,
               Flags flags,
               const ast::ASTNode* sourceAST,
               ArenaAllocator& arena);

public:
    /**
     * @brief Virtual destructor for proper inheritance.
     */
    virtual ~RecordType() = default;

    // Basic properties
    [[nodiscard]] const InternedString& getName() const { return name_; }
    [[nodiscard]] bool isAnonymous() const { return name_.empty(); }

    // Field access (preserving declaration order)
    [[nodiscard]] const ArenaVector<FieldType>& getFields() const { return fields_; }
    [[nodiscard]] size_t getFieldCount() const { return fields_.size(); }

    // Method access (preserving declaration order)
    [[nodiscard]] const ArenaVector<MethodType>& getMethods() const { return methods_; }
    [[nodiscard]] size_t getMethodCount() const { return methods_.size(); }

    // Field lookup methods
    [[nodiscard]] virtual const Type* getFieldType(const InternedString& name) const;
    [[nodiscard]] virtual bool hasField(const InternedString& name) const;
    [[nodiscard]] virtual size_t getFieldIndex(const InternedString& name) const; // SIZE_MAX if not found

    // Method lookup methods
    [[nodiscard]] ArenaVector<const MethodType*> getMethodsByName(const InternedString& name) const;
    [[nodiscard]] const MethodType* getMethod(const InternedString& name, const FunctionType* signature) const;
    [[nodiscard]] bool hasMethod(const InternedString& name) const;
    [[nodiscard]] size_t getMethodIndex(const InternedString& name, const FunctionType* signature) const; // SIZE_MAX if not found

    // Layout calculation methods (virtual for type-specific behavior)
    [[nodiscard]] virtual size_t getFieldOffset(size_t index) const;
    [[nodiscard]] size_t getFieldOffset(const InternedString& name) const;
    [[nodiscard]] size_t calculateNaturalSize() const;
    
    // Virtual methods for type-specific behavior
    [[nodiscard]] virtual bool isValueType() const = 0;        // struct=true, class=false  
    [[nodiscard]] virtual bool supportsInheritance() const = 0; // struct=false, class=true
    [[nodiscard]] virtual std::string getTypeKeyword() const = 0; // "struct" vs "class"

    // Type interface implementation (shared base implementations)
    [[nodiscard]] bool equals(const Type* other) const override;
    [[nodiscard]] virtual std::string toString() const override;
    [[nodiscard]] size_t hash() const override;

    // Type relationship queries (shared implementations)
    [[nodiscard]] bool isAssignableFrom(const Type* other) const override;
    [[nodiscard]] bool isImplicitlyConvertibleTo(const Type* other) const override;
    [[nodiscard]] bool isExplicitlyConvertibleTo(const Type* other) const override;
    [[nodiscard]] bool isCompatibleWith(const Type* other) const override;

    // Size and alignment information (delegated to derived types)
    [[nodiscard]] size_t getStaticSize() const override = 0;
    [[nodiscard]] size_t getAlignment() const override = 0;
    [[nodiscard]] bool hasStaticSize() const override;
    [[nodiscard]] bool isDynamicallySized() const override;

    // Type classification helpers (shared)
    [[nodiscard]] bool isCallable() const override { return false; }
    [[nodiscard]] bool isNumeric() const override { return false; }
    [[nodiscard]] bool isIntegral() const override { return false; }
    [[nodiscard]] bool isFloatingPoint() const override { return false; }

protected:
    // Helper methods for layout calculations (protected so derived classes can use)
    [[nodiscard]] size_t alignUp(size_t size, size_t alignment) const;

private:
    // Disable copying and moving - record types should be unique instances
    RecordType(const RecordType&) = delete;
    RecordType& operator=(const RecordType&) = delete;
    RecordType(RecordType&&) = delete;
    RecordType& operator=(RecordType&&) = delete;
};

/**
 * @brief Struct type implementation with named fields and layout control.
 * 
 * Represents struct types with value semantics, named fields, and support
 * for both natural and packed memory layouts. Structs preserve field
 * declaration order for C/C++ compatibility.
 */
class StructType : public RecordType {
public:
    /**
     * @brief Construct a struct type with specified fields and flags.
     * 
     * @param name Struct name (can be empty for anonymous structs)
     * @param fields Vector of field definitions (moved into struct)
     * @param methods Vector of method definitions (moved into struct)
     * @param flags Type flags (flgPacked for packed layout, etc.)
     * @param sourceAST AST node that declares this struct (optional)
     * @param arena Arena allocator for memory management
     */
    StructType(InternedString name,
               ArenaVector<FieldType> fields,
               ArenaVector<MethodType> methods,
               Flags flags,
               const ast::ASTNode* sourceAST,
               ArenaAllocator& arena);

    /**
     * @brief Virtual destructor for proper inheritance.
     */
    virtual ~StructType() = default;

    // Struct-specific properties
    [[nodiscard]] bool isPacked() const { return hasFlag(flgPacked); }
    [[nodiscard]] size_t calculatePackedSize() const;

    // Type interface implementation
    [[nodiscard]] TypeKind kind() const override { return typStruct; }

    // Virtual methods from RecordType  
    [[nodiscard]] bool isValueType() const override { return true; }
    [[nodiscard]] bool supportsInheritance() const override { return false; }
    [[nodiscard]] std::string getTypeKeyword() const override { return "struct"; }

    // Override field offset calculation for packed layout support
    [[nodiscard]] size_t getFieldOffset(size_t index) const override;
    
    // Bring base class name-based getFieldOffset into scope
    using RecordType::getFieldOffset;
    
    // Override toString to add struct-specific formatting
    [[nodiscard]] std::string toString() const override;
    
    // Size and alignment information (struct-specific)
    [[nodiscard]] size_t getStaticSize() const override;
    [[nodiscard]] size_t getAlignment() const override;

private:
    // Disable copying and moving - struct types should be unique instances
    StructType(const StructType&) = delete;
    StructType& operator=(const StructType&) = delete;
    StructType(StructType&&) = delete;
    StructType& operator=(StructType&&) = delete;
};

/**
 * @brief Class type implementation with inheritance and virtual methods.
 * 
 * Represents class types with reference semantics, inheritance support,
 * and virtual method dispatch. Classes can have base classes and support
 * polymorphism through virtual methods.
 */
class ClassType : public RecordType {
private:
    const ClassType* baseClass_;    ///< Single base class (nullptr if no inheritance)
    
public:
    /**
     * @brief Construct a class type with specified fields, methods, and base class.
     * 
     * @param name Class name (can be empty for anonymous classes)
     * @param fields Vector of field definitions (moved into class)
     * @param methods Vector of method definitions (moved into class)
     * @param baseClass Single base class type for inheritance (nullptr if none)
     * @param flags Type flags (virtual, abstract, etc.)
     * @param sourceAST AST node that declares this class (optional)
     * @param arena Arena allocator for memory management
     */
    ClassType(InternedString name,
              ArenaVector<FieldType> fields,
              ArenaVector<MethodType> methods,
              const ClassType* baseClass,
              Flags flags,
              const ast::ASTNode* sourceAST,
              ArenaAllocator& arena);

    /**
     * @brief Virtual destructor for proper inheritance.
     */
    virtual ~ClassType() = default;

    // Class-specific properties
    [[nodiscard]] const ClassType* getBaseClass() const { return baseClass_; }
    [[nodiscard]] bool hasBaseClass() const { return baseClass_ != nullptr; }
    
    // Inheritance queries
    [[nodiscard]] bool isBaseOf(const ClassType* derived) const;
    [[nodiscard]] bool isDerivedFrom(const ClassType* base) const;
    [[nodiscard]] const ClassType* findCommonBase(const ClassType* other) const;

    // Virtual method support
    [[nodiscard]] bool hasVirtualMethods() const;
    [[nodiscard]] bool isAbstract() const;  // Has pure virtual methods
    [[nodiscard]] const MethodType* resolveVirtualMethod(const InternedString& name, const FunctionType* signature) const;

    // Type interface implementation
    [[nodiscard]] TypeKind kind() const override { return typClass; }

    // Virtual methods from RecordType  
    [[nodiscard]] bool isValueType() const override { return false; }
    [[nodiscard]] bool supportsInheritance() const override { return true; }
    [[nodiscard]] std::string getTypeKeyword() const override { return "class"; }

    // Field access overrides for inheritance support
    [[nodiscard]] virtual const Type* getFieldType(const InternedString& name) const override;
    [[nodiscard]] virtual bool hasField(const InternedString& name) const override;
    [[nodiscard]] virtual size_t getFieldIndex(const InternedString& name) const override;

    // Flattened field layout helpers for code generation
    [[nodiscard]] size_t getFlattenedFieldCount() const;
    [[nodiscard]] size_t getFlattenedFieldIndex(const InternedString& name) const;
    [[nodiscard]] size_t getFlattenedFieldOffset(const InternedString& name) const;
    [[nodiscard]] size_t getFlattenedFieldOffset(size_t flattenedIndex) const;

    // Size and alignment information (class-specific - reference semantics)
    [[nodiscard]] size_t getStaticSize() const override;
    [[nodiscard]] size_t getAlignment() const override;

    // Type relationship queries (inheritance-aware overrides)
    [[nodiscard]] bool isAssignableFrom(const Type* other) const override;
    [[nodiscard]] bool isImplicitlyConvertibleTo(const Type* other) const override;
    [[nodiscard]] bool isExplicitlyConvertibleTo(const Type* other) const override;

private:
    // Disable copying and moving - class types should be unique instances
    ClassType(const ClassType&) = delete;
    ClassType& operator=(const ClassType&) = delete;
    ClassType(ClassType&&) = delete;
    ClassType& operator=(ClassType&&) = delete;
};

} // namespace cxy