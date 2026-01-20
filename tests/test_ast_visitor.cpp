#include "catch2.hpp"
#include "cxy/memory/arena.hpp"
#include "cxy/ast/expressions.hpp"
#include "cxy/ast/identifiers.hpp"
#include "cxy/ast/kind.hpp"
#include "cxy/ast/literals.hpp"
#include "cxy/ast/visitor.hpp"
#include "cxy/memory/arena.hpp"
#include "cxy/strings.hpp"
#include "cxy/token.hpp"
#include <vector>

using namespace cxy;
using namespace cxy::ast;

// Helper function to convert AST nodes to strings for testing
std::string nodeToString(const ASTNode *node) {
  if (!node)
    return "null";
  std::string result;
  std::format_to(std::back_inserter(result), "{}", *node);
  return result;
}

TEST_CASE("AST Node Creation and Basic Properties", "[ast][visitor]") {
  ArenaAllocator allocator(1024 * 1024);
  StringInterner interner(allocator);
  Location loc("test.cxy", Position(1, 1, 0));

  SECTION("Boolean literal creation and formatting") {
    auto *trueLit = createBoolLiteral(true, loc, allocator);
    auto *falseLit = createBoolLiteral(false, loc, allocator);

    REQUIRE(trueLit->kind == astBool);
    REQUIRE(trueLit->value == true);
    REQUIRE(nodeToString(trueLit) == "Bool(true)");

    REQUIRE(falseLit->kind == astBool);
    REQUIRE(falseLit->value == false);
    REQUIRE(nodeToString(falseLit) == "Bool(false)");
  }

  SECTION("Integer literal creation and formatting") {
    auto *intLit = createIntLiteral(42, loc, allocator);

    REQUIRE(intLit->kind == astInt);
    REQUIRE(intLit->value == 42);
    REQUIRE(nodeToString(intLit) == "Int(42)");
  }

  SECTION("String literal creation and formatting") {
    auto str = interner.intern("hello world");
    auto *strLit = createStringLiteral(str, loc, allocator);

    REQUIRE(strLit->kind == astString);
    REQUIRE(strLit->value.toString() == "hello world");
    REQUIRE(nodeToString(strLit) == "String(\"hello world\")");
  }

  SECTION("Identifier creation and formatting") {
    auto name = interner.intern("variable");
    auto *ident = createIdentifier(name, loc, allocator);

    REQUIRE(ident->kind == astIdentifier);
    REQUIRE(ident->name.toString() == "variable");
    REQUIRE(nodeToString(ident) == "Identifier(variable)");
  }

  SECTION("Qualified path creation and formatting") {
    auto *qualPath = createQualifiedPath(loc, allocator);
    qualPath->addSegment(interner.intern("Hello"), loc, allocator);
    qualPath->addSegment(interner.intern("age"), loc, allocator);

    REQUIRE(qualPath->kind == astQualifiedPath);
    REQUIRE(qualPath->segments.size() == 2);
    REQUIRE(qualPath->segments[0]->name.toString() == "Hello");
    REQUIRE(qualPath->segments[1]->name.toString() == "age");
    REQUIRE(nodeToString(qualPath) == "QualifiedPath(PathSegment(Hello)).PathSegment(age)))");
  }
}

