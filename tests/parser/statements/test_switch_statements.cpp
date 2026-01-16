#include "catch2.hpp"
#include "../../parser_test_utils.hpp"
#include "cxy/ast/statements.hpp"
#include "cxy/ast/declarations.hpp"
#include "cxy/ast/literals.hpp"
#include "cxy/ast/expressions.hpp"
#include "cxy/ast/identifiers.hpp"

using namespace cxy;
using namespace cxy::ast;
using namespace cxy::test;

TEST_CASE("Switch Statement Parsing - Basic Single Values", "[parser][statements][switch-stmt]") {
    SECTION("switch value { 0 => println(\"zero\") }") {
        auto fixture = createParserFixture("switch value { 0 => println(\"zero\") }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astSwitchStmt);

        auto *switchStmt = static_cast<SwitchStatementNode *>(stmt);
        REQUIRE(switchStmt->discriminant != nullptr);
        REQUIRE(switchStmt->discriminant->kind == astIdentifier);
        REQUIRE(switchStmt->cases.size() == 1);

        auto *caseStmt = static_cast<CaseStatementNode *>(switchStmt->cases[0]);
        REQUIRE(caseStmt->kind == astCaseStmt);
        REQUIRE(caseStmt->values.size() == 1);
        REQUIRE(caseStmt->values[0]->kind == astInt);
        REQUIRE(caseStmt->statements.size() == 1);
        REQUIRE(caseStmt->isDefault == false);

        REQUIRE_AST_MATCHES(stmt, R"((SwitchStmt
  (Identifier value)
  (CaseStmt
    (Int 0)
    (ExprStmt
      (CallExpr
        (Identifier println)
        (String "zero"))))))");
    }

    SECTION("switch (value) { 1 => { println(\"one\") } }") {
        auto fixture = createParserFixture("switch (value) { 1 => { println(\"one\") } }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astSwitchStmt);

        auto *switchStmt = static_cast<SwitchStatementNode *>(stmt);
        REQUIRE(switchStmt->discriminant != nullptr);
        REQUIRE(switchStmt->cases.size() == 1);

        auto *caseStmt = static_cast<CaseStatementNode *>(switchStmt->cases[0]);
        REQUIRE(caseStmt->values.size() == 1);
        REQUIRE(caseStmt->values[0]->kind == astInt);
        REQUIRE(caseStmt->statements.size() == 1);
        REQUIRE(caseStmt->statements[0]->kind == astBlockStmt);

        REQUIRE_AST_MATCHES(stmt, R"((SwitchStmt
  (Identifier value)
  (CaseStmt
    (Int 1)
    (BlockStmt
      (ExprStmt
        (CallExpr
          (Identifier println)
          (String "one")))))))");
    }
}

TEST_CASE("Switch Statement Parsing - Multiple Values Per Case", "[parser][statements][switch-stmt]") {
    SECTION("switch code { 0, 1, 2 => println(\"success\") }") {
        auto fixture = createParserFixture("switch code { 0, 1, 2 => println(\"success\") }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astSwitchStmt);

        auto *switchStmt = static_cast<SwitchStatementNode *>(stmt);
        REQUIRE(switchStmt->cases.size() == 1);

        auto *caseStmt = static_cast<CaseStatementNode *>(switchStmt->cases[0]);
        REQUIRE(caseStmt->values.size() == 3);
        REQUIRE(caseStmt->values[0]->kind == astInt);
        REQUIRE(caseStmt->values[1]->kind == astInt);
        REQUIRE(caseStmt->values[2]->kind == astInt);

        REQUIRE_AST_MATCHES(stmt, R"((SwitchStmt
  (Identifier code)
  (CaseStmt
    (Int 0)
    (Int 1)
    (Int 2)
    (ExprStmt
      (CallExpr
        (Identifier println)
        (String "success"))))))");
    }

    SECTION("switch status { \"ok\", \"success\" => handleGood() \"error\", \"fail\" => handleBad() }") {
        auto fixture = createParserFixture("switch status { \"ok\", \"success\" => handleGood() \"error\", \"fail\" => handleBad() }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astSwitchStmt);

        auto *switchStmt = static_cast<SwitchStatementNode *>(stmt);
        REQUIRE(switchStmt->cases.size() == 2);

        // First case: "ok", "success"
        auto *case1 = static_cast<CaseStatementNode *>(switchStmt->cases[0]);
        REQUIRE(case1->values.size() == 2);
        REQUIRE(case1->values[0]->kind == astString);
        REQUIRE(case1->values[1]->kind == astString);

        // Second case: "error", "fail"
        auto *case2 = static_cast<CaseStatementNode *>(switchStmt->cases[1]);
        REQUIRE(case2->values.size() == 2);
        REQUIRE(case2->values[0]->kind == astString);
        REQUIRE(case2->values[1]->kind == astString);

        REQUIRE_AST_MATCHES(stmt, R"((SwitchStmt
  (Identifier status)
  (CaseStmt
    (String "ok")
    (String "success")
    (ExprStmt
      (CallExpr
        (Identifier handleGood))))
  (CaseStmt
    (String "error")
    (String "fail")
    (ExprStmt
      (CallExpr
        (Identifier handleBad))))))");
    }
}

