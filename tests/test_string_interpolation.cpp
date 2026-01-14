#include "catch2.hpp"
#include "cxy/diagnostics.hpp"
#include "cxy/lexer.hpp"
#include "cxy/token.hpp"
#include "lexer_test_helper.hpp"

#include <iostream>
#include <memory>

using namespace cxy;

// Phase 6: String Interpolation Tests

TEST_CASE("Lexer handles basic string interpolation", "[lexer][phase6]") {
  LexerTestHelper helper;

  // Test simple interpolation: "Hello {name}!"
  auto tokens = helper.tokenize(R"("Hello {name}!")");

  // Expected tokens: LString + "Hello " + LBrace + name + RBrace + "!" +
  // RString + EOF
  std::vector<TokenKind> expected = {TokenKind::LString, // "Hello "
                                     TokenKind::Ident,   // name
                                     TokenKind::RString, // "!"
                                     TokenKind::EoF};

  REQUIRE(tokens.size() == expected.size());
  for (size_t i = 0; i < expected.size(); ++i) {
    CHECK(tokens[i].kind == expected[i]);
  }
  CHECK_FALSE(helper.hasErrors());
}

TEST_CASE("Lexer handles complex interpolation expressions",
          "[lexer][phase6]") {
  LexerTestHelper helper;

  // Test expression in interpolation: "Result: {calculate(x + y)}"
  auto tokens = helper.tokenize(R"("Result: {calculate(x + y)}")");

  // Should tokenize interpolated expression correctly
  bool foundLString = false, foundRString = false;
  bool foundIdent = false, foundLParen = false, foundPlus = false;

  for (const auto &token : tokens) {
    switch (token.kind) {
    case TokenKind::LString:
      foundLString = true;
      break;
    case TokenKind::RString:
      foundRString = true;
      break;
    case TokenKind::Ident:
      foundIdent = true;
      break;
    case TokenKind::LParen:
      foundLParen = true;
      break;
    case TokenKind::Plus:
      foundPlus = true;
      break;
    default:
      break;
    }
  }

  CHECK(foundLString);
  CHECK(foundRString);
  CHECK(foundIdent);
  CHECK(foundLParen);
  CHECK(foundPlus);
  CHECK_FALSE(helper.hasErrors());
}

TEST_CASE("Lexer handles nested interpolation", "[lexer][phase6]") {
  LexerTestHelper helper;

  // Test nested interpolation: "Outer {format("Inner {x}")}"
  auto tokens = helper.tokenize(R"("Outer {format("Inner {x}")}")");

  // Should handle nested interpolation correctly
  bool foundMultipleLString = false, foundMultipleRString = false;
  int lStringCount = 0, rStringCount = 0;

  for (const auto &token : tokens) {
    if (token.kind == TokenKind::LString) {
      lStringCount++;
    } else if (token.kind == TokenKind::RString) {
      rStringCount++;
    }
  }

  foundMultipleLString = (lStringCount > 1);
  foundMultipleRString = (rStringCount > 1);

  CHECK(foundMultipleLString);
  CHECK(foundMultipleRString);
  CHECK_FALSE(helper.hasErrors());
}

TEST_CASE("Lexer handles multiline interpolated strings", "[lexer][phase6]") {
  LexerTestHelper helper;

  // Test multiline interpolation with actual newlines
  std::string multiline = "\"Hello {name}\nWelcome to {place}!\"";
  auto tokens = helper.tokenize(multiline);

  // Should handle multiline interpolated strings
  bool foundLString = false, foundRString = false;
  bool foundIdent = false;

  for (const auto &token : tokens) {
    switch (token.kind) {
    case TokenKind::LString:
      foundLString = true;
      break;
    case TokenKind::RString:
      foundRString = true;
      break;
    case TokenKind::Ident:
      foundIdent = true;
      break;
    default:
      break;
    }
  }

  CHECK(foundLString);
  CHECK(foundRString);
  CHECK(foundIdent);
  CHECK_FALSE(helper.hasErrors());
}

TEST_CASE("Lexer handles interpolation with escape sequences",
          "[lexer][phase6]") {
  LexerTestHelper helper;

  // Test interpolation with escape sequences: "Hello {name}\n\t{greeting}!"
  auto tokens = helper.tokenize(R"("Hello {name}\\n\\t{greeting}!")");

  // Should handle escape sequences within interpolated strings
  bool foundLString = false, foundRString = false;
  bool foundIdent = false;

  for (const auto &token : tokens) {
    switch (token.kind) {
    case TokenKind::LString:
      foundLString = true;
      break;
    case TokenKind::RString:
      foundRString = true;
      break;
    case TokenKind::Ident:
      foundIdent = true;
      break;
    default:
      break;
    }
  }

  CHECK(foundLString);
  CHECK(foundRString);
  CHECK(foundIdent);
  CHECK_FALSE(helper.hasErrors());
}

TEST_CASE("Lexer handles malformed interpolation errors", "[lexer][phase6]") {
  LexerTestHelper helper;

  // Test unterminated interpolation (missing closing quote)
  auto tokens1 = helper.tokenize(R"("Hello {name")");
  CHECK(helper.hasErrors()); // Should have error for unterminated string

  helper.clearDiagnostics();

  // Test unbalanced braces in interpolation (extra opening brace)
  auto tokens2 = helper.tokenize(R"("Hello {func{x}}")");
  CHECK_FALSE(helper.hasErrors()); // This should actually work - nested braces
                                   // are valid in expressions

  helper.clearDiagnostics();

  // Test unbalanced braces (missing closing brace)
  auto tokens3 = helper.tokenize(R"("Hello {name + other")");
  CHECK(helper.hasErrors()); // Should have error for unbalanced braces (no
                             // closing brace before quote)
}

TEST_CASE("Lexer distinguishes interpolated strings from regular strings",
          "[lexer][phase6]") {
  LexerTestHelper helper;

  // Test regular string (no interpolation)
  auto tokens1 = helper.tokenize(R"("Hello world!")");

  // Should be regular StringLiteral, not interpolated
  REQUIRE(tokens1.size() == 2); // string + EOF
  CHECK(tokens1[0].kind == TokenKind::StringLiteral);
  CHECK(tokens1[1].kind == TokenKind::EoF);
  CHECK_FALSE(helper.hasErrors());

  helper.clearDiagnostics();

  // Test string with braces but no interpolation (escaped braces)
  auto tokens2 = helper.tokenize(R"("Hello \{world\}!")");

  // Should be regular StringLiteral
  REQUIRE(tokens2.size() == 2); // string + EOF
  CHECK(tokens2[0].kind == TokenKind::StringLiteral);
  CHECK(tokens2[1].kind == TokenKind::EoF);
  CHECK_FALSE(helper.hasErrors());
}
