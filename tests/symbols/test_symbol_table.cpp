#include "catch2.hpp"
#include "cxy/symbols.hpp"
#include "cxy/diagnostics.hpp"
#include "cxy/memory/arena.hpp"
#include "cxy/strings.hpp"
#include "cxy/ast/node.hpp"

using namespace cxy;
using namespace cxy::symbols;

// Mock AST node for testing
class MockASTNode : public ast::ASTNode {
public:
    explicit MockASTNode(const std::string& name, ArenaAllocator& arena) 
        : ast::ASTNode(ast::astIdentifier, Location("test.cxy", Position(1, 1, 0)), arena), 
          name_(name) {}
    
    std::format_context::iterator toString(std::format_context& ctx) const override { 
        return std::format_to(ctx.out(), "{}", name_); 
    }
    
    const std::string& getName() const { return name_; }
    
private:
    std::string name_;
};

// Test fixture for symbol table tests
class SymbolTableTestFixture {
private:
    InMemoryDiagnosticSink* diagnosticSink_;

public:
    SymbolTableTestFixture() 
        : arena(1024), 
          interner(arena),
          logger(),
          symbolTable(logger, arena) {
        logger.removeAllSinks();
        auto sinkPtr = std::make_unique<InMemoryDiagnosticSink>();
        diagnosticSink_ = sinkPtr.get();
        logger.addSink(std::move(sinkPtr));
    }

    InternedString intern(const std::string& str) {
        return interner.intern(str);
    }

    std::unique_ptr<MockASTNode> createMockNode(const std::string& name) {
        return std::make_unique<MockASTNode>(name, arena);
    }

    const InMemoryDiagnosticSink& getDiagnostics() const { return *diagnosticSink_; }

    ArenaAllocator arena;
    StringInterner interner;
    DiagnosticLogger logger;
    SymbolTable symbolTable;
};

TEST_CASE("Symbol creation and basic operations", "[symbols][symbol]") {
    SECTION("Symbol construction") {
        ArenaAllocator arena(512);
        StringInterner interner(arena);
        auto testName = interner.intern("testSymbol");
        auto mockNode = std::make_unique<MockASTNode>("test", arena);
        
        Symbol symbol(0, testName, mockNode.get());
        
        REQUIRE(symbol.getIndex() == 0);
        REQUIRE(symbol.getName() == testName);
        REQUIRE(symbol.getDeclaration() == mockNode.get());
        REQUIRE(symbol.getLastReference() == nullptr);
    }
    
    SECTION("Symbol reference tracking") {
        ArenaAllocator arena(512);
        StringInterner interner(arena);
        auto testName = interner.intern("testSymbol");
        auto declNode = std::make_unique<MockASTNode>("declaration", arena);
        auto refNode = std::make_unique<MockASTNode>("reference", arena);
        
        Symbol symbol(0, testName, declNode.get());
        symbol.updateLastReference(refNode.get());
        
        REQUIRE(symbol.getLastReference() == refNode.get());
    }
    
    SECTION("Symbol equality") {
        ArenaAllocator arena(512);
        StringInterner interner(arena);
        auto name1 = interner.intern("symbol1");
        auto name2 = interner.intern("symbol2");
        auto node1 = std::make_unique<MockASTNode>("node1", arena);
        auto node2 = std::make_unique<MockASTNode>("node2", arena);
        
        Symbol symbol1(0, name1, node1.get());
        Symbol symbol2(0, name1, node1.get());  // Same name and node
        Symbol symbol3(0, name2, node2.get());  // Different name
        
        REQUIRE(symbol1 == symbol2);
        REQUIRE(symbol1 != symbol3);
    }
}

