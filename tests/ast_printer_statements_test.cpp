#include "cxy/ast/printer.hpp"
#include "cxy/ast/statements.hpp"
#include "cxy/ast/literals.hpp"
#include "cxy/ast/identifiers.hpp"
#include "cxy/memory/arena.hpp"
#include "cxy/flags.hpp"
#include "cxy/strings.hpp"
#include "catch2.hpp"

using namespace cxy;
using namespace cxy::ast;

class StatementPrinterTestFixture {
public:
    ArenaAllocator arena{1024 * 1024};
    StringInterner interner{arena};
    ASTPrinter printer;

    StatementPrinterTestFixture() {
        PrinterConfig config;
        config.flags = PrinterFlags::None;  // Let framework handle closing and spacing
        printer.setConfig(config);
    }

    Location createLocation() {
        return Location{"<test>", Position{1, 1, 0}, Position{1, 1, 0}};
    }
};

TEST_CASE("AST Printer: Break and Continue statements", "[ast][printer][statements]") {
    StatementPrinterTestFixture fixture;
    auto loc = fixture.createLocation();

    SECTION("Break statement") {
        auto *breakStmt = createBreakStatement(loc, fixture.arena);
        std::string output = fixture.printer.print(breakStmt);
        REQUIRE(output == "(BreakStmt)");
    }

    SECTION("Continue statement") {
        auto *continueStmt = createContinueStatement(loc, fixture.arena);
        std::string output = fixture.printer.print(continueStmt);
        REQUIRE(output == "(ContinueStmt)");
    }
}

TEST_CASE("AST Printer: Return and Yield statements", "[ast][printer][statements]") {
    StatementPrinterTestFixture fixture;
    auto loc = fixture.createLocation();

    SECTION("Return statement without expression") {
        auto *returnStmt = createReturnStatement(loc, fixture.arena);
        std::string output = fixture.printer.print(returnStmt);
        REQUIRE(output == "(ReturnStmt)");
    }

    SECTION("Return statement with expression") {
        auto *literal = createIntLiteral(42, loc, fixture.arena);
        auto *returnStmt = createReturnStatement(loc, fixture.arena, literal);
        std::string output = fixture.printer.print(returnStmt);
        REQUIRE(output == R"((ReturnStmt
  (Int 42)))");
    }

    SECTION("Yield statement without expression") {
        auto *yieldStmt = createYieldStatement(loc, fixture.arena);
        std::string output = fixture.printer.print(yieldStmt);
        REQUIRE(output == "(YieldStmt)");
    }

    SECTION("Yield statement with expression") {
        auto *literal = createBoolLiteral(true, loc, fixture.arena);
        auto *yieldStmt = createYieldStatement(loc, fixture.arena, literal);
        std::string output = fixture.printer.print(yieldStmt);
        REQUIRE(output == R"((YieldStmt
  (Bool true)))");
    }
}

TEST_CASE("AST Printer: Expression and Defer statements", "[ast][printer][statements]") {
    StatementPrinterTestFixture fixture;
    auto loc = fixture.createLocation();

    SECTION("Expression statement") {
        auto *literal = createIntLiteral(123, loc, fixture.arena);
        auto *exprStmt = createExprStatement(literal, loc, fixture.arena);
        std::string output = fixture.printer.print(exprStmt);
        REQUIRE(output == R"((ExprStmt
  (Int 123)))");
    }

    SECTION("Defer statement") {
        auto *literal = createStringLiteral(fixture.interner.intern("cleanup"), loc, fixture.arena);
        auto *exprStmt = createExprStatement(literal, loc, fixture.arena);
        auto *deferStmt = createDeferStatement(exprStmt, loc, fixture.arena);
        std::string output = fixture.printer.print(deferStmt);
        REQUIRE(output == R"((DeferStmt
  (ExprStmt
    (String "cleanup"))))");
    }
}

