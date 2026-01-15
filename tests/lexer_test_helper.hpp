#pragma once

#include "cxy/arena_allocator.hpp"
#include "cxy/diagnostics.hpp"
#include "cxy/lexer.hpp"
#include "cxy/strings.hpp"
#include "cxy/token.hpp"
#include <memory>
#include <string>
#include <vector>

namespace cxy {

/**
 * @brief Helper class for lexer testing with in-memory diagnostics
 *
 * Provides utilities for tokenizing strings and accessing token content
 * while maintaining diagnostic information for error checking.
 */
class LexerTestHelper {
public:
  LexerTestHelper() : arena(1024 * 1024) { // 1MB arena
    logger = std::make_unique<DiagnosticLogger>();
    // Remove default console sink to avoid noise in tests
    logger->removeAllSinks();
    memoryDiagnostics = std::make_unique<InMemoryDiagnosticSink>();
    // Store raw pointer for easy access, but let DiagnosticLogger own the sink
    diagnosticsPtr = memoryDiagnostics.get();
    logger->addSink(std::move(memoryDiagnostics));
    sourceManager = std::make_unique<SourceManager>();
    interner = std::make_unique<StringInterner>(arena);
  }

  std::vector<Token> tokenize(const std::string &input,
                              const std::string &filename = "test.cxy") {
    // Clear any previous diagnostics
    diagnosticsPtr->clear();

    // Register the source content with SourceManager
    sourceManager->registerFile(filename, input);

    Lexer lexer(filename, input, *logger, *interner);
    std::vector<Token> tokens;

    Token token;
    do {
      token = lexer.nextToken();
      tokens.push_back(token);
    } while (token.kind != TokenKind::EoF);

    return tokens;
  }

  std::string_view getTokenText(const Token &token) {
    return cxy::getTokenText(token, *sourceManager);
  }

  std::string_view getStringValue(const Token &token) {
    if (token.hasLiteralValue() && (token.kind == TokenKind::StringLiteral ||
                                    token.kind == TokenKind::Ident)) {
      return token.value.stringValue.view();
    }
    return "";
  }

  DiagnosticLogger &getLogger() { return *logger; }

  // Diagnostic helper methods
  size_t getErrorCount() const { return diagnosticsPtr->getErrorCount(); }
  size_t getWarningCount() const { return diagnosticsPtr->getWarningCount(); }
  bool hasErrors() const { return getErrorCount() > 0; }
  bool hasWarnings() const { return getWarningCount() > 0; }
  bool hasErrorContaining(const std::string &text) const {
    return diagnosticsPtr->hasErrorContaining(text);
  }
  bool hasWarningContaining(const std::string &text) const {
    return diagnosticsPtr->hasWarningContaining(text);
  }

  const std::vector<DiagnosticMessage> &getDiagnostics() const {
    return diagnosticsPtr->getMessages();
  }
  std::vector<DiagnosticMessage> getErrors() const {
    return diagnosticsPtr->getErrors();
  }
  std::vector<DiagnosticMessage> getWarnings() const {
    return diagnosticsPtr->getWarnings();
  }

  void clearDiagnostics() { diagnosticsPtr->clear(); }

private:
  ArenaAllocator arena;
  std::unique_ptr<DiagnosticLogger> logger;
  std::unique_ptr<SourceManager> sourceManager;
  std::unique_ptr<StringInterner> interner;
  std::unique_ptr<InMemoryDiagnosticSink> memoryDiagnostics;
  InMemoryDiagnosticSink *diagnosticsPtr; // Non-owning pointer for easy access
};

} // namespace cxy
