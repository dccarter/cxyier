#pragma once

#include "catch2.hpp"
#include "cxy/ast/node.hpp"
#include "cxy/ast/printer.hpp"
#include <cctype>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace cxy::ast::testing {

/**
 * @brief Represents a parsed S-expression for structural comparison.
 */
struct SExpr {
  std::string atom;
  std::vector<SExpr> children;

  bool isAtom() const { return children.empty(); }

  // Equality comparison
  bool operator==(const SExpr &other) const {
    if (isAtom() != other.isAtom())
      return false;
    if (isAtom())
      return atom == other.atom;

    if (children.size() != other.children.size())
      return false;
    for (size_t i = 0; i < children.size(); ++i) {
      if (children[i] != other.children[i])
        return false;
    }
    return true;
  }

  bool operator!=(const SExpr &other) const { return !(*this == other); }
};

/**
 * @brief Normalize S-expression string for whitespace-insensitive comparison.
 *
 * Removes unnecessary whitespace while preserving structure and string literal
 * contents.
 *
 * @param sexpr Input S-expression string
 * @return Normalized string suitable for comparison
 */
inline std::string normalizeSerial(const std::string &sexpr) {
  std::string result;
  result.reserve(sexpr.size());

  bool inString = false;
  bool inEscape = false;
  bool prevWasSpace = false;

  for (size_t i = 0; i < sexpr.size(); ++i) {
    char c = sexpr[i];

    if (inEscape) {
      result += c;
      inEscape = false;
      prevWasSpace = false;
      continue;
    }

    if (c == '\\' && inString) {
      result += c;
      inEscape = true;
      prevWasSpace = false;
      continue;
    }

    if (c == '"') {
      inString = !inString;
      result += c;
      prevWasSpace = false;
      continue;
    }

    if (inString) {
      result += c;
      prevWasSpace = false;
      continue;
    }

    // Not in string - handle whitespace normalization
    if (std::isspace(c)) {
      if (!prevWasSpace && !result.empty() && result.back() != '(') {
        result += ' ';
      }
      prevWasSpace = true;
    } else {
      if (c == ')') {
        // Remove space before closing parens only
        if (!result.empty() && result.back() == ' ') {
          result.pop_back();
        }
      }
      result += c;
      prevWasSpace = false;
    }
  }

  // Remove trailing whitespace
  while (!result.empty() && std::isspace(result.back())) {
    result.pop_back();
  }

  return result;
}

/**
 * @brief Parse S-expression string into structured representation.
 *
 * @param sexpr Input S-expression string
 * @return Parsed SExpr structure
 * @throws std::runtime_error on parse error
 */
inline SExpr parseSerial(const std::string &sexpr) {
  std::string normalized = normalizeSerial(sexpr);
  size_t pos = 0;

  auto skipWhitespace = [&]() {
    while (pos < normalized.size() && std::isspace(normalized[pos])) {
      pos++;
    }
  };

  std::function<SExpr()> parseExpr = [&]() -> SExpr {
    skipWhitespace();

    if (pos >= normalized.size()) {
      throw std::runtime_error("Unexpected end of input");
    }

    if (normalized[pos] == '(') {
      // Parse list
      pos++; // Skip '('
      SExpr expr;

      while (true) {
        skipWhitespace();
        if (pos >= normalized.size()) {
          throw std::runtime_error("Missing closing parenthesis");
        }

        if (normalized[pos] == ')') {
          pos++; // Skip ')'
          break;
        }

        expr.children.push_back(parseExpr());
      }

      return expr;
    } else {
      // Parse atom
      SExpr expr;
      std::string atom;

      if (normalized[pos] == '"') {
        // Parse string literal
        atom += normalized[pos++]; // Include opening quote

        while (pos < normalized.size() && normalized[pos] != '"') {
          if (normalized[pos] == '\\' && pos + 1 < normalized.size()) {
            atom += normalized[pos++]; // Backslash
            atom += normalized[pos++]; // Escaped char
          } else {
            atom += normalized[pos++];
          }
        }

        if (pos >= normalized.size()) {
          throw std::runtime_error("Unterminated string literal");
        }

        atom += normalized[pos++]; // Include closing quote
      } else {
        // Parse regular atom
        while (pos < normalized.size() && !std::isspace(normalized[pos]) &&
               normalized[pos] != '(' && normalized[pos] != ')') {
          atom += normalized[pos++];
        }
      }

      expr.atom = atom;
      return expr;
    }
  };

  return parseExpr();
}

/**
 * @brief Check if two SExpr structures are equal.
 */
inline bool structurallyEqual(const SExpr &a, const SExpr &b) { return a == b; }

/**
 * @brief Test utility class for AST comparison and debugging.
 */
class ASTTestUtils {
public:
  /**
   * @brief Fast whitespace-insensitive comparison of AST and expected
   * S-expression.
   */
  static bool matches(const ASTNode *ast, const std::string &expected,
                      const PrinterConfig &config = {PrinterFlags::None}) {
    if (!ast)
      return false;

    ASTPrinter printer(config);
    std::string actual = printer.print(ast);

    return normalizeSerial(actual) == normalizeSerial(expected);
  }

