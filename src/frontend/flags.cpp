#include "cxy/flags.hpp"
#include <sstream>

namespace cxy {

std::string flagsToString(Flags flags) {
  if (flags == flgNone) {
    return std::string(flagToString(flgNone));
  }

  std::ostringstream oss;
  bool first = true;

  // Check each individual flag
#define CXY_FLAG_CHECK(name, bit)                                              \
  if (hasAnyFlag(flags, flg##name)) {                                          \
    if (!first)                                                                \
      oss << "|";                                                              \
    oss << flagToString(flg##name);                                            \
    first = false;                                                             \
  }

  CXY_FLAGS(CXY_FLAG_CHECK)
#undef CXY_FLAG_CHECK

  return oss.str();
}

} // namespace cxy
