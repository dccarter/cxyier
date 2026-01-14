#pragma once

// Convenience header that includes all arena memory management components

#include <cxy/arena_allocator.hpp>
#include <cxy/arena_ptr.hpp>
#include <cxy/arena_stl.hpp>
#include <cxy/stack_arena.hpp>
#include <cxy/strings.hpp>

namespace cxy {

// Export all arena-related types for convenience
using namespace cxy;

// Version and feature information
constexpr int ARENA_VERSION_MAJOR = 1;
constexpr int ARENA_VERSION_MINOR = 0;
constexpr int ARENA_VERSION_PATCH = 0;

constexpr const char *ARENA_VERSION_STRING = "1.0.0";

} // namespace cxy