  /**
   * @brief Robust structural comparison using S-expression parsing.
   */
  static bool
  structurallyMatches(const ASTNode *ast, const std::string &expected,
                      const PrinterConfig &config = {PrinterFlags::None}) {
    if (!ast)
      return false;

    try {
      ASTPrinter printer(config);
      std::string actual = printer.print(ast);

      SExpr actualExpr = parseSerial(actual);
      SExpr expectedExpr = parseSerial(expected);

      return structurallyEqual(actualExpr, expectedExpr);
    } catch (const std::exception &) {
      // Fall back to string comparison if parsing fails
      return matches(ast, expected, config);
    }
  }

  /**
   * @brief Generate diff information for debugging test failures.
   */
  static std::string diff(const ASTNode *ast, const std::string &expected,
                          const PrinterConfig &config = {PrinterFlags::None}) {
    if (!ast) {
      return "AST is null, expected: " + expected;
    }

    ASTPrinter printer(config);
    std::string actual = printer.print(ast);

    std::ostringstream diff;
    diff << "Expected: " << expected << "\n";
    diff << "Actual:   " << actual << "\n";

    std::string normActual = normalizeSerial(actual);
    std::string normExpected = normalizeSerial(expected);

    if (normActual != normExpected) {
      diff << "Normalized Expected: " << normExpected << "\n";
      diff << "Normalized Actual:   " << normActual << "\n";

      // Find first difference
      size_t minLen = std::min(normActual.size(), normExpected.size());
      for (size_t i = 0; i < minLen; ++i) {
        if (normActual[i] != normExpected[i]) {
          diff << "First difference at position " << i << ": ";
          diff << "expected '" << normExpected[i] << "', got '" << normActual[i]
               << "'\n";
          break;
        }
      }

      if (normActual.size() != normExpected.size()) {
        diff << "Length difference: expected " << normExpected.size()
             << ", got " << normActual.size() << "\n";
      }
    } else {
      diff << "Normalized strings match - possible configuration difference\n";
    }

    return diff.str();
  }

  /**
   * @brief Quick debug print of AST without extra formatting.
   */
  static std::string debug(const ASTNode *ast, PrinterFlags flags = PrinterFlags::None) {
    if (!ast)
      return "(null)";

    ASTPrinter printer({flags});
    return printer.print(ast);
  }

  /**
   * @brief Pretty print AST with default formatting.
   */
  static std::string pretty(const ASTNode *ast) {
    if (!ast)
      return "(null)";

    PrinterConfig config;
    config.flags = PrinterFlags::IncludeLocation;
    ASTPrinter printer(config);
    return printer.print(ast);
  }
};

} // namespace cxy::ast::testing

// Convenient test macros
#define REQUIRE_AST_MATCHES_FLAGS(ast_node, expected_str, flags)                  \
do {                                                                              \
    auto actual_result = cxy::ast::testing::ASTTestUtils::debug(ast_node, flags); \
    auto normalized_actual =                                                   \
        cxy::ast::testing::normalizeSerial(actual_result);                     \
    auto normalized_expected =                                                 \
        cxy::ast::testing::normalizeSerial(expected_str);                      \
    REQUIRE(normalized_actual == normalized_expected);                         \
  } while (0)

#define CHECK_AST_MATCHES_FLAGS(ast_node, expected_str, flags)                 \
  do {                                                                         \
    CHECK(cxy::ast::testing::ASTTestUtils::matches(                            \
        ast_node, expected_str, {flags}));                                     \
  } while (0)

#define REQUIRE_AST_STRUCTURALLY_MATCHES_FLAGS(ast_node, expected_str, flags)  \
  do {                                                                         \
    REQUIRE(cxy::ast::testing::ASTTestUtils::structurallyMatches(              \
        ast_node, expected_str, {flags}));                                     \
  } while (0)

#define CHECK_AST_STRUCTURALLY_MATCHES_FLAGS(ast_node, expected_str, flags)    \
  do {                                                                         \
    CHECK(cxy::ast::testing::ASTTestUtils::structurallyMatches(                \
        ast_node, expected_str, {flags}));                                     \
  } while (0)

#define REQUIRE_AST_MATCHES(ast_node, expected_str)                            \
  REQUIRE_AST_MATCHES_FLAGS(ast_node, expected_str, cxy::ast::PrinterFlags::None)

#define CHECK_AST_MATCHES(ast_node, expected_str)                              \
  CHECK_AST_MATCHES_FLAGS(ast_node, expected_str, cxy::ast::PrinterFlags::None)

#define REQUIRE_AST_STRUCTURALLY_MATCHES(ast_node, expected_str)               \
  REQUIRE_AST_STRUCTURALLY_MATCHES_FLAGS(ast_node, expected_str, cxy::ast::PrinterFlags::None)

#define CHECK_AST_STRUCTURALLY_MATCHES(ast_node, expected_str)                 \
  CHECK_AST_STRUCTURALLY_MATCHES_FLAGS(ast_node, expected_str, cxy::ast::PrinterFlags::None)
