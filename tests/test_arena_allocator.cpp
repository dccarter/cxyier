#include "catch2.hpp"
#include <cxy/memory.hpp>
#include <string>
#include <unordered_set>
#include <vector>

using namespace cxy;

TEST_CASE("Arena allocator basic functionality", "[arena]") {
  SECTION("Basic allocation and construction") {
    ArenaAllocator arena(1024);

    // Test basic allocation
    int *ptr = arena.allocate<int>();
    REQUIRE(ptr != nullptr);

    // Test construction
    int *constructed = arena.construct<int>(42);
    REQUIRE(constructed != nullptr);
    REQUIRE(*constructed == 42);

    // Test array allocation
    int *arr = arena.allocateArray<int>(10);
    REQUIRE(arr != nullptr);

    // Test array construction
    int *constructed_arr = arena.constructArray<int>(5, 99);
    REQUIRE(constructed_arr != nullptr);
    for (size_t i = 0; i < 5; ++i) {
      REQUIRE(constructed_arr[i] == 99);
    }
  }

  SECTION("Memory alignment") {
    ArenaAllocator arena(1024);

    // Test natural alignment
    double *d = arena.allocate<double>();
    REQUIRE(reinterpret_cast<uintptr_t>(d) % alignof(double) == 0);

    // Test over-aligned allocation
    struct alignas(64) OverAligned {
      char data[32];
    };

    OverAligned *oa = arena.allocate<OverAligned>();
    REQUIRE(reinterpret_cast<uintptr_t>(oa) % 64 == 0);
  }

  SECTION("Statistics tracking") {
    ArenaAllocator arena(1024);

    // Initially no blocks should be allocated (lazy allocation)
    REQUIRE(arena.getTotalAllocated() == 0);
    REQUIRE(arena.getTotalUsed() == 0);
    REQUIRE(arena.getBlockCount() == 0);

    // Allocate some memory - this should trigger first block allocation
    void *allocated_ptr = arena.allocate(100);
    REQUIRE(allocated_ptr != nullptr);
    REQUIRE(arena.getTotalAllocated() >= 1024); // At least one block
    REQUIRE(arena.getTotalUsed() == 100);
    REQUIRE(arena.getBlockCount() == 1);

    // Reset should clear usage but keep blocks
    arena.reset();
    REQUIRE(arena.getTotalUsed() == 0);
    REQUIRE(arena.getBlockCount() == 1);

    // Clear should free everything
    arena.clear();
    REQUIRE(arena.getTotalAllocated() == 0);
    REQUIRE(arena.getBlockCount() == 0);
  }
}

TEST_CASE("Stack arena checkpoint functionality", "[arena][stack]") {
  SECTION("Basic checkpoints") {
    StackArena arena(1024);

    // Allocate some data
    int *ptr1 = arena.construct<int>(1);
    REQUIRE(*ptr1 == 1);

    // Save checkpoint
    auto checkpoint = arena.saveCheckpoint();
    REQUIRE(arena.getCheckpointCount() == 1);

    // Allocate more data
    int *ptr2 = arena.construct<int>(2);
    int *ptr3 = arena.construct<int>(3);
    REQUIRE(*ptr2 == 2);
    REQUIRE(*ptr3 == 3);

    // Restore checkpoint
    arena.restoreCheckpoint(checkpoint);

    // ptr1 should still be valid, ptr2 and ptr3 should be "freed"
    REQUIRE(*ptr1 == 1);

    // Allocate again - should reuse the space
    int *ptr4 = arena.construct<int>(4);
    REQUIRE(*ptr4 == 4);
  }

  SECTION("RAII scoped checkpoint") {
    StackArena arena(1024);

    int *ptr1 = arena.construct<int>(1);

    {
      StackArena::ScopedCheckpoint cp(arena);

      int *ptr2 = arena.construct<int>(2);
      int *ptr3 = arena.construct<int>(3);
      REQUIRE(*ptr2 == 2);
      REQUIRE(*ptr3 == 3);

      // Checkpoint automatically restores on scope exit
    }

    // Should be back to checkpoint state
    int *ptr4 = arena.construct<int>(4);
    REQUIRE(*ptr4 == 4);
    REQUIRE(*ptr1 == 1);
  }
}

