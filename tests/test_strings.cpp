#include "catch2.hpp"
#include <cxy/strings.hpp>

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace cxy;

TEST_CASE("InternedString basic functionality", "[strings][interned_string]") {
  SECTION("Default construction") {
    InternedString empty;

    REQUIRE(empty.empty());
    REQUIRE(empty.size() == 0);
    REQUIRE(empty.getHash() == 0);
    REQUIRE(std::string(empty.c_str()) == "");
    REQUIRE(empty.view() == "");
    REQUIRE(empty.toString() == "");
  }

  SECTION("Comparison operators") {
    ArenaAllocator arena(1024);
    StringInterner interner(arena);

    InternedString str1 = interner.intern("hello");
    InternedString str2 = interner.intern("hello"); // Same string
    InternedString str3 = interner.intern("world");

    // Equality
    REQUIRE(str1 == str2);
    REQUIRE(str1 != str3);
    REQUIRE(str2 != str3);

    // Ordering
    REQUIRE(str1 < str3); // "hello" < "world"
    REQUIRE(str3 > str1);
    REQUIRE(str1 <= str2); // Same strings
    REQUIRE(str1 >= str2);

    // Three-way comparison
    REQUIRE((str1 <=> str2) == std::strong_ordering::equal);
    REQUIRE((str1 <=> str3) == std::strong_ordering::less);
    REQUIRE((str3 <=> str1) == std::strong_ordering::greater);
  }

  SECTION("Hash functionality") {
    ArenaAllocator arena(1024);
    StringInterner interner(arena);

    InternedString str1 = interner.intern("test");
    InternedString str2 = interner.intern("test");
    InternedString str3 = interner.intern("different");

    // Same strings have same hash
    REQUIRE(str1.getHash() == str2.getHash());

    // Different strings have different hash (highly likely)
    REQUIRE(str1.getHash() != str3.getHash());

    // Hash functor works
    InternedString::Hash hasher;
    REQUIRE(hasher(str1) == str1.getHash());
    REQUIRE(hasher(str1) == hasher(str2));
  }

  SECTION("String conversions") {
    ArenaAllocator arena(1024);
    StringInterner interner(arena);

    std::string original = "test_string";
    InternedString interned = interner.intern(original);

    REQUIRE(interned.view() == original);
    REQUIRE(interned.toString() == original);
    REQUIRE(std::string(interned.c_str()) == original);
    REQUIRE(interned.size() == original.size());
  }
}

TEST_CASE("StringInterner functionality", "[strings][string_interner]") {
  SECTION("Basic interning") {
    ArenaAllocator arena(1024);
    StringInterner interner(arena);

    InternedString str1 = interner.intern("hello");
    InternedString str2 = interner.intern("world");
    InternedString str3 = interner.intern("hello"); // Duplicate

    REQUIRE(str1 == str3); // Same pointer
    REQUIRE(str1 != str2); // Different strings

    REQUIRE(interner.getStringCount() == 2); // Only unique strings counted
  }

  SECTION("Empty string handling") {
    ArenaAllocator arena(1024);
    StringInterner interner(arena);

    InternedString empty1 = interner.intern("");
    InternedString empty2 = interner.intern(std::string());
    InternedString empty3 = interner.intern(std::string_view());

    REQUIRE(empty1 == empty2);
    REQUIRE(empty2 == empty3);
    REQUIRE(empty1.empty());

    // Empty strings might not be stored in the map
    // depending on implementation details
  }

  SECTION("Different input types") {
    ArenaAllocator arena(1024);
    StringInterner interner(arena);

    std::string stdStr = "test";
    std::string_view strView = "test";
    const char *cStr = "test";

    InternedString str1 = interner.intern(stdStr);
    InternedString str2 = interner.intern(strView);
    InternedString str3 = interner.intern(cStr);

    REQUIRE(str1 == str2);
    REQUIRE(str2 == str3);
    REQUIRE(str1 == str3);

    REQUIRE(interner.getStringCount() == 1);
  }

  SECTION("Statistics") {
    ArenaAllocator arena(1024);
    StringInterner interner(arena);

    REQUIRE(interner.getStringCount() == 0);
    REQUIRE(interner.getLoadFactor() == 0.0);

    auto str1 = interner.intern("first");
    REQUIRE(interner.getStringCount() == 1);
    REQUIRE(interner.getBucketCount() >
            0); // Should have buckets after first insert
    REQUIRE(interner.getLoadFactor() > 0.0);

    auto str2 = interner.intern("second");
    REQUIRE(interner.getStringCount() == 2);

    auto str3 = interner.intern("first");    // Duplicate
    REQUIRE(interner.getStringCount() == 2); // No change

    size_t memUsed = interner.getTotalMemoryUsed();
    REQUIRE(memUsed >= 12); // "first" + "second" + null terminators
  }

  SECTION("Performance with many strings") {
    ArenaAllocator arena(64 * 1024);
    StringInterner interner(arena);

    std::vector<InternedString> strings;
    const size_t numStrings = 1000;

    // Create many unique strings
    for (size_t i = 0; i < numStrings; ++i) {
      std::string str = "string_" + std::to_string(i);
      strings.push_back(interner.intern(str));
    }

    REQUIRE(interner.getStringCount() == numStrings);

    // All strings should be unique
    for (size_t i = 0; i < strings.size(); ++i) {
      for (size_t j = i + 1; j < strings.size(); ++j) {
        REQUIRE(strings[i] != strings[j]);
      }
    }

    // Re-interning should not create duplicates
    for (size_t i = 0; i < numStrings; ++i) {
      std::string str = "string_" + std::to_string(i);
      InternedString duplicate = interner.intern(str);
      REQUIRE(duplicate == strings[i]);
    }

    REQUIRE(interner.getStringCount() == numStrings); // Still the same count
  }
}

