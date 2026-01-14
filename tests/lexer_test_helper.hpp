#pragma once

#include "cxy/diagnostics.hpp"
#include "cxy/lexer.hpp"
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
  LexerTestHelper() {
    logger = std::make_unique<DiagnosticLogger>();
    // Remove default console sink to avoid noise in tests
    logger->removeAllSinks();
    memoryDiagnostics = std::make_unique<InMemoryDiagnosticSink>();
    // Store raw pointer for easy access, but let DiagnosticLogger own the sink
    diagnosticsPtr = memoryDiagnostics.get();
    logger->addSink(std::move(memoryDiagnostics));
    sourceManager = std::make_unique<SourceManager>();
  }

  std::vector<Token> tokenize(const std::string &input,
                              const std::string &filename = "test.cxy") {
    // Clear any previous diagnostics
    diagnosticsPtr->clear();

    // Register the source content with SourceManager
    sourceManager->registerFile(filename, input);

    Lexer lexer(filename, input, *logger);
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
  std::unique_ptr<DiagnosticLogger> logger;
  std::unique_ptr<SourceManager> sourceManager;
  std::unique_ptr<InMemoryDiagnosticSink> memoryDiagnostics;
  InMemoryDiagnosticSink *diagnosticsPtr; // Non-owning pointer for easy access
};

} // namespace cxy
