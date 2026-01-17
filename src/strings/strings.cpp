#include <cxy/strings.hpp>
#include <cxy/token.hpp>
#include <iostream>

namespace cxy {

// InternedString is mostly header-only, but we can add implementation details
// here if needed For now, all functionality is in the header file using inline
// methods

} // namespace cxy

namespace cxy {
InternedString StringInterner::intern(std::string_view str) {
  if (str.empty()) {
    static const InternedString emptyString; // Default constructed empty string
    return emptyString;
  }

  // Check if already exists
  auto it = internedStrings.find(str);
  if (it != internedStrings.end()) {
    return it->second;
  }

  // Need to intern new string
  return internNewString(str);
}

InternedString StringInterner::internNewString(std::string_view str) {
  // Allocate string data in arena
  char *arenaStr = arena.allocateArray<char>(str.length() + 1);
  std::memcpy(arenaStr, str.data(), str.length());
  arenaStr[str.length()] = '\0';

  // Create InternedString
  const size_t hash = std::hash<std::string_view>{}(str);
  InternedString result(arenaStr, str.length(), hash);

  // Store in map using string_view of arena-allocated data
  std::string_view arenaView(arenaStr, str.length());
  internedStrings[arenaView] = result;

  return result;
}

size_t StringInterner::getTotalMemoryUsed() const noexcept {
  size_t total = 0;
  for (const auto &[key, value] : internedStrings) {
    total += value.size() + 1; // +1 for null terminator
  }
  return total;
}

void StringInterner::printStatistics() const {
  std::cout << "String Interner Statistics:\n";
  std::cout << "  Total strings: " << getStringCount() << "\n";
  std::cout << "  Bucket count: " << getBucketCount() << "\n";
  std::cout << "  Load factor: " << getLoadFactor() << "\n";
  std::cout << "  Total memory used: " << getTotalMemoryUsed() << " bytes\n";
}

void StringInterner::printAllStrings() const {
  std::cout << "All interned strings:\n";
  size_t index = 0;
  for (const auto &[key, value] : internedStrings) {
    std::cout << "  [" << index << "] \"" << value.c_str()
              << "\" (hash: " << value.getHash() << ")\n";
    ++index;
  }
}

void StringInterner::internCommonStrings() {
  // Pre-intern all keywords from KEYWORD_LIST for performance
  #define INTERN_KEYWORD(name, str) (void)intern(str);
  KEYWORD_LIST(INTERN_KEYWORD)
  #undef INTERN_KEYWORD
  
  // Pre-intern builtin names
  #define INTERN_BUILTIN(name) (void)intern(#name);
  #define INTERN_BUILTIN_STR(name, str) (void)intern(str);
  CXY_BUILTIN_NAMES(INTERN_BUILTIN, INTERN_BUILTIN_STR)
  #undef INTERN_BUILTIN
  #undef INTERN_BUILTIN_STR
  
  // Pre-intern other common strings
  (void)intern("");  // Empty string
}

// Define static builtin name variables
namespace S {
  // Define all builtin names as static variables with S_ prefix
  #define DEFINE_BUILTIN_NAME(name) const InternedString S_##name{};
  #define DEFINE_BUILTIN_NAME_STR(name, str) const InternedString S_##name{};
  CXY_BUILTIN_NAMES(DEFINE_BUILTIN_NAME, DEFINE_BUILTIN_NAME_STR)
  #undef DEFINE_BUILTIN_NAME
  #undef DEFINE_BUILTIN_NAME_STR

  void initializeBuiltinNames(StringInterner& interner) {
    // Initialize all builtin names with interned values
    #define INIT_BUILTIN_NAME(name) const_cast<InternedString&>(S_##name) = interner.intern(#name);
    #define INIT_BUILTIN_NAME_STR(name, str) const_cast<InternedString&>(S_##name) = interner.intern(str);
    CXY_BUILTIN_NAMES(INIT_BUILTIN_NAME, INIT_BUILTIN_NAME_STR)
    #undef INIT_BUILTIN_NAME
    #undef INIT_BUILTIN_NAME_STR
  }
}

} // namespace cxy
