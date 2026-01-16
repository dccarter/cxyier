#pragma once

#include "cxy/arena_allocator.hpp"
#include "cxy/ast/identifiers.hpp"
#include "cxy/ast/literals.hpp"
#include "cxy/ast/node.hpp"
#include "cxy/diagnostics.hpp"
#include "cxy/lexer.hpp"
#include "cxy/parser.hpp"
#include "cxy/strings.hpp"
#include "cxy/token.hpp"
#include "cxy/types/registry.hpp"

#include "ast_test_utils.hpp"

#include "catch2.hpp"
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace cxy::test {

/**
 * @brief Test fixture for parser testing.
 *
 * Provides a complete setup with lexer, parser, and arena allocator
 * for testing parser functionality in isolation. All diagnostic output
 * is captured in memory instead of printed to console to avoid test noise.
 *
 * @example
 * // Basic usage
 * ParserTestFixture fixture("42 + 3.14");
 * auto result = fixture.parseExpression();
 * REQUIRE(result != nullptr);
 *
 * // Testing error cases
 * ParserTestFixture errorFixture("invalid syntax");
 * auto errorResult = errorFixture.parseExpression();
 * REQUIRE(errorFixture.hasErrors());
 */
class ParserTestFixture {
public:
  /**
   * @brief Create a parser test fixture with the given source code.
   *
   * @param source Source code to parse
   * @param filename Optional filename for diagnostics
   */
  explicit ParserTestFixture(std::string_view source,
                             std::string_view filename = "<test>")
      : source_(source), filename_(filename), arena_(1024 * 1024), // 1MB arena
        stringPool_(arena_), logger_(), sourceManager_(),
        lexer_(filename_, source_, logger_, stringPool_),
        typeRegistry_(), parser_(lexer_, arena_, sourceManager_, stringPool_, logger_, typeRegistry_) {
    // Configure logger to use in-memory sink to avoid console noise during
    // tests while still capturing diagnostic messages for test validation
    logger_.removeAllSinks();
    auto inMemorySink = std::make_unique<InMemoryDiagnosticSink>();
    inMemorySink_ = inMemorySink.get(); // Keep raw pointer for access
    logger_.addSink(std::move(inMemorySink));

    // Register source with source manager
    sourceManager_.registerFile(std::string(filename_), std::string(source_));
    // Initialize parser
    parser_.initialize();
  }

  /**
   * @brief Get the parser instance.
   */
  Parser &parser() { return parser_; }

  /**
   * @brief Get the lexer instance.
   */
  Lexer &lexer() { return lexer_; }

  /**
   * @brief Get the arena allocator.
   */
  ArenaAllocator &arena() { return arena_; }

  /**
   * @brief Get the string pool.
   */
  StringInterner &stringPool() { return stringPool_; }

  /**
   * @brief Parse an expression and return the result.
   */
  ast::ASTNode *parseExpression() { return parser_.parseExpression(); }

  /**
   * @brief Parse a primary expression and return the result.
   */
  ast::ASTNode *parsePrimaryExpression() {
    return parser_.parsePrimaryExpression();
  }

  /**
   * @brief Parse a literal expression and return the result.
   */
  ast::ASTNode *parseLiteralExpression() {
    return parser_.parseLiteralExpression();
  }

  /**
   * @brief Parse an identifier expression and return the result.
   */
  ast::ASTNode *parseIdentifierExpression() {
    return parser_.parseIdentifierExpression();
  }

  /**
   * @brief Parse an expression statement and return the result.
   */
  ast::ASTNode *parseExpressionStatement() {
    return parser_.parseExpressionStatement();
  }

  /**
   * @brief Parse a statement and return the result.
   */
  ast::ASTNode *parseStatement() {
    return parser_.parseStatement();
  }

  /**
   * @brief Check if parser is at end of input.
   */
  bool isAtEnd() const { return parser_.isAtEnd(); }

  /**
   * @brief Get current token from parser.
   */
  Token current() const { return parser_.current(); }

  /**
   * @brief Get lookahead token from parser.
   */
  Token lookahead(int offset = 1) const { return parser_.lookahead(offset); }

  /**
   * @brief Advance parser to next token.
   */
  void advance() { parser_.advance(); }

  /**
   * @brief Get diagnostic messages collected during parsing.
   *
   * Useful for testing error cases and diagnostic reporting.
   * All diagnostic output is captured in memory instead of printed to console.
   *
   * @example
   * auto fixture = ParserTestFixture("invalid syntax");
   * auto result = fixture.parseExpression();
   * auto diagnostics = fixture.getDiagnostics();
   * REQUIRE(diagnostics.size() > 0);
   * REQUIRE(diagnostics[0].severity == Severity::Error);
   */
  const std::vector<DiagnosticMessage> &getDiagnostics() const {
    return inMemorySink_->getMessages();
  }

  /**
   * @brief Check if any errors were reported during parsing.
   *
   * Convenience method to check for Error or Fatal diagnostics.
   *
   * @example
   * auto fixture = ParserTestFixture("+ +");  // Invalid syntax
   * fixture.parseExpression();
   * REQUIRE(fixture.hasErrors());  // Should report parse error
   */
  bool hasErrors() const {
    for (const auto &msg : inMemorySink_->getMessages()) {
      if (msg.severity == Severity::Error || msg.severity == Severity::Fatal) {
        return true;
      }
    }
    return false;
  }

private:
  std::string source_;
  std::string filename_;
  ArenaAllocator arena_;
  StringInterner stringPool_;
  InMemoryDiagnosticSink *inMemorySink_; // Raw pointer to sink owned by logger
  DiagnosticLogger logger_;
  SourceManager sourceManager_;
  Lexer lexer_;
  TypeRegistry typeRegistry_;
  Parser parser_;
};

/**
 * @brief Helper to create a parser fixture from source code.
 */
inline std::unique_ptr<ParserTestFixture>
createParserFixture(std::string_view source,
                    std::string_view filename = "<test>") {
  return std::make_unique<ParserTestFixture>(source, filename);
}

// Token buffer testing helpers

/**
 * @brief Test helper to verify token buffer state.
 */
inline void checkTokenBuffer(Parser &parser, TokenKind expectedCurrent,
                             TokenKind expectedLookahead1 = TokenKind::EoF,
                             TokenKind expectedLookahead2 = TokenKind::EoF) {
  REQUIRE(parser.current().kind == expectedCurrent);
  if (expectedLookahead1 != TokenKind::EoF) {
    REQUIRE(parser.lookahead(1).kind == expectedLookahead1);
  }
  if (expectedLookahead2 != TokenKind::EoF) {
    REQUIRE(parser.lookahead(2).kind == expectedLookahead2);
  }
}

/**
 * @brief Test helper to verify token buffer advancement.
 */
inline void advanceAndCheck(Parser &parser, TokenKind expectedNewCurrent,
                            TokenKind expectedNewLookahead1 = TokenKind::EoF,
                            TokenKind expectedNewLookahead2 = TokenKind::EoF) {
  parser.advance();
  checkTokenBuffer(parser, expectedNewCurrent, expectedNewLookahead1,
                   expectedNewLookahead2);
}

// Literal parsing test helpers

/**
 * @brief Test that parsing produces an integer literal with expected value.
 */
template <typename T>
inline void expectIntegerLiteral(ast::ASTNode *node, T expectedValue) {
  REQUIRE(node != nullptr);
  REQUIRE(node->kind == ast::astInt);

  auto *intNode = static_cast<ast::IntLiteralNode *>(node);
  REQUIRE(static_cast<T>(intNode->value) == expectedValue);
}

/**
 * @brief Test that parsing produces a float literal with expected value.
 */
inline void expectFloatLiteral(ast::ASTNode *node, double expectedValue) {
  REQUIRE(node != nullptr);
  REQUIRE(node->kind == ast::astFloat);

  auto *floatNode = static_cast<ast::FloatLiteralNode *>(node);
  REQUIRE(floatNode->value == Catch::Approx(expectedValue));
}

/**
 * @brief Test that parsing produces a character literal with expected value.
 */
inline void expectCharLiteral(ast::ASTNode *node, uint32_t expectedValue) {
  REQUIRE(node != nullptr);
  REQUIRE(node->kind == ast::astChar);

  auto *charNode = static_cast<ast::CharLiteralNode *>(node);
  REQUIRE(charNode->value == expectedValue);
}

/**
 * @brief Test that parsing produces a string literal with expected value.
 */
inline void expectStringLiteral(ast::ASTNode *node,
                                std::string_view expectedValue) {
  REQUIRE(node != nullptr);
  REQUIRE(node->kind == ast::astString);

  auto *stringNode = static_cast<ast::StringLiteralNode *>(node);
  REQUIRE(stringNode->value.view() == expectedValue);
}

/**
 * @brief Test that parsing produces a boolean literal with expected value.
 */
inline void expectBoolLiteral(ast::ASTNode *node, bool expectedValue) {
  REQUIRE(node != nullptr);
  REQUIRE(node->kind == ast::astBool);

  auto *boolNode = static_cast<ast::BoolLiteralNode *>(node);
  REQUIRE(boolNode->value == expectedValue);
}

/**
 * @brief Test that parsing produces a null literal.
 */
inline void expectNullLiteral(ast::ASTNode *node) {
  REQUIRE(node != nullptr);
  REQUIRE(node->kind == ast::astNull);
}

/**
 * @brief Test that parsing produces an identifier with expected name.
 */
inline void expectIdentifier(ast::ASTNode *node,
                             std::string_view expectedName) {
  REQUIRE(node != nullptr);
  REQUIRE(node->kind == ast::astIdentifier);

  auto *identNode = static_cast<ast::IdentifierNode *>(node);
  REQUIRE(identNode->name.view() == expectedName);
}

// Error testing helpers

/**
 * @brief Test that parsing fails and returns nullptr.
 */
inline void expectParseFailure(ast::ASTNode *node) { REQUIRE(node == nullptr); }

/**
 * @brief Test multiple tokens in sequence.
 */
inline void testTokenSequence(ParserTestFixture &fixture,
                              const std::vector<TokenKind> &expectedTokens) {
  for (size_t i = 0; i < expectedTokens.size(); ++i) {
    REQUIRE(fixture.current().kind == expectedTokens[i]);
    if (i < expectedTokens.size() - 1) {
      fixture.advance();
    }
  }
}

/**
 * @brief Create a simple test case for literal parsing.
 */
#define LITERAL_TEST_CASE(name, source, expectationCall)                       \
  TEST_CASE("Parser: " name, "[parser][literals]") {                           \
    auto fixture = createParserFixture(source);                                \
    auto *node = fixture->parseLiteralExpression();                            \
    expectationCall;                                                           \
    REQUIRE(fixture->isAtEnd());                                               \
  }

/**
 * @brief Create a simple test case for identifier parsing.
 */
#define IDENTIFIER_TEST_CASE(name, source, expectedName)                       \
  TEST_CASE("Parser: " name, "[parser][identifiers]") {                        \
    auto fixture = createParserFixture(source);                                \
    auto *node = fixture->parseIdentifierExpression();                         \
    expectIdentifier(node, expectedName);                                      \
    REQUIRE(fixture->isAtEnd());                                               \
  }

/**
 * @brief Create a simple test case for primary expression parsing.
 */
#define PRIMARY_EXPRESSION_TEST_CASE(name, source, expectationCall)            \
  TEST_CASE("Parser: " name, "[parser][primary]") {                            \
    auto fixture = createParserFixture(source);                                \
    auto *node = fixture->parsePrimaryExpression();                            \
    expectationCall;                                                           \
    REQUIRE(fixture->isAtEnd());                                               \
  }

} // namespace cxy::test
