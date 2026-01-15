#include "../../parser_test_utils.hpp"

#include "catch2.hpp"

using namespace cxy;
using namespace cxy::test;

TEST_CASE("Parser: Basic string interpolation",
          "[parser][expressions][interpolation]") {
  SECTION("Simple interpolation with identifier") {
    auto fixture = createParserFixture("\"Hello {name}!\"");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr,
                        "(StringExpr \"Hello \" (Identifier name) \"!\")");
  }

  SECTION("Interpolation with integer literal") {
    auto fixture = createParserFixture("\"Value: {42}\"");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(StringExpr \"Value: \" (Int 42))");
  }

  SECTION("String without interpolation") {
    auto fixture = createParserFixture("\"Plain string\"");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(String \"Plain string\")");
  }

  SECTION("Empty string interpolation") {
    auto fixture = createParserFixture("\"{}\"");
    auto *expr = fixture->parseExpression();
    REQUIRE(expr == nullptr);
    REQUIRE(fixture->hasErrors());
  }

  SECTION("Whitespace-only string interpolation") {
    auto fixture = createParserFixture("\"{ }\"");
    auto *expr = fixture->parseExpression();
    REQUIRE(expr == nullptr);
    REQUIRE(fixture->hasErrors());
  }

  SECTION("Empty interpolation with surrounding text") {
    auto fixture = createParserFixture("\"Hello {} world\"");
    auto *expr = fixture->parseExpression();
    REQUIRE(expr == nullptr);
    REQUIRE(fixture->hasErrors());
  }

  SECTION("Interpolation at start") {
    auto fixture = createParserFixture("\"{greeting} world\"");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(StringExpr (Identifier greeting) \" world\")");
  }

  SECTION("Interpolation at end") {
    auto fixture = createParserFixture("\"Hello {name}\"");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(StringExpr \"Hello \" (Identifier name))");
  }
}

TEST_CASE("Parser: Multiple string interpolations",
          "[parser][expressions][interpolation]") {
  SECTION("Two interpolations") {
    auto fixture = createParserFixture("\"Hello {name} you are {age} old\"");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(StringExpr \"Hello \" (Identifier name) \" you "
                              "are \" (Identifier age) \" old\")");
  }

  SECTION("Three interpolations") {
    auto fixture = createParserFixture("\"{greeting} {name}, today is {day}\"");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(StringExpr (Identifier greeting) \" \" "
                              "(Identifier name) \", today is \" (Identifier "
                              "day))");
  }

  SECTION("Adjacent interpolations") {
    auto fixture = createParserFixture("\"{first}{second}\"");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr,
                        "(StringExpr (Identifier first) (Identifier second))");
  }

  SECTION("Multiple interpolations with complex expressions") {
    auto fixture =
        createParserFixture("\"User {user.name} has {user.score} points\"");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(StringExpr \"User \" (MemberExpr . (Identifier "
                              "user) (Identifier name)) \" has \" (MemberExpr "
                              ". (Identifier user) (Identifier score)) \" "
                              "points\")");
  }
}

TEST_CASE("Parser: Complex expression interpolation",
          "[parser][expressions][interpolation]") {
  SECTION("Function call interpolation") {
    auto fixture = createParserFixture("\"Result: {getValue()}\"");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr, "(StringExpr \"Result: \" (CallExpr (Identifier getValue)))");
  }

  SECTION("Arithmetic expression interpolation") {
    auto fixture = createParserFixture("\"Sum: {a + b}\"");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(StringExpr \"Sum: \" (BinaryExpr + (Identifier "
                              "a) (Identifier b)))");
  }

  SECTION("Member access interpolation") {
    auto fixture = createParserFixture("\"Name: {obj.field}\"");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr,
                        "(StringExpr \"Name: \" (MemberExpr . (Identifier obj) "
                        "(Identifier field)))");
  }

  SECTION("Array indexing interpolation") {
    auto fixture = createParserFixture("\"Item: {arr[index]}\"");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(StringExpr \"Item: \" (IndexExpr (Identifier "
                              "arr) (Identifier index)))");
  }

  SECTION("Complex chained expression") {
    auto fixture = createParserFixture("\"Result: {getValue().items[0]}\"");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr, "(StringExpr \"Result: \" (IndexExpr (MemberExpr . (CallExpr "
              "(Identifier getValue)) (Identifier items)) (Int 0)))");
  }
}