TEST_CASE("Scope creation and symbol management", "[symbols][scope]") {
    SECTION("Scope construction") {
        ArenaAllocator arena(512);
        auto mockNode = std::make_unique<MockASTNode>("function", arena);
        
        Scope scope(mockNode.get(), nullptr, 0, arena);
        
        REQUIRE(scope.getNode() == mockNode.get());
        REQUIRE(scope.getParent() == nullptr);
        REQUIRE(scope.getLevel() == 0);
        REQUIRE(scope.getSymbolCount() == 0);
    }
    
    SECTION("Symbol definition in scope") {
        ArenaAllocator arena(512);
        StringInterner interner(arena);
        auto symbolName = interner.intern("variable");
        auto scopeNode = std::make_unique<MockASTNode>("function", arena);
        auto declNode = std::make_unique<MockASTNode>("declaration", arena);
        
        Scope scope(scopeNode.get(), nullptr, 0, arena);
        
        Symbol* symbol = scope.defineSymbol(symbolName, declNode.get());
        
        REQUIRE(symbol != nullptr);
        REQUIRE(symbol->getName() == symbolName);
        REQUIRE(symbol->getDeclaration() == declNode.get());
        REQUIRE(symbol->getIndex() == 0);
        REQUIRE(scope.getSymbolCount() == 1);
    }
    
    SECTION("Symbol redefinition in same scope") {
        ArenaAllocator arena(512);
        StringInterner interner(arena);
        auto symbolName = interner.intern("variable");
        auto scopeNode = std::make_unique<MockASTNode>("function", arena);
        auto declNode1 = std::make_unique<MockASTNode>("declaration1", arena);
        auto declNode2 = std::make_unique<MockASTNode>("declaration2", arena);
        
        Scope scope(scopeNode.get(), nullptr, 0, arena);
        
        Symbol* symbol1 = scope.defineSymbol(symbolName, declNode1.get());
        Symbol* symbol2 = scope.defineSymbol(symbolName, declNode2.get());
        
        REQUIRE(symbol1 != nullptr);
        REQUIRE(symbol2 == nullptr);  // Redefinition should fail
        REQUIRE(scope.getSymbolCount() == 1);  // Only first definition counts
    }
    
    SECTION("Symbol lookup in scope") {
        ArenaAllocator arena(512);
        StringInterner interner(arena);
        auto symbolName = interner.intern("variable");
        auto unknownName = interner.intern("unknown");
        auto scopeNode = std::make_unique<MockASTNode>("function", arena);
        auto declNode = std::make_unique<MockASTNode>("declaration", arena);
        
        Scope scope(scopeNode.get(), nullptr, 0, arena);
        scope.defineSymbol(symbolName, declNode.get());
        
        Symbol* found = scope.lookupLocal(symbolName);
        Symbol* notFound = scope.lookupLocal(unknownName);
        
        REQUIRE(found != nullptr);
        REQUIRE(found->getName() == symbolName);
        REQUIRE(notFound == nullptr);
    }
    
    SECTION("Symbol existence check") {
        ArenaAllocator arena(512);
        StringInterner interner(arena);
        auto symbolName = interner.intern("variable");
        auto unknownName = interner.intern("unknown");
        auto scopeNode = std::make_unique<MockASTNode>("function", arena);
        auto declNode = std::make_unique<MockASTNode>("declaration", arena);
        
        Scope scope(scopeNode.get(), nullptr, 0, arena);
        scope.defineSymbol(symbolName, declNode.get());
        
        REQUIRE(scope.hasSymbol(symbolName) == true);
        REQUIRE(scope.hasSymbol(unknownName) == false);
    }
}

