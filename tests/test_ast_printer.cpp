#include "ast_test_utils.hpp"
#include "catch2.hpp"
#include "cxy/ast/attributes.hpp"
#include "cxy/ast/expressions.hpp"
#include "cxy/ast/identifiers.hpp"
#include "cxy/ast/literals.hpp"
#include "cxy/ast/printer.hpp"
#include "cxy/diagnostics.hpp"
#include "cxy/memory.hpp"
#include "cxy/strings.hpp"

using namespace cxy;
using namespace cxy::ast;
using namespace cxy::ast::testing;

TEST_CASE("AST Printer - Basic Literals", "[ast][printer]") {
  ArenaAllocator arena;
  StringInterner interner(arena);
  Location loc("test.cxy", Position(1, 1, 0), Position(1, 5, 4));

  SECTION("Boolean literals") {
    auto *trueNode = createBoolLiteral(true, loc, arena);
    auto *falseNode = createBoolLiteral(false, loc, arena);

    ASTPrinter printer({PrinterFlags::None});

    REQUIRE(printer.print(trueNode) == "(Bool true)");
    REQUIRE(printer.print(falseNode) == "(Bool false)");
  }

  SECTION("Integer literals") {
    auto *intNode = createIntLiteral(42, loc, arena);
    auto *negativeNode = createIntLiteral(-123, loc, arena);

    ASTPrinter printer({PrinterFlags::None});

    REQUIRE(printer.print(intNode) == "(Int 42)");
    REQUIRE(printer.print(negativeNode) == "(Int -123)");
  }

  SECTION("Float literals") {
    auto *floatNode = createFloatLiteral(3.14, loc, arena);

    ASTPrinter printer({PrinterFlags::None});

    REQUIRE(printer.print(floatNode) == "(Float 3.14)");
  }

  SECTION("String literals") {
    InternedString str = interner.intern("hello world");
    auto *stringNode = createStringLiteral(str, loc, arena);

    ASTPrinter printer({PrinterFlags::None});

    REQUIRE(printer.print(stringNode) == R"((String "hello world"))");
  }

  SECTION("Character literals") {
    auto *charNode = createCharLiteral('A', loc, arena);
    auto *unicodeNode = createCharLiteral(0x1F680, loc, arena); // Rocket emoji

    ASTPrinter printer({PrinterFlags::None});

    REQUIRE(printer.print(charNode) == "(Char 'A')");
    REQUIRE(printer.print(unicodeNode) == "(Char '\\u{1f680}')");
  }

  SECTION("Null literal") {
    auto *nullNode = createNullLiteral(loc, arena);

    ASTPrinter printer({PrinterFlags::None});

    REQUIRE(printer.print(nullNode) == "(Null)");
  }
}

TEST_CASE("AST Printer - Identifiers", "[ast][printer]") {
  ArenaAllocator arena;
  StringInterner interner(arena);
  Location loc("test.cxy", Position(1, 1, 0), Position(1, 5, 4));

  SECTION("Simple identifier") {
    InternedString name = interner.intern("variable");
    auto *identNode = arena.construct<IdentifierNode>(name, loc, arena);

    ASTPrinter printer({PrinterFlags::None});

    REQUIRE(printer.print(identNode) == "(Identifier variable)");
  }

  SECTION("Qualified path") {
    InternedString name1 = interner.intern("module");
    InternedString name2 = interner.intern("function");

    auto *pathNode = arena.construct<QualifiedPathNode>(loc, arena);
    auto *ident1 = arena.construct<IdentifierNode>(name1, loc, arena);
    auto *ident2 = arena.construct<IdentifierNode>(name2, loc, arena);

    pathNode->addChild(ident1);
    pathNode->addChild(ident2);

    ASTPrinter printer({PrinterFlags::None});
    std::string result = printer.print(pathNode);

    // Should contain both identifiers as children
    REQUIRE(result.find("QualifiedPath") != std::string::npos);
    REQUIRE(result.find("Identifier module") != std::string::npos);
    REQUIRE(result.find("Identifier function") != std::string::npos);
  }
}

