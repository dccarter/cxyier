#pragma once

#include <cstdint>
#include <string_view>

namespace cxy {

/**
 * @brief Macro defining all AST node flags for the Cxy compiler.
 *
 * This macro is used to generate both the enum values and related functionality
 * like toString functions. Each flag should have a unique bit position to allow
 * bitwise combinations.
 *
 * Usage: CXY_FLAGS(MACRO_NAME) where MACRO_NAME(name, bit_position) is applied
 * to each flag definition.
 */
#define CXY_FLAGS(fn) fn(None, 0)

/**
 * @brief 64-bit flags enum for AST nodes.
 *
 * These flags are used throughout the compiler to track the state of AST nodes
 * during various compiler passes. Flags can be combined using bitwise
 * operations.
 *
 * Design principles:
 * - Each flag represents a single bit for efficient bitwise operations
 * - Flags are cumulative - once set, they typically remain set
 * - Used for pass coordination and optimization tracking
 * - Memory efficient - single 64-bit field per AST node
 */
enum Flags : uint64_t {
#define CXY_FLAG_ENUM(name, bit) flg##name = (1ULL << bit),
  CXY_FLAGS(CXY_FLAG_ENUM)
#undef CXY_FLAG_ENUM
};

/**
 * @brief Convert a single flag to its string representation.
 *
 * @param flag The flag to convert (should be a single flag, not a combination)
 * @return String view containing the flag name, or "Unknown" for invalid flags
 *
 * Example:
 *   flagToString(flgNone) returns "flgNone"
 */
constexpr std::string_view flagToString(Flags flag) {
  switch (flag) {
#define CXY_FLAG_STRING(name, bit)                                             \
  case flg##name:                                                              \
    return "flg" #name;
    CXY_FLAGS(CXY_FLAG_STRING)
#undef CXY_FLAG_STRING
  default:
    return "Unknown";
  }
}

/**
 * @brief Convert a combination of flags to a human-readable string.
 *
 * For flag combinations, this will return a pipe-separated list of flag names.
 * For efficiency, this function allocates and returns a std::string.
 *
 * @param flags The flag combination to convert
 * @return String representation of the flags
 *
 * Example:
 *   flagsToString(flgNone) returns "flgNone"
 *   flagsToString(flgNone | flgStatic) returns "flgNone|flgStatic" (when
 * flgStatic is added)
 */
std::string flagsToString(Flags flags);

/**
 * @brief Bitwise OR operator for combining flags.
 */
constexpr Flags operator|(Flags lhs, Flags rhs) {
  return static_cast<Flags>(static_cast<uint64_t>(lhs) |
                            static_cast<uint64_t>(rhs));
}

/**
 * @brief Bitwise AND operator for checking flag combinations.
 */
constexpr Flags operator&(Flags lhs, Flags rhs) {
  return static_cast<Flags>(static_cast<uint64_t>(lhs) &
                            static_cast<uint64_t>(rhs));
}

/**
 * @brief Bitwise XOR operator for toggling flags.
 */
constexpr Flags operator^(Flags lhs, Flags rhs) {
  return static_cast<Flags>(static_cast<uint64_t>(lhs) ^
                            static_cast<uint64_t>(rhs));
}

/**
 * @brief Bitwise NOT operator for flag negation.
 */
constexpr Flags operator~(Flags flags) {
  return static_cast<Flags>(~static_cast<uint64_t>(flags));
}

/**
 * @brief Compound assignment OR operator.
 */
constexpr Flags &operator|=(Flags &lhs, Flags rhs) { return lhs = lhs | rhs; }

/**
 * @brief Compound assignment AND operator.
 */
constexpr Flags &operator&=(Flags &lhs, Flags rhs) { return lhs = lhs & rhs; }

/**
 * @brief Compound assignment XOR operator.
 */
constexpr Flags &operator^=(Flags &lhs, Flags rhs) { return lhs = lhs ^ rhs; }

/**
 * @brief Check if any of the specified flags are set.
 *
 * @param flags The flags to check
 * @param mask The flags to test for
 * @return true if any flags in mask are set in flags
 */
constexpr bool hasAnyFlag(Flags flags, Flags mask) {
  return (flags & mask) != flgNone;
}

/**
 * @brief Check if all of the specified flags are set.
 *
 * @param flags The flags to check
 * @param mask The flags to test for
 * @return true if all flags in mask are set in flags
 */
constexpr bool hasAllFlags(Flags flags, Flags mask) {
  return (flags & mask) == mask;
}

} // namespace cxy
