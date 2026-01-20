#pragma once

#include <cstddef>
#include <memory>
#include <new>
#include <vector>

namespace cxy {

struct MemoryBlock {
  void *data;
  size_t size;
  size_t used;
  MemoryBlock *next;

  explicit MemoryBlock(size_t blockSize);
  ~MemoryBlock();

  // Non-copyable, movable
  MemoryBlock(const MemoryBlock &) = delete;
  MemoryBlock &operator=(const MemoryBlock &) = delete;
  MemoryBlock(MemoryBlock &&other) noexcept;
  MemoryBlock &operator=(MemoryBlock &&other) noexcept;

  [[nodiscard]] bool
  hasSpace(size_t requestedSize,
           size_t alignment = alignof(std::max_align_t)) const noexcept;
  [[nodiscard]] void *
  allocate(size_t size, size_t alignment = alignof(std::max_align_t)) noexcept;
  void reset() noexcept;

private:
  [[nodiscard]] static size_t alignAddress(size_t address,
                                           size_t alignment) noexcept;
};

class ArenaAllocator {
private:
  MemoryBlock *firstBlock;
  MemoryBlock *currentBlock;
  size_t defaultBlockSize;
  size_t totalAllocated;
  size_t totalUsed;

  friend class StackArena; // Allow StackArena to access private members

public:
  explicit ArenaAllocator(size_t blockSize = 64 * 1024); // 64KB default
  ~ArenaAllocator();

  // Non-copyable, movable
  ArenaAllocator(const ArenaAllocator &) = delete;
  ArenaAllocator &operator=(const ArenaAllocator &) = delete;
  ArenaAllocator(ArenaAllocator &&other) noexcept;
  ArenaAllocator &operator=(ArenaAllocator &&other) noexcept;

  // Core allocation interface
  [[nodiscard]] void *allocate(size_t size,
                               size_t alignment = alignof(std::max_align_t));

  // Typed allocation helpers
  template <typename T> [[nodiscard]] T *allocate() {
    return static_cast<T *>(allocate(sizeof(T), alignof(T)));
  }

  template <typename T> [[nodiscard]] T *allocateArray(size_t count) {
    return static_cast<T *>(allocate(sizeof(T) * count, alignof(T)));
  }

  // Construction helpers using C++23 perfect forwarding
  template <typename T, typename... Args>
  [[nodiscard]] T *construct(Args &&...args)
    requires std::constructible_from<T, Args...>
  {
    T *ptr = allocate<T>();
    return new (ptr) T(std::forward<Args>(args)...);
  }

  template <typename T, typename... Args>
  [[nodiscard]] T *constructArray(size_t count, Args &&...args)
    requires std::constructible_from<T, Args...>
  {
    T *ptr = allocateArray<T>(count);
    for (size_t i = 0; i < count; ++i) {
      new (ptr + i) T(args...);
    }
    return ptr;
  }

  // Memory management
  void reset() noexcept; // Clear all allocations, keep blocks
  void clear() noexcept; // Free all blocks, reset to initial state

  // Statistics
  [[nodiscard]] size_t getTotalAllocated() const noexcept {
    return totalAllocated;
  }
  [[nodiscard]] size_t getTotalUsed() const noexcept { return totalUsed; }
  [[nodiscard]] size_t getWastePercentage() const noexcept;
  [[nodiscard]] size_t getBlockCount() const noexcept;

private:
  MemoryBlock *allocateNewBlock(size_t minSize);
  void freeAllBlocks() noexcept;
};

class StackArena : public ArenaAllocator {
public:
  struct Checkpoint {
    MemoryBlock *block;
    size_t offset;

    bool operator==(const Checkpoint &) const = default;
  };

private:
  std::vector<Checkpoint> checkpoints;

public:
  explicit StackArena(size_t blockSize = 64 * 1024);

  // Stack-like operations
  [[nodiscard]] Checkpoint saveCheckpoint() noexcept;
  void restoreCheckpoint(const Checkpoint &checkpoint) noexcept;
  void popCheckpoint() noexcept; // Restore to last checkpoint and remove it

  // RAII checkpoint wrapper using C++23 features
  class ScopedCheckpoint {
    StackArena *arena;
    Checkpoint checkpoint;

  public:
    explicit ScopedCheckpoint(StackArena &a) noexcept
        : arena(&a), checkpoint(a.saveCheckpoint()) {}

    ~ScopedCheckpoint() {
      if (arena) {
        arena->restoreCheckpoint(checkpoint);
      }
    }

    // Non-copyable, movable
    ScopedCheckpoint(const ScopedCheckpoint &) = delete;
    ScopedCheckpoint &operator=(const ScopedCheckpoint &) = delete;

    ScopedCheckpoint(ScopedCheckpoint &&other) noexcept
        : arena(other.arena), checkpoint(other.checkpoint) {
      other.arena = nullptr;
    }

    ScopedCheckpoint &operator=(ScopedCheckpoint &&other) noexcept {
      if (this != &other) {
        if (arena) {
          arena->restoreCheckpoint(checkpoint);
        }
        arena = other.arena;
        checkpoint = other.checkpoint;
        other.arena = nullptr;
      }
      return *this;
    }

    // Access to checkpoint for advanced usage
    [[nodiscard]] const Checkpoint &getCheckpoint() const noexcept {
      return checkpoint;
    }
  };

  // Get checkpoint count for debugging
  [[nodiscard]] size_t getCheckpointCount() const noexcept {
    return checkpoints.size();
  }
};

} // namespace cxy