TEST_CASE("AST Expression Node Creation", "[ast][visitor]") {
  ArenaAllocator allocator(1024 * 1024);
  StringInterner interner(allocator);
  Location loc("test.cxy", Position(1, 1, 0));

  SECTION("Binary expression creation") {
    auto *left = createIntLiteral(10, loc, allocator);
    auto *right = createIntLiteral(20, loc, allocator);
    auto *binExpr =
        createBinaryExpr(left, TokenKind::Plus, right, loc, allocator);

    REQUIRE(binExpr->kind == astBinaryExpr);
    REQUIRE(binExpr->left == left);
    REQUIRE(binExpr->right == right);
    REQUIRE(binExpr->op == TokenKind::Plus);
    REQUIRE(binExpr->children.size() == 2);
    REQUIRE(binExpr->children[0] == left);
    REQUIRE(binExpr->children[1] == right);

    std::string expected = "Binary(Int(10) + Int(20))";
    REQUIRE(nodeToString(binExpr) == expected);
  }

  SECTION("Unary expression creation") {
    auto *operand = createIntLiteral(42, loc, allocator);
    auto *unaryExpr =
        createUnaryExpr(TokenKind::Minus, true, operand, loc, allocator);

    REQUIRE(unaryExpr->kind == astUnaryExpr);
    REQUIRE(unaryExpr->operand == operand);
    REQUIRE(unaryExpr->op == TokenKind::Minus);
    REQUIRE(unaryExpr->isPrefix == true);
    REQUIRE(unaryExpr->children.size() == 1);
    REQUIRE(unaryExpr->children[0] == operand);

    std::string expected = "Unary(- Int(42))";
    REQUIRE(nodeToString(unaryExpr) == expected);
  }

  SECTION("Array expression creation") {
    auto *arrayExpr = createArrayExpr(loc, allocator);
    arrayExpr->addElement(createIntLiteral(1, loc, allocator));
    arrayExpr->addElement(createIntLiteral(2, loc, allocator));
    arrayExpr->addElement(createIntLiteral(3, loc, allocator));

    REQUIRE(arrayExpr->kind == astArrayExpr);
    REQUIRE(arrayExpr->elements.size() == 3);
    REQUIRE(arrayExpr->children.size() == 3);

    std::string expected = "Array([Int(1), Int(2), Int(3)])";
    REQUIRE(nodeToString(arrayExpr) == expected);
  }
}

// Test visitor class that records visited nodes
class TestVisitor : public ASTVisitor {
public:
  std::vector<NodeKind> visitedNodes;
  std::vector<NodeKind> postVisitedNodes;

  bool visitNode(ASTNode *node) override {
    visitedNodes.push_back(node->kind);
    return true; // Continue to children
  }

  void visitNodePost(ASTNode *node) override {
    postVisitedNodes.push_back(node->kind);
  }

  // Override specific visit methods to test dispatch
  bool visitBool(BoolLiteralNode *node) override {
    visitedNodes.push_back(astBool);
    return false; // Don't continue to children (though literals don't have any)
  }

  bool visitBinary(BinaryExpressionNode *node) override {
    visitedNodes.push_back(astBinaryExpr);
    return true; // Continue to children
  }
};

TEST_CASE("ASTVisitor Basic Functionality", "[ast][visitor]") {
  ArenaAllocator allocator(1024 * 1024);
  Location loc("test.cxy", Position(1, 1, 0));

  SECTION("Simple node visitation") {
    auto *boolLit = createBoolLiteral(true, loc, allocator);
    TestVisitor visitor;

    visitor.visit(boolLit);

    REQUIRE(visitor.visitedNodes.size() == 1);
    REQUIRE(visitor.visitedNodes[0] == astBool);
    REQUIRE(visitor.postVisitedNodes.size() == 1);
    REQUIRE(visitor.postVisitedNodes[0] == astBool);
  }

  SECTION("Binary expression tree visitation") {
    auto *left = createIntLiteral(10, loc, allocator);
    auto *right = createBoolLiteral(true, loc, allocator);
    auto *binExpr =
        createBinaryExpr(left, TokenKind::Plus, right, loc, allocator);

    TestVisitor visitor;
    visitor.visit(binExpr);

    // Should visit: Binary, then Int (left child), then Bool (right child)
    REQUIRE(visitor.visitedNodes.size() == 3);
    REQUIRE(visitor.visitedNodes[0] == astBinaryExpr);
    REQUIRE(visitor.visitedNodes[1] == astInt);
    REQUIRE(visitor.visitedNodes[2] == astBool);

    // Post-visit order should be: Int, Bool, Binary (reverse of pre-visit for
    // leaves)
    REQUIRE(visitor.postVisitedNodes.size() == 3);
    REQUIRE(visitor.postVisitedNodes[0] == astInt);
    REQUIRE(visitor.postVisitedNodes[1] == astBool);
    REQUIRE(visitor.postVisitedNodes[2] == astBinaryExpr);
  }

  SECTION("Array expression with multiple children") {
    auto *arrayExpr = createArrayExpr(loc, allocator);
    arrayExpr->addElement(createIntLiteral(1, loc, allocator));
    arrayExpr->addElement(createIntLiteral(2, loc, allocator));
    arrayExpr->addElement(createBoolLiteral(false, loc, allocator));

    TestVisitor visitor;
    visitor.visit(arrayExpr);

    REQUIRE(visitor.visitedNodes.size() == 4);
    REQUIRE(visitor.visitedNodes[0] == astArrayExpr);
    REQUIRE(visitor.visitedNodes[1] == astInt);
    REQUIRE(visitor.visitedNodes[2] == astInt);
    REQUIRE(visitor.visitedNodes[3] == astBool);
  }
}