TEST_CASE("AST Printer - Binary Expressions", "[ast][printer]") {
  ArenaAllocator arena;
  StringInterner interner(arena);
  Location loc("test.cxy", Position(1, 1, 0), Position(1, 5, 4));

  SECTION("Simple binary expression") {
    InternedString nameA = interner.intern("a");
    InternedString nameB = interner.intern("b");

    auto *leftNode = arena.construct<IdentifierNode>(nameA, loc, arena);
    auto *rightNode = arena.construct<IdentifierNode>(nameB, loc, arena);
    auto *binaryNode = arena.construct<BinaryExpressionNode>(
        leftNode, TokenKind::Plus, rightNode, loc, arena);

    ASTPrinter printer({PrinterFlags::None});
    std::string result = printer.print(binaryNode);

    REQUIRE_AST_MATCHES(binaryNode,
                        "(BinaryExpr + (Identifier a) (Identifier b))");
  }

  SECTION("Nested binary expressions") {
    // Create (a + b) * c
    InternedString nameA = interner.intern("a");
    InternedString nameB = interner.intern("b");
    InternedString nameC = interner.intern("c");

    auto *aNode = arena.construct<IdentifierNode>(nameA, loc, arena);
    auto *bNode = arena.construct<IdentifierNode>(nameB, loc, arena);
    auto *cNode = arena.construct<IdentifierNode>(nameC, loc, arena);

    auto *addNode = arena.construct<BinaryExpressionNode>(
        aNode, TokenKind::Plus, bNode, loc, arena);
    auto *mulNode = arena.construct<BinaryExpressionNode>(
        addNode, TokenKind::Mult, cNode, loc, arena);

    std::string expected = R"(
(BinaryExpr *
  (BinaryExpr +
    (Identifier a)
    (Identifier b))
  (Identifier c))
        )";

    REQUIRE_AST_MATCHES(mulNode, expected);
  }
}

TEST_CASE("AST Printer - Complex Expressions", "[ast][printer]") {
  ArenaAllocator arena;
  StringInterner interner(arena);
  Location loc("test.cxy", Position(1, 1, 0), Position(1, 10, 9));

  SECTION("Function call") {
    InternedString funcName = interner.intern("foo");
    InternedString argName = interner.intern("x");

    auto *funcNode = arena.construct<IdentifierNode>(funcName, loc, arena);
    auto *argNode = arena.construct<IdentifierNode>(argName, loc, arena);
    auto *intArg = createIntLiteral(42, loc, arena);

    auto *callNode = arena.construct<CallExpressionNode>(funcNode, loc, arena);
    callNode->addChild(argNode);
    callNode->addChild(intArg);

    std::string expected = R"(
(CallExpr
  (Identifier foo)
  (Identifier x)
  (Int 42))
        )";

    REQUIRE_AST_MATCHES(callNode, expected);
  }

  SECTION("Array indexing") {
    InternedString arrName = interner.intern("array");
    InternedString idxName = interner.intern("index");

    auto *arrNode = arena.construct<IdentifierNode>(arrName, loc, arena);
    auto *idxNode = arena.construct<IdentifierNode>(idxName, loc, arena);
    auto *indexNode =
        arena.construct<IndexExpressionNode>(arrNode, idxNode, loc, arena);

    std::string expected = R"(
(IndexExpr
  (Identifier array)
  (Identifier index))
        )";

    REQUIRE_AST_MATCHES(indexNode, expected);
  }

  SECTION("Member access") {
    InternedString objName = interner.intern("object");
    InternedString fieldName = interner.intern("field");

    auto *objNode = arena.construct<IdentifierNode>(objName, loc, arena);
    auto *fieldNode = arena.construct<IdentifierNode>(fieldName, loc, arena);
    auto *memberNode = arena.construct<MemberExpressionNode>(objNode, fieldNode,
                                                             false, loc, arena);

    std::string expected = R"(
(MemberExpr
  (Identifier object)
  (Identifier field))
        )";

    REQUIRE_AST_MATCHES(memberNode, expected);
  }
}