TEST_CASE("Switch Statement Parsing - Range Expressions", "[parser][statements][switch-stmt]") {
    SECTION("switch score { 0..59 => println(\"F\") 90..100 => println(\"A\") }") {
        auto fixture = createParserFixture("switch score { 0..59 => println(\"F\") 90..100 => println(\"A\") }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astSwitchStmt);

        auto *switchStmt = static_cast<SwitchStatementNode *>(stmt);
        REQUIRE(switchStmt->cases.size() == 2);

        // First case: 0..59
        auto *case1 = static_cast<CaseStatementNode *>(switchStmt->cases[0]);
        REQUIRE(case1->values.size() == 1);
        REQUIRE(case1->values[0]->kind == astRangeExpr);

        // Second case: 90..100
        auto *case2 = static_cast<CaseStatementNode *>(switchStmt->cases[1]);
        REQUIRE(case2->values.size() == 1);
        REQUIRE(case2->values[0]->kind == astRangeExpr);

        REQUIRE_AST_MATCHES(stmt, R"((SwitchStmt
  (Identifier score)
  (CaseStmt
    (RangeExpr ..
      (Int 0)
      (Int 59))
    (ExprStmt
      (CallExpr
        (Identifier println)
        (String "F"))))
  (CaseStmt
    (RangeExpr ..
      (Int 90)
      (Int 100))
    (ExprStmt
      (CallExpr
        (Identifier println)
        (String "A"))))))");
    }

    SECTION("switch value { 1, 5..10, 20 => process() }") {
        auto fixture = createParserFixture("switch value { 1, 5..10, 20 => process() }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astSwitchStmt);

        auto *switchStmt = static_cast<SwitchStatementNode *>(stmt);
        REQUIRE(switchStmt->cases.size() == 1);

        auto *caseStmt = static_cast<CaseStatementNode *>(switchStmt->cases[0]);
        REQUIRE(caseStmt->values.size() == 3);
        REQUIRE(caseStmt->values[0]->kind == astInt);
        REQUIRE(caseStmt->values[1]->kind == astRangeExpr);
        REQUIRE(caseStmt->values[2]->kind == astInt);

        REQUIRE_AST_MATCHES(stmt, R"((SwitchStmt
  (Identifier value)
  (CaseStmt
    (Int 1)
    (RangeExpr ..
      (Int 5)
      (Int 10))
    (Int 20)
    (ExprStmt
      (CallExpr
        (Identifier process))))))");
    }
}

TEST_CASE("Switch Statement Parsing - Default Cases", "[parser][statements][switch-stmt]") {
    SECTION("switch value { 0 => handleZero() ... => handleDefault() }") {
        auto fixture = createParserFixture("switch value { 0 => handleZero() ... => handleDefault() }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astSwitchStmt);

        auto *switchStmt = static_cast<SwitchStatementNode *>(stmt);
        REQUIRE(switchStmt->cases.size() == 2);

        // First case: regular case
        auto *case1 = static_cast<CaseStatementNode *>(switchStmt->cases[0]);
        REQUIRE(case1->isDefault == false);
        REQUIRE(case1->values.size() == 1);

        // Second case: default case
        auto *case2 = static_cast<CaseStatementNode *>(switchStmt->cases[1]);
        REQUIRE(case2->isDefault == true);
        REQUIRE(case2->values.size() == 0);

        REQUIRE_AST_MATCHES(stmt, R"((SwitchStmt
  (Identifier value)
  (CaseStmt
    (Int 0)
    (ExprStmt
      (CallExpr
        (Identifier handleZero))))
  (CaseStmt default
    (ExprStmt
      (CallExpr
        (Identifier handleDefault))))))");
    }

    SECTION("switch status { ... => { println(\"unknown\") return null } }") {
        auto fixture = createParserFixture("switch status { ... => { println(\"unknown\") return null } }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astSwitchStmt);

        auto *switchStmt = static_cast<SwitchStatementNode *>(stmt);
        REQUIRE(switchStmt->cases.size() == 1);

        auto *caseStmt = static_cast<CaseStatementNode *>(switchStmt->cases[0]);
        REQUIRE(caseStmt->isDefault == true);
        REQUIRE(caseStmt->values.size() == 0);
        REQUIRE(caseStmt->statements.size() == 1);
        REQUIRE(caseStmt->statements[0]->kind == astBlockStmt);

        REQUIRE_AST_MATCHES(stmt, R"((SwitchStmt
  (Identifier status)
  (CaseStmt default
    (BlockStmt
      (ExprStmt
        (CallExpr
          (Identifier println)
          (String "unknown")))
      (ReturnStmt
        (Null))))))");
    }
}

