#pragma once

#include <compare>
#include <cstddef>
#include <cstring>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>

#include <cxy/memory/arena.hpp>
#include <cxy/memory/arena_stl.hpp>

// Define builtin names that need to be interned
#define CXY_BUILTIN_NAMES(f, ff)    \
    f(main)                         \
    f(super)                    \
    f(static)                   \
    f(transient)                \
    f(abstract)                 \
    f(inline)                   \
    f(noinline)                 \
    f(optimize)                 \
    f(volatile)                 \
    f(explicit)                 \
    f(pure)                     \
    f(strlen)                   \
    f(memset)                   \
    f(char)                     \
    f(wputc)                    \
    f(sb)                       \
    f(s)                        \
    f(Optional)               \
    f(Slice)                    \
    f(String)                   \
    f(__string)                  \
    ff(_assert, "assert")       \
    f(baseof)                  \
    f(column)                   \
    f(ptr)                      \
    f(cstr)                     \
    f(data)                     \
    f(init)                     \
    f(destructor)               \
    f(file)                     \
    f(len)                      \
    f(line)                     \
    f(mkIdent)                  \
    f(mkInteger)                \
    f(ptroff)                   \
    f(sizeof)                   \
    f(typeof)                   \
    f(allocate)                 \
    f(alias)                    \
    f(align)                    \
    f(name)                     \
    f(None)                     \
    f(Some)                     \
    f(unchecked)                \
    f(unused)                   \
    f(_Variadic)                \
    f(consistent)               \
    f(final)                    \
    f(newClass)                 \
    f(release)                  \
    f(vtable)                   \
    f(poco)                     \
    f(allTestCases)             \
    f(External)                 \
    f(Appending)                \
    f(linkage)                  \
    f(section)                  \
    f(packed)                   \
    f(Exception)                \
    f(Void)                     \
    f(what)                     \
    f(push)                     \
    f(ex)                       \
    f(thread)                   \
    f(likely)                   \
    f(unlikely)                 \
    f(atomic)                   \
    f(__init)                   \
    f(__default_init)           \
    f(__startup)                \
    f(__name)                   \
    f(__construct0)             \
    f(__construct1)             \
    f(__fwd)                    \
    f(__copy)                   \
    f(__destroy)                \
    f(__destructor_fwd)         \
    f(__tuple_dctor)            \
    f(__tuple_copy)             \
    f(__union_dctor)            \
    f(__union_copy)             \
    f(__async)                  \
    f(__tid)                    \
    f(resolve)                  \
    f(reject)                   \
    f(result)                   \
    f(clib)                     \
    f(src)                      \
    ff(AsmInputPrefix,      "\"r\"")             \
    ff(AsmOutputPrefix,     "\"=r\"")            \
    ff(underscore,          "_")

namespace cxy {

// Forward declaration
class StringInterner;
class InternedString;

// Static builtin name accessors for easy access
namespace S {
  // Forward declare all builtin names as extern
  #define DECLARE_BUILTIN_NAME(name) extern const InternedString S_##name;
  #define DECLARE_BUILTIN_NAME_STR(name, str) extern const InternedString S_##name;
  CXY_BUILTIN_NAMES(DECLARE_BUILTIN_NAME, DECLARE_BUILTIN_NAME_STR)
  #undef DECLARE_BUILTIN_NAME
  #undef DECLARE_BUILTIN_NAME_STR
  
  // Initialize all builtin names (defined in strings.cpp)
  void initializeBuiltinNames(StringInterner& interner);
}

class InternedString {
private:
  const char *data; // Pointer to arena-allocated string data
  size_t length;    // Length of the string (excluding null terminator)
  size_t hash;      // Pre-computed hash for fast lookups

  // Private constructor - only StringInterner can create these
  InternedString(const char *str, size_t len, size_t h)
      : data(str), length(len), hash(h) {}

  friend class StringInterner;

public:
  // Default constructor for empty string
  InternedString() : data(nullptr), length(0), hash(0) {}

  // Accessors
  [[nodiscard]] const char *c_str() const noexcept { return data ? data : ""; }
  [[nodiscard]] size_t size() const noexcept { return length; }
  [[nodiscard]] size_t getHash() const noexcept { return hash; }
  [[nodiscard]] bool empty() const noexcept { return length == 0; }

  // Fast O(1) equality comparison
  bool operator==(const InternedString &other) const noexcept {
    return data == other.data; // Pointer comparison!
  }

  bool operator!=(const InternedString &other) const noexcept {
    return data != other.data;
  }

  // Lexicographic comparison for ordering
  auto operator<=>(const InternedString &other) const noexcept {
    if (data == other.data)
      return std::strong_ordering::equal;
    return std::string_view(c_str(), length) <=>
           std::string_view(other.c_str(), other.length);
  }

  // String view conversion
  [[nodiscard]] std::string_view view() const noexcept {
    return std::string_view(c_str(), length);
  }

  // Hash support for unordered containers
  struct Hash {
    size_t operator()(const InternedString &str) const noexcept {
      return str.getHash();
    }
  };

  // Explicit conversion to string (for when you need a copy)
  [[nodiscard]] std::string toString() const {
    return std::string(c_str(), length);
  }
};

class StringInterner {
private:
  ArenaAllocator &arena;

  // Use arena-allocated map to store string mappings
  using StringMap = std::unordered_map<
      std::string_view, InternedString, std::hash<std::string_view>,
      std::equal_to<std::string_view>,
      ArenaSTLAllocator<std::pair<const std::string_view, InternedString>>>;

  StringMap internedStrings;

public:
  explicit StringInterner(ArenaAllocator &allocator, bool preInternKeywords = true)
      : arena(allocator),
        internedStrings(
            ArenaSTLAllocator<
                std::pair<const std::string_view, InternedString>>(allocator)) {
    if (preInternKeywords) {
      internCommonStrings();
      S::initializeBuiltinNames(*this);
    }
  }

  ~StringInterner() = default;

  // Non-copyable, movable (but move assignment deleted due to reference member)
  StringInterner(const StringInterner &) = delete;
  StringInterner &operator=(const StringInterner &) = delete;
  StringInterner(StringInterner &&) = default;
  StringInterner &operator=(StringInterner &&) = delete;

  // Main interning interface
  [[nodiscard]] InternedString intern(std::string_view str);

  [[nodiscard]] InternedString intern(const char *str) {
    return intern(std::string_view(str));
  }

  [[nodiscard]] InternedString intern(const std::string &str) {
    return intern(std::string_view(str));
  }

  // Statistics
  [[nodiscard]] size_t getStringCount() const noexcept {
    return internedStrings.size();
  }
  [[nodiscard]] size_t getBucketCount() const noexcept {
    return internedStrings.bucket_count();
  }
  [[nodiscard]] double getLoadFactor() const noexcept {
    return internedStrings.load_factor();
  }

  // Memory usage
  [[nodiscard]] size_t getTotalMemoryUsed() const noexcept;

  // Debug utilities
  void printStatistics() const;
  void printAllStrings() const; // Iterate over internedStrings map

private:
  [[nodiscard]] InternedString internNewString(std::string_view str);
  
  // Pre-intern common strings for performance
  void internCommonStrings();
};



} // namespace cxy