TEST_CASE("Arena STL containers", "[arena][stl]") {
  SECTION("ArenaVector functionality") {
    ArenaAllocator arena(1024);

    auto vec = makeArenaVector<int>(arena);

    // Test basic operations
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);

    REQUIRE(vec.size() == 3);
    REQUIRE(vec[0] == 1);
    REQUIRE(vec[1] == 2);
    REQUIRE(vec[2] == 3);

    // Test initialization
    auto vec2 = makeArenaVector<int>(arena, {10, 20, 30});
    REQUIRE(vec2.size() == 3);
    REQUIRE(vec2[0] == 10);
  }

  SECTION("ArenaUnorderedMap functionality") {
    ArenaAllocator arena(1024);

    auto map = makeArenaMap<std::string, int>(arena);

    map["hello"] = 1;
    map["world"] = 2;

    REQUIRE(map.size() == 2);
    REQUIRE(map["hello"] == 1);
    REQUIRE(map["world"] == 2);

    // Test initialization
    auto map2 =
        makeArenaMap<std::string, int>(arena, {{"a", 1}, {"b", 2}, {"c", 3}});
    REQUIRE(map2.size() == 3);
    REQUIRE(map2["a"] == 1);
  }
}

TEST_CASE("ArenaPtr smart pointer", "[arena][ptr]") {
  SECTION("Basic functionality") {
    ArenaAllocator arena(1024);

    auto ptr = makeArenaPtr<int>(arena, 42);
    REQUIRE(ptr);
    REQUIRE(*ptr == 42);
    REQUIRE(ptr.get() != nullptr);

    // Test assignment
    auto ptr2 = ptr;
    REQUIRE(ptr2);
    REQUIRE(*ptr2 == 42);
    REQUIRE(ptr.get() == ptr2.get());

    // Test move
    auto ptr3 = std::move(ptr2);
    REQUIRE(ptr3);
    REQUIRE(!ptr2);
    REQUIRE(*ptr3 == 42);
  }

  SECTION("Array functionality") {
    ArenaAllocator arena(1024);

    auto arr = makeArenaPtrArray<int>(arena, 5, 99);
    REQUIRE(arr);

    for (size_t i = 0; i < 5; ++i) {
      REQUIRE(arr.get()[i] == 99);
    }
  }
}