TEST_CASE("AST Printer - Configuration Options", "[ast][printer]") {
  ArenaAllocator arena;
  StringInterner interner(arena);
  Location loc("test.cxy", Position(1, 5, 4), Position(1, 10, 9));

  auto *intNode = createIntLiteral(42, loc, arena);

  SECTION("Include location info") {
    PrinterConfig config{PrinterFlags::IncludeLocation};
    ASTPrinter printer(config);

    std::string result = printer.print(intNode);
    REQUIRE(result.find("@1:5-1:10") != std::string::npos);
  }

  SECTION("Compact mode") {
    InternedString nameA = interner.intern("a");
    InternedString nameB = interner.intern("b");

    auto *leftNode = arena.construct<IdentifierNode>(nameA, loc, arena);
    auto *rightNode = arena.construct<IdentifierNode>(nameB, loc, arena);
    auto *binaryNode = arena.construct<BinaryExpressionNode>(
        leftNode, TokenKind::Plus, rightNode, loc, arena);

    PrinterConfig config{PrinterFlags::CompactMode};
    ASTPrinter printer(config);

    std::string result = printer.print(binaryNode);
    REQUIRE(result.find('\n') ==
            std::string::npos); // No newlines in compact mode
  }

  SECTION("Max depth limit") {
    // Create nested expression a + (b + c)
    InternedString nameA = interner.intern("a");
    InternedString nameB = interner.intern("b");
    InternedString nameC = interner.intern("c");

    auto *aNode = arena.construct<IdentifierNode>(nameA, loc, arena);
    auto *bNode = arena.construct<IdentifierNode>(nameB, loc, arena);
    auto *cNode = arena.construct<IdentifierNode>(nameC, loc, arena);

    auto *innerAdd = arena.construct<BinaryExpressionNode>(
        bNode, TokenKind::Plus, cNode, loc, arena);
    auto *outerAdd = arena.construct<BinaryExpressionNode>(
        aNode, TokenKind::Plus, innerAdd, loc, arena);

    PrinterConfig config{PrinterFlags::None};
    config.maxDepth = 2;

    ASTPrinter printer(config);
    std::string result = printer.print(outerAdd);

    REQUIRE(result.find("...") !=
            std::string::npos); // Should truncate deep nodes
  }
}

TEST_CASE("AST Testing Utilities - Normalization", "[ast][testing]") {
  SECTION("Basic whitespace normalization") {
    std::string input =
        "  ( BinaryExpr   +   ( Identifier   a )  ( Identifier   b )  )  ";
    std::string expected = "(BinaryExpr + (Identifier a) (Identifier b))";

    REQUIRE(normalizeSerial(input) == expected);
  }

  SECTION("Preserve string literal contents") {
    std::string input = R"(( String  "hello   world"  ))";
    std::string expected = R"((String "hello   world"))";

    REQUIRE(normalizeSerial(input) == expected);
  }

  SECTION("Handle escape sequences in strings") {
    std::string input = R"(( String  "hello\n\tworld"  ))";
    std::string expected = R"((String "hello\n\tworld"))";

    REQUIRE(normalizeSerial(input) == expected);
  }

  SECTION("Complex nested structure") {
    std::string input = R"(
        ( BinaryExpr   +
          ( BinaryExpr   *
            ( Identifier   x )
            ( Int   2 ) )
          ( Identifier   y ) )
        )";
    std::string expected =
        "(BinaryExpr + (BinaryExpr * (Identifier x) (Int 2)) (Identifier y))";

    REQUIRE(normalizeSerial(input) == expected);
  }
}

TEST_CASE("AST Testing Utilities - S-Expression Parsing", "[ast][testing]") {
  SECTION("Parse simple atom") {
    SExpr result = parseSerial("hello");

    REQUIRE(result.isAtom());
    REQUIRE(result.atom == "hello");
  }

  SECTION("Parse simple list") {
    SExpr result = parseSerial("(hello world)");

    REQUIRE_FALSE(result.isAtom());
    REQUIRE(result.children.size() == 2);
    REQUIRE(result.children[0].atom == "hello");
    REQUIRE(result.children[1].atom == "world");
  }

  SECTION("Parse nested lists") {
    SExpr result = parseSerial("(outer (inner a b) c)");

    REQUIRE_FALSE(result.isAtom());
    REQUIRE(result.children.size() == 3);
    REQUIRE(result.children[0].atom == "outer");
    REQUIRE_FALSE(result.children[1].isAtom());
    REQUIRE(result.children[1].children.size() == 3);
    REQUIRE(result.children[2].atom == "c");
  }

  SECTION("Parse string literals") {
    SExpr result = parseSerial(R"((String "hello world"))");

    REQUIRE_FALSE(result.isAtom());
    REQUIRE(result.children.size() == 2);
    REQUIRE(result.children[0].atom == "String");
    REQUIRE(result.children[1].atom == R"("hello world")");
  }
}

