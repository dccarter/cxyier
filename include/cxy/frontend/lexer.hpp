#pragma once

#include "cxy/diagnostics.hpp"
#include "cxy/token.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace cxy {

// Forward declarations
class DiagnosticLogger;

// Lexical error types for diagnostic reporting
enum class LexError {
  InvalidCharacter,     // Unknown character in input
  UnterminatedString,   // Missing closing quote
  UnterminatedComment,  // Missing */ in block comment
  InvalidEscape,        // Unknown escape sequence
  InvalidUnicodeEscape, // Malformed \u{...} sequence
  InvalidNumber,        // Malformed numeric literal
  InvalidInterpolation, // Malformed string interpolation
  FileNotFound,         // Include file not found
  RecursiveInclude,     // Circular include dependency
  BufferOverflow,       // Too many nested includes
  InvalidUtf8           // Invalid UTF-8 sequence
};

class Lexer {
public:
  // Core tokenization interface
  Token nextToken();

  // Buffer management for includes (Phase 7)
  bool pushBuffer(std::string_view filename, std::string_view content);

  // State queries
  bool isAtEnd() const;
  Location getCurrentLocation() const;
  Location makeLocation(const Position &start) const; // Create range location

  // Constructor
  Lexer(std::string_view filename, std::string_view content,
        DiagnosticLogger &logger, StringInterner &interner);

  // Template context management for >> splitting
  void enterTemplateContext();
  void exitTemplateContext();
  bool inTemplateContext() const;

private:
  // Buffer stack for include directives
  struct LexerBuffer {
    std::string_view filename;
    std::string_view content;
    size_t position;
    size_t line;
    size_t column;
    size_t byteOffset;
  };

  std::vector<LexerBuffer> bufferStack;
  DiagnosticLogger &logger;
  StringInterner &interner;

  // Phase 6: String interpolation state (stack-based for nesting)
  struct InterpolationContext {
    bool active;       // Are we currently parsing interpolated string?
    bool inExpression; // Are we parsing an expression inside {...}?
    size_t braceDepth; // Depth of nested braces in current expression

    InterpolationContext()
        : active(false), inExpression(false), braceDepth(0) {}
  };

  std::vector<InterpolationContext> interpolationStack;

  // Template context tracking for >> splitting in generics
  int templateDepth;

  // Core lexing methods
  Token lexNextToken();
  char currentChar() const;
  char peekChar(size_t offset = 1) const;
  void advance();
  void skipWhitespace();
  void skipLineComment();
  void skipBlockComment();

  // Phase 6: String interpolation
  bool hasInterpolation();
  Token lexInterpolatedString();
  Token continueStringAfterExpression();
  Token lexRegularString(const Position &start);
  void pushInterpolationContext();
  void popInterpolationContext();
  void enterExpressionMode();
  void exitExpressionMode();
  InterpolationContext &currentInterpolationContext();
  const InterpolationContext &currentInterpolationContext() const;
  bool inInterpolation() const;

  // Phase 7: Cycle detection for include files
  bool wouldCreateCycle(std::string_view filename) const;
  void popBuffer();

  // Token parsing methods (will be implemented in later phases)
  Token lexNumber();
  Token lexFloat(const Position &start, int base, __uint128_t integerPart,
                 bool hasIntegerPart);
  Token lexString();
  Token lexRawString();
  Token lexCharacter();
  Token lexIdentifierOrKeyword();
  Token lexSymbol();

  // String processing helpers
  Token createProcessedStringToken(const char *contentStart,
                                   size_t sourceLength, bool hasEscapes,
                                   size_t estimatedLength,
                                   const Position &start, TokenKind tokenKind);

  // Helper for scanning interpolated string content
  struct InterpolatedScanResult {
    const char *contentStart;
    size_t sourceLength;
    bool hasEscapes;
    size_t estimatedLength;
    bool foundInterpolation; // true if stopped at {, false if stopped at "
  };
  InterpolatedScanResult scanInterpolatedStringContent(const Position &start);

  size_t processEscapeSequences(const char *source, size_t sourceLength,
                                char *dest);

  // Helper methods
  bool isAtBufferEnd() const;
  void advancePosition();

  // Error reporting
  void reportError(LexError error, const std::string &message);
  void reportError(LexError error, const Location &location,
                   const std::string &message);
  Token createErrorToken();

  // Character classification helpers
  static bool isIdentifierStart(char c);
  static bool isIdentifierContinue(char c);
  static bool isDigit(char c);
  static bool isHexDigit(char c);
  static bool isWhitespace(char c);

  // Phase 2: Number parsing helpers
  std::string_view parseTypeSuffix();
  IntegerKind parseIntegerTypeSuffix(const std::string_view &suffix);

  // Phase 3: Float parsing helpers
  FloatKind parseFloatTypeSuffix(const std::string_view &suffix);

  // Phase 4: String/Character parsing helpers
  char parseBasicEscapeSequence(); // For basic escapes only
  uint32_t
  parseEscapeSequenceForChar(); // For character literals (returns full Unicode)
  uint32_t parseUnicodeEscape(int digitCount);
  uint32_t parseUTF8Codepoint();
  bool isValidUTF8Start(char c) const;

  // Current buffer access
  LexerBuffer &currentBuffer();
  const LexerBuffer &currentBuffer() const;
};

} // namespace cxy
