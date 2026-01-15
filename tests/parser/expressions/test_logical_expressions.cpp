#include "../../parser_test_utils.hpp"

#include "catch2.hpp"

using namespace cxy;
using namespace cxy::test;

TEST_CASE("Logical Expressions", "[parser][expressions][logical]") {

  SECTION("Basic logical AND") {
    ParserTestFixture fixture("a && b");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result,
                        "(BinaryExpr && (Identifier a) (Identifier b))");
  }

  SECTION("Left associativity with multiple AND") {
    ParserTestFixture fixture("a && b && c");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(BinaryExpr && (BinaryExpr && (Identifier a) "
                                "(Identifier b)) (Identifier c))");
  }

  SECTION("Multiple logical AND operators") {
    ParserTestFixture fixture("x && y && z && w");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result,
                        "(BinaryExpr && (BinaryExpr && (BinaryExpr && "
                        "(Identifier x) (Identifier y)) (Identifier z)) "
                        "(Identifier w))");
  }

  SECTION("Logical AND with lower precedence - bitwise OR") {
    ParserTestFixture fixture("a | b && c | d");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(BinaryExpr && (BinaryExpr | (Identifier a) "
                                "(Identifier b)) (BinaryExpr | "
                                "(Identifier c) (Identifier d)))");
  }

  SECTION("Logical AND with lower precedence - equality") {
    ParserTestFixture fixture("a == b && c != d");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(BinaryExpr && (BinaryExpr == (Identifier a) "
                                "(Identifier b)) (BinaryExpr != "
                                "(Identifier c) (Identifier d)))");
  }

  SECTION("Logical AND with lower precedence - relational") {
    ParserTestFixture fixture("a < b && c > d");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(BinaryExpr && (BinaryExpr < (Identifier a) "
                                "(Identifier b)) (BinaryExpr > "
                                "(Identifier c) (Identifier d)))");
  }

  SECTION("Parenthesized logical AND expressions") {
    ParserTestFixture fixture("(a && b) | c");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(BinaryExpr | (BinaryExpr && (Identifier a) "
                                "(Identifier b)) (Identifier c))");
  }

  SECTION("Logical AND with literals") {
    ParserTestFixture fixture("true && false");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(BinaryExpr && (Bool true) (Bool false))");
  }

  SECTION("Complex nested expression") {
    ParserTestFixture fixture("a + b > c && d | e == f");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result,
                        "(BinaryExpr && (BinaryExpr > (BinaryExpr + "
                        "(Identifier a) (Identifier b)) (Identifier c)) "
                        "(BinaryExpr | (Identifier d) (BinaryExpr == "
                        "(Identifier e) (Identifier f))))");
  }

  // Logical OR expressions (||)
  SECTION("Basic logical OR") {
    ParserTestFixture fixture("a || b");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result,
                        "(BinaryExpr || (Identifier a) (Identifier b))");
  }

  SECTION("Left associativity with multiple OR") {
    ParserTestFixture fixture("a || b || c");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(BinaryExpr || (BinaryExpr || (Identifier a) "
                                "(Identifier b)) (Identifier c))");
  }

  SECTION("OR with lower precedence - logical AND") {
    ParserTestFixture fixture("a && b || c && d");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(BinaryExpr || (BinaryExpr && (Identifier a) "
                                "(Identifier b)) (BinaryExpr && "
                                "(Identifier c) (Identifier d)))");
  }

  SECTION("Logical OR with literals") {
    ParserTestFixture fixture("true || false");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(BinaryExpr || (Bool true) (Bool false))");
  }

  SECTION("Complex OR expression") {
    ParserTestFixture fixture("a && b || c > d");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(BinaryExpr || (BinaryExpr && (Identifier a) "
                                "(Identifier b)) (BinaryExpr > "
                                "(Identifier c) (Identifier d)))");
  }
}