TEST_CASE("AST Testing Utilities - Test Macros", "[ast][testing]") {
  ArenaAllocator arena;
  StringInterner interner(arena);
  Location loc("test.cxy", Position(1, 1, 0), Position(1, 5, 4));

  SECTION("REQUIRE_AST_MATCHES with various formatting") {
    auto *intNode = createIntLiteral(42, loc, arena);

    // All these should pass
    REQUIRE_AST_MATCHES(intNode, "(Int 42)");
    REQUIRE_AST_MATCHES(intNode, "( Int   42 )");
    REQUIRE_AST_MATCHES(intNode, R"(
        ( Int 42 )
        )");
  }

  SECTION("Complex expression matching") {
    InternedString nameA = interner.intern("x");
    InternedString nameB = interner.intern("y");

    auto *leftNode = arena.construct<IdentifierNode>(nameA, loc, arena);
    auto *rightNode = arena.construct<IdentifierNode>(nameB, loc, arena);
    auto *binaryNode = arena.construct<BinaryExpressionNode>(
        leftNode, TokenKind::Plus, rightNode, loc, arena);

    // Various formatting styles should all work
    REQUIRE_AST_MATCHES(binaryNode,
                        "(BinaryExpr + (Identifier x) (Identifier y))");

    REQUIRE_AST_MATCHES(binaryNode, R"(
(BinaryExpr +
  (Identifier x)
  (Identifier y))
        )");

    REQUIRE_AST_MATCHES(
        binaryNode, "( BinaryExpr   +   ( Identifier x )   ( Identifier y ) )");
  }

  SECTION("Basic structural matching") {
    InternedString name = interner.intern("test");
    auto *identNode = arena.construct<IdentifierNode>(name, loc, arena);

    REQUIRE_AST_STRUCTURALLY_MATCHES(identNode, "(Identifier test)");
    REQUIRE_AST_STRUCTURALLY_MATCHES(identNode, "( Identifier   test )");
  }

  SECTION("Complex structural matching") {
    // Create: foo(bar + 42, "hello")
    InternedString funcName = interner.intern("foo");
    InternedString argName = interner.intern("bar");

    auto *funcNode = arena.construct<IdentifierNode>(funcName, loc, arena);
    auto *barNode = arena.construct<IdentifierNode>(argName, loc, arena);
    auto *intNode = createIntLiteral(42, loc, arena);
    auto *strNode = createStringLiteral(interner.intern("hello"), loc, arena);

    auto *addNode = arena.construct<BinaryExpressionNode>(
        barNode, TokenKind::Plus, intNode, loc, arena);

    auto *callNode = arena.construct<CallExpressionNode>(funcNode, loc, arena);
    callNode->addChild(addNode);
    callNode->addChild(strNode);

    // Test structural equivalence with different formatting
    REQUIRE_AST_STRUCTURALLY_MATCHES(callNode, R"(
      (CallExpr
        (Identifier foo)
        (BinaryExpr +
          (Identifier bar)
          (Int 42))
        (String "hello"))
    )");

    REQUIRE_AST_STRUCTURALLY_MATCHES(
        callNode,
        "(CallExpr (Identifier foo) (BinaryExpr + (Identifier bar) (Int 42)) "
        "(String \"hello\"))");

    REQUIRE_AST_STRUCTURALLY_MATCHES(callNode, R"(
      (    CallExpr
        (   Identifier    foo   )
        (   BinaryExpr   +
          (   Identifier   bar   )
          (   Int   42   )   )
        (   String   "hello"   )   )
    )");
  }

  SECTION("Structural vs string comparison differences") {
    // Create a simple expression
    auto *intNode = createIntLiteral(123, loc, arena);

    // String comparison would fail due to extra spaces
    std::string withExtraSpaces = "(    Int     123    )";

    // But structural comparison should succeed
    REQUIRE_AST_STRUCTURALLY_MATCHES(intNode, withExtraSpaces);

    // Verify that the normalized string comparison also works
    REQUIRE_AST_MATCHES(intNode, withExtraSpaces);
  }

  SECTION("Structural comparison with nested expressions") {
    // Create: (a + b) * (c - d)
    InternedString nameA = interner.intern("a");
    InternedString nameB = interner.intern("b");
    InternedString nameC = interner.intern("c");
    InternedString nameD = interner.intern("d");

    auto *aNode = arena.construct<IdentifierNode>(nameA, loc, arena);
    auto *bNode = arena.construct<IdentifierNode>(nameB, loc, arena);
    auto *cNode = arena.construct<IdentifierNode>(nameC, loc, arena);
    auto *dNode = arena.construct<IdentifierNode>(nameD, loc, arena);

    auto *addNode = arena.construct<BinaryExpressionNode>(
        aNode, TokenKind::Plus, bNode, loc, arena);
    auto *subNode = arena.construct<BinaryExpressionNode>(
        cNode, TokenKind::Minus, dNode, loc, arena);
    auto *mulNode = arena.construct<BinaryExpressionNode>(
        addNode, TokenKind::Mult, subNode, loc, arena);

    REQUIRE_AST_STRUCTURALLY_MATCHES(mulNode, R"(
      (BinaryExpr *
        (BinaryExpr +
          (Identifier a)
          (Identifier b))
        (BinaryExpr -
          (Identifier c)
          (Identifier d)))
    )");

    // Test with completely different whitespace formatting
    REQUIRE_AST_STRUCTURALLY_MATCHES(
        mulNode,
        "(BinaryExpr *(BinaryExpr +(Identifier a)(Identifier b))(BinaryExpr "
        "-(Identifier c)(Identifier d)))");
  }

  SECTION("Structural comparison catches differences") {
    // Create: a + b
    InternedString nameA = interner.intern("a");
    InternedString nameB = interner.intern("b");

    auto *aNode = arena.construct<IdentifierNode>(nameA, loc, arena);
    auto *bNode = arena.construct<IdentifierNode>(nameB, loc, arena);
    auto *addNode = arena.construct<BinaryExpressionNode>(
        aNode, TokenKind::Plus, bNode, loc, arena);

    // These should NOT match - different operators
    REQUIRE_FALSE(ASTTestUtils::structurallyMatches(
        addNode, "(BinaryExpr - (Identifier a) (Identifier b))"));

    // These should NOT match - different identifiers
    REQUIRE_FALSE(ASTTestUtils::structurallyMatches(
        addNode, "(BinaryExpr + (Identifier x) (Identifier b))"));

    // These should NOT match - different structure
    REQUIRE_FALSE(ASTTestUtils::structurallyMatches(
        addNode,
        "(BinaryExpr + (Identifier a))")); // Missing second operand

    // These should NOT match - wrong node type
    REQUIRE_FALSE(ASTTestUtils::structurallyMatches(
        addNode, "(CallExpr (Identifier a) (Identifier b))"));
  }

  SECTION("Structural comparison with string literals") {
    InternedString str1 = interner.intern("hello");
    InternedString str2 = interner.intern("world");

    auto *strNode1 = createStringLiteral(str1, loc, arena);
    auto *strNode2 = createStringLiteral(str2, loc, arena);

    // Should match with different whitespace
    REQUIRE_AST_STRUCTURALLY_MATCHES(strNode1, "(String \"hello\")");
    REQUIRE_AST_STRUCTURALLY_MATCHES(strNode1, "( String   \"hello\" )");

    // Should NOT match different content
    REQUIRE_FALSE(
        ASTTestUtils::structurallyMatches(strNode1, "(String \"world\")"));
    REQUIRE_FALSE(ASTTestUtils::structurallyMatches(
        strNode1, "(String \"hello world\")"));
  }
}

