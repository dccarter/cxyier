#pragma once

#include <cxy/arena_allocator.hpp>
#include <vector>

namespace cxy {

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