TEST_CASE("SymbolTable basic operations", "[symbols][symbol-table]") {
    SECTION("SymbolTable construction") {
        SymbolTableTestFixture fixture;
        
        REQUIRE(fixture.symbolTable.getCurrentScope() != nullptr);
        REQUIRE(fixture.symbolTable.getGlobalScope() != nullptr);
        REQUIRE(fixture.symbolTable.getCurrentScope() == fixture.symbolTable.getGlobalScope());
        REQUIRE(fixture.symbolTable.getCurrentScopeLevel() == 0);
    }
    
    SECTION("Symbol definition in global scope") {
        SymbolTableTestFixture fixture;
        auto symbolName = fixture.intern("globalVar");
        auto declNode = fixture.createMockNode("declaration");
        Location location("test.cxy", Position(1, 1, 0));
        
        bool result = fixture.symbolTable.defineSymbol(symbolName, declNode.get(), location);
        
        REQUIRE(result == true);
        REQUIRE(fixture.symbolTable.getGlobalScope()->hasSymbol(symbolName) == true);
    }
    
    SECTION("Symbol lookup in global scope") {
        SymbolTableTestFixture fixture;
        auto symbolName = fixture.intern("globalVar");
        auto declNode = fixture.createMockNode("declaration");
        Location defLocation("test.cxy", Position(1, 1, 0));
        Location lookupLocation("test.cxy", Position(2, 5, 0));
        
        fixture.symbolTable.defineSymbol(symbolName, declNode.get(), defLocation);
        
        const ast::ASTNode* found = fixture.symbolTable.lookupSymbol(symbolName, lookupLocation);
        
        REQUIRE(found == declNode.get());
    }
    
    SECTION("Undefined symbol lookup") {
        SymbolTableTestFixture fixture;
        auto unknownName = fixture.intern("unknown");
        Location location("test.cxy", Position(1, 1, 0));
        
        const ast::ASTNode* found = fixture.symbolTable.lookupSymbol(unknownName, location);
        
        REQUIRE(found == nullptr);
        // Should have logged an error for undefined symbol
        REQUIRE(fixture.getDiagnostics().getMessages().size() > 0);
    }
    
    SECTION("Symbol redefinition error") {
        SymbolTableTestFixture fixture;
        auto symbolName = fixture.intern("duplicate");
        auto declNode1 = fixture.createMockNode("declaration1");
        auto declNode2 = fixture.createMockNode("declaration2");
        Location location1("test.cxy", Position(1, 1, 0));
        Location location2("test.cxy", Position(2, 1, 0));
        
        bool result1 = fixture.symbolTable.defineSymbol(symbolName, declNode1.get(), location1);
        bool result2 = fixture.symbolTable.defineSymbol(symbolName, declNode2.get(), location2);
        
        REQUIRE(result1 == true);
        REQUIRE(result2 == false);
        // Should have logged a redefinition error
        REQUIRE(fixture.getDiagnostics().getMessages().size() > 0);
    }
}

