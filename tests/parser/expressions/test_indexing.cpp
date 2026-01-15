#include "../../parser_test_utils.hpp"

#include "catch2.hpp"

using namespace cxy;
using namespace cxy::test;

TEST_CASE("Parser: Simple array indexing", "[parser][expressions][indexing]") {
  SECTION("Single index access") {
    auto fixture = createParserFixture("array[0]");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(IndexExpr (Identifier array) (Int 0))");
  }

  SECTION("Variable index") {
    auto fixture = createParserFixture("array[i]");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(IndexExpr (Identifier array) (Identifier i))");
  }

  SECTION("Expression index") {
    auto fixture = createParserFixture("array[i + 1]");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (IndexExpr
        (Identifier array)
        (BinaryExpr +
          (Identifier i)
          (Int 1)))
    )");
  }

  SECTION("Complex expression index") {
    auto fixture = createParserFixture("array[x * 2 + y]");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (IndexExpr
        (Identifier array)
        (BinaryExpr +
          (BinaryExpr *
            (Identifier x)
            (Int 2))
          (Identifier y)))
    )");
  }
}

TEST_CASE("Parser: Chained array indexing", "[parser][expressions][indexing]") {
  SECTION("Two-dimensional indexing") {
    auto fixture = createParserFixture("matrix[i][j]");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (IndexExpr
        (IndexExpr
          (Identifier matrix)
          (Identifier i))
        (Identifier j))
    )");
  }

  SECTION("Three-dimensional indexing") {
    auto fixture = createParserFixture("cube[x][y][z]");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (IndexExpr
        (IndexExpr
          (IndexExpr
            (Identifier cube)
            (Identifier x))
          (Identifier y))
        (Identifier z))
    )");
  }

  SECTION("Mixed literal and variable indices") {
    auto fixture = createParserFixture("matrix[0][i]");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (IndexExpr
        (IndexExpr
          (Identifier matrix)
          (Int 0))
        (Identifier i))
    )");
  }
}

TEST_CASE("Parser: Indexing with complex expressions",
          "[parser][expressions][indexing]") {
  SECTION("Indexing function call result") {
    auto fixture = createParserFixture("getArray()[0]");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (IndexExpr
        (CallExpr (Identifier getArray))
        (Int 0))
    )");
  }

  SECTION("Indexing array literal") {
    auto fixture = createParserFixture("[1, 2, 3][i]");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (IndexExpr
        (ArrayExpr (Int 1) (Int 2) (Int 3))
        (Identifier i))
    )");
  }

  SECTION("Indexing tuple literal") {
    auto fixture = createParserFixture("(x, y, z)[1]");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (IndexExpr
        (TupleExpr (Identifier x) (Identifier y) (Identifier z))
        (Int 1))
    )");
  }

  SECTION("Complex chained operations") {
    auto fixture = createParserFixture("getMatrix()[i + 1][j * 2]");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (IndexExpr
        (IndexExpr
          (CallExpr (Identifier getMatrix))
          (BinaryExpr +
            (Identifier i)
            (Int 1)))
        (BinaryExpr *
          (Identifier j)
          (Int 2)))
    )");
  }
}

TEST_CASE("Parser: Indexing with whitespace variations",
          "[parser][expressions][indexing]") {
  SECTION("No spaces around index") {
    auto fixture = createParserFixture("array[index]");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr,
                        "(IndexExpr (Identifier array) (Identifier index))");
  }

  SECTION("Spaces around index") {
    auto fixture = createParserFixture("array[ index ]");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr,
                        "(IndexExpr (Identifier array) (Identifier index))");
  }

  SECTION("Newlines in complex index") {
    auto fixture = createParserFixture("array[\n  i + 1\n]");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (IndexExpr
        (Identifier array)
        (BinaryExpr +
          (Identifier i)
          (Int 1)))
    )");
  }
}

TEST_CASE("Parser: Indexing precedence",
          "[parser][expressions][indexing][precedence]") {
  SECTION("Indexing has higher precedence than arithmetic") {
    auto fixture = createParserFixture("array[i] + 1");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr +
        (IndexExpr
          (Identifier array)
          (Identifier i))
        (Int 1))
    )");
  }

  SECTION("Arithmetic expression as index") {
    auto fixture = createParserFixture("array[i + 1]");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (IndexExpr
        (Identifier array)
        (BinaryExpr +
          (Identifier i)
          (Int 1)))
    )");
  }

  SECTION("Complex precedence with indexing") {
    auto fixture = createParserFixture("array[i] * matrix[j] + 1");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr +
        (BinaryExpr *
          (IndexExpr
            (Identifier array)
            (Identifier i))
          (IndexExpr
            (Identifier matrix)
            (Identifier j)))
        (Int 1))
    )");
  }
}

TEST_CASE("Parser: Indexing error cases",
          "[parser][expressions][indexing][error]") {
  SECTION("Missing closing bracket") {
    auto fixture = createParserFixture("array[0");
    auto *expr = fixture->parseExpression();
    REQUIRE(expr == nullptr);
    REQUIRE(fixture->hasErrors());
  }

  SECTION("Missing index expression") {
    auto fixture = createParserFixture("array[]");
    auto *expr = fixture->parseExpression();
    REQUIRE(expr == nullptr);
    REQUIRE(fixture->hasErrors());
  }

  SECTION("Invalid index expression") {
    auto fixture = createParserFixture("array[,]");
    auto *expr = fixture->parseExpression();
    REQUIRE(expr == nullptr);
    REQUIRE(fixture->hasErrors());
  }

  SECTION("Nested bracket mismatch") {
    auto fixture = createParserFixture("matrix[i][j");
    auto *expr = fixture->parseExpression();
    REQUIRE(expr == nullptr);
    REQUIRE(fixture->hasErrors());
  }
}

TEST_CASE("Parser: Mixed indexing and function calls",
          "[parser][expressions][indexing][function][calls]") {
  SECTION("Index then call") {
    auto fixture = createParserFixture("array[0]()");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (CallExpr
        (IndexExpr
          (Identifier array)
          (Int 0)))
    )");
  }

  SECTION("Call then index") {
    auto fixture = createParserFixture("getArray()[0]");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (IndexExpr
        (CallExpr (Identifier getArray))
        (Int 0))
    )");
  }

  SECTION("Complex chaining") {
    auto fixture = createParserFixture("getMatrix()[i](arg)[j]");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (IndexExpr
        (CallExpr
          (IndexExpr
            (CallExpr (Identifier getMatrix))
            (Identifier i))
          (Identifier arg))
        (Identifier j))
    )");
  }

  SECTION("Function call with indexed arguments") {
    auto fixture = createParserFixture("myFunc(array[0], matrix[i][j])");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (CallExpr
        (Identifier myFunc)
        (IndexExpr
          (Identifier array)
          (Int 0))
        (IndexExpr
          (IndexExpr
            (Identifier matrix)
            (Identifier i))
          (Identifier j)))
    )");
  }
}