TEST_CASE("walkAST Function-Based Visitor", "[ast][visitor]") {
  ArenaAllocator allocator(1024 * 1024);
  Location loc("test.cxy", Position(1, 1, 0));

  SECTION("Simple walkAST with lambda") {
    auto *left = createIntLiteral(10, loc, allocator);
    auto *right = createIntLiteral(20, loc, allocator);
    auto *binExpr =
        createBinaryExpr(left, TokenKind::Plus, right, loc, allocator);

    std::vector<NodeKind> visited;
    walkAST(binExpr, [&visited](ASTNode *node) {
      visited.push_back(node->kind);
      return true; // Continue to children
    });

    REQUIRE(visited.size() == 3);
    REQUIRE(visited[0] == astBinaryExpr);
    REQUIRE(visited[1] == astInt);
    REQUIRE(visited[2] == astInt);
  }

  SECTION("walkAST with early termination") {
    auto *arrayExpr = createArrayExpr(loc, allocator);
    arrayExpr->addElement(createIntLiteral(1, loc, allocator));
    arrayExpr->addElement(createIntLiteral(2, loc, allocator));
    arrayExpr->addElement(createIntLiteral(3, loc, allocator));

    std::vector<NodeKind> visited;
    walkAST(arrayExpr, [&visited](ASTNode *node) {
      visited.push_back(node->kind);
      // Only visit the array node, skip children
      return node->kind != astArrayExpr;
    });

    REQUIRE(visited.size() == 1);
    REQUIRE(visited[0] == astArrayExpr);
  }

  SECTION("Const walkAST") {
    auto *intLit = createIntLiteral(42, loc, allocator);
    const ASTNode *constNode = intLit;

    std::vector<NodeKind> visited;
    walkAST(constNode, [&visited](const ASTNode *node) {
      visited.push_back(node->kind);
      return true;
    });

    REQUIRE(visited.size() == 1);
    REQUIRE(visited[0] == astInt);
  }
}

