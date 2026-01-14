#include "catch2.hpp"
#include "cxy/ast/literals.hpp"
#include "cxy/ast/printer.hpp"
#include "cxy/diagnostics.hpp"
#include "cxy/memory.hpp"
#include "cxy/strings.hpp"

using namespace cxy;
using namespace cxy::ast;

TEST_CASE("AST Printer - Simple Test", "[ast][printer]") {
  ArenaAllocator arena;
  StringInterner interner(arena);
  Location loc("test.cxy", Position(1, 1, 0), Position(1, 5, 4));

  SECTION("Boolean literal") {
    auto *trueNode = createBoolLiteral(true, loc, arena);

    ASTPrinter printer({PrinterFlags::None});
    std::string result = printer.print(trueNode);

    REQUIRE(result == "(Bool true)");
  }

  SECTION("Integer literal") {
    auto *intNode = createIntLiteral(42, loc, arena);

    ASTPrinter printer({PrinterFlags::None});
    std::string result = printer.print(intNode);

    REQUIRE(result == "(Int 42)");
  }

  SECTION("String literal") {
    InternedString str = interner.intern("hello");
    auto *stringNode = createStringLiteral(str, loc, arena);

    ASTPrinter printer({PrinterFlags::None});
    std::string result = printer.print(stringNode);

    REQUIRE(result == R"((String "hello"))");
  }

  SECTION("Test direct comparison") {
    auto *intNode = createIntLiteral(123, loc, arena);

    ASTPrinter printer({PrinterFlags::None});
    std::string result = printer.print(intNode);

    REQUIRE(result == "(Int 123)");
  }
}
