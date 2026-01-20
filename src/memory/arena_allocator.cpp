#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <cxy/memory/arena.hpp>
#include <stdexcept>

namespace cxy {

// MemoryBlock implementation
MemoryBlock::MemoryBlock(size_t blockSize)
    : data(nullptr), size(blockSize), used(0), next(nullptr) {
  if (blockSize == 0) {
    throw std::invalid_argument("Block size cannot be zero");
  }

  data = std::aligned_alloc(alignof(std::max_align_t), blockSize);
  if (!data) {
    throw std::bad_alloc();
  }
}

MemoryBlock::~MemoryBlock() {
  if (data) {
    std::free(data);
    data = nullptr;
  }
}

MemoryBlock::MemoryBlock(MemoryBlock &&other) noexcept
    : data(other.data), size(other.size), used(other.used), next(other.next) {
  other.data = nullptr;
  other.size = 0;
  other.used = 0;
  other.next = nullptr;
}

MemoryBlock &MemoryBlock::operator=(MemoryBlock &&other) noexcept {
  if (this != &other) {
    if (data) {
      std::free(data);
    }

    data = other.data;
    size = other.size;
    used = other.used;
    next = other.next;

    other.data = nullptr;
    other.size = 0;
    other.used = 0;
    other.next = nullptr;
  }
  return *this;
}

bool MemoryBlock::hasSpace(size_t requestedSize,
                           size_t alignment) const noexcept {
  if (!data)
    return false;

  const size_t currentAddress =
      reinterpret_cast<size_t>(static_cast<char *>(data) + used);
  const size_t alignedAddress = alignAddress(currentAddress, alignment);
  const size_t alignedUsed = alignedAddress - reinterpret_cast<size_t>(data);

  return alignedUsed + requestedSize <= size;
}

void *MemoryBlock::allocate(size_t requestedSize, size_t alignment) noexcept {
  if (!hasSpace(requestedSize, alignment)) {
    return nullptr;
  }

  const size_t currentAddress =
      reinterpret_cast<size_t>(static_cast<char *>(data) + used);
  const size_t alignedAddress = alignAddress(currentAddress, alignment);
  const size_t alignedUsed = alignedAddress - reinterpret_cast<size_t>(data);

  used = alignedUsed + requestedSize;
  return reinterpret_cast<void *>(alignedAddress);
}

void MemoryBlock::reset() noexcept { used = 0; }

size_t MemoryBlock::alignAddress(size_t address, size_t alignment) noexcept {
  return (address + alignment - 1) & ~(alignment - 1);
}

// ArenaAllocator implementation
ArenaAllocator::ArenaAllocator(size_t blockSize)
    : firstBlock(nullptr), currentBlock(nullptr), defaultBlockSize(blockSize),
      totalAllocated(0), totalUsed(0) {
  if (blockSize == 0) {
    throw std::invalid_argument("Block size cannot be zero");
  }
}

ArenaAllocator::~ArenaAllocator() { clear(); }

ArenaAllocator::ArenaAllocator(ArenaAllocator &&other) noexcept
    : firstBlock(other.firstBlock), currentBlock(other.currentBlock),
      defaultBlockSize(other.defaultBlockSize),
      totalAllocated(other.totalAllocated), totalUsed(other.totalUsed) {
  other.firstBlock = nullptr;
  other.currentBlock = nullptr;
  other.totalAllocated = 0;
  other.totalUsed = 0;
}

ArenaAllocator &ArenaAllocator::operator=(ArenaAllocator &&other) noexcept {
  if (this != &other) {
    clear();

    firstBlock = other.firstBlock;
    currentBlock = other.currentBlock;
    defaultBlockSize = other.defaultBlockSize;
    totalAllocated = other.totalAllocated;
    totalUsed = other.totalUsed;

    other.firstBlock = nullptr;
    other.currentBlock = nullptr;
    other.totalAllocated = 0;
    other.totalUsed = 0;
  }
  return *this;
}

void *ArenaAllocator::allocate(size_t size, size_t alignment) {
  if (size == 0) {
    return nullptr;
  }

  // Try current block first
  if (currentBlock) {
    if (void *ptr = currentBlock->allocate(size, alignment)) {
      totalUsed += size;
      return ptr;
    }
  }

  // Need a new block
  const size_t minBlockSize = std::max(size + alignment, defaultBlockSize);
  currentBlock = allocateNewBlock(minBlockSize);

  void *ptr = currentBlock->allocate(size, alignment);
  assert(ptr != nullptr); // Should always succeed on new block
  totalUsed += size;
  return ptr;
}

void ArenaAllocator::reset() noexcept {
  for (auto *block = firstBlock; block; block = block->next) {
    block->reset();
  }
  currentBlock = firstBlock;
  totalUsed = 0;
}

void ArenaAllocator::clear() noexcept {
  freeAllBlocks();
  firstBlock = nullptr;
  currentBlock = nullptr;
  totalAllocated = 0;
  totalUsed = 0;
}

size_t ArenaAllocator::getWastePercentage() const noexcept {
  if (totalAllocated == 0)
    return 0;
  return ((totalAllocated - totalUsed) * 100) / totalAllocated;
}

size_t ArenaAllocator::getBlockCount() const noexcept {
  size_t count = 0;
  for (auto *block = firstBlock; block; block = block->next) {
    ++count;
  }
  return count;
}

MemoryBlock *ArenaAllocator::allocateNewBlock(size_t minSize) {
  auto newBlock = std::make_unique<MemoryBlock>(minSize);
  totalAllocated += minSize;

  MemoryBlock *blockPtr = newBlock.release();

  if (!firstBlock) {
    firstBlock = blockPtr;
  } else {
    // Link to the end of the chain
    auto *last = firstBlock;
    while (last->next) {
      last = last->next;
    }
    last->next = blockPtr;
  }

  return blockPtr;
}

void ArenaAllocator::freeAllBlocks() noexcept {
  auto *current = firstBlock;
  while (current) {
    auto *next = current->next;
    delete current;
    current = next;
  }
}

} // namespace cxy