TEST_CASE("Scope management", "[symbols][symbol-table][scopes]") {
    SECTION("Push and pop scopes") {
        SymbolTableTestFixture fixture;
        auto functionNode = fixture.createMockNode("function");
        Location location("test.cxy", Position(1, 1, 0));
        
        // Initially in global scope
        REQUIRE(fixture.symbolTable.getCurrentScopeLevel() == 0);
        
        // Push function scope
        fixture.symbolTable.pushScope(functionNode.get(), location);
        REQUIRE(fixture.symbolTable.getCurrentScopeLevel() == 1);
        REQUIRE(fixture.symbolTable.getCurrentScope()->getNode() == functionNode.get());
        
        // Pop back to global scope
        fixture.symbolTable.popScope(location);
        REQUIRE(fixture.symbolTable.getCurrentScopeLevel() == 0);
        REQUIRE(fixture.symbolTable.getCurrentScope() == fixture.symbolTable.getGlobalScope());
    }
    
    SECTION("Symbol shadowing") {
        SymbolTableTestFixture fixture;
        auto symbolName = fixture.intern("variable");
        auto globalDecl = fixture.createMockNode("globalDecl");
        auto localDecl = fixture.createMockNode("localDecl");
        auto functionNode = fixture.createMockNode("function");
        Location location("test.cxy", Position(1, 1, 0));
        
        // Define in global scope
        fixture.symbolTable.defineSymbol(symbolName, globalDecl.get(), location);
        
        // Push local scope and define same name
        fixture.symbolTable.pushScope(functionNode.get(), location);
        fixture.symbolTable.defineSymbol(symbolName, localDecl.get(), location);
        
        // Lookup should find local symbol (shadowing)
        const ast::ASTNode* found = fixture.symbolTable.lookupSymbol(symbolName, location);
        REQUIRE(found == localDecl.get());
        
        // Pop scope and lookup should find global symbol
        fixture.symbolTable.popScope(location);
        const ast::ASTNode* foundGlobal = fixture.symbolTable.lookupSymbol(symbolName, location);
        REQUIRE(foundGlobal == globalDecl.get());
    }
    
    SECTION("Deep scope nesting") {
        SymbolTableTestFixture fixture;
        auto function1 = fixture.createMockNode("function1");
        auto block1 = fixture.createMockNode("block1");
        auto block2 = fixture.createMockNode("block2");
        Location location("test.cxy", Position(1, 1, 0));
        
        // Create nested scopes: global -> function -> block -> block
        REQUIRE(fixture.symbolTable.getCurrentScopeLevel() == 0);
        
        fixture.symbolTable.pushScope(function1.get(), location);
        REQUIRE(fixture.symbolTable.getCurrentScopeLevel() == 1);
        
        fixture.symbolTable.pushScope(block1.get(), location);
        REQUIRE(fixture.symbolTable.getCurrentScopeLevel() == 2);
        
        fixture.symbolTable.pushScope(block2.get(), location);
        REQUIRE(fixture.symbolTable.getCurrentScopeLevel() == 3);
        
        // Pop all the way back
        fixture.symbolTable.popScope(location);
        REQUIRE(fixture.symbolTable.getCurrentScopeLevel() == 2);
        
        fixture.symbolTable.popScope(location);
        REQUIRE(fixture.symbolTable.getCurrentScopeLevel() == 1);
        
        fixture.symbolTable.popScope(location);
        REQUIRE(fixture.symbolTable.getCurrentScopeLevel() == 0);
    }
}

TEST_CASE("Symbol reference tracking", "[symbols][symbol-table][references]") {
    SECTION("Update symbol reference") {
        SymbolTableTestFixture fixture;
        auto symbolName = fixture.intern("variable");
        auto declNode = fixture.createMockNode("declaration");
        auto refNode = fixture.createMockNode("reference");
        Location defLocation("test.cxy", Position(1, 1, 0));
        Location refLocation("test.cxy", Position(2, 5, 0));
        
        // Define symbol
        fixture.symbolTable.defineSymbol(symbolName, declNode.get(), defLocation);
        
        // Update reference
        fixture.symbolTable.updateSymbolReference(symbolName, refNode.get(), refLocation);
        
        // Verify the reference was tracked (we'll need to access this through scope)
        Symbol* symbol = fixture.symbolTable.getCurrentScope()->lookupLocal(symbolName);
        REQUIRE(symbol != nullptr);
        REQUIRE(symbol->getLastReference() == refNode.get());
    }
}

TEST_CASE("Symbol iteration", "[symbols][symbol-table][iteration]") {
    SECTION("Iterate symbols in current scope") {
        SymbolTableTestFixture fixture;
        auto name1 = fixture.intern("var1");
        auto name2 = fixture.intern("var2");
        auto decl1 = fixture.createMockNode("decl1");
        auto decl2 = fixture.createMockNode("decl2");
        Location location("test.cxy", Position(1, 1, 0));
        
        // Define symbols
        fixture.symbolTable.defineSymbol(name1, decl1.get(), location);
        fixture.symbolTable.defineSymbol(name2, decl2.get(), location);
        
        // Collect symbols through iteration
        std::vector<const ast::ASTNode*> collected;
        fixture.symbolTable.iterateSymbols([&](const ast::ASTNode* decl) {
            collected.push_back(decl);
        }, true);  // Current scope only
        
        REQUIRE(collected.size() == 2);
        // Order is not guaranteed, so check both declarations are present
        REQUIRE((collected[0] == decl1.get() || collected[0] == decl2.get()));
        REQUIRE((collected[1] == decl1.get() || collected[1] == decl2.get()));
        REQUIRE(collected[0] != collected[1]);
    }
}