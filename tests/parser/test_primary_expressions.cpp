#include "../parser_test_utils.hpp"

#include "catch2.hpp"

using namespace cxy;
using namespace cxy::test;

TEST_CASE("Parser: Primary expression with literals", "[parser][primary]") {
  SECTION("Integer literal") {
    auto fixture = createParserFixture("42");
    auto *node = fixture->parsePrimaryExpression();
    expectIntegerLiteral(node, 42);
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Float literal") {
    auto fixture = createParserFixture("3.14");
    auto *node = fixture->parsePrimaryExpression();
    expectFloatLiteral(node, 3.14);
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("String literal") {
    auto fixture = createParserFixture("\"hello\"");
    auto *node = fixture->parsePrimaryExpression();
    expectStringLiteral(node, "hello");
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Character literal") {
    auto fixture = createParserFixture("'x'");
    auto *node = fixture->parsePrimaryExpression();
    expectCharLiteral(node, 'x');
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Boolean true literal") {
    auto fixture = createParserFixture("true");
    auto *node = fixture->parsePrimaryExpression();
    expectBoolLiteral(node, true);
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Boolean false literal") {
    auto fixture = createParserFixture("false");
    auto *node = fixture->parsePrimaryExpression();
    expectBoolLiteral(node, false);
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Null literal") {
    auto fixture = createParserFixture("null");
    auto *node = fixture->parsePrimaryExpression();
    expectNullLiteral(node);
    REQUIRE(fixture->isAtEnd());
  }
}

TEST_CASE("Parser: Primary expression with identifiers", "[parser][primary]") {
  SECTION("Simple identifier") {
    auto fixture = createParserFixture("variable");
    auto *node = fixture->parsePrimaryExpression();
    expectIdentifier(node, "variable");
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Complex identifier") {
    auto fixture = createParserFixture("my_variable_123");
    auto *node = fixture->parsePrimaryExpression();
    expectIdentifier(node, "my_variable_123");
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Single letter identifier") {
    auto fixture = createParserFixture("x");
    auto *node = fixture->parsePrimaryExpression();
    expectIdentifier(node, "x");
    REQUIRE(fixture->isAtEnd());
  }
}

TEST_CASE("Parser: Parenthesized expressions",
          "[parser][primary][parentheses]") {
  SECTION("Simple parenthesized integer") {
    auto fixture = createParserFixture("(42)");
    auto *node = fixture->parsePrimaryExpression();
    expectIntegerLiteral(node, 42);
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Simple parenthesized identifier") {
    auto fixture = createParserFixture("(variable)");
    auto *node = fixture->parsePrimaryExpression();
    expectIdentifier(node, "variable");
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Parenthesized string literal") {
    auto fixture = createParserFixture("(\"hello world\")");
    auto *node = fixture->parsePrimaryExpression();
    expectStringLiteral(node, "hello world");
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Parenthesized boolean") {
    auto fixture = createParserFixture("(true)");
    auto *node = fixture->parsePrimaryExpression();
    expectBoolLiteral(node, true);
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Parenthesized null") {
    auto fixture = createParserFixture("(null)");
    auto *node = fixture->parsePrimaryExpression();
    expectNullLiteral(node);
    REQUIRE(fixture->isAtEnd());
  }
}

TEST_CASE("Parser: Nested parenthesized expressions",
          "[parser][primary][parentheses]") {
  SECTION("Double parentheses") {
    auto fixture = createParserFixture("((42))");
    auto *node = fixture->parsePrimaryExpression();
    expectIntegerLiteral(node, 42);
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Triple parentheses") {
    auto fixture = createParserFixture("(((variable)))");
    auto *node = fixture->parsePrimaryExpression();
    expectIdentifier(node, "variable");
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Nested with different literals") {
    auto fixture = createParserFixture("((\"nested\"))");
    auto *node = fixture->parsePrimaryExpression();
    expectStringLiteral(node, "nested");
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Deep nesting") {
    auto fixture = createParserFixture("((((((true))))))");
    auto *node = fixture->parsePrimaryExpression();
    expectBoolLiteral(node, true);
    REQUIRE(fixture->isAtEnd());
  }
}

TEST_CASE("Parser: Parenthesized expressions with whitespace",
          "[parser][primary][parentheses]") {
  SECTION("Spaces around content") {
    auto fixture = createParserFixture("( 42 )");
    auto *node = fixture->parsePrimaryExpression();
    expectIntegerLiteral(node, 42);
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Tabs and newlines") {
    auto fixture = createParserFixture("(\t\nvariable\n\t)");
    auto *node = fixture->parsePrimaryExpression();
    expectIdentifier(node, "variable");
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Nested with whitespace") {
    auto fixture = createParserFixture("( ( 123 ) )");
    auto *node = fixture->parsePrimaryExpression();
    expectIntegerLiteral(node, 123);
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Complex whitespace patterns") {
    auto fixture = createParserFixture("  (  \n\t (  \"test\"  )  \n  )  ");
    auto *node = fixture->parsePrimaryExpression();
    expectStringLiteral(node, "test");
    REQUIRE(fixture->isAtEnd());
  }
}

TEST_CASE("Parser: Primary expression error cases", "[parser][primary]") {
  SECTION("Missing closing parenthesis") {
    auto fixture = createParserFixture("(42");
    auto *node = fixture->parsePrimaryExpression();
    expectParseFailure(node);
  }

  SECTION("Missing opening parenthesis") {
    auto fixture = createParserFixture("42)");
    auto *node = fixture->parsePrimaryExpression();
    // Should parse 42 successfully, leaving ) as next token
    expectIntegerLiteral(node, 42);
    REQUIRE(fixture->current().kind == TokenKind::RParen);
  }

  SECTION("Empty parentheses") {
    auto fixture = createParserFixture("()");
    auto *node = fixture->parsePrimaryExpression();
    expectParseFailure(node);
  }

  SECTION("Nested missing closing parenthesis") {
    auto fixture = createParserFixture("((42)");
    auto *node = fixture->parsePrimaryExpression();
    expectParseFailure(node);
  }

  SECTION("Empty input") {
    auto fixture = createParserFixture("");
    auto *node = fixture->parsePrimaryExpression();
    expectParseFailure(node);
  }

  SECTION("Invalid token") {
    auto fixture = createParserFixture("@#$");
    auto *node = fixture->parsePrimaryExpression();
    expectParseFailure(node);
  }
}

TEST_CASE("Parser: Primary expression token buffer behavior",
          "[parser][primary]") {
  auto fixture = createParserFixture("(42) 3.14");

  // Initially at opening parenthesis
  REQUIRE(fixture->current().kind == TokenKind::LParen);
  REQUIRE(fixture->lookahead(1).kind == TokenKind::IntLiteral);
  REQUIRE(fixture->lookahead(2).kind == TokenKind::RParen);

  // Parse parenthesized expression
  auto *node = fixture->parsePrimaryExpression();
  expectIntegerLiteral(node, 42);

  // Should be at token after closing parenthesis
  REQUIRE(fixture->current().kind == TokenKind::FloatLiteral);
}

TEST_CASE("Parser: Primary expression vs literal expression distinction",
          "[parser][primary]") {
  SECTION("Primary expression includes identifiers") {
    auto fixture = createParserFixture("identifier");
    auto *primaryNode = fixture->parsePrimaryExpression();
    expectIdentifier(primaryNode, "identifier");

    // Reset fixture
    fixture = createParserFixture("identifier");
    auto *literalNode = fixture->parseLiteralExpression();
    expectParseFailure(literalNode);
  }

  SECTION("Primary expression includes parenthesized") {
    auto fixture = createParserFixture("(42)");
    auto *primaryNode = fixture->parsePrimaryExpression();
    expectIntegerLiteral(primaryNode, 42);

    // Reset fixture
    fixture = createParserFixture("(42)");
    auto *literalNode = fixture->parseLiteralExpression();
    expectParseFailure(literalNode);
  }

  SECTION("Both handle literals") {
    auto fixture = createParserFixture("42");
    auto *primaryNode = fixture->parsePrimaryExpression();
    expectIntegerLiteral(primaryNode, 42);

    // Reset fixture
    fixture = createParserFixture("42");
    auto *literalNode = fixture->parseLiteralExpression();
    expectIntegerLiteral(literalNode, 42);
  }
}

TEST_CASE("Parser: Primary expression sequence parsing", "[parser][primary]") {
  auto fixture = createParserFixture("42 \"hello\" (true) variable null");

  // Parse sequence of primary expressions
  std::vector<std::function<void(ast::ASTNode *)>> expectations = {
      [](ast::ASTNode *node) { expectIntegerLiteral(node, 42); },
      [](ast::ASTNode *node) { expectStringLiteral(node, "hello"); },
      [](ast::ASTNode *node) { expectBoolLiteral(node, true); },
      [](ast::ASTNode *node) { expectIdentifier(node, "variable"); },
      [](ast::ASTNode *node) { expectNullLiteral(node); }};

  for (size_t i = 0; i < expectations.size(); ++i) {
    auto *node = fixture->parsePrimaryExpression();
    expectations[i](node);
  }

  REQUIRE(fixture->isAtEnd());
}

TEST_CASE("Parser: Complex parenthesized expression parsing",
          "[parser][primary][parentheses]") {
  SECTION("Mixed nested content") {
    auto fixture = createParserFixture("(((42)))");
    auto *node = fixture->parsePrimaryExpression();
    expectIntegerLiteral(node, 42);
  }

  SECTION("Alternating parens and content") {
    auto fixture = createParserFixture("(identifier)");
    auto *node = fixture->parsePrimaryExpression();
    expectIdentifier(node, "identifier");
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Complex whitespace in deep nesting") {
    auto fixture = createParserFixture(
        "( \n\t( \n\t\t( \n\t\t\t\"deep\" \n\t\t) \n\t) \n)");
    auto *node = fixture->parsePrimaryExpression();
    expectStringLiteral(node, "deep");
    REQUIRE(fixture->isAtEnd());
  }
}

TEST_CASE("Parser: Primary expression location tracking", "[parser][primary]") {
  SECTION("Literal location") {
    auto fixture = createParserFixture("42");
    Location expectedLocation = fixture->current().location;
    auto *node = fixture->parsePrimaryExpression();

    REQUIRE(node != nullptr);
    REQUIRE(node->location.isValid());
    REQUIRE(node->location == expectedLocation);
  }

  SECTION("Identifier location") {
    auto fixture = createParserFixture("variable");
    Location expectedLocation = fixture->current().location;
    auto *node = fixture->parsePrimaryExpression();

    REQUIRE(node != nullptr);
    REQUIRE(node->location.isValid());
    REQUIRE(node->location == expectedLocation);
  }

  SECTION("Parenthesized expression location") {
    auto fixture = createParserFixture("(42)");
    auto *node = fixture->parsePrimaryExpression();

    REQUIRE(node != nullptr);
    REQUIRE(node->location.isValid());
    // Location should be of the inner expression (42), not the parentheses
  }
}

// Macro-based tests for consistency
PRIMARY_EXPRESSION_TEST_CASE("Integer primary", "42",
                             expectIntegerLiteral(node, 42))
PRIMARY_EXPRESSION_TEST_CASE("Float primary", "3.14",
                             expectFloatLiteral(node, 3.14))
PRIMARY_EXPRESSION_TEST_CASE("String primary", "\"test\"",
                             expectStringLiteral(node, "test"))
PRIMARY_EXPRESSION_TEST_CASE("Char primary", "'x'",
                             expectCharLiteral(node, 'x'))
PRIMARY_EXPRESSION_TEST_CASE("Bool true primary", "true",
                             expectBoolLiteral(node, true))
PRIMARY_EXPRESSION_TEST_CASE("Bool false primary", "false",
                             expectBoolLiteral(node, false))
PRIMARY_EXPRESSION_TEST_CASE("Null primary", "null", expectNullLiteral(node))
PRIMARY_EXPRESSION_TEST_CASE("Identifier primary", "variable",
                             expectIdentifier(node, "variable"))
PRIMARY_EXPRESSION_TEST_CASE("Parenthesized int", "(42)",
                             expectIntegerLiteral(node, 42))
PRIMARY_EXPRESSION_TEST_CASE("Parenthesized identifier", "(variable)",
                             expectIdentifier(node, "variable"))
PRIMARY_EXPRESSION_TEST_CASE("Nested parentheses", "((42))",
                             expectIntegerLiteral(node, 42))
PRIMARY_EXPRESSION_TEST_CASE("Deep nesting", "(((((true)))))",
                             expectBoolLiteral(node, true))