TEST_CASE("Parser: Nested string interpolation",
          "[parser][expressions][interpolation]") {
  SECTION("Simple nested interpolation") {
    auto fixture =
        createParserFixture("\"Hello {getGreeting(\"User {name}\")}!\"");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr, "(StringExpr \"Hello \" (CallExpr (Identifier getGreeting) "
              "(StringExpr \"User \" (Identifier name))) \"!\")");
  }

  SECTION("Deep nested interpolation") {
    auto fixture =
        createParserFixture("\"Status: {format(\"Value: {x}\", getValue())}\"");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr, "(StringExpr \"Status: \" (CallExpr (Identifier format) "
              "(StringExpr \"Value: \" (Identifier x)) (CallExpr (Identifier "
              "getValue))))");
  }

  SECTION("Multiple nested interpolations") {
    auto fixture = createParserFixture(
        "\"Result: {process(\"Input {a}\", \"Output {b}\")}\"");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr, "(StringExpr \"Result: \" (CallExpr (Identifier process) "
              "(StringExpr \"Input \" (Identifier a)) (StringExpr \"Output \" "
              "(Identifier b))))");
  }
}

TEST_CASE("Parser: String interpolation edge cases",
          "[parser][expressions][interpolation]") {
  SECTION("Interpolation with parenthesized expression") {
    auto fixture = createParserFixture("\"Result: {(a + b)}\"");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(StringExpr \"Result: \" (BinaryExpr "
                              "+ (Identifier a) (Identifier b)))");
  }

  SECTION("Interpolation with whitespace") {
    auto fixture = createParserFixture("\"Value: { x }\"");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(StringExpr \"Value: \" (Identifier x))");
  }

  SECTION("Interpolation with complex whitespace") {
    auto fixture = createParserFixture("\"Sum: {  a + b  }\"");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(StringExpr \"Sum: \" (BinaryExpr + (Identifier "
                              "a) (Identifier b)))");
  }

  SECTION("String only interpolation") {
    auto fixture = createParserFixture("\"{value}\"");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(StringExpr (Identifier value))");
  }

  SECTION("Empty string with only interpolation") {
    auto fixture = createParserFixture("\"{x}\"");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(StringExpr (Identifier x))");
  }
}

TEST_CASE("Parser: String interpolation in various contexts",
          "[parser][expressions][interpolation]") {
  SECTION("Interpolated string as function argument") {
    auto fixture = createParserFixture("println(\"Hello {name}!\")");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr,
                        "(CallExpr (Identifier println) (StringExpr \"Hello \" "
                        "(Identifier name) \"!\"))");
  }

  SECTION("Interpolated string in array literal") {
    auto fixture = createParserFixture("[\"Item {i}\", \"Value {v}\"]");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr, "(ArrayExpr (StringExpr \"Item \" (Identifier i)) (StringExpr "
              "\"Value \" (Identifier v)))");
  }

  SECTION("Interpolated string as struct field") {
    auto fixture = createParserFixture("{ message: \"Hello {name}!\" }");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr, "(StructExpr (FieldExpr (Identifier message) (StringExpr "
              "\"Hello \" (Identifier name) \"!\")))");
  }

  SECTION("Interpolated string in assignment") {
    auto fixture = createParserFixture("msg = \"User {user} logged in\"");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr, "(AssignmentExpr = (Identifier msg) (StringExpr \"User \" "
              "(Identifier user) \" logged in\"))");
  }
}

TEST_CASE("Parser: String interpolation error cases",
          "[parser][expressions][interpolation]") {
  SECTION("Unclosed interpolation brace") {
    auto fixture = createParserFixture("\"Hello {name\"");
    auto *expr = fixture->parseExpression();
    REQUIRE(expr == nullptr);
    REQUIRE(fixture->hasErrors());
  }

  SECTION("Invalid expression in interpolation") {
    auto fixture = createParserFixture("\"Value: {+}\"");
    auto *expr = fixture->parseExpression();
    REQUIRE(expr == nullptr);
    REQUIRE(fixture->hasErrors());
  }

  SECTION("Nested unclosed brace") {
    auto fixture =
        createParserFixture("\"Hello {getGreeting(\"User {name\"}\"");
    auto *expr = fixture->parseExpression();
    REQUIRE(expr == nullptr);
    REQUIRE(fixture->hasErrors());
  }

  SECTION("Missing closing quote") {
    auto fixture = createParserFixture("\"Hello {name}!");
    auto *expr = fixture->parseExpression();
    REQUIRE(expr == nullptr);
    REQUIRE(fixture->hasErrors());
  }
}