TEST_CASE("AST Printer - Statistics", "[ast][printer]") {
  ArenaAllocator arena;
  StringInterner interner(arena);
  Location loc("test.cxy", Position(1, 1, 0), Position(1, 5, 4));

  SECTION("Node visit statistics") {
    // Create a binary expression with 3 nodes total
    InternedString nameA = interner.intern("a");
    InternedString nameB = interner.intern("b");

    auto *leftNode = arena.construct<IdentifierNode>(nameA, loc, arena);
    auto *rightNode = arena.construct<IdentifierNode>(nameB, loc, arena);
    auto *binaryNode = arena.construct<BinaryExpressionNode>(
        leftNode, TokenKind::Plus, rightNode, loc, arena);

    ASTPrinter printer;
    printer.print(binaryNode);

    REQUIRE(printer.getNodesVisited() == 3);
    REQUIRE(printer.getMaxDepthReached() >= 1);
  }
}

TEST_CASE("AST Printer - Utility Functions", "[ast][printer]") {
  ArenaAllocator arena;
  StringInterner interner(arena);
  Location loc("test.cxy", Position(1, 1, 0), Position(1, 5, 4));

  SECTION("printAST utility function") {
    auto *intNode = createIntLiteral(123, loc, arena);

    std::string result = printAST(intNode);
    REQUIRE(result == "(Int 123)");

    // With config
    PrinterConfig configWithLoc;
    configWithLoc.flags = PrinterFlags::IncludeLocation;
    std::string resultWithLoc = printAST(intNode, configWithLoc);
    REQUIRE(resultWithLoc.find("@1:1-1:5") != std::string::npos);
  }

  SECTION("Debug utility") {
    auto *intNode = createIntLiteral(456, loc, arena);

    std::string debug = ASTTestUtils::debug(intNode);
    std::string pretty = ASTTestUtils::pretty(intNode);

    REQUIRE(debug.find("Int 456") != std::string::npos);
    REQUIRE(pretty.find("456") != std::string::npos);
  }
}

