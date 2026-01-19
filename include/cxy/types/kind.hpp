#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <functional>

namespace cxy {

// Forward declarations
class ArenaAllocator;

// clang-format off
#define CXY_TYPES(fn) \
    fn(Auto)         \
    fn(Integer)      \
    fn(Float)        \
    fn(Bool)         \
    fn(Char)         \
    fn(Void)         \
    fn(Array)        \
    fn(Tuple)        \
    fn(Struct)       \
    fn(Class)        \
    fn(Union)        \
    fn(Function)     \
    fn(Closure)      \
    fn(Generic)      \
    fn(TypeAlias)    \
    fn(Unknown)
// clang-format on

enum TypeKind {
#define DECLARE_TYPE_KIND(name) typ##name,
    CXY_TYPES(DECLARE_TYPE_KIND)
#undef DECLARE_TYPE_KIND
};

// Convert TypeKind to string for debugging and diagnostics
const char* typeKindToString(TypeKind kind);

/**
 * Base class for all types in the CXY type system.
 * 
 * Design principles:
 * - All types are immutable once created
 * - Types are allocated using arena allocators for fast cleanup
 * - Value-based equality for type comparison
 * - Support for both primitive and composite types
 */
class Type {
public:
    // Virtual destructor for proper inheritance
    virtual ~Type() = default;

    // Core type interface
    [[nodiscard]] virtual TypeKind kind() const = 0;
    [[nodiscard]] virtual bool equals(const Type* other) const = 0;
    [[nodiscard]] virtual std::string toString() const = 0;
    [[nodiscard]] virtual size_t hash() const = 0;

    // Type relationship queries
    [[nodiscard]] virtual bool isAssignableFrom(const Type* other) const = 0;
    [[nodiscard]] virtual bool isImplicitlyConvertibleTo(const Type* other) const = 0;
    [[nodiscard]] virtual bool isExplicitlyConvertibleTo(const Type* other) const = 0;
    [[nodiscard]] virtual bool isCompatibleWith(const Type* other) const = 0;

    /**
     * @brief Check if this type can be implicitly passed as a function argument.
     * 
     * This is more permissive than isImplicitlyConvertibleTo and follows
     * C-style function call conversion rules which allow some narrowing
     * conversions that are not allowed in assignment contexts.
     * 
     * @param parameterType The function parameter type to check against
     * @return true if this type can be passed to a parameter of the given type
     */
    [[nodiscard]] virtual bool canBeImplicitlyPassedTo(const Type* parameterType) const;

    // Size and alignment information
    [[nodiscard]] virtual size_t getStaticSize() const = 0;
    [[nodiscard]] virtual size_t getAlignment() const = 0;
    [[nodiscard]] virtual bool hasStaticSize() const = 0;
    [[nodiscard]] virtual bool isDynamicallySized() const = 0;

    // Type classification helpers
    [[nodiscard]] virtual bool isPrimitive() const = 0;
    [[nodiscard]] virtual bool isComposite() const = 0;
    [[nodiscard]] virtual bool isCallable() const = 0;
    [[nodiscard]] virtual bool isNumeric() const = 0;
    [[nodiscard]] virtual bool isIntegral() const = 0;
    [[nodiscard]] virtual bool isFloatingPoint() const = 0;

    // Safe type casting
    template<typename T>
    [[nodiscard]] const T* as() const {
        return dynamic_cast<const T*>(this);
    }

    template<typename T>
    [[nodiscard]] bool is() const {
        return dynamic_cast<const T*>(this) != nullptr;
    }

    // Arena allocation
    void* operator new(size_t size, ArenaAllocator& arena);
    void operator delete(void* ptr) {} // Empty - arena manages deallocation

protected:
    // Protected constructor - only derived classes can create instances
    explicit Type() = default;

    // Disable copying - types should be unique instances
    Type(const Type&) = delete;
    Type& operator=(const Type&) = delete;

    // Enable moving for efficiency
    Type(Type&&) = default;
    Type& operator=(Type&&) = default;
};

// Type comparison operators
bool operator==(const Type& lhs, const Type& rhs);
bool operator!=(const Type& lhs, const Type& rhs);

// Hash support for using types in hash tables
struct TypeHash {
    size_t operator()(const Type* type) const {
        return type ? type->hash() : 0;
    }
};

// Type equality comparison for hash tables
struct TypeEqual {
    bool operator()(const Type* lhs, const Type* rhs) const {
        if (lhs == rhs) return true;
        if (!lhs || !rhs) return false;
        return lhs->equals(rhs);
    }
};

} // namespace cxy