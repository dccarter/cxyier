#include "catch2.hpp"
#include "cxy/arena_allocator.hpp"
#include "cxy/ast/declarations.hpp"
#include "cxy/ast/identifiers.hpp"
#include "cxy/ast/literals.hpp"
#include "cxy/ast/expressions.hpp"
#include "cxy/ast/printer.hpp"
#include "cxy/strings.hpp"

using namespace cxy;
using namespace cxy::ast;

// Helper function to create test location
Location testLoc() {
    return Location("<test>", Position{1, 1, 0});
}

class VariableDeclPrinterTestFixture {
public:
    ArenaAllocator arena{1024 * 1024};
    StringInterner interner{arena};
    ASTPrinter printer;

    VariableDeclPrinterTestFixture() {
        PrinterConfig config;
        config.flags = PrinterFlags::None;  // Let framework handle closing and spacing
        printer.setConfig(config);
    }

    Location createLocation() {
        return Location{"<test>", Position{1, 1, 0}, Position{1, 1, 0}};
    }
};

TEST_CASE("Variable Declaration Node Creation", "[ast][declarations][variable]") {
    ArenaAllocator arena(1024);
    StringInterner interner(arena);
    Location loc = testLoc();
    
    SECTION("Basic variable declaration creation") {
        auto* varDecl = createVariableDeclaration(loc, arena, false);
        
        REQUIRE(varDecl != nullptr);
        REQUIRE(varDecl->kind == astVariableDeclaration);
        REQUIRE(varDecl->isConst() == false);
        REQUIRE(varDecl->names.empty());
        REQUIRE(varDecl->type == nullptr);
        REQUIRE(varDecl->initializer == nullptr);
    }
    
    SECTION("Constant declaration creation") {
        auto* constDecl = createVariableDeclaration(loc, arena, true);
        
        REQUIRE(constDecl != nullptr);
        REQUIRE(constDecl->kind == astVariableDeclaration);
        REQUIRE(constDecl->isConst() == true);
        REQUIRE(constDecl->names.empty());
        REQUIRE(constDecl->type == nullptr);
        REQUIRE(constDecl->initializer == nullptr);
    }
}

TEST_CASE("Variable Declaration with Names", "[ast][declarations][variable][names]") {
    ArenaAllocator arena(1024);
    StringInterner interner(arena);
    Location loc = testLoc();
    auto* varDecl = createVariableDeclaration(loc, arena, false);
    
    SECTION("Single variable name") {
        auto name = interner.intern("x");
        auto* nameNode = createIdentifier(name, loc, arena);
        
        varDecl->addName(nameNode);
        
        REQUIRE(varDecl->names.size() == 1);
        REQUIRE(varDecl->names[0] == nameNode);
        REQUIRE(varDecl->children.size() == 1);
        REQUIRE(varDecl->children[0] == nameNode);
    }
    
    SECTION("Multiple variable names") {
        auto nameA = interner.intern("a");
        auto nameB = interner.intern("b");
        auto nameC = interner.intern("c");
        
        auto* nodeA = createIdentifier(nameA, loc, arena);
        auto* nodeB = createIdentifier(nameB, loc, arena);
        auto* nodeC = createIdentifier(nameC, loc, arena);
        
        varDecl->addName(nodeA);
        varDecl->addName(nodeB);
        varDecl->addName(nodeC);
        
        REQUIRE(varDecl->names.size() == 3);
        REQUIRE(varDecl->names[0] == nodeA);
        REQUIRE(varDecl->names[1] == nodeB);
        REQUIRE(varDecl->names[2] == nodeC);
        REQUIRE(varDecl->children.size() == 3);
    }
    
    SECTION("Adding null name is ignored") {
        size_t initialSize = varDecl->names.size();
        size_t initialChildren = varDecl->children.size();
        
        varDecl->addName(nullptr);
        
        REQUIRE(varDecl->names.size() == initialSize);
        REQUIRE(varDecl->children.size() == initialChildren);
    }
}