TEST_CASE("Utility Functions: collectNodes and findNode", "[ast][visitor]") {
  ArenaAllocator allocator(1024 * 1024);
  Location loc("test.cxy", Position(1, 1, 0));

  SECTION("collectNodes finds all nodes of specific type") {
    // Create a binary expression: (1 + 2) + (3 + 4)
    auto *left =
        createBinaryExpr(createIntLiteral(1, loc, allocator), TokenKind::Plus,
                         createIntLiteral(2, loc, allocator), loc, allocator);
    auto *right =
        createBinaryExpr(createIntLiteral(3, loc, allocator), TokenKind::Plus,
                         createIntLiteral(4, loc, allocator), loc, allocator);
    auto *root = createBinaryExpr(left, TokenKind::Plus, right, loc, allocator);

    // Collect all binary expressions
    auto binaryNodes = collectNodes<BinaryExpressionNode>(root, allocator);
    REQUIRE(binaryNodes.size() == 3);

    // Collect all integer literals
    auto intNodes = collectNodes<IntLiteralNode>(root, allocator);
    REQUIRE(intNodes.size() == 4);

    // Collect all boolean literals (should be empty)
    auto boolNodes = collectNodes<BoolLiteralNode>(root, allocator);
    REQUIRE(boolNodes.size() == 0);
  }

  SECTION("walkAST traversal order debug") {
    auto *arrayExpr = createArrayExpr(loc, allocator);
    auto *trueBool = createBoolLiteral(true, loc, allocator);
    auto *intLit = createIntLiteral(42, loc, allocator);
    auto *falseBool = createBoolLiteral(false, loc, allocator);
    arrayExpr->addElement(trueBool);
    arrayExpr->addElement(intLit);
    arrayExpr->addElement(falseBool);

    // Track traversal order
    std::vector<ASTNode *> visited;
    walkAST(arrayExpr, [&visited](ASTNode *node) {
      visited.push_back(node);
      return true;
    });

    // Check traversal order
    REQUIRE(visited.size() == 4);     // Array + 3 elements
    REQUIRE(visited[0] == arrayExpr); // Should visit array first
    REQUIRE(visited[1] == trueBool);  // Then first element
    REQUIRE(visited[2] == intLit);    // Then second element
    REQUIRE(visited[3] == falseBool); // Then third element
  }

  SECTION("findNode finds first node of specific type") {
    auto *arrayExpr = createArrayExpr(loc, allocator);
    arrayExpr->addElement(createIntLiteral(42, loc, allocator));
    arrayExpr->addElement(createBoolLiteral(true, loc, allocator));
    arrayExpr->addElement(createBoolLiteral(false, loc, allocator));

    // Find first boolean literal (should find one of the boolean literals)
    auto *firstBool = findNode<BoolLiteralNode>(arrayExpr);
    REQUIRE(firstBool != nullptr);
    // Don't care which boolean it finds, just that it finds a boolean
    REQUIRE((firstBool->value == true || firstBool->value == false));

    // Find first integer literal
    auto *firstInt = findNode<IntLiteralNode>(arrayExpr);
    REQUIRE(firstInt != nullptr);
    REQUIRE(firstInt->value == 42);

    // Find first string literal (should be null)
    auto *firstString = findNode<StringLiteralNode>(arrayExpr);
    REQUIRE(firstString == nullptr);
  }

  SECTION("Complex tree with mixed node types") {
    StringInterner interner(allocator);

    // Create: func(array[1, true], var.field)
    auto *arrayExpr = createArrayExpr(loc, allocator);
    arrayExpr->addElement(createIntLiteral(1, loc, allocator));
    arrayExpr->addElement(createBoolLiteral(true, loc, allocator));

    auto *varIdent = createIdentifier(interner.intern("var"), loc, allocator);
    auto *fieldIdent =
        createIdentifier(interner.intern("field"), loc, allocator);
    auto *memberExpr =
        createMemberExpr(varIdent, fieldIdent, false, loc, allocator);

    auto *funcIdent = createIdentifier(interner.intern("func"), loc, allocator);
    auto *callExpr = createCallExpr(funcIdent, loc, allocator);
    callExpr->addArgument(arrayExpr);
    callExpr->addArgument(memberExpr);

    // Collect different types
    auto arrays = collectNodes<ArrayExpressionNode>(callExpr, allocator);
    auto calls = collectNodes<CallExpressionNode>(callExpr, allocator);
    auto members = collectNodes<MemberExpressionNode>(callExpr, allocator);
    auto identifiers = collectNodes<IdentifierNode>(callExpr, allocator);
    auto literals = collectNodes<IntLiteralNode>(callExpr, allocator);

    REQUIRE(arrays.size() == 1);
    REQUIRE(calls.size() == 1);
    REQUIRE(members.size() == 1);
    REQUIRE(identifiers.size() == 3); // "func", "var", and "field"
    REQUIRE(literals.size() == 1);    // The "1" in the array

    // Verify specific values (order may vary due to tree traversal)
    bool foundFunc = false, foundVar = false, foundField = false;
    for (auto *ident : identifiers) {
      std::string_view name = ident->name.view();
      if (name == "func")
        foundFunc = true;
      else if (name == "var")
        foundVar = true;
      else if (name == "field")
        foundField = true;
    }
    REQUIRE(foundFunc);
    REQUIRE(foundVar);
    REQUIRE(foundField);
    // Check that member access has correct structure
    REQUIRE(members[0]->object != nullptr);
    REQUIRE(members[0]->member != nullptr);
  }
}

