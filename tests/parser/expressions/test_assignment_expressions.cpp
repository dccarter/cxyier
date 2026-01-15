#include "../../parser_test_utils.hpp"

#include "catch2.hpp"

using namespace cxy;
using namespace cxy::test;

TEST_CASE("Assignment Expressions", "[parser][expressions][assignment]") {

  SECTION("Basic assignment") {
    ParserTestFixture fixture("a = b");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result,
                        "(AssignmentExpr = (Identifier a) (Identifier b))");
  }

  SECTION("Assignment with literals") {
    ParserTestFixture fixture("x = 42");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(AssignmentExpr = (Identifier x) (Int 42))");
  }

  SECTION("Plus assignment") {
    ParserTestFixture fixture("a += b");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result,
                        "(AssignmentExpr += (Identifier a) (Identifier b))");
  }

  SECTION("Minus assignment") {
    ParserTestFixture fixture("a -= b");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result,
                        "(AssignmentExpr -= (Identifier a) (Identifier b))");
  }

  SECTION("Multiply assignment") {
    ParserTestFixture fixture("a *= b");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result,
                        "(AssignmentExpr *= (Identifier a) (Identifier b))");
  }

  SECTION("Divide assignment") {
    ParserTestFixture fixture("a /= b");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result,
                        "(AssignmentExpr /= (Identifier a) (Identifier b))");
  }

  SECTION("Modulo assignment") {
    ParserTestFixture fixture("a %= b");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result,
                        "(AssignmentExpr %= (Identifier a) (Identifier b))");
  }

  SECTION("Bitwise AND assignment") {
    ParserTestFixture fixture("a &= b");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result,
                        "(AssignmentExpr &= (Identifier a) (Identifier b))");
  }

  SECTION("Bitwise XOR assignment") {
    ParserTestFixture fixture("a ^= b");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result,
                        "(AssignmentExpr ^= (Identifier a) (Identifier b))");
  }

  SECTION("Bitwise OR assignment") {
    ParserTestFixture fixture("a |= b");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result,
                        "(AssignmentExpr |= (Identifier a) (Identifier b))");
  }

  SECTION("Left shift assignment") {
    ParserTestFixture fixture("a <<= b");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result,
                        "(AssignmentExpr <<= (Identifier a) (Identifier b))");
  }

  SECTION("Right shift assignment") {
    ParserTestFixture fixture("a >>= b");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result,
                        "(AssignmentExpr >>= (Identifier a) (Identifier b))");
  }

  SECTION("Right associative assignment") {
    ParserTestFixture fixture("a = b = c");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(AssignmentExpr = (Identifier a) "
                                "(AssignmentExpr = (Identifier b) "
                                "(Identifier c)))");
  }

  SECTION("Multiple assignment operators") {
    ParserTestFixture fixture("a = b += c");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(AssignmentExpr = (Identifier a) "
                                "(AssignmentExpr += (Identifier b) "
                                "(Identifier c)))");
  }

  SECTION("Assignment with conditional expression") {
    ParserTestFixture fixture("a = b ? c : d");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(AssignmentExpr = (Identifier a) "
                                "(TernaryExpr (Identifier b) (Identifier c) "
                                "(Identifier d)))");
  }

  SECTION("Assignment with logical operators") {
    ParserTestFixture fixture("a = b || c && d");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result,
                        "(AssignmentExpr = (Identifier a) "
                        "(BinaryExpr || (Identifier b) "
                        "(BinaryExpr && (Identifier c) (Identifier d))))");
  }

  SECTION("Assignment with arithmetic expression") {
    ParserTestFixture fixture("result = a + b * c");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(AssignmentExpr = (Identifier result) "
                                "(BinaryExpr + (Identifier a) "
                                "(BinaryExpr * (Identifier b) "
                                "(Identifier c))))");
  }

  SECTION("Complex nested assignment") {
    ParserTestFixture fixture("a += b *= c");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(AssignmentExpr += (Identifier a) "
                                "(AssignmentExpr *= (Identifier b) "
                                "(Identifier c)))");
  }

  SECTION("Assignment with parenthesized expression") {
    ParserTestFixture fixture("a = (b + c)");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(AssignmentExpr = (Identifier a) "
                                "(BinaryExpr + (Identifier b) "
                                "(Identifier c)))");
  }

  SECTION("Assignment chain with different operators") {
    ParserTestFixture fixture("a = b += c *= d");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(AssignmentExpr = (Identifier a) "
                                "(AssignmentExpr += (Identifier b) "
                                "(AssignmentExpr *= (Identifier c) "
                                "(Identifier d))))");
  }

  SECTION("Assignment with complex conditional") {
    ParserTestFixture fixture("x = a > b ? c + d : e * f");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result,
                        "(AssignmentExpr = (Identifier x) "
                        "(TernaryExpr (BinaryExpr > (Identifier a) "
                        "(Identifier b)) "
                        "(BinaryExpr + (Identifier c) (Identifier d)) "
                        "(BinaryExpr * (Identifier e) (Identifier f))))");
  }

  SECTION("Assignment with bitwise operations") {
    ParserTestFixture fixture("flags |= mask & value");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(AssignmentExpr |= (Identifier flags) "
                                "(BinaryExpr & (Identifier mask) "
                                "(Identifier value)))");
  }
}