TEST_CASE("AST Printer: Block statement", "[ast][printer][statements]") {
    StatementPrinterTestFixture fixture;
    auto loc = fixture.createLocation();

    SECTION("Empty block") {
        auto *block = createBlockStatement(loc, fixture.arena);
        std::string output = fixture.printer.print(block);
        REQUIRE(output == "(BlockStmt)");
    }

    SECTION("Block with statements") {
        auto *block = createBlockStatement(loc, fixture.arena);
        auto *breakStmt = createBreakStatement(loc, fixture.arena);
        auto *continueStmt = createContinueStatement(loc, fixture.arena);

        block->addStatement(breakStmt);
        block->addStatement(continueStmt);

        std::string output = fixture.printer.print(block);
        REQUIRE(output == R"((BlockStmt
  (BreakStmt)
  (ContinueStmt)))");
    }
}

TEST_CASE("AST Printer: If statement", "[ast][printer][statements]") {
    StatementPrinterTestFixture fixture;
    auto loc = fixture.createLocation();

    SECTION("If statement without else") {
        auto *condition = createBoolLiteral(true, loc, fixture.arena);
        auto *thenBlock = createBlockStatement(loc, fixture.arena);
        auto *ifStmt = createIfStatement(condition, thenBlock, loc, fixture.arena);

        std::string output = fixture.printer.print(ifStmt);
        REQUIRE(output == R"((IfStmt
  (Bool true)
  (BlockStmt)))");
    }

    SECTION("If statement with else") {
        auto *condition = createBoolLiteral(false, loc, fixture.arena);
        auto *thenBlock = createBlockStatement(loc, fixture.arena);
        auto *elseBlock = createBlockStatement(loc, fixture.arena);
        auto *ifStmt = createIfStatement(condition, thenBlock, loc, fixture.arena, elseBlock);

        std::string output = fixture.printer.print(ifStmt);
        REQUIRE(output == R"((IfStmt
  (Bool false)
  (BlockStmt)
  (BlockStmt)))");
    }
}

TEST_CASE("AST Printer: For statement", "[ast][printer][statements]") {
    StatementPrinterTestFixture fixture;
    auto loc = fixture.createLocation();

    SECTION("For statement without condition") {
        auto name = fixture.interner.intern("i");
        auto *variable = createIdentifier(name, loc, fixture.arena);
        auto *range = createIntLiteral(10, loc, fixture.arena);
        auto *body = createBlockStatement(loc, fixture.arena);
        auto *forStmt = createForStatement(range, body, loc, fixture.arena);
        forStmt->addVariable(variable);

        std::string output = fixture.printer.print(forStmt);
        REQUIRE(output == R"((ForStmt
  (Variables i)
  (Int 10)
  (BlockStmt)))");
    }

    SECTION("For statement with condition") {
        auto name = fixture.interner.intern("item");
        auto *variable = createIdentifier(name, loc, fixture.arena);
        auto *range = createIntLiteral(100, loc, fixture.arena);
        auto *condition = createBoolLiteral(true, loc, fixture.arena);
        auto *body = createBlockStatement(loc, fixture.arena);
        auto *forStmt = createForStatement(range, body, loc, fixture.arena, condition);
        forStmt->addVariable(variable);

        ast::ASTPrinter printer({ast::PrinterFlags::CompactMode});
        std::string output = fixture.printer.print(forStmt);
        REQUIRE(output == R"((ForStmt
  (Variables item)
  (Int 100)
  (Bool true)
  (BlockStmt)))");
    }
}

TEST_CASE("AST Printer: While statement", "[ast][printer][statements]") {
    StatementPrinterTestFixture fixture;
    auto loc = fixture.createLocation();

    auto *condition = createBoolLiteral(true, loc, fixture.arena);
    auto *body = createBlockStatement(loc, fixture.arena);
    auto *whileStmt = createWhileStatement(condition, body, loc, fixture.arena);

    std::string output = fixture.printer.print(whileStmt);
    REQUIRE(output == R"((WhileStmt
  (Bool true)
  (BlockStmt)))");
}

