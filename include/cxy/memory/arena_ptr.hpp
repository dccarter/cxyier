#pragma once

#include <concepts>
#include <cxy/memory/arena.hpp>
#include <memory>
#include <type_traits>

namespace cxy {

template <typename T> class ArenaPtr {
private:
  T *ptr;
  ArenaAllocator *arena; // For debugging/validation only

public:
  // Default constructor
  constexpr ArenaPtr() noexcept : ptr(nullptr), arena(nullptr) {}

  // Constructor from raw pointer and arena
  constexpr ArenaPtr(T *p, ArenaAllocator *a) noexcept : ptr(p), arena(a) {}

  // Null pointer constructor
  constexpr ArenaPtr(std::nullptr_t) noexcept : ptr(nullptr), arena(nullptr) {}

  // Copy constructor (copyable because we don't own memory)
  constexpr ArenaPtr(const ArenaPtr &) noexcept = default;
  constexpr ArenaPtr &operator=(const ArenaPtr &) noexcept = default;

  // Move constructor
  constexpr ArenaPtr(ArenaPtr &&other) noexcept
      : ptr(other.ptr), arena(other.arena) {
    other.ptr = nullptr;
    other.arena = nullptr;
  }

  // Move assignment
  constexpr ArenaPtr &operator=(ArenaPtr &&other) noexcept {
    if (this != &other) {
      ptr = other.ptr;
      arena = other.arena;
      other.ptr = nullptr;
      other.arena = nullptr;
    }
    return *this;
  }

  // Assignment from nullptr
  constexpr ArenaPtr &operator=(std::nullptr_t) noexcept {
    ptr = nullptr;
    arena = nullptr;
    return *this;
  }

  // No automatic destruction - arena manages memory
  ~ArenaPtr() = default;

  // Access operators
  [[nodiscard]] constexpr T *get() const noexcept { return ptr; }

  [[nodiscard]] constexpr T &operator*() const noexcept
    requires(!std::is_void_v<T>)
  {
    return *ptr;
  }

  [[nodiscard]] constexpr T *operator->() const noexcept { return ptr; }

  // Array access (if T is array type)
  template <typename U = T>
  [[nodiscard]] constexpr auto &operator[](size_t index) const noexcept
    requires std::is_array_v<U>
  {
    return ptr[index];
  }

  // Boolean conversion
  [[nodiscard]] constexpr explicit operator bool() const noexcept {
    return ptr != nullptr;
  }

  // Comparison operators
  [[nodiscard]] constexpr bool
  operator==(const ArenaPtr &other) const noexcept = default;
  [[nodiscard]] constexpr bool
  operator!=(const ArenaPtr &other) const noexcept = default;
  [[nodiscard]] constexpr bool operator==(std::nullptr_t) const noexcept {
    return ptr == nullptr;
  }
  [[nodiscard]] constexpr bool operator!=(std::nullptr_t) const noexcept {
    return ptr != nullptr;
  }

  // Ordering (for use in containers)
  [[nodiscard]] constexpr auto
  operator<=>(const ArenaPtr &other) const noexcept {
    return ptr <=> other.ptr;
  }

  // Reset to null
  constexpr void reset() noexcept {
    ptr = nullptr;
    arena = nullptr;
  }

  // Reset to new pointer
  constexpr void reset(T *newPtr, ArenaAllocator *newArena = nullptr) noexcept {
    ptr = newPtr;
    arena = newArena;
  }

  // Get arena (for debugging)
  [[nodiscard]] constexpr ArenaAllocator *getArena() const noexcept {
    return arena;
  }
};

// Factory function using C++23 deduction guides
template <typename T, typename... Args>
[[nodiscard]] ArenaPtr<T> makeArenaPtr(ArenaAllocator &arena, Args &&...args)
  requires std::constructible_from<T, Args...>
{
  T *ptr = arena.construct<T>(std::forward<Args>(args)...);
  return ArenaPtr<T>(ptr, &arena);
}

// Factory function for arrays
template <typename T>
[[nodiscard]] ArenaPtr<T> makeArenaPtrArray(ArenaAllocator &arena, size_t count)
  requires(!std::is_array_v<T>)
{
  T *ptr = arena.allocateArray<T>(count);
  return ArenaPtr<T>(ptr, &arena);
}

// Factory function for arrays with initialization
template <typename T, typename... Args>
[[nodiscard]] ArenaPtr<T> makeArenaPtrArray(ArenaAllocator &arena, size_t count,
                                            Args &&...args)
  requires std::constructible_from<T, Args...>
{
  T *ptr = arena.constructArray<T>(count, std::forward<Args>(args)...);
  return ArenaPtr<T>(ptr, &arena);
}

// Comparison with nullptr (free functions for symmetry)
template <typename T>
[[nodiscard]] constexpr bool operator==(std::nullptr_t,
                                        const ArenaPtr<T> &ptr) noexcept {
  return ptr == nullptr;
}

template <typename T>
[[nodiscard]] constexpr bool operator!=(std::nullptr_t,
                                        const ArenaPtr<T> &ptr) noexcept {
  return ptr != nullptr;
}

} // namespace cxy