TEST_CASE("InternedString in STL containers", "[strings][stl]") {
  SECTION("unordered_set integration") {
    ArenaAllocator arena(1024);
    StringInterner interner(arena);

    std::unordered_set<InternedString, InternedString::Hash> stringSet;

    InternedString str1 = interner.intern("apple");
    InternedString str2 = interner.intern("banana");
    InternedString str3 = interner.intern("apple"); // Duplicate

    stringSet.insert(str1);
    stringSet.insert(str2);
    stringSet.insert(str3); // Should not increase size

    REQUIRE(stringSet.size() == 2);
    REQUIRE(stringSet.count(str1) == 1);
    REQUIRE(stringSet.count(str2) == 1);
    REQUIRE(stringSet.count(str3) == 1); // Same as str1
  }

  SECTION("unordered_map as symbol table") {
    ArenaAllocator arena(1024);
    StringInterner interner(arena);

    std::unordered_map<InternedString, int, InternedString::Hash> symbolTable;

    InternedString varName = interner.intern("variable");
    InternedString funcName = interner.intern("function");

    symbolTable[varName] = 42;
    symbolTable[funcName] = 100;

    // Lookup using same strings
    InternedString lookupVar = interner.intern("variable");
    InternedString lookupFunc = interner.intern("function");

    REQUIRE(symbolTable[lookupVar] == 42);
    REQUIRE(symbolTable[lookupFunc] == 100);

    // Fast O(1) lookup due to pointer equality
    REQUIRE(symbolTable.count(lookupVar) == 1);
    REQUIRE(symbolTable.count(lookupFunc) == 1);
  }

  SECTION("Sorting and ordering") {
    ArenaAllocator arena(1024);
    StringInterner interner(arena);

    std::vector<InternedString> strings;
    strings.push_back(interner.intern("zebra"));
    strings.push_back(interner.intern("apple"));
    strings.push_back(interner.intern("banana"));
    strings.push_back(interner.intern("cherry"));

    std::sort(strings.begin(), strings.end());

    REQUIRE(strings[0].view() == "apple");
    REQUIRE(strings[1].view() == "banana");
    REQUIRE(strings[2].view() == "cherry");
    REQUIRE(strings[3].view() == "zebra");
  }
}

TEST_CASE("String interner edge cases", "[strings][edge_cases]") {
  SECTION("Very long strings") {
    ArenaAllocator arena(64 * 1024);
    StringInterner interner(arena);

    std::string longString(10000, 'A');
    InternedString interned = interner.intern(longString);

    REQUIRE(interned.size() == 10000);
    REQUIRE(interned.view() == longString);
    REQUIRE(interned.toString() == longString);
  }

  SECTION("Strings with null characters") {
    ArenaAllocator arena(1024);
    StringInterner interner(arena);

    std::string stringWithNull = "hello\0world";
    stringWithNull.resize(11); // Ensure null is included

    InternedString interned = interner.intern(stringWithNull);

    REQUIRE(interned.size() == 11);
    REQUIRE(interned.view().size() == 11);
    // String content comparison
    REQUIRE(std::memcmp(interned.c_str(), stringWithNull.data(), 11) == 0);
  }

  SECTION("Unicode strings") {
    ArenaAllocator arena(1024);
    StringInterner interner(arena);

    std::string unicode = "Hello, 世界! 🌍";
    InternedString interned = interner.intern(unicode);

    REQUIRE(interned.view() == unicode);
    REQUIRE(interned.toString() == unicode);

    // Test that duplicate unicode strings are properly interned
    InternedString duplicate = interner.intern(unicode);
    REQUIRE(interned == duplicate);
    REQUIRE(interner.getStringCount() == 1);
  }

  SECTION("Hash collision handling") {
    ArenaAllocator arena(1024);
    StringInterner interner(arena);

    // Create many strings to potentially force hash collisions
    std::vector<InternedString> strings;
    for (int i = 0; i < 100; ++i) {
      std::string str = "collision_test_" + std::to_string(i);
      strings.push_back(interner.intern(str));
    }

    REQUIRE(interner.getStringCount() == 100);

    // All strings should be unique despite potential collisions
    for (size_t i = 0; i < strings.size(); ++i) {
      for (size_t j = i + 1; j < strings.size(); ++j) {
        REQUIRE(strings[i] != strings[j]);
      }
    }
  }
}
