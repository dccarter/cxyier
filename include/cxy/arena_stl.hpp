#pragma once

#include <cxy/arena_allocator.hpp>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace cxy {

template <typename T> class ArenaSTLAllocator {
private:
  ArenaAllocator *arena;

public:
  // Standard allocator type definitions
  using value_type = T;
  using pointer = T *;
  using const_pointer = const T *;
  using reference = T &;
  using const_reference = const T &;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

  // Rebind for different types (C++17+ style)
  template <typename U> struct rebind {
    using other = ArenaSTLAllocator<U>;
  };

  // C++20 allocator requirements
  using is_always_equal = std::false_type;
  using propagate_on_container_copy_assignment = std::false_type;
  using propagate_on_container_move_assignment = std::true_type;
  using propagate_on_container_swap = std::true_type;

  // Constructors
  explicit ArenaSTLAllocator(ArenaAllocator &a) noexcept : arena(&a) {}

  template <typename U>
  ArenaSTLAllocator(const ArenaSTLAllocator<U> &other) noexcept
      : arena(other.arena) {}

  ArenaSTLAllocator(const ArenaSTLAllocator &) noexcept = default;
  ArenaSTLAllocator &operator=(const ArenaSTLAllocator &) noexcept = default;

  // Core allocator interface
  [[nodiscard]] pointer allocate(size_type n) {
    if (n == 0)
      return nullptr;

    // Check for overflow
    if (n > std::numeric_limits<size_type>::max() / sizeof(T)) {
      throw std::bad_array_new_length{};
    }

    return static_cast<pointer>(arena->allocate(n * sizeof(T), alignof(T)));
  }

  void deallocate(pointer, size_type) noexcept {
    // Arena allocator doesn't support individual deallocation
    // This is intentionally a no-op
  }

  // C++20 style construct/destroy (using placement new)
  template <typename U, typename... Args>
  void construct(U *p, Args &&...args) noexcept(
      std::is_nothrow_constructible_v<U, Args...>) {
    new (p) U(std::forward<Args>(args)...);
  }

  template <typename U>
  void destroy(U *p) noexcept(std::is_nothrow_destructible_v<U>) {
    if constexpr (!std::is_trivially_destructible_v<U>) {
      p->~U();
    }
  }

  // Comparison operators
  template <typename U>
  [[nodiscard]] constexpr bool
  operator==(const ArenaSTLAllocator<U> &other) const noexcept {
    return arena == other.arena;
  }

  template <typename U>
  [[nodiscard]] constexpr bool
  operator!=(const ArenaSTLAllocator<U> &other) const noexcept {
    return !(*this == other);
  }

  // Get arena pointer (for advanced usage)
  [[nodiscard]] ArenaAllocator *getArena() const noexcept { return arena; }

  // Allow access to arena from other template instantiations
  template <typename U> friend class ArenaSTLAllocator;
};

// Type aliases for common containers using C++23 features
template <typename T> using ArenaVector = std::vector<T, ArenaSTLAllocator<T>>;

template <typename Key, typename Value, typename Hash = std::hash<Key>,
          typename KeyEqual = std::equal_to<Key>>
using ArenaUnorderedMap =
    std::unordered_map<Key, Value, Hash, KeyEqual,
                       ArenaSTLAllocator<std::pair<const Key, Value>>>;

template <typename T> using ArenaList = std::list<T, ArenaSTLAllocator<T>>;

template <typename Key, typename Hash = std::hash<Key>,
          typename KeyEqual = std::equal_to<Key>>
using ArenaUnorderedSet =
    std::unordered_set<Key, Hash, KeyEqual, ArenaSTLAllocator<Key>>;

template <typename Key, typename Compare = std::less<Key>>
using ArenaSet = std::set<Key, Compare, ArenaSTLAllocator<Key>>;

template <typename Key, typename Value, typename Compare = std::less<Key>>
using ArenaMap = std::map<Key, Value, Compare,
                          ArenaSTLAllocator<std::pair<const Key, Value>>>;

// Helper factory functions with improved C++23 syntax
template <typename T>
[[nodiscard]] ArenaVector<T> makeArenaVector(ArenaAllocator &arena) {
  return ArenaVector<T>(ArenaSTLAllocator<T>(arena));
}

template <typename T>
[[nodiscard]] ArenaVector<T> makeArenaVector(ArenaAllocator &arena,
                                             std::initializer_list<T> init) {
  ArenaVector<T> vec{ArenaSTLAllocator<T>(arena)};
  vec.assign(init);
  return vec;
}

template <typename T>
[[nodiscard]] ArenaVector<T> makeArenaVector(ArenaAllocator &arena,
                                             size_t size) {
  return ArenaVector<T>(size, T{}, ArenaSTLAllocator<T>(arena));
}

template <typename T>
[[nodiscard]] ArenaVector<T> makeArenaVector(ArenaAllocator &arena, size_t size,
                                             const T &value) {
  return ArenaVector<T>(size, value, ArenaSTLAllocator<T>(arena));
}

template <typename Key, typename Value>
[[nodiscard]] ArenaUnorderedMap<Key, Value>
makeArenaMap(ArenaAllocator &arena) {
  return ArenaUnorderedMap<Key, Value>(
      ArenaSTLAllocator<std::pair<const Key, Value>>(arena));
}

template <typename Key, typename Value>
[[nodiscard]] ArenaUnorderedMap<Key, Value>
makeArenaMap(ArenaAllocator &arena,
             std::initializer_list<std::pair<const Key, Value>> init) {
  ArenaUnorderedMap<Key, Value> map{
      ArenaSTLAllocator<std::pair<const Key, Value>>(arena)};
  map.insert(init);
  return map;
}

template <typename T>
[[nodiscard]] ArenaUnorderedSet<T> makeArenaSet(ArenaAllocator &arena) {
  return ArenaUnorderedSet<T>(ArenaSTLAllocator<T>(arena));
}

template <typename T>
[[nodiscard]] ArenaUnorderedSet<T> makeArenaSet(ArenaAllocator &arena,
                                                std::initializer_list<T> init) {
  ArenaUnorderedSet<T> set{ArenaSTLAllocator<T>(arena)};
  set.insert(init);
  return set;
}

template <typename T>
[[nodiscard]] ArenaList<T> makeArenaList(ArenaAllocator &arena) {
  return ArenaList<T>(ArenaSTLAllocator<T>(arena));
}

template <typename T>
[[nodiscard]] ArenaList<T> makeArenaList(ArenaAllocator &arena,
                                         std::initializer_list<T> init) {
  ArenaList<T> list{ArenaSTLAllocator<T>(arena)};
  list.assign(init);
  return list;
}

} // namespace cxy
