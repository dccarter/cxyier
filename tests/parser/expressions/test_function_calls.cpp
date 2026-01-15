#include "../../parser_test_utils.hpp"

#include "catch2.hpp"

using namespace cxy;
using namespace cxy::test;

TEST_CASE("Parser: Simple function calls",
          "[parser][expressions][function][calls]") {
  SECTION("Function call with no arguments") {
    auto fixture = createParserFixture("myFunc()");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(CallExpr (Identifier myFunc))");
  }

  SECTION("Function call with single argument") {
    auto fixture = createParserFixture("myFunc(42)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(CallExpr (Identifier myFunc) (Int 42))");
  }

  SECTION("Function call with multiple arguments") {
    auto fixture = createParserFixture("myFunc(1, 2, 3)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr, "(CallExpr (Identifier myFunc) (Int 1) (Int 2) (Int 3))");
  }

  SECTION("Function call with mixed argument types") {
    auto fixture = createParserFixture("myFunc(42, x, 3.14, true)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(CallExpr (Identifier myFunc) (Int 42) "
                              "(Identifier x) (Float 3.14) (Bool true))");
  }
}

TEST_CASE("Parser: Function calls with expressions as arguments",
          "[parser][expressions][function][calls]") {
  SECTION("Function call with arithmetic expression arguments") {
    auto fixture = createParserFixture("myFunc(1 + 2, 3 * 4)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (CallExpr
        (Identifier myFunc)
        (BinaryExpr +
          (Int 1)
          (Int 2))
        (BinaryExpr *
          (Int 3)
          (Int 4)))
    )");
  }

  SECTION("Function call with complex expression arguments") {
    auto fixture = createParserFixture("myFunc(a + b * c, x == y)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (CallExpr
        (Identifier myFunc)
        (BinaryExpr +
          (Identifier a)
          (BinaryExpr *
            (Identifier b)
            (Identifier c)))
        (BinaryExpr ==
          (Identifier x)
          (Identifier y)))
    )");
  }
}

TEST_CASE("Parser: Function calls with collection arguments",
          "[parser][expressions][function][calls]") {
  SECTION("Function call with array arguments") {
    auto fixture = createParserFixture("myFunc([1, 2, 3])");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (CallExpr
        (Identifier myFunc)
        (ArrayExpr (Int 1) (Int 2) (Int 3)))
    )");
  }

  SECTION("Function call with tuple arguments") {
    auto fixture = createParserFixture("myFunc((1, 2))");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (CallExpr
        (Identifier myFunc)
        (TupleExpr (Int 1) (Int 2)))
    )");
  }

  SECTION("Function call with mixed collection arguments") {
    auto fixture = createParserFixture("myFunc([1, 2], (3, 4))");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (CallExpr
        (Identifier myFunc)
        (ArrayExpr (Int 1) (Int 2))
        (TupleExpr (Int 3) (Int 4)))
    )");
  }
}

TEST_CASE("Parser: Function calls with whitespace variations",
          "[parser][expressions][function][calls]") {
  SECTION("No spaces around arguments") {
    auto fixture = createParserFixture("myFunc(1,2,3)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr, "(CallExpr (Identifier myFunc) (Int 1) (Int 2) (Int 3))");
  }

  SECTION("Extra spaces around arguments") {
    auto fixture = createParserFixture("myFunc( 1 , 2 , 3 )");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr, "(CallExpr (Identifier myFunc) (Int 1) (Int 2) (Int 3))");
  }

  SECTION("Newlines between arguments") {
    auto fixture = createParserFixture("myFunc(\n  1,\n  2,\n  3\n)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr, "(CallExpr (Identifier myFunc) (Int 1) (Int 2) (Int 3))");
  }
}

TEST_CASE("Parser: Nested function calls",
          "[parser][expressions][function][calls][nested]") {
  SECTION("Function call as argument") {
    auto fixture = createParserFixture("outer(inner(42))");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (CallExpr
        (Identifier outer)
        (CallExpr
          (Identifier inner)
          (Int 42)))
    )");
  }

  SECTION("Multiple nested function calls") {
    auto fixture = createParserFixture("f(g(1), h(2, 3))");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (CallExpr
        (Identifier f)
        (CallExpr
          (Identifier g)
          (Int 1))
        (CallExpr
          (Identifier h)
          (Int 2)
          (Int 3)))
    )");
  }

  SECTION("Deeply nested function calls") {
    auto fixture = createParserFixture("a(b(c(d())))");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (CallExpr
        (Identifier a)
        (CallExpr
          (Identifier b)
          (CallExpr
            (Identifier c)
            (CallExpr
              (Identifier d)))))
    )");
  }
}

TEST_CASE("Parser: Function calls error cases",
          "[parser][expressions][function][calls][error]") {
  SECTION("Missing closing parenthesis") {
    auto fixture = createParserFixture("myFunc(1, 2, 3");
    auto *expr = fixture->parseExpression();
    REQUIRE(expr == nullptr);
    REQUIRE(fixture->hasErrors());
  }

  SECTION("Missing comma between arguments") {
    auto fixture = createParserFixture("myFunc(1 2 3)");
    auto *expr = fixture->parseExpression();
    REQUIRE(expr == nullptr);
    REQUIRE(fixture->hasErrors());
  }

  SECTION("Trailing comma - should be allowed") {
    auto fixture = createParserFixture("myFunc(1, 2, 3,)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr, "(CallExpr (Identifier myFunc) (Int 1) (Int 2) (Int 3))");
  }

  SECTION("Double comma") {
    auto fixture = createParserFixture("myFunc(1,, 2)");
    auto *expr = fixture->parseExpression();
    REQUIRE(expr == nullptr);
    REQUIRE(fixture->hasErrors());
  }

  SECTION("Empty argument with comma") {
    auto fixture = createParserFixture("myFunc(, 1)");
    auto *expr = fixture->parseExpression();
    REQUIRE(expr == nullptr);
    REQUIRE(fixture->hasErrors());
  }
}

TEST_CASE("Parser: Function call chaining precedence",
          "[parser][expressions][function][calls][precedence]") {
  SECTION("Function call has higher precedence than arithmetic") {
    auto fixture = createParserFixture("myFunc() + 1");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr +
        (CallExpr (Identifier myFunc))
        (Int 1))
    )");
  }

  SECTION("Arithmetic expressions as function arguments") {
    auto fixture = createParserFixture("myFunc(1 + 2)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (CallExpr
        (Identifier myFunc)
        (BinaryExpr +
          (Int 1)
          (Int 2)))
    )");
  }

  SECTION("Complex precedence with function calls") {
    auto fixture = createParserFixture("myFunc(a * b) + g(x, y)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr +
        (CallExpr
          (Identifier myFunc)
          (BinaryExpr *
            (Identifier a)
            (Identifier b)))
        (CallExpr
          (Identifier g)
          (Identifier x)
          (Identifier y)))
    )");
  }
}