TEST_CASE("AST Printer - Error Conditions", "[ast][printer]") {
  SECTION("Null AST") {
    ASTPrinter printer;

    std::string result = printer.print(nullptr);
    REQUIRE(result == "(Null)");

    REQUIRE_FALSE(ASTTestUtils::matches(nullptr, "(Int 42)"));
  }

  SECTION("Invalid S-expression parsing") {
    // Malformed S-expressions should fall back gracefully
    REQUIRE_THROWS(parseSerial("(unclosed"));
    REQUIRE_THROWS(parseSerial("(unterminated \"string"));
  }
}

TEST_CASE("AST Printer - Node Attributes", "[ast][printer][attributes]") {
  ArenaAllocator arena;
  StringInterner interner(arena);
  Location loc("test.cxy", Position(1, 1, 0), Position(1, 5, 4));

  SECTION("Node without attributes") {
    auto *node = createBoolLiteral(true, loc, arena);

    // Without IncludeAttributes flag - no attributes shown
    ASTPrinter printer({PrinterFlags::None});
    REQUIRE(printer.print(node) == "(Bool true)");

    // With IncludeAttributes flag but no attributes - still nothing shown
    ASTPrinter printerWithAttrs({PrinterFlags::IncludeAttributes});
    REQUIRE(printerWithAttrs.print(node) == "(Bool true)");
  }

  SECTION("Node with single attribute") {
    auto *node = createIntLiteral(42, loc, arena);
    auto *attr = createAttribute(interner.intern("Test"), loc, arena);
    node->addAttribute(attr);

    // Without IncludeAttributes flag - attributes not shown
    ASTPrinter printer({PrinterFlags::None});
    REQUIRE(printer.print(node) == "(Int 42)");

    // With IncludeAttributes flag - attributes shown
    ASTPrinter printerWithAttrs({PrinterFlags::IncludeAttributes});
    REQUIRE(printerWithAttrs.print(node) == "(Int 42 [Test])");
  }

  SECTION("Node with multiple attributes") {
    auto *node = createFloatLiteral(3.14, loc, arena);
    auto *attr1 = createAttribute(interner.intern("Test1"), loc, arena);
    auto *attr2 = createAttribute(interner.intern("Test2"), loc, arena);
    auto *attr3 = createAttribute(interner.intern("Test3"), loc, arena);

    node->addAttribute(attr1);
    node->addAttribute(attr2);
    node->addAttribute(attr3);

    ASTPrinter printer({PrinterFlags::IncludeAttributes});
    REQUIRE(printer.print(node) == "(Float 3.14 [Test1 Test2 Test3])");
  }

  SECTION("Attribute management methods") {
    auto *node = createIntLiteral(123, loc, arena);
    auto *attr1 = createAttribute(interner.intern("Attr1"), loc, arena);
    auto *attr2 = createAttribute(interner.intern("Attr2"), loc, arena);

    REQUIRE_FALSE(node->hasAttributes());
    REQUIRE(node->getAttributeCount() == 0);
    REQUIRE(node->getAttribute(0) == nullptr);

    node->addAttribute(attr1);
    REQUIRE(node->hasAttributes());
    REQUIRE(node->getAttributeCount() == 1);
    REQUIRE(node->getAttribute(0) == attr1);

    node->addAttribute(attr2);
    REQUIRE(node->getAttributeCount() == 2);
    REQUIRE(node->getAttribute(1) == attr2);

    REQUIRE(node->removeAttribute(attr1));
    REQUIRE(node->getAttributeCount() == 1);
    REQUIRE(node->getAttribute(0) == attr2);
    REQUIRE_FALSE(node->removeAttribute(attr1)); // Already removed

    // Test adding null attribute (should be ignored)
    node->addAttribute(nullptr);
    REQUIRE(node->getAttributeCount() == 1);
  }

  SECTION("Attributes with positional arguments") {
    auto *node = createArrayExpr(loc, arena);

    // Create attribute with positional literal arguments
    auto *attr = createAttribute(interner.intern("Config"), loc, arena);
    attr->addArg(createIntLiteral(10, loc, arena));
    attr->addArg(createBoolLiteral(true, loc, arena));
    attr->addArg(createStringLiteral(interner.intern("test"), loc, arena));

    node->addAttribute(attr);

    ASTPrinter printer({PrinterFlags::IncludeAttributes});
    std::string result = printer.print(node);
    REQUIRE(result.find("[Config(10 true \"test\")]") != std::string::npos);
  }

  SECTION("Attributes with named arguments") {
    auto *node = createBoolLiteral(false, loc, arena);

    // Create attribute with named field arguments
    auto *attr = createAttribute(interner.intern("Setup"), loc, arena);
    auto *field1 =
        createFieldExpr(createIdentifier(interner.intern("width"), loc, arena),
                        createIntLiteral(800, loc, arena), loc, arena);
    auto *field2 =
        createFieldExpr(createIdentifier(interner.intern("height"), loc, arena),
                        createIntLiteral(600, loc, arena), loc, arena);
    attr->addArg(field1);
    attr->addArg(field2);

    node->addAttribute(attr);

    ASTPrinter printer({PrinterFlags::IncludeAttributes});
    std::string result = printer.print(node);
    REQUIRE(result.find("[Setup(width 800 height 600)]") != std::string::npos);
  }

  SECTION("Combined with other printer flags") {
    auto *node = createBoolLiteral(false, loc, arena);
    auto *attr = createAttribute(interner.intern("Test"), loc, arena);
    node->addAttribute(attr);

    // Combine attributes with location
    ASTPrinter printer(
        {PrinterFlags::IncludeAttributes | PrinterFlags::IncludeLocation});
    std::string result = printer.print(node);
    REQUIRE(result.find("@1:1") != std::string::npos);
    REQUIRE(result.find("[Test]") != std::string::npos);

    // Combine with metadata
    node->setMetadata("test_key", std::string("test_value"));
    ASTPrinter printerWithMeta(
        {PrinterFlags::IncludeAttributes | PrinterFlags::IncludeMetadata});
    std::string resultWithMeta = printerWithMeta.print(node);
    REQUIRE(resultWithMeta.find("[Test]") != std::string::npos);
    REQUIRE(resultWithMeta.find("[metadata=1 entries]") != std::string::npos);
  }

  SECTION("Simple attribute names") {
    auto *node = createIdentifier(interner.intern("myVar"), loc, arena);
    auto *attr = createAttribute(interner.intern("deprecated"), loc, arena);
    node->addAttribute(attr);

    ASTPrinter printer({PrinterFlags::IncludeAttributes});
    REQUIRE(printer.print(node) == "(Identifier myVar [deprecated])");
  }
}