TEST_CASE("Variable Declaration with Type", "[ast][declarations][variable][type]") {
    ArenaAllocator arena(1024);
    StringInterner interner(arena);
    Location loc = testLoc();
    auto* varDecl = createVariableDeclaration(loc, arena, false);
    
    SECTION("Set type annotation") {
        auto typeName = interner.intern("i32");
        auto* typeNode = createIdentifier(typeName, loc, arena);
        
        varDecl->setType(typeNode);
        
        REQUIRE(varDecl->type == typeNode);
        REQUIRE(varDecl->children.size() == 1);
        REQUIRE(varDecl->children[0] == typeNode);
    }
    
    SECTION("Replace type annotation") {
        auto typeName1 = interner.intern("i32");
        auto typeName2 = interner.intern("String");
        
        auto* typeNode1 = createIdentifier(typeName1, loc, arena);
        auto* typeNode2 = createIdentifier(typeName2, loc, arena);
        
        varDecl->setType(typeNode1);
        REQUIRE(varDecl->type == typeNode1);
        REQUIRE(varDecl->children.size() == 1);
        
        varDecl->setType(typeNode2);
        REQUIRE(varDecl->type == typeNode2);
        REQUIRE(varDecl->children.size() == 1);
        REQUIRE(varDecl->children[0] == typeNode2);
    }
    
    SECTION("Clear type annotation") {
        auto typeName = interner.intern("i32");
        auto* typeNode = createIdentifier(typeName, loc, arena);
        
        varDecl->setType(typeNode);
        REQUIRE(varDecl->type == typeNode);
        
        varDecl->setType(nullptr);
        REQUIRE(varDecl->type == nullptr);
        REQUIRE(varDecl->children.empty());
    }
}

TEST_CASE("Variable Declaration with Initializer", "[ast][declarations][variable][initializer]") {
    ArenaAllocator arena(1024);
    StringInterner interner(arena);
    Location loc = testLoc();
    auto* varDecl = createVariableDeclaration(loc, arena, false);
    
    SECTION("Set integer literal initializer") {
        auto* initNode = createIntLiteral(42, loc, arena);
        
        varDecl->setInitializer(initNode);
        
        REQUIRE(varDecl->initializer == initNode);
        REQUIRE(varDecl->children.size() == 1);
        REQUIRE(varDecl->children[0] == initNode);
    }
    
    SECTION("Set expression initializer") {
        auto funcName = interner.intern("getValue");
        auto* funcNode = createIdentifier(funcName, loc, arena);
        auto* callNode = createCallExpr(funcNode, loc, arena);
        
        varDecl->setInitializer(callNode);
        
        REQUIRE(varDecl->initializer == callNode);
        REQUIRE(varDecl->children.size() == 1);
        REQUIRE(varDecl->children[0] == callNode);
    }
    
    SECTION("Replace initializer") {
        auto* init1 = createIntLiteral(42, loc, arena);
        auto* init2 = createIntLiteral(24, loc, arena);
        
        varDecl->setInitializer(init1);
        REQUIRE(varDecl->initializer == init1);
        REQUIRE(varDecl->children.size() == 1);
        
        varDecl->setInitializer(init2);
        REQUIRE(varDecl->initializer == init2);
        REQUIRE(varDecl->children.size() == 1);
        REQUIRE(varDecl->children[0] == init2);
    }
    
    SECTION("Clear initializer") {
        auto* initNode = createIntLiteral(42, loc, arena);
        
        varDecl->setInitializer(initNode);
        REQUIRE(varDecl->initializer == initNode);
        
        varDecl->setInitializer(nullptr);
        REQUIRE(varDecl->initializer == nullptr);
        REQUIRE(varDecl->children.empty());
    }
}

