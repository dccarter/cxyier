#pragma once

#include <compare>
#include <cstddef>
#include <cstring>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>

#include <cxy/arena_allocator.hpp>
#include <cxy/arena_stl.hpp>

namespace cxy {

// Forward declaration
class StringInterner;

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
  explicit StringInterner(ArenaAllocator &allocator)
      : arena(allocator),
        internedStrings(
            ArenaSTLAllocator<
                std::pair<const std::string_view, InternedString>>(allocator)) {
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
};

} // namespace cxy