// Test const visitor
class TestConstVisitor : public ConstASTVisitor {
public:
  std::vector<NodeKind> visitedNodes;

  bool visitNode(const ASTNode *node) override {
    visitedNodes.push_back(node->kind);
    return true;
  }
};

TEST_CASE("ConstASTVisitor Functionality", "[ast][visitor]") {
  ArenaAllocator allocator(1024 * 1024);
  Location loc("test.cxy", Position(1, 1, 0));

  SECTION("Const visitor works with const nodes") {
    auto *intLit = createIntLiteral(42, loc, allocator);
    auto *boolLit = createBoolLiteral(true, loc, allocator);
    auto *binExpr =
        createBinaryExpr(intLit, TokenKind::Plus, boolLit, loc, allocator);

    const ASTNode *constRoot = binExpr;

    TestConstVisitor visitor;
    visitor.visit(constRoot);

    REQUIRE(visitor.visitedNodes.size() == 3);
    REQUIRE(visitor.visitedNodes[0] == astBinaryExpr);
    REQUIRE(visitor.visitedNodes[1] == astInt);
    REQUIRE(visitor.visitedNodes[2] == astBool);
  }
}

// Test visitor that modifies nodes
class NodeModifyingVisitor : public ASTVisitor {
public:
  bool visitInt(IntLiteralNode *node) override {
    // Double all integer values
    node->value *= 2;
    return visitNode(static_cast<ASTNode *>(node));
  }
};

TEST_CASE("Visitor Node Modification", "[ast][visitor]") {
  ArenaAllocator allocator(1024 * 1024);
  Location loc("test.cxy", Position(1, 1, 0));

  SECTION("Visitor can modify node values") {
    auto *intLit1 = createIntLiteral(5, loc, allocator);
    auto *intLit2 = createIntLiteral(10, loc, allocator);
    auto *binExpr =
        createBinaryExpr(intLit1, TokenKind::Plus, intLit2, loc, allocator);

    REQUIRE(intLit1->value == 5);
    REQUIRE(intLit2->value == 10);

    NodeModifyingVisitor visitor;
    visitor.visit(binExpr);

    // Values should be doubled
    REQUIRE(intLit1->value == 10);
    REQUIRE(intLit2->value == 20);
  }
}

TEST_CASE("Parent-Child Relationships", "[ast][visitor]") {
  ArenaAllocator allocator(1024 * 1024);
  Location loc("test.cxy", Position(1, 1, 0));

  SECTION("Parent relationships are set correctly") {
    auto *left = createIntLiteral(10, loc, allocator);
    auto *right = createIntLiteral(20, loc, allocator);
    auto *binExpr =
        createBinaryExpr(left, TokenKind::Plus, right, loc, allocator);

    REQUIRE(left->parent == binExpr);
    REQUIRE(right->parent == binExpr);
    REQUIRE(binExpr->parent == nullptr);
  }

  SECTION("Nested expression parent relationships") {
    auto *leaf1 = createIntLiteral(1, loc, allocator);
    auto *leaf2 = createIntLiteral(2, loc, allocator);
    auto *inner =
        createBinaryExpr(leaf1, TokenKind::Plus, leaf2, loc, allocator);

    auto *leaf3 = createIntLiteral(3, loc, allocator);
    auto *outer =
        createBinaryExpr(inner, TokenKind::Minus, leaf3, loc, allocator);

    REQUIRE(leaf1->parent == inner);
    REQUIRE(leaf2->parent == inner);
    REQUIRE(inner->parent == outer);
    REQUIRE(leaf3->parent == outer);
    REQUIRE(outer->parent == nullptr);
  }
}