TEST_CASE("Variable Declaration Complete Examples", "[ast][declarations][variable][integration]") {
    ArenaAllocator arena(1024);
    StringInterner interner(arena);
    Location loc = testLoc();
    
    SECTION("var x: i32 = 42") {
        auto* varDecl = createVariableDeclaration(loc, arena, false);
        
        // Add name
        auto varName = interner.intern("x");
        auto* nameNode = createIdentifier(varName, loc, arena);
        varDecl->addName(nameNode);
        
        // Add type
        auto typeName = interner.intern("i32");
        auto* typeNode = createIdentifier(typeName, loc, arena);
        varDecl->setType(typeNode);
        
        // Add initializer
        auto* initNode = createIntLiteral(42, loc, arena);
        varDecl->setInitializer(initNode);
        
        REQUIRE(varDecl->isConst() == false);
        REQUIRE(varDecl->names.size() == 1);
        REQUIRE(varDecl->type != nullptr);
        REQUIRE(varDecl->initializer != nullptr);
        REQUIRE(varDecl->children.size() == 3); // name, type, initializer
    }
    
    SECTION("const PI = 3.14") {
        auto* constDecl = createVariableDeclaration(loc, arena, true);
        
        // Add name
        auto constName = interner.intern("PI");
        auto* nameNode = createIdentifier(constName, loc, arena);
        constDecl->addName(nameNode);
        
        // Add initializer (no type annotation for type inference)
        auto* initNode = createFloatLiteral(3.14, loc, arena);
        constDecl->setInitializer(initNode);
        
        REQUIRE(constDecl->isConst() == true);
        REQUIRE(constDecl->names.size() == 1);
        REQUIRE(constDecl->type == nullptr);
        REQUIRE(constDecl->initializer != nullptr);
        REQUIRE(constDecl->children.size() == 2); // name, initializer
    }
    
    SECTION("var a, b, c = getTuple()") {
        auto* multiDecl = createVariableDeclaration(loc, arena, false);
        
        // Add multiple names
        auto nameA = interner.intern("a");
        auto nameB = interner.intern("b");
        auto nameC = interner.intern("c");
        
        auto* nodeA = createIdentifier(nameA, loc, arena);
        auto* nodeB = createIdentifier(nameB, loc, arena);
        auto* nodeC = createIdentifier(nameC, loc, arena);
        
        multiDecl->addName(nodeA);
        multiDecl->addName(nodeB);
        multiDecl->addName(nodeC);
        
        // Add function call initializer
        auto funcName = interner.intern("getTuple");
        auto* funcNode = createIdentifier(funcName, loc, arena);
        auto* callNode = createCallExpr(funcNode, loc, arena);
        multiDecl->setInitializer(callNode);
        
        REQUIRE(multiDecl->isConst() == false);
        REQUIRE(multiDecl->names.size() == 3);
        REQUIRE(multiDecl->type == nullptr);
        REQUIRE(multiDecl->initializer != nullptr);
        REQUIRE(multiDecl->children.size() == 4); // 3 names + initializer
    }
    
    SECTION("var name: String") {
        auto* uninitDecl = createVariableDeclaration(loc, arena, false);
        
        // Add name
        auto varName = interner.intern("name");
        auto* nameNode = createIdentifier(varName, loc, arena);
        uninitDecl->addName(nameNode);
        
        // Add type annotation
        auto typeName = interner.intern("String");
        auto* typeNode = createIdentifier(typeName, loc, arena);
        uninitDecl->setType(typeNode);
        
        REQUIRE(uninitDecl->isConst() == false);
        REQUIRE(uninitDecl->names.size() == 1);
        REQUIRE(uninitDecl->type != nullptr);
        REQUIRE(uninitDecl->initializer == nullptr);
        REQUIRE(uninitDecl->children.size() == 2); // name, type
    }
}

