#include "../parser_test_utils.hpp"

#include "catch2.hpp"

using namespace cxy;
using namespace cxy::test;

TEST_CASE("Expression Integration Tests",
          "[parser][expressions][integration]") {

  SECTION("Complete precedence hierarchy") {
    ParserTestFixture fixture(
        "a = b ? c || d && e | f ^ g & h == i < j << k + l * m : n");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    // This tests the full precedence hierarchy in one expression
    REQUIRE_AST_MATCHES(result, "(AssignmentExpr = (Identifier a) "
                                "(TernaryExpr (Identifier b) "
                                "(BinaryExpr || (Identifier c) "
                                "(BinaryExpr && (Identifier d) "
                                "(BinaryExpr | (Identifier e) "
                                "(BinaryExpr ^ (Identifier f) "
                                "(BinaryExpr & (Identifier g) "
                                "(BinaryExpr == (Identifier h) "
                                "(BinaryExpr < (Identifier i) "
                                "(BinaryExpr << (Identifier j) "
                                "(BinaryExpr + (Identifier k) "
                                "(BinaryExpr * (Identifier l) (Identifier "
                                "m))))))))))) (Identifier n)))");
  }

  SECTION("Assignment chain with all operators") {
    ParserTestFixture fixture(
        "a = b += c -= d *= e /= f %= g &= h ^= i |= j <<= k >>= l");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    // Tests right-associativity of all assignment operators
    REQUIRE_AST_MATCHES(
        result, "(AssignmentExpr = (Identifier a) "
                "(AssignmentExpr += (Identifier b) "
                "(AssignmentExpr -= (Identifier c) "
                "(AssignmentExpr *= (Identifier d) "
                "(AssignmentExpr /= (Identifier e) "
                "(AssignmentExpr %= (Identifier f) "
                "(AssignmentExpr &= (Identifier g) "
                "(AssignmentExpr ^= (Identifier h) "
                "(AssignmentExpr |= (Identifier i) "
                "(AssignmentExpr <<= (Identifier j) "
                "(AssignmentExpr >>= (Identifier k) (Identifier l))))))))))))");
  }

  SECTION("Nested ternary with complex expressions") {
    ParserTestFixture fixture("a > b ? c += d * e : f ? g || h : i && j");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(
        result, "(TernaryExpr (BinaryExpr > (Identifier a) (Identifier b)) "
                "(AssignmentExpr += (Identifier c) "
                "(BinaryExpr * (Identifier d) (Identifier e))) "
                "(TernaryExpr (Identifier f) "
                "(BinaryExpr || (Identifier g) (Identifier h)) "
                "(BinaryExpr && (Identifier i) (Identifier j))))");
  }

  SECTION("Unary operators with complex expressions") {
    ParserTestFixture fixture("result = ++a + --b");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(AssignmentExpr = (Identifier result) "
                                "(BinaryExpr + (UnaryExpr ++ (Identifier a)) "
                                "(UnaryExpr -- (Identifier b))))");
  }

  SECTION("Parentheses override precedence") {
    ParserTestFixture fixture("(a + b) * (c - d) == (e | f) & (g ^ h)");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result,
                        "(BinaryExpr & "
                        "(BinaryExpr == "
                        "(BinaryExpr * "
                        "(BinaryExpr + (Identifier a) (Identifier b)) "
                        "(BinaryExpr - (Identifier c) (Identifier d))) "
                        "(BinaryExpr | (Identifier e) (Identifier f))) "
                        "(BinaryExpr ^ (Identifier g) (Identifier h)))");
  }

  SECTION("Postfix operators with expressions") {
    ParserTestFixture fixture("a++ * ++b");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result,
                        "(BinaryExpr * (UnaryExpr ++ [postfix] (Identifier a)) "
                        "(UnaryExpr ++ (Identifier b)))");
  }

  SECTION("Complex arithmetic with all operators") {
    ParserTestFixture fixture("((a + b) * c - d / e % f) << g >> h");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(
        result, "(BinaryExpr >> "
                "(BinaryExpr << "
                "(BinaryExpr - "
                "(BinaryExpr * "
                "(BinaryExpr + (Identifier a) (Identifier b)) (Identifier c)) "
                "(BinaryExpr % (BinaryExpr / (Identifier d) (Identifier e)) "
                "(Identifier f))) "
                "(Identifier g)) (Identifier h))");
  }

  SECTION("All comparison operators") {
    ParserTestFixture fixture("a < b <= c > d >= e == f != g");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(
        result,
        "(BinaryExpr != "
        "(BinaryExpr == "
        "(BinaryExpr >= "
        "(BinaryExpr > "
        "(BinaryExpr <= "
        "(BinaryExpr < (Identifier a) (Identifier b)) (Identifier c)) "
        "(Identifier d)) (Identifier e)) (Identifier f)) (Identifier g))");
  }

  SECTION("All bitwise operators") {
    ParserTestFixture fixture("a & b ^ c | d");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(
        result, "(BinaryExpr | "
                "(BinaryExpr ^ "
                "(BinaryExpr & (Identifier a) (Identifier b)) (Identifier c)) "
                "(Identifier d))");
  }

  SECTION("All logical operators") {
    ParserTestFixture fixture("a && b || c && d");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result,
                        "(BinaryExpr || "
                        "(BinaryExpr && (Identifier a) (Identifier b)) "
                        "(BinaryExpr && (Identifier c) (Identifier d)))");
  }

  SECTION("Literals with complex expressions") {
    ParserTestFixture fixture(
        "result = 42 + 3.14 * 'A' == \"test\" ? true : false");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(AssignmentExpr = (Identifier result) "
                                "(TernaryExpr "
                                "(BinaryExpr == "
                                "(BinaryExpr + (Int 42) "
                                "(BinaryExpr * (Float 3.14) (Char 'A'))) "
                                "(String \"test\")) "
                                "(Bool true) (Bool false)))");
  }
}