TEST_CASE("AST Printer: Switch and Case statements", "[ast][printer][statements]") {
    StatementPrinterTestFixture fixture;
    auto loc = fixture.createLocation();

    SECTION("Empty switch") {
        auto *discriminant = createIntLiteral(42, loc, fixture.arena);
        auto *switchStmt = createSwitchStatement(discriminant, loc, fixture.arena);

        std::string output = fixture.printer.print(switchStmt);
        REQUIRE(output == R"((SwitchStmt
  (Int 42)))");
    }

    SECTION("Switch with case") {
        auto *discriminant = createIntLiteral(42, loc, fixture.arena);
        auto *switchStmt = createSwitchStatement(discriminant, loc, fixture.arena);

        auto *caseStmt = createCaseStatement(loc, fixture.arena);
        auto *caseValue = createIntLiteral(1, loc, fixture.arena);
        auto *breakStmt = createBreakStatement(loc, fixture.arena);

        caseStmt->addValue(caseValue);
        caseStmt->addStatement(breakStmt);
        switchStmt->addCase(caseStmt);

        std::string output = fixture.printer.print(switchStmt);
        REQUIRE(output == R"((SwitchStmt
  (Int 42)
  (CaseStmt
    (Int 1)
    (BreakStmt))))");
    }

    SECTION("Default case") {
        auto *discriminant = createIntLiteral(42, loc, fixture.arena);
        auto *switchStmt = createSwitchStatement(discriminant, loc, fixture.arena);

        auto *defaultCase = createCaseStatement(loc, fixture.arena, true);
        auto *returnStmt = createReturnStatement(loc, fixture.arena);
        defaultCase->addStatement(returnStmt);
        switchStmt->addCase(defaultCase);

        std::string output = fixture.printer.print(switchStmt);
        REQUIRE(output == R"((SwitchStmt
  (Int 42)
  (CaseStmt default
    (ReturnStmt))))");
    }
}

TEST_CASE("AST Printer: Match statement", "[ast][printer][statements]") {
    StatementPrinterTestFixture fixture;
    auto loc = fixture.createLocation();

    SECTION("Empty match") {
        auto *discriminant = createIntLiteral(5, loc, fixture.arena);
        auto *matchStmt = createMatchStatement(discriminant, loc, fixture.arena);

        std::string output = fixture.printer.print(matchStmt);
        REQUIRE(output == R"((MatchStmt
  (Int 5)))");
    }

    SECTION("Match with patterns") {
        auto *discriminant = createIntLiteral(5, loc, fixture.arena);
        auto *matchStmt = createMatchStatement(discriminant, loc, fixture.arena);

        // Add a pattern (using a case statement as a pattern for now)
        auto *pattern = createCaseStatement(loc, fixture.arena);
        auto *patternValue = createIntLiteral(1, loc, fixture.arena);
        pattern->addValue(patternValue);
        matchStmt->addPattern(pattern);

        std::string output = fixture.printer.print(matchStmt);
        REQUIRE(output == R"((MatchStmt
  (Int 5)
  (CaseStmt
    (Int 1))))");
    }
}

TEST_CASE("AST Printer: Complex nested statements", "[ast][printer][statements]") {
    StatementPrinterTestFixture fixture;
    auto loc = fixture.createLocation();

    // Create: if (true) { return 42; } else { break; }
    auto *condition = createBoolLiteral(true, loc, fixture.arena);

    auto *thenBlock = createBlockStatement(loc, fixture.arena);
    auto *returnStmt = createReturnStatement(loc, fixture.arena, createIntLiteral(42, loc, fixture.arena));
    thenBlock->addStatement(returnStmt);

    auto *elseBlock = createBlockStatement(loc, fixture.arena);
    auto *breakStmt = createBreakStatement(loc, fixture.arena);
    elseBlock->addStatement(breakStmt);

    auto *ifStmt = createIfStatement(condition, thenBlock, loc, fixture.arena, elseBlock);

    std::string output = fixture.printer.print(ifStmt);
    REQUIRE(output == R"((IfStmt
  (Bool true)
  (BlockStmt
    (ReturnStmt
      (Int 42)))
  (BlockStmt
    (BreakStmt))))");
}
