#include "../../parser_test_utils.hpp"

#include "catch2.hpp"

using namespace cxy;
using namespace cxy::test;

TEST_CASE("Parser: Basic character literals", "[parser][literals][char]") {
  SECTION("Single ASCII character") {
    auto fixture = createParserFixture("'a'");
    auto *node = fixture->parseLiteralExpression();
    expectCharLiteral(node, 'a');
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Uppercase character") {
    auto fixture = createParserFixture("'Z'");
    auto *node = fixture->parseLiteralExpression();
    expectCharLiteral(node, 'Z');
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Digit character") {
    auto fixture = createParserFixture("'7'");
    auto *node = fixture->parseLiteralExpression();
    expectCharLiteral(node, '7');
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Special character") {
    auto fixture = createParserFixture("'@'");
    auto *node = fixture->parseLiteralExpression();
    expectCharLiteral(node, '@');
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Space character") {
    auto fixture = createParserFixture("' '");
    auto *node = fixture->parseLiteralExpression();
    expectCharLiteral(node, ' ');
    REQUIRE(fixture->isAtEnd());
  }
}

TEST_CASE("Parser: Character literals with escape sequences",
          "[parser][literals][char]") {
  SECTION("Newline escape") {
    auto fixture = createParserFixture("'\\n'");
    auto *node = fixture->parseLiteralExpression();
    expectCharLiteral(node, '\n');
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Tab escape") {
    auto fixture = createParserFixture("'\\t'");
    auto *node = fixture->parseLiteralExpression();
    expectCharLiteral(node, '\t');
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Carriage return escape") {
    auto fixture = createParserFixture("'\\r'");
    auto *node = fixture->parseLiteralExpression();
    expectCharLiteral(node, '\r');
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Backslash escape") {
    auto fixture = createParserFixture("'\\\\'");
    auto *node = fixture->parseLiteralExpression();
    expectCharLiteral(node, '\\');
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Single quote escape") {
    auto fixture = createParserFixture("'\\''");
    auto *node = fixture->parseLiteralExpression();
    expectCharLiteral(node, '\'');
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Double quote escape") {
    auto fixture = createParserFixture("'\\\"'");
    auto *node = fixture->parseLiteralExpression();
    expectCharLiteral(node, '\"');
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Null character escape") {
    auto fixture = createParserFixture("'\\0'");
    auto *node = fixture->parseLiteralExpression();
    expectCharLiteral(node, '\0');
    REQUIRE(fixture->isAtEnd());
  }
}

TEST_CASE("Parser: Character literals with hex escapes",
          "[parser][literals][char]") {
  SECTION("Basic hex escape") {
    auto fixture = createParserFixture("'\\x41'");
    auto *node = fixture->parseLiteralExpression();
    expectCharLiteral(node, 0x41); // 'A'
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Lowercase hex escape") {
    auto fixture = createParserFixture("'\\x61'");
    auto *node = fixture->parseLiteralExpression();
    expectCharLiteral(node, 0x61); // 'a'
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Uppercase hex escape") {
    auto fixture = createParserFixture("'\\xFF'");
    auto *node = fixture->parseLiteralExpression();
    expectCharLiteral(node, 0xFF);
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Mixed case hex escape") {
    auto fixture = createParserFixture("'\\xaB'");
    auto *node = fixture->parseLiteralExpression();
    expectCharLiteral(node, 0xAB);
    REQUIRE(fixture->isAtEnd());
  }
}