TEST_CASE("AST Printer: Variable Declaration Basic Forms", "[ast][printer][declarations][variable]") {
    VariableDeclPrinterTestFixture fixture;
    auto loc = fixture.createLocation();

    SECTION("var x") {
        auto* varDecl = createVariableDeclaration(loc, fixture.arena, false);
        auto varName = fixture.interner.intern("x");
        auto* nameNode = createIdentifier(varName, loc, fixture.arena);
        varDecl->addName(nameNode);

        std::string output = fixture.printer.print(varDecl);
        REQUIRE(output == R"((VariableDeclaration
  (Identifier x)))");
    }

    SECTION("const PI") {
        auto* constDecl = createVariableDeclaration(loc, fixture.arena, true);
        auto constName = fixture.interner.intern("PI");
        auto* nameNode = createIdentifier(constName, loc, fixture.arena);
        constDecl->addName(nameNode);

        std::string output = fixture.printer.print(constDecl);
        REQUIRE(output == R"((VariableDeclaration
  (Identifier PI)))");
    }

    SECTION("var x: i32") {
        auto* varDecl = createVariableDeclaration(loc, fixture.arena, false);
        
        auto varName = fixture.interner.intern("x");
        auto* nameNode = createIdentifier(varName, loc, fixture.arena);
        varDecl->addName(nameNode);

        auto typeName = fixture.interner.intern("i32");
        auto* typeNode = createIdentifier(typeName, loc, fixture.arena);
        varDecl->setType(typeNode);

        std::string output = fixture.printer.print(varDecl);
        REQUIRE(output == R"((VariableDeclaration
  (Identifier x)
  (Identifier i32)))");
    }

    SECTION("var x = 42") {
        auto* varDecl = createVariableDeclaration(loc, fixture.arena, false);
        
        auto varName = fixture.interner.intern("x");
        auto* nameNode = createIdentifier(varName, loc, fixture.arena);
        varDecl->addName(nameNode);

        auto* initNode = createIntLiteral(42, loc, fixture.arena);
        varDecl->setInitializer(initNode);

        std::string output = fixture.printer.print(varDecl);
        REQUIRE(output == R"((VariableDeclaration
  (Identifier x)
  (Int 42)))");
    }

    SECTION("const PI = 3.14") {
        auto* constDecl = createVariableDeclaration(loc, fixture.arena, true);
        
        auto constName = fixture.interner.intern("PI");
        auto* nameNode = createIdentifier(constName, loc, fixture.arena);
        constDecl->addName(nameNode);

        auto* initNode = createFloatLiteral(3.14, loc, fixture.arena);
        constDecl->setInitializer(initNode);

        std::string output = fixture.printer.print(constDecl);
        REQUIRE(output == R"((VariableDeclaration
  (Identifier PI)
  (Float 3.14)))");
    }

    SECTION("var x: i32 = 42") {
        auto* varDecl = createVariableDeclaration(loc, fixture.arena, false);
        
        auto varName = fixture.interner.intern("x");
        auto* nameNode = createIdentifier(varName, loc, fixture.arena);
        varDecl->addName(nameNode);

        auto typeName = fixture.interner.intern("i32");
        auto* typeNode = createIdentifier(typeName, loc, fixture.arena);
        varDecl->setType(typeNode);

        auto* initNode = createIntLiteral(42, loc, fixture.arena);
        varDecl->setInitializer(initNode);

        std::string output = fixture.printer.print(varDecl);
        REQUIRE(output == R"((VariableDeclaration
  (Identifier x)
  (Identifier i32)
  (Int 42)))");
    }
}

