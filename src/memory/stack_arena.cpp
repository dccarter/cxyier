#include <algorithm>
#include <cxy/stack_arena.hpp>

namespace cxy {

StackArena::StackArena(size_t blockSize) : ArenaAllocator(blockSize) {
  // Reserve some space for checkpoints to avoid allocations
  checkpoints.reserve(32);
}

StackArena::Checkpoint StackArena::saveCheckpoint() noexcept {
  Checkpoint cp{currentBlock, currentBlock ? currentBlock->used : 0};
  checkpoints.push_back(cp);
  return cp;
}

void StackArena::restoreCheckpoint(const Checkpoint &checkpoint) noexcept {
  // Restore memory state to the checkpoint
  if (checkpoint.block) {
    // Reset all blocks after the checkpoint block
    auto *block = checkpoint.block->next;
    while (block) {
      block->reset();
      block = block->next;
    }

    // Set current block and used amount
    currentBlock = checkpoint.block;
    currentBlock->used = checkpoint.offset;

    // Recalculate total used
    totalUsed = 0;
    for (auto *b = firstBlock; b; b = b->next) {
      if (b == currentBlock) {
        totalUsed += checkpoint.offset;
        break;
      } else {
        totalUsed += b->used;
      }
    }
  } else {
    // Checkpoint was at the beginning, reset everything
    reset();
  }

  // Remove checkpoints that are after this one
  auto it = std::find(checkpoints.begin(), checkpoints.end(), checkpoint);
  if (it != checkpoints.end()) {
    checkpoints.erase(it + 1, checkpoints.end());
  }
}

void StackArena::popCheckpoint() noexcept {
  if (!checkpoints.empty()) {
    const auto checkpoint = checkpoints.back();
    checkpoints.pop_back();
    restoreCheckpoint(checkpoint);
  }
}

} // namespace cxy