TEST_CASE("Parser: Character literals with Unicode escapes",
          "[parser][literals][char]") {
  SECTION("Basic Unicode escape") {
    auto fixture = createParserFixture("'\\u{41}'");
    auto *node = fixture->parseLiteralExpression();
    expectCharLiteral(node, 0x41); // 'A'
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Unicode escape with multiple digits") {
    auto fixture = createParserFixture("'\\u{1F600}'");
    auto *node = fixture->parseLiteralExpression();
    expectCharLiteral(node, 0x1F600); // 😀 emoji
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Unicode escape with lowercase") {
    auto fixture = createParserFixture("'\\u{2764}'");
    auto *node = fixture->parseLiteralExpression();
    expectCharLiteral(node, 0x2764); // ❤ heart
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Unicode escape with uppercase") {
    auto fixture = createParserFixture("'\\u{2764}'");
    auto *node = fixture->parseLiteralExpression();
    expectCharLiteral(node, 0x2764);
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Small Unicode escape") {
    auto fixture = createParserFixture("'\\u{20}'");
    auto *node = fixture->parseLiteralExpression();
    expectCharLiteral(node, 0x20); // space
    REQUIRE(fixture->isAtEnd());
  }
}

TEST_CASE("Parser: Character literals in expressions",
          "[parser][literals][char]") {
  SECTION("Character in primary expression") {
    auto fixture = createParserFixture("'x'");
    auto *node = fixture->parsePrimaryExpression();
    expectCharLiteral(node, 'x');
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Character in expression") {
    auto fixture = createParserFixture("'y'");
    auto *node = fixture->parseExpression();
    expectCharLiteral(node, 'y');
    REQUIRE(fixture->isAtEnd());
  }
}

TEST_CASE("Parser: Multiple character literals", "[parser][literals][char]") {
  auto fixture = createParserFixture("'a' 'b' 'c'");

  // Parse first character
  auto *node1 = fixture->parseLiteralExpression();
  expectCharLiteral(node1, 'a');

  // Parse second character (parser advanced automatically)
  auto *node2 = fixture->parseLiteralExpression();
  expectCharLiteral(node2, 'b');

  // Parse third character (parser advanced automatically)
  auto *node3 = fixture->parseLiteralExpression();
  expectCharLiteral(node3, 'c');

  // Should be at end
  REQUIRE(fixture->isAtEnd());
}

TEST_CASE("Parser: Character literal token buffer behavior",
          "[parser][literals][char]") {
  auto fixture = createParserFixture("'x' 42");

  // Initially at character
  REQUIRE(fixture->current().kind == TokenKind::CharLiteral);
  REQUIRE(fixture->lookahead(1).kind == TokenKind::IntLiteral);

  // Parse character literal
  auto *node = fixture->parseLiteralExpression();
  expectCharLiteral(node, 'x');

  // Should have advanced to next token after parsing
  REQUIRE(fixture->current().kind == TokenKind::IntLiteral);
}

TEST_CASE("Parser: Character literal location information",
          "[parser][literals][char]") {
  auto fixture = createParserFixture("'z'");

  // Store the token location before parsing (parser will advance)
  Location expectedLocation = fixture->current().location;
  auto *node = fixture->parseLiteralExpression();

  REQUIRE(node != nullptr);
  REQUIRE(node->kind == ast::astChar);

  // Location should be set
  REQUIRE(node->location.isValid());

  // Location should correspond to the original token
  REQUIRE(node->location == expectedLocation);
}