TEST_CASE("AST Printer: Variable Declaration Multiple Names", "[ast][printer][declarations][variable][multiple]") {
    VariableDeclPrinterTestFixture fixture;
    auto loc = fixture.createLocation();

    SECTION("var a, b, c") {
        auto* multiDecl = createVariableDeclaration(loc, fixture.arena, false);
        
        auto nameA = fixture.interner.intern("a");
        auto nameB = fixture.interner.intern("b");
        auto nameC = fixture.interner.intern("c");
        
        auto* nodeA = createIdentifier(nameA, loc, fixture.arena);
        auto* nodeB = createIdentifier(nameB, loc, fixture.arena);
        auto* nodeC = createIdentifier(nameC, loc, fixture.arena);
        
        multiDecl->addName(nodeA);
        multiDecl->addName(nodeB);
        multiDecl->addName(nodeC);

        std::string output = fixture.printer.print(multiDecl);
        REQUIRE(output == R"((VariableDeclaration
  (Identifier a)
  (Identifier b)
  (Identifier c)))");
    }

    SECTION("var a, b, c = getTuple()") {
        auto* multiDecl = createVariableDeclaration(loc, fixture.arena, false);
        
        auto nameA = fixture.interner.intern("a");
        auto nameB = fixture.interner.intern("b");
        auto nameC = fixture.interner.intern("c");
        
        auto* nodeA = createIdentifier(nameA, loc, fixture.arena);
        auto* nodeB = createIdentifier(nameB, loc, fixture.arena);
        auto* nodeC = createIdentifier(nameC, loc, fixture.arena);
        
        multiDecl->addName(nodeA);
        multiDecl->addName(nodeB);
        multiDecl->addName(nodeC);

        auto funcName = fixture.interner.intern("getTuple");
        auto* funcNode = createIdentifier(funcName, loc, fixture.arena);
        auto* callNode = createCallExpr(funcNode, loc, fixture.arena);
        multiDecl->setInitializer(callNode);

        std::string output = fixture.printer.print(multiDecl);
        REQUIRE(output == R"((VariableDeclaration
  (Identifier a)
  (Identifier b)
  (Identifier c)
  (CallExpr
    (Identifier getTuple))))");
    }

    SECTION("const x, y: String = getValue()") {
        auto* constDecl = createVariableDeclaration(loc, fixture.arena, true);
        
        auto nameX = fixture.interner.intern("x");
        auto nameY = fixture.interner.intern("y");
        
        auto* nodeX = createIdentifier(nameX, loc, fixture.arena);
        auto* nodeY = createIdentifier(nameY, loc, fixture.arena);
        
        constDecl->addName(nodeX);
        constDecl->addName(nodeY);

        auto typeName = fixture.interner.intern("String");
        auto* typeNode = createIdentifier(typeName, loc, fixture.arena);
        constDecl->setType(typeNode);

        auto funcName = fixture.interner.intern("getValue");
        auto* funcNode = createIdentifier(funcName, loc, fixture.arena);
        auto* callNode = createCallExpr(funcNode, loc, fixture.arena);
        constDecl->setInitializer(callNode);

        std::string output = fixture.printer.print(constDecl);
        REQUIRE(output == R"((VariableDeclaration
  (Identifier x)
  (Identifier y)
  (Identifier String)
  (CallExpr
    (Identifier getValue))))");
    }
}

TEST_CASE("AST Printer: Variable Declaration Complex Expressions", "[ast][printer][declarations][variable][complex]") {
    VariableDeclPrinterTestFixture fixture;
    auto loc = fixture.createLocation();

    SECTION("var result = functionCall()") {
        auto* varDecl = createVariableDeclaration(loc, fixture.arena, false);
        
        auto varName = fixture.interner.intern("result");
        auto* nameNode = createIdentifier(varName, loc, fixture.arena);
        varDecl->addName(nameNode);

        auto funcName = fixture.interner.intern("functionCall");
        auto* funcNode = createIdentifier(funcName, loc, fixture.arena);
        auto* callNode = createCallExpr(funcNode, loc, fixture.arena);
        varDecl->setInitializer(callNode);

        std::string output = fixture.printer.print(varDecl);
        REQUIRE(output == R"((VariableDeclaration
  (Identifier result)
  (CallExpr
    (Identifier functionCall))))");
    }

    SECTION("const message: String = \"hello\"") {
        auto* constDecl = createVariableDeclaration(loc, fixture.arena, true);
        
        auto varName = fixture.interner.intern("message");
        auto* nameNode = createIdentifier(varName, loc, fixture.arena);
        constDecl->addName(nameNode);

        auto typeName = fixture.interner.intern("String");
        auto* typeNode = createIdentifier(typeName, loc, fixture.arena);
        constDecl->setType(typeNode);

        auto stringValue = fixture.interner.intern("hello");
        auto* initNode = createStringLiteral(stringValue, loc, fixture.arena);
        constDecl->setInitializer(initNode);

        std::string output = fixture.printer.print(constDecl);
        REQUIRE(output == R"((VariableDeclaration
  (Identifier message)
  (Identifier String)
  (String "hello")))");
    }

    SECTION("var flag = true") {
        auto* varDecl = createVariableDeclaration(loc, fixture.arena, false);
        
        auto varName = fixture.interner.intern("flag");
        auto* nameNode = createIdentifier(varName, loc, fixture.arena);
        varDecl->addName(nameNode);

        auto* initNode = createBoolLiteral(true, loc, fixture.arena);
        varDecl->setInitializer(initNode);

        std::string output = fixture.printer.print(varDecl);
        REQUIRE(output == R"((VariableDeclaration
  (Identifier flag)
  (Bool true)))");
    }
}