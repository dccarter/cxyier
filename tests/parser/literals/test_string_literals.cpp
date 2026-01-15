#include "../../parser_test_utils.hpp"

#include "catch2.hpp"

using namespace cxy;
using namespace cxy::test;

TEST_CASE("Parser: Basic string literals", "[parser][literals][string]") {
  SECTION("Simple string") {
    auto fixture = createParserFixture("\"hello\"");
    auto *node = fixture->parseLiteralExpression();
    expectStringLiteral(node, "hello");
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Empty string") {
    auto fixture = createParserFixture("\"\"");
    auto *node = fixture->parseLiteralExpression();
    expectStringLiteral(node, "");
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Single character string") {
    auto fixture = createParserFixture("\"a\"");
    auto *node = fixture->parseLiteralExpression();
    expectStringLiteral(node, "a");
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("String with spaces") {
    auto fixture = createParserFixture("\"hello world\"");
    auto *node = fixture->parseLiteralExpression();
    expectStringLiteral(node, "hello world");
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("String with numbers") {
    auto fixture = createParserFixture("\"test123\"");
    auto *node = fixture->parseLiteralExpression();
    expectStringLiteral(node, "test123");
    REQUIRE(fixture->isAtEnd());
  }
}

TEST_CASE("Parser: String literals with escape sequences",
          "[parser][literals][string]") {
  SECTION("Basic escape sequences") {
    auto fixture = createParserFixture("\"hello\\nworld\"");
    auto *node = fixture->parseLiteralExpression();
    expectStringLiteral(node, "hello\nworld");
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Tab escape") {
    auto fixture = createParserFixture("\"hello\\tworld\"");
    auto *node = fixture->parseLiteralExpression();
    expectStringLiteral(node, "hello\tworld");
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Carriage return escape") {
    auto fixture = createParserFixture("\"hello\\rworld\"");
    auto *node = fixture->parseLiteralExpression();
    expectStringLiteral(node, "hello\rworld");
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Backslash escape") {
    auto fixture = createParserFixture("\"path\\\\to\\\\file\"");
    auto *node = fixture->parseLiteralExpression();
    expectStringLiteral(node, "path\\to\\file");
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Quote escape") {
    auto fixture = createParserFixture("\"She said \\\"hello\\\"\"");
    auto *node = fixture->parseLiteralExpression();
    expectStringLiteral(node, "She said \"hello\"");
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Multiple escape sequences") {
    auto fixture = createParserFixture("\"line1\\nline2\\tcolumn\"");
    auto *node = fixture->parseLiteralExpression();
    expectStringLiteral(node, "line1\nline2\tcolumn");
    REQUIRE(fixture->isAtEnd());
  }
}

TEST_CASE("Parser: String literals with Unicode escapes",
          "[parser][literals][string]") {
  SECTION("Basic Unicode escape") {
    auto fixture = createParserFixture("\"\\u{41}\"");
    auto *node = fixture->parseLiteralExpression();
    expectStringLiteral(node, "A");
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Unicode escape with hex digits") {
    auto fixture = createParserFixture("\"\\u{2764}\"");
    auto *node = fixture->parseLiteralExpression();
    expectStringLiteral(node, "❤");
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Multiple Unicode escapes") {
    auto fixture =
        createParserFixture("\"\\u{48}\\u{65}\\u{6C}\\u{6C}\\u{6F}\"");
    auto *node = fixture->parseLiteralExpression();
    expectStringLiteral(node, "Hello");
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Mixed Unicode and regular escapes") {
    auto fixture = createParserFixture("\"Hello\\u{2C}\\u{20}\\u{57}orld\\n\"");
    auto *node = fixture->parseLiteralExpression();
    expectStringLiteral(node, "Hello, World\n");
    REQUIRE(fixture->isAtEnd());
  }
}

TEST_CASE("Parser: String literals with special characters",
          "[parser][literals][string]") {
  SECTION("Punctuation") {
    auto fixture = createParserFixture("\"!@#$%^&*()\"");
    auto *node = fixture->parseLiteralExpression();
    expectStringLiteral(node, "!@#$%^&*()");
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Brackets and braces") {
    auto fixture = createParserFixture("\"[]\\{\\}()<>\"");
    auto *node = fixture->parseLiteralExpression();
    expectStringLiteral(node, "[]{}()<>");
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Mathematical symbols") {
    auto fixture = createParserFixture("\"+=-*/\"");
    auto *node = fixture->parseLiteralExpression();
    expectStringLiteral(node, "+=-*/");
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Mixed content") {
    auto fixture = createParserFixture("\"Hello, World! 123 @#$\"");
    auto *node = fixture->parseLiteralExpression();
    expectStringLiteral(node, "Hello, World! 123 @#$");
    REQUIRE(fixture->isAtEnd());
  }
}

TEST_CASE("Parser: String literals in expressions",
          "[parser][literals][string]") {
  SECTION("String in primary expression") {
    auto fixture = createParserFixture("\"test\"");
    auto *node = fixture->parsePrimaryExpression();
    expectStringLiteral(node, "test");
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("String in expression") {
    auto fixture = createParserFixture("\"expression\"");
    auto *node = fixture->parseExpression();
    expectStringLiteral(node, "expression");
    REQUIRE(fixture->isAtEnd());
  }
}

TEST_CASE("Parser: Multiple string literals", "[parser][literals][string]") {
  auto fixture = createParserFixture("\"first\" \"second\" \"third\"");

  // Parse first string
  auto *node1 = fixture->parseLiteralExpression();
  expectStringLiteral(node1, "first");

  // Parse second string (parser advanced automatically)
  auto *node2 = fixture->parseLiteralExpression();
  expectStringLiteral(node2, "second");

  // Parse third string (parser advanced automatically)
  auto *node3 = fixture->parseLiteralExpression();
  expectStringLiteral(node3, "third");

  // Should be at end
  REQUIRE(fixture->isAtEnd());
}

TEST_CASE("Parser: Long string literals", "[parser][literals][string]") {
  SECTION("Long string") {
    std::string longString = "This is a very long string that contains many "
                             "words and should test the parser's ability to "
                             "handle longer text content without any issues.";
    auto fixture = createParserFixture("\"" + longString + "\"");
    auto *node = fixture->parseLiteralExpression();
    expectStringLiteral(node, longString);
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("String with repeated content") {
    auto fixture = createParserFixture("\"abcabcabcabcabc\"");
    auto *node = fixture->parseLiteralExpression();
    expectStringLiteral(node, "abcabcabcabcabc");
    REQUIRE(fixture->isAtEnd());
  }
}

TEST_CASE("Parser: String literal error cases", "[parser][literals][string]") {
  SECTION("Wrong token type") {
    auto fixture = createParserFixture("42");
    auto *node = fixture->parseLiteralExpression();
    // Should parse as integer, not string
    REQUIRE(node != nullptr);
    REQUIRE(node->kind == ast::astInt);
  }

  SECTION("Empty input") {
    auto fixture = createParserFixture("");
    auto *node = fixture->parseLiteralExpression();
    expectParseFailure(node);
  }

  SECTION("Non-literal token") {
    auto fixture = createParserFixture("identifier");
    auto *node = fixture->parseLiteralExpression();
    expectParseFailure(node);
  }

  SECTION("Boolean instead of string") {
    auto fixture = createParserFixture("true");
    auto *node = fixture->parseLiteralExpression();
    // Should parse as boolean, not string
    REQUIRE(node != nullptr);
    REQUIRE(node->kind == ast::astBool);
  }
}

TEST_CASE("Parser: String literal token buffer behavior",
          "[parser][literals][string]") {
  auto fixture = createParserFixture("\"hello\" 42");

  // Initially at string
  REQUIRE(fixture->current().kind == TokenKind::StringLiteral);
  REQUIRE(fixture->lookahead(1).kind == TokenKind::IntLiteral);

  // Parse string literal
  auto *node = fixture->parseLiteralExpression();
  expectStringLiteral(node, "hello");

  // Should have advanced to next token after parsing
  REQUIRE(fixture->current().kind == TokenKind::IntLiteral);
}

TEST_CASE("Parser: String literal location information",
          "[parser][literals][string]") {
  auto fixture = createParserFixture("\"test string\"");

  // Store the token location before parsing (parser will advance)
  Location expectedLocation = fixture->current().location;
  auto *node = fixture->parseLiteralExpression();

  REQUIRE(node != nullptr);
  REQUIRE(node->kind == ast::astString);

  // Location should be set
  REQUIRE(node->location.isValid());

  // Location should correspond to the original token
  REQUIRE(node->location == expectedLocation);
}

TEST_CASE("Parser: Raw string literals", "[parser][literals][string]") {
  SECTION("Basic raw string") {
    auto fixture = createParserFixture("r\"hello world\"");
    auto *node = fixture->parseLiteralExpression();
    expectStringLiteral(node, "hello world");
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Raw string with backslashes") {
    auto fixture = createParserFixture("r\"C:\\path\\to\\file\"");
    auto *node = fixture->parseLiteralExpression();
    expectStringLiteral(node, "C:\\path\\to\\file");
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Raw string with quotes") {
    auto fixture = createParserFixture("r\"She said hello to me\"");
    auto *node = fixture->parseLiteralExpression();
    expectStringLiteral(node, "She said hello to me");
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Raw string with newlines") {
    auto fixture = createParserFixture("r\"line1\nline2\nline3\"");
    auto *node = fixture->parseLiteralExpression();
    expectStringLiteral(node, "line1\nline2\nline3");
    REQUIRE(fixture->isAtEnd());
  }
}

TEST_CASE("Parser: String literal interning", "[parser][literals][string]") {
  auto fixture = createParserFixture("\"test\"");
  auto *node1 = fixture->parseLiteralExpression();

  REQUIRE(node1 != nullptr);
  REQUIRE(node1->kind == ast::astString);

  // Parse the same string again in a new fixture
  auto fixture2 = createParserFixture("\"test\"");
  auto *node2 = fixture2->parseLiteralExpression();

  REQUIRE(node2 != nullptr);
  REQUIRE(node2->kind == ast::astString);

  // Interned strings should be the same object
  auto *stringNode1 = static_cast<ast::StringLiteralNode *>(node1);
  auto *stringNode2 = static_cast<ast::StringLiteralNode *>(node2);

  // The InternedString should have the same underlying data
  REQUIRE(stringNode1->value.view() == stringNode2->value.view());
}

// Macro-based tests for consistency
LITERAL_TEST_CASE("Simple string", "\"hello\"",
                  expectStringLiteral(node, "hello"))
LITERAL_TEST_CASE("Empty string", "\"\"", expectStringLiteral(node, ""))
LITERAL_TEST_CASE("String with escapes", "\"hello\\nworld\"",
                  expectStringLiteral(node, "hello\nworld"))
LITERAL_TEST_CASE("String with Unicode", "\"\\u{48}ello\"",
                  expectStringLiteral(node, "Hello"))
LITERAL_TEST_CASE("Raw string", "r\"C:\\path\"",
                  expectStringLiteral(node, "C:\\path"))
LITERAL_TEST_CASE("String with spaces", "\"hello world\"",
                  expectStringLiteral(node, "hello world"))
LITERAL_TEST_CASE("String with quotes", "\"She said \\\"hi\\\"\"",
                  expectStringLiteral(node, "She said \"hi\""))