TEST_CASE("Parser: Character literal error cases", "[parser][literals][char]") {
  SECTION("Wrong token type") {
    auto fixture = createParserFixture("42");
    auto *node = fixture->parseLiteralExpression();
    // Should parse as integer, not character
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

  SECTION("String instead of character") {
    auto fixture = createParserFixture("\"a\"");
    auto *node = fixture->parseLiteralExpression();
    // Should parse as string, not character
    REQUIRE(node != nullptr);
    REQUIRE(node->kind == ast::astString);
  }
}

TEST_CASE("Parser: Special character values", "[parser][literals][char]") {
  SECTION("Control characters") {
    auto fixture = createParserFixture("'\\x01'");
    auto *node = fixture->parseLiteralExpression();
    expectCharLiteral(node, 0x01);
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Extended ASCII") {
    auto fixture = createParserFixture("'\\x80'");
    auto *node = fixture->parseLiteralExpression();
    expectCharLiteral(node, 0x80);
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("High Unicode values") {
    auto fixture = createParserFixture("'\\u{10000}'");
    auto *node = fixture->parseLiteralExpression();
    expectCharLiteral(node, 0x10000);
    REQUIRE(fixture->isAtEnd());
  }
}

TEST_CASE("Parser: Character vs string distinction",
          "[parser][literals][char]") {
  SECTION("Single character") {
    auto fixture = createParserFixture("'a'");
    auto *node = fixture->parseLiteralExpression();
    REQUIRE(node->kind == ast::astChar);
    expectCharLiteral(node, 'a');
  }

  SECTION("Single character string") {
    auto fixture = createParserFixture("\"a\"");
    auto *node = fixture->parseLiteralExpression();
    REQUIRE(node->kind == ast::astString);
    expectStringLiteral(node, "a");
  }

  SECTION("Empty string") {
    auto fixture = createParserFixture("\"\"");
    auto *node = fixture->parseLiteralExpression();
    REQUIRE(node->kind == ast::astString);
    expectStringLiteral(node, "");
  }
}

TEST_CASE("Parser: Character literals with punctuation",
          "[parser][literals][char]") {
  SECTION("Punctuation marks") {
    std::vector<std::pair<std::string, char>> testCases = {
        {"'!'", '!'}, {"'?'", '?'}, {"'.'", '.'}, {"','", ','}, {"';'", ';'},
        {"':'", ':'}, {"'('", '('}, {"')'", ')'}, {"'['", '['}, {"']'", ']'},
        {"'{'", '{'}, {"'}'", '}'}, {"'<'", '<'}, {"'>'", '>'}, {"'='", '='},
        {"'+'", '+'}, {"'-'", '-'}, {"'*'", '*'}, {"'/'", '/'}, {"'%'", '%'},
        {"'&'", '&'}, {"'|'", '|'}, {"'^'", '^'}, {"'~'", '~'}, {"'#'", '#'},
        {"'$'", '$'}};

    for (const auto &testCase : testCases) {
      auto fixture = createParserFixture(testCase.first);
      auto *node = fixture->parseLiteralExpression();
      expectCharLiteral(node, testCase.second);
      REQUIRE(fixture->isAtEnd());
    }
  }
}

TEST_CASE("Parser: Character literals mixed with other literals",
          "[parser][literals][char]") {
  auto fixture = createParserFixture("'a' 42 \"hello\" true 'z'");

  // Test mixed sequence
  auto *node1 = fixture->parseLiteralExpression();
  expectCharLiteral(node1, 'a');

  auto *node2 = fixture->parseLiteralExpression();
  expectIntegerLiteral(node2, 42);

  auto *node3 = fixture->parseLiteralExpression();
  expectStringLiteral(node3, "hello");

  auto *node4 = fixture->parseLiteralExpression();
  expectBoolLiteral(node4, true);

  auto *node5 = fixture->parseLiteralExpression();
  expectCharLiteral(node5, 'z');

  REQUIRE(fixture->isAtEnd());
}

// Macro-based tests for consistency
LITERAL_TEST_CASE("Simple character", "'a'", expectCharLiteral(node, 'a'))
LITERAL_TEST_CASE("Escape character", "'\\n'", expectCharLiteral(node, '\n'))
LITERAL_TEST_CASE("Hex character", "'\\x41'", expectCharLiteral(node, 0x41))
LITERAL_TEST_CASE("Unicode character", "'\\u{41}'",
                  expectCharLiteral(node, 0x41))
LITERAL_TEST_CASE("Special character", "'@'", expectCharLiteral(node, '@'))
LITERAL_TEST_CASE("Quote character", "'\\''", expectCharLiteral(node, '\''))
LITERAL_TEST_CASE("Backslash character", "'\\\\'",
                  expectCharLiteral(node, '\\'))
LITERAL_TEST_CASE("Space character", "' '", expectCharLiteral(node, ' '))
