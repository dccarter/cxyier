#include "../../parser_test_utils.hpp"

#include "catch2.hpp"

using namespace cxy;
using namespace cxy::test;

TEST_CASE("Conditional Expressions", "[parser][expressions][conditional]") {

  SECTION("Basic ternary operator") {
    ParserTestFixture fixture("a ? b : c");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(
        result, "(TernaryExpr (Identifier a) (Identifier b) (Identifier c))");
  }

  SECTION("Ternary with literals") {
    ParserTestFixture fixture("true ? 42 : 0");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(TernaryExpr (Bool true) (Int 42) (Int 0))");
  }

  SECTION("Ternary with string literals") {
    ParserTestFixture fixture("condition ? \"yes\" : \"no\"");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(TernaryExpr (Identifier condition) (String "
                                "\"yes\") (String \"no\"))");
  }

  SECTION("Right associative nested ternary") {
    ParserTestFixture fixture("a ? b : c ? d : e");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(
        result, "(TernaryExpr (Identifier a) (Identifier b) "
                "(TernaryExpr (Identifier c) (Identifier d) (Identifier e)))");
  }

  SECTION("Left side nested ternary") {
    ParserTestFixture fixture("a ? b ? c : d : e");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(
        result, "(TernaryExpr (Identifier a) "
                "(TernaryExpr (Identifier b) (Identifier c) (Identifier d)) "
                "(Identifier e))");
  }

  SECTION("Ternary with lower precedence - logical OR") {
    ParserTestFixture fixture("a || b ? c : d");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(
        result, "(TernaryExpr (BinaryExpr || (Identifier a) (Identifier b)) "
                "(Identifier c) (Identifier d))");
  }

  SECTION("Ternary with lower precedence - logical AND") {
    ParserTestFixture fixture("a && b ? c && d : e");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(
        result,
        "(TernaryExpr (BinaryExpr && (Identifier a) (Identifier b)) "
        "(BinaryExpr && (Identifier c) (Identifier d)) (Identifier e))");
  }

  SECTION("Ternary with arithmetic expressions") {
    ParserTestFixture fixture("a + b > c ? x * y : z / w");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(
        result, "(TernaryExpr (BinaryExpr > (BinaryExpr + (Identifier a) "
                "(Identifier b)) (Identifier c)) "
                "(BinaryExpr * (Identifier x) (Identifier y)) "
                "(BinaryExpr / (Identifier z) (Identifier w)))");
  }

  SECTION("Parenthesized ternary condition") {
    ParserTestFixture fixture("(a && b) ? c : d");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(
        result, "(TernaryExpr (BinaryExpr && (Identifier a) (Identifier b)) "
                "(Identifier c) (Identifier d))");
  }

  SECTION("Parenthesized ternary branches") {
    ParserTestFixture fixture("a ? (b + c) : (d * e)");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result,
                        "(TernaryExpr (Identifier a) "
                        "(BinaryExpr + (Identifier b) (Identifier c)) "
                        "(BinaryExpr * (Identifier d) (Identifier e)))");
  }

  SECTION("Complex nested expression") {
    ParserTestFixture fixture("a == b ? c | d : e && f");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(
        result, "(TernaryExpr (BinaryExpr == (Identifier a) (Identifier b)) "
                "(BinaryExpr | (Identifier c) (Identifier d)) "
                "(BinaryExpr && (Identifier e) (Identifier f)))");
  }

  SECTION("Triple nested ternary") {
    ParserTestFixture fixture("a ? b ? c : d ? e : f : g");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(
        result, "(TernaryExpr (Identifier a) "
                "(TernaryExpr (Identifier b) (Identifier c) "
                "(TernaryExpr (Identifier d) (Identifier e) (Identifier f))) "
                "(Identifier g))");
  }

  SECTION("Ternary with equality and comparison") {
    ParserTestFixture fixture("x < y ? a == b : c != d");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(
        result, "(TernaryExpr (BinaryExpr < (Identifier x) (Identifier y)) "
                "(BinaryExpr == (Identifier a) (Identifier b)) "
                "(BinaryExpr != (Identifier c) (Identifier d)))");
  }
}