TEST_CASE("Switch Statement Parsing - Variable Declarations", "[parser][statements][switch-stmt]") {
    SECTION("switch var result = compute() { 0 => useResult(result) ... => handleError(result) }") {
        auto fixture = createParserFixture("switch var result = compute() { 0 => useResult(result) ... => handleError(result) }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astSwitchStmt);

        auto *switchStmt = static_cast<SwitchStatementNode *>(stmt);
        REQUIRE(switchStmt->discriminant != nullptr);
        REQUIRE(switchStmt->discriminant->kind == astVariableDeclaration);
        REQUIRE(switchStmt->cases.size() == 2);

        REQUIRE_AST_MATCHES(stmt, R"((SwitchStmt
  (VariableDeclaration (Identifier result) (CallExpr (Identifier compute)))
  (CaseStmt
    (Int 0)
    (ExprStmt
      (CallExpr
        (Identifier useResult)
        (Identifier result))))
  (CaseStmt default
    (ExprStmt
      (CallExpr
        (Identifier handleError)
        (Identifier result))))))");
    }

    SECTION("switch (const status = getStatus()) { \"ok\" => handleOk(status) \"error\" => handleError(status) }") {
        auto fixture = createParserFixture("switch (const status = getStatus()) { \"ok\" => handleOk(status) \"error\" => handleError(status) }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astSwitchStmt);

        auto *switchStmt = static_cast<SwitchStatementNode *>(stmt);
        REQUIRE(switchStmt->discriminant != nullptr);
        REQUIRE(switchStmt->discriminant->kind == astVariableDeclaration);
        REQUIRE(switchStmt->cases.size() == 2);

        REQUIRE_AST_MATCHES(stmt, R"((SwitchStmt
  (VariableDeclaration (Identifier status) (CallExpr (Identifier getStatus)))
  (CaseStmt
    (String "ok")
    (ExprStmt
      (CallExpr
        (Identifier handleOk)
        (Identifier status))))
  (CaseStmt
    (String "error")
    (ExprStmt
      (CallExpr
        (Identifier handleError)
        (Identifier status))))))");
    }
}

TEST_CASE("Switch Statement Parsing - Complex Cases", "[parser][statements][switch-stmt]") {
    SECTION("Multi-case switch with mixed types") {
        auto fixture = createParserFixture(R"(
            switch operation {
                "add" => {
                    result = a + b
                    println("Addition: {result}")
                }
                "subtract", "sub" => {
                    result = a - b
                    println("Subtraction: {result}")
                }
                1..10 => println("Numeric mode")
                ... => {
                    println("Unknown operation")
                    return null
                }
            }
        )");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astSwitchStmt);

        auto *switchStmt = static_cast<SwitchStatementNode *>(stmt);
        REQUIRE(switchStmt->cases.size() == 4);

        // First case: "add"
        auto *case1 = static_cast<CaseStatementNode *>(switchStmt->cases[0]);
        REQUIRE(case1->values.size() == 1);
        REQUIRE(case1->isDefault == false);

        // Second case: "subtract", "sub"
        auto *case2 = static_cast<CaseStatementNode *>(switchStmt->cases[1]);
        REQUIRE(case2->values.size() == 2);
        REQUIRE(case2->isDefault == false);

        // Third case: 1..10
        auto *case3 = static_cast<CaseStatementNode *>(switchStmt->cases[2]);
        REQUIRE(case3->values.size() == 1);
        REQUIRE(case3->values[0]->kind == astRangeExpr);
        REQUIRE(case3->isDefault == false);

        // Fourth case: default
        auto *case4 = static_cast<CaseStatementNode *>(switchStmt->cases[3]);
        REQUIRE(case4->values.size() == 0);
        REQUIRE(case4->isDefault == true);
    }

    SECTION("Trailing commas in value lists") {
        auto fixture = createParserFixture("switch value { 1, 2, 3, => handleNumbers() }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astSwitchStmt);

        auto *switchStmt = static_cast<SwitchStatementNode *>(stmt);
        REQUIRE(switchStmt->cases.size() == 1);

        auto *caseStmt = static_cast<CaseStatementNode *>(switchStmt->cases[0]);
        REQUIRE(caseStmt->values.size() == 3);
    }
}

TEST_CASE("Switch Statement Parsing - Error Cases", "[parser][statements][switch-stmt][errors]") {
    SECTION("Missing arrow") {
        auto fixture = createParserFixture("switch value { 0 println(\"zero\") }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Missing case body") {
        auto fixture = createParserFixture("switch value { 0 => }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Missing braces") {
        auto fixture = createParserFixture("switch value 0 => println(\"zero\")");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Empty case pattern") {
        auto fixture = createParserFixture("switch value { => println(\"empty\") }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Missing discriminant") {
        auto fixture = createParserFixture("switch { 0 => println(\"zero\") }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Invalid variable declaration") {
        auto fixture = createParserFixture("switch var = getValue() { 0 => handle() }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Malformed case values") {
        auto fixture = createParserFixture("switch value { 1,, 2 => handle() }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Missing case body with default") {
        auto fixture = createParserFixture("switch value { ... => }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }
}
