#include "../../parser_test_utils.hpp"

#include "catch2.hpp"

using namespace cxy;
using namespace cxy::test;

TEST_CASE("Parser: Basic integer literals", "[parser][literals][integer]") {
  SECTION("Positive integers") {
    auto fixture = createParserFixture("42");
    auto *node = fixture->parseLiteralExpression();
    expectIntegerLiteral(node, 42);
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Zero") {
    auto fixture = createParserFixture("0");
    auto *node = fixture->parseLiteralExpression();
    expectIntegerLiteral(node, 0);
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Single digit") {
    auto fixture = createParserFixture("7");
    auto *node = fixture->parseLiteralExpression();
    expectIntegerLiteral(node, 7);
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Large integers") {
    auto fixture = createParserFixture("1234567890");
    auto *node = fixture->parseLiteralExpression();
    expectIntegerLiteral(node, 1234567890);
    REQUIRE(fixture->isAtEnd());
  }
}

TEST_CASE("Parser: Hexadecimal integer literals",
          "[parser][literals][integer]") {
  SECTION("Basic hex") {
    auto fixture = createParserFixture("0xFF");
    auto *node = fixture->parseLiteralExpression();
    expectIntegerLiteral(node, 255);
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Lowercase hex") {
    auto fixture = createParserFixture("0xabcd");
    auto *node = fixture->parseLiteralExpression();
    expectIntegerLiteral(node, 0xabcd);
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Uppercase hex") {
    auto fixture = createParserFixture("0XABCD");
    auto *node = fixture->parseLiteralExpression();
    expectIntegerLiteral(node, 0xABCD);
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Mixed case hex") {
    auto fixture = createParserFixture("0xaBcD");
    auto *node = fixture->parseLiteralExpression();
    expectIntegerLiteral(node, 0xaBcD);
    REQUIRE(fixture->isAtEnd());
  }
}

TEST_CASE("Parser: Binary integer literals", "[parser][literals][integer]") {
  SECTION("Basic binary") {
    auto fixture = createParserFixture("0b1010");
    auto *node = fixture->parseLiteralExpression();
    expectIntegerLiteral(node, 10);
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Binary with uppercase B") {
    auto fixture = createParserFixture("0B1111");
    auto *node = fixture->parseLiteralExpression();
    expectIntegerLiteral(node, 15);
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("All zeros binary") {
    auto fixture = createParserFixture("0b0000");
    auto *node = fixture->parseLiteralExpression();
    expectIntegerLiteral(node, 0);
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("All ones binary") {
    auto fixture = createParserFixture("0b11111111");
    auto *node = fixture->parseLiteralExpression();
    expectIntegerLiteral(node, 255);
    REQUIRE(fixture->isAtEnd());
  }
}

TEST_CASE("Parser: Octal integer literals", "[parser][literals][integer]") {
  SECTION("Basic octal") {
    auto fixture = createParserFixture("0o755");
    auto *node = fixture->parseLiteralExpression();
    expectIntegerLiteral(node, 493); // 7*64 + 5*8 + 5
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Octal zero") {
    auto fixture = createParserFixture("0o0");
    auto *node = fixture->parseLiteralExpression();
    expectIntegerLiteral(node, 0);
    REQUIRE(fixture->isAtEnd());
  }
}

TEST_CASE("Parser: Integer literals in expressions",
          "[parser][literals][integer]") {
  SECTION("Integer in primary expression") {
    auto fixture = createParserFixture("42");
    auto *node = fixture->parsePrimaryExpression();
    expectIntegerLiteral(node, 42);
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Integer in expression") {
    auto fixture = createParserFixture("123");
    auto *node = fixture->parseExpression();
    expectIntegerLiteral(node, 123);
    REQUIRE(fixture->isAtEnd());
  }
}

TEST_CASE("Parser: Multiple integer literals", "[parser][literals][integer]") {
  auto fixture = createParserFixture("10 20 30");

  // Parse first integer
  auto *node1 = fixture->parseLiteralExpression();
  expectIntegerLiteral(node1, 10);

  // Parse second integer (parser advanced automatically)
  auto *node2 = fixture->parseLiteralExpression();
  expectIntegerLiteral(node2, 20);

  // Parse third integer (parser advanced automatically)
  auto *node3 = fixture->parseLiteralExpression();
  expectIntegerLiteral(node3, 30);

  // Should be at end
  REQUIRE(fixture->isAtEnd());
}

TEST_CASE("Parser: Integer literal with large values",
          "[parser][literals][integer]") {
  SECTION("32-bit max value") {
    auto fixture = createParserFixture("2147483647");
    auto *node = fixture->parseLiteralExpression();
    expectIntegerLiteral(node, 2147483647);
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("64-bit value") {
    auto fixture = createParserFixture("9223372036854775807");
    auto *node = fixture->parseLiteralExpression();
    expectIntegerLiteral(node, 9223372036854775807LL);
    REQUIRE(fixture->isAtEnd());
  }
}

TEST_CASE("Parser: Integer literal error cases",
          "[parser][literals][integer]") {
  SECTION("Wrong token type") {
    auto fixture = createParserFixture("\"not an integer\"");
    auto *node = fixture->parseLiteralExpression();
    // Should parse as string, not integer
    REQUIRE(node != nullptr);
    REQUIRE(node->kind == ast::astString);
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
}

TEST_CASE("Parser: Integer literal token buffer behavior",
          "[parser][literals][integer]") {
  auto fixture = createParserFixture("42 3.14");

  // Initially at integer
  REQUIRE(fixture->current().kind == TokenKind::IntLiteral);
  REQUIRE(fixture->lookahead(1).kind == TokenKind::FloatLiteral);

  // Parse integer literal
  auto *node = fixture->parseLiteralExpression();
  expectIntegerLiteral(node, 42);

  // Should have advanced to next token after parsing
  REQUIRE(fixture->current().kind == TokenKind::FloatLiteral);
}

TEST_CASE("Parser: Integer literal location information",
          "[parser][literals][integer]") {
  auto fixture = createParserFixture("12345");

  // Store the token location before parsing (parser will advance)
  Location expectedLocation = fixture->current().location;
  auto *node = fixture->parseLiteralExpression();

  REQUIRE(node != nullptr);
  REQUIRE(node->kind == ast::astInt);

  // Location should be set
  REQUIRE(node->location.isValid());

  // Location should correspond to the original token
  REQUIRE(node->location == expectedLocation);
}

LITERAL_TEST_CASE("Simple integer", "42", expectIntegerLiteral(node, 42))
LITERAL_TEST_CASE("Zero integer", "0", expectIntegerLiteral(node, 0))
LITERAL_TEST_CASE("Hex integer", "0xFF", expectIntegerLiteral(node, 255))
LITERAL_TEST_CASE("Binary integer", "0b1010", expectIntegerLiteral(node, 10))
LITERAL_TEST_CASE("Octal integer", "0o755", expectIntegerLiteral(node, 493))