TEST_CASE("String interning functionality", "[interned_string]") {
  SECTION("Basic string interning") {
    ArenaAllocator arena(1024);
    StringInterner interner(arena);

    // Intern some strings
    InternedString str1 = interner.intern("hello");
    InternedString str2 = interner.intern("world");
    InternedString str3 = interner.intern("hello"); // Same as str1

    REQUIRE(!str1.empty());
    REQUIRE(!str2.empty());
    REQUIRE(!str3.empty());

    // Pointer equality for same strings
    REQUIRE(str1 == str3);
    REQUIRE(str1.c_str() == str3.c_str()); // Same pointer

    // Different strings are different
    REQUIRE(str1 != str2);
    REQUIRE(str1.c_str() != str2.c_str());

    // String content is correct
    REQUIRE(std::string_view(str1.c_str(), str1.size()) == "hello");
    REQUIRE(std::string_view(str2.c_str(), str2.size()) == "world");
    REQUIRE(str1.view() == "hello");
    REQUIRE(str2.view() == "world");

    // Hash values
    REQUIRE(str1.getHash() == str3.getHash());
    REQUIRE(str1.getHash() != str2.getHash());
  }

  SECTION("Empty string handling") {
    ArenaAllocator arena(1024);
    StringInterner interner(arena);

    InternedString empty1 = interner.intern("");
    InternedString empty2 = interner.intern(std::string());
    InternedString empty3 = interner.intern(std::string_view());

    REQUIRE(empty1.empty());
    REQUIRE(empty2.empty());
    REQUIRE(empty3.empty());

    REQUIRE(empty1 == empty2);
    REQUIRE(empty2 == empty3);

    REQUIRE(empty1.size() == 0);
    REQUIRE(std::string(empty1.c_str()) == "");
  }

  SECTION("String comparison and ordering") {
    ArenaAllocator arena(1024);
    StringInterner interner(arena);

    InternedString apple = interner.intern("apple");
    InternedString banana = interner.intern("banana");
    InternedString apple2 = interner.intern("apple");

    // Equality
    REQUIRE(apple == apple2);
    REQUIRE(apple != banana);

    // Ordering
    REQUIRE(apple < banana);
    REQUIRE(banana > apple);
    REQUIRE(apple <= apple2);
    REQUIRE(apple >= apple2);

    // String conversion
    REQUIRE(apple.toString() == "apple");
    REQUIRE(banana.toString() == "banana");
  }

  SECTION("Hash container integration") {
    ArenaAllocator arena(1024);
    StringInterner interner(arena);

    std::unordered_set<InternedString, InternedString::Hash> stringSet;

    InternedString str1 = interner.intern("first");
    InternedString str2 = interner.intern("second");
    InternedString str3 = interner.intern("first"); // Same as str1

    stringSet.insert(str1);
    stringSet.insert(str2);
    stringSet.insert(str3); // Should not increase size

    REQUIRE(stringSet.size() == 2);
    REQUIRE(stringSet.find(str1) != stringSet.end());
    REQUIRE(stringSet.find(str2) != stringSet.end());
    REQUIRE(stringSet.find(str3) != stringSet.end()); // Same as str1
  }

  SECTION("String interner statistics") {
    ArenaAllocator arena(1024);
    StringInterner interner(arena);

    REQUIRE(interner.getStringCount() == 0);

    auto hello1 = interner.intern("hello");
    REQUIRE(interner.getStringCount() == 1);

    auto world1 = interner.intern("world");
    REQUIRE(interner.getStringCount() == 2);

    auto hello2 = interner.intern("hello");  // Duplicate
    REQUIRE(interner.getStringCount() == 2); // Should not increase

    // Memory usage should be reasonable
    size_t memUsed = interner.getTotalMemoryUsed();
    REQUIRE(memUsed >= 10); // "hello" + "world" + null terminators
    REQUIRE(memUsed < 100); // Should not be excessive

    REQUIRE(interner.getBucketCount() > 0);
    REQUIRE(interner.getLoadFactor() >= 0.0);
  }

  SECTION("Different input types") {
    ArenaAllocator arena(1024);
    StringInterner interner(arena);

    std::string stdString = "test_string";
    std::string_view stringView = "test_string";
    const char *cString = "test_string";

    InternedString str1 = interner.intern(stdString);
    InternedString str2 = interner.intern(stringView);
    InternedString str3 = interner.intern(cString);

    // All should be the same interned string
    REQUIRE(str1 == str2);
    REQUIRE(str2 == str3);
    REQUIRE(str1.c_str() == str2.c_str());
    REQUIRE(str2.c_str() == str3.c_str());

    REQUIRE(interner.getStringCount() == 1);
  }

  SECTION("Large number of strings") {
    ArenaAllocator arena(64 * 1024); // Larger arena
    StringInterner interner(arena);

    std::vector<InternedString> strings;

    // Create many unique strings
    for (int i = 0; i < 1000; ++i) {
      std::string str = "string_" + std::to_string(i);
      strings.push_back(interner.intern(str));
    }

    REQUIRE(interner.getStringCount() == 1000);

    // Verify all strings are different
    for (size_t i = 0; i < strings.size(); ++i) {
      for (size_t j = i + 1; j < strings.size(); ++j) {
        REQUIRE(strings[i] != strings[j]);
      }
    }

    // Re-intern the same strings - should not increase count
    for (int i = 0; i < 1000; ++i) {
      std::string str = "string_" + std::to_string(i);
      InternedString duplicate = interner.intern(str);
      REQUIRE(duplicate == strings[i]);
    }

    REQUIRE(interner.getStringCount() == 1000); // Should not have increased
  }
}
