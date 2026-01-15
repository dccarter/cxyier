#include "cxy/lexer.hpp"
#include "cxy/diagnostics.hpp"

#include <cctype>
#include <fstream>
#include <unordered_map>

namespace cxy {

// Constructor
Lexer::Lexer(std::string_view filename, std::string_view content,
             DiagnosticLogger &logger, StringInterner &interner)
    : logger(logger), interner(interner) {
  bufferStack.push_back({filename, content, 0, 1, 1, 0});
}

// Core tokenization interface
Token Lexer::nextToken() {
  while (true) {
    skipWhitespace();

    if (isAtEnd()) {
      // If we're at the end of the current buffer
      if (bufferStack.size() <= 1) {
        // This is the main file, return EOF
        return Token(TokenKind::EoF, getCurrentLocation());
      } else {
        // We're at the end of an included file, pop back to parent
        popBuffer();
        continue; // Continue with the parent buffer
      }
    }

    return lexNextToken();
  }
}

// State queries
bool Lexer::isAtEnd() const { return bufferStack.empty() || isAtBufferEnd(); }

Location Lexer::getCurrentLocation() const {
  if (bufferStack.empty()) {
    return Location("", Position());
  }

  const auto &buffer = currentBuffer();
  Position pos(buffer.line, buffer.column, buffer.byteOffset);
  return Location(std::string(buffer.filename), pos,
                  pos); // Single position location
}

// Helper to create location with start and end positions
Location Lexer::makeLocation(const Position &start) const {
  if (bufferStack.empty()) {
    return Location("", Position());
  }

  const auto &buffer = currentBuffer();
  Position end(buffer.line, buffer.column, buffer.byteOffset);
  return Location(std::string(buffer.filename), start, end);
}

// Buffer management for Phase 7
bool Lexer::pushBuffer(std::string_view filename, std::string_view content) {
  // Check for cycles before pushing
  if (wouldCreateCycle(filename)) {
    reportError(LexError::RecursiveInclude,
                "Circular include detected: " + std::string(filename));
    return false;
  }

  bufferStack.push_back({filename, content, 0, 1, 1, 0});
  return true;
}

void Lexer::popBuffer() {
  if (bufferStack.size() > 1) {
    bufferStack.pop_back();
  }
}

// Core lexing methods
Token Lexer::lexNextToken() {
  char c = currentChar();
  const auto &buffer = currentBuffer();
  Position start(buffer.line, buffer.column, buffer.byteOffset);

  // Handle basic punctuation and operators
  switch (c) {
  case '+':
    advance();
    if (currentChar() == '+') {
      advance();
      return Token(TokenKind::PlusPlus, makeLocation(start));
    } else if (currentChar() == '=') {
      advance();
      return Token(TokenKind::PlusEqual, makeLocation(start));
    }
    return Token(TokenKind::Plus, makeLocation(start));
  case '-':
    advance();
    if (currentChar() == '-') {
      advance();
      return Token(TokenKind::MinusMinus, makeLocation(start));
    } else if (currentChar() == '=') {
      advance();
      return Token(TokenKind::MinusEqual, makeLocation(start));
    } else if (currentChar() == '>') {
      advance();
      return Token(TokenKind::ThinArrow, makeLocation(start));
    }
    return Token(TokenKind::Minus, makeLocation(start));
  case '*':
    advance();
    if (currentChar() == '=') {
      advance();
      return Token(TokenKind::MultEqual, makeLocation(start));
    }
    return Token(TokenKind::Mult, makeLocation(start));
  case '/':
    advance();
    if (currentChar() == '/') {
      // Line comment - skip to end of line
      skipLineComment();
      return nextToken(); // Use nextToken() to handle whitespace properly
    } else if (currentChar() == '*') {
      // Block comment - skip to closing */
      skipBlockComment();
      return nextToken(); // Use nextToken() to handle whitespace properly
    } else if (currentChar() == '=') {
      advance();
      return Token(TokenKind::DivEqual, makeLocation(start));
    }
    return Token(TokenKind::Div, makeLocation(start));
  case '%':
    advance();
    if (currentChar() == '=') {
      advance();
      return Token(TokenKind::ModEqual, makeLocation(start));
    }
    return Token(TokenKind::Mod, makeLocation(start));
  case '=':
    advance();
    if (currentChar() == '=') {
      advance();
      return Token(TokenKind::Equal, makeLocation(start));
    } else if (currentChar() == '>') {
      advance();
      return Token(TokenKind::FatArrow, makeLocation(start));
    }
    return Token(TokenKind::Assign, makeLocation(start));
  case '!':
    advance();
    if (currentChar() == '=') {
      advance();
      return Token(TokenKind::NotEqual, makeLocation(start));
    } else if (currentChar() == ':') {
      advance();
      return Token(TokenKind::BangColon, makeLocation(start));
    }
    return Token(TokenKind::LNot, makeLocation(start));
  case '<':
    advance();
    if (currentChar() == '=') {
      advance();
      return Token(TokenKind::LessEqual, makeLocation(start));
    } else if (currentChar() == '<') {
      advance();
      if (currentChar() == '=') {
        advance();
        return Token(TokenKind::ShlEqual, makeLocation(start));
      }
      return Token(TokenKind::Shl, makeLocation(start));
    }
    return Token(TokenKind::Less, makeLocation(start));
  case '>':
    advance();
    if (currentChar() == '=') {
      advance();
      return Token(TokenKind::GreaterEqual, makeLocation(start));
    } else if (currentChar() == '>') {
      advance();
      if (currentChar() == '=') {
        advance();
        return Token(TokenKind::ShrEqual, makeLocation(start));
      }
      return Token(TokenKind::Shr, makeLocation(start));
    }
    return Token(TokenKind::Greater, makeLocation(start));
  case '&':
    advance();
    if (currentChar() == '&') {
      advance();
      return Token(TokenKind::LAnd, makeLocation(start));
    } else if (currentChar() == '=') {
      advance();
      return Token(TokenKind::BAndEqual, makeLocation(start));
    } else if (currentChar() == '.') {
      advance();
      return Token(TokenKind::BAndDot, makeLocation(start));
    }
    return Token(TokenKind::BAnd, makeLocation(start));
  case '|':
    advance();
    if (currentChar() == '|') {
      advance();
      return Token(TokenKind::LOr, makeLocation(start));
    } else if (currentChar() == '=') {
      advance();
      return Token(TokenKind::BOrEqual, makeLocation(start));
    }
    return Token(TokenKind::BOr, makeLocation(start));
  case '^':
    advance();
    if (currentChar() == '=') {
      advance();
      return Token(TokenKind::BXorEqual, makeLocation(start));
    }
    return Token(TokenKind::BXor, makeLocation(start));
  case '~':
    advance();
    return Token(TokenKind::BNot, makeLocation(start));
  case ';':
    advance();
    return Token(TokenKind::Semicolon, makeLocation(start));
  case ',':
    advance();
    return Token(TokenKind::Comma, makeLocation(start));
  case ':':
    advance();
    return Token(TokenKind::Colon, makeLocation(start));
  case '?':
    advance();
    return Token(TokenKind::Question, makeLocation(start));
  case '@':
    advance();
    return Token(TokenKind::At, makeLocation(start));
  case '`':
    advance();
    return Token(TokenKind::Quote, makeLocation(start));
  case '#':
    advance();
    if (!isAtEnd() && currentChar() == '#') {
      advance();
      return Token(TokenKind::Define, makeLocation(start));
    } else if (!isAtEnd() && currentChar() == '.') {
      advance();
      return Token(TokenKind::AstMacroAccess, makeLocation(start));
    }
    return Token(TokenKind::Hash, makeLocation(start));
  case '.':
    advance();
    if (currentChar() == '.') {
      advance();
      if (currentChar() == '<') {
        advance();
        return Token(TokenKind::DotDotLess, makeLocation(start));
      }
      if (currentChar() == '.') {
        advance();
        return Token(TokenKind::Elipsis, makeLocation(start));
      }
      return Token(TokenKind::DotDot, makeLocation(start));
    }
    return Token(TokenKind::Dot, makeLocation(start));
  case '(':
    advance();
    return Token(TokenKind::LParen, makeLocation(start));
  case ')':
    advance();
    return Token(TokenKind::RParen, makeLocation(start));
  case '{':
    advance();
    if (inInterpolation() && currentInterpolationContext().inExpression) {
      currentInterpolationContext().braceDepth++;
    }
    return Token(TokenKind::LBrace, makeLocation(start));
  case '}':
    advance();
    if (inInterpolation() && currentInterpolationContext().inExpression) {
      if (currentInterpolationContext().braceDepth > 0) {
        currentInterpolationContext().braceDepth--;
        return Token(TokenKind::RBrace, makeLocation(start));
      } else {
        // End of interpolation expression - transition back to string parsing
        exitExpressionMode();
        return continueStringAfterExpression();
      }
    }
    return Token(TokenKind::RBrace, makeLocation(start));
  case '[':
    advance();
    return Token(TokenKind::LBracket, makeLocation(start));
  case ']':
    advance();
    return Token(TokenKind::RBracket, makeLocation(start));
  case '"': {
    // For nested strings within expressions, the string will manage its own
    // interpolation context via the stack
    return lexString();
  }
  case '\'':
    return lexCharacter();
  default:
    break;
  }

  // Handle identifiers, keywords, and raw strings
  if (isIdentifierStart(c)) {
    // Check for raw string literal
    if (c == 'r' && peekChar(1) == '"') {
      return lexRawString();
    }
    return lexIdentifierOrKeyword();
  }

  // Handle numbers (basic implementation for Phase 1)
  if (isDigit(c)) {
    return lexNumber();
  }

  // Unknown character - report error and skip
  reportError(LexError::InvalidCharacter,
              "Invalid character: '" + std::string(1, c) + "'");
  advance();
  return createErrorToken();
}

char Lexer::currentChar() const {
  if (isAtBufferEnd()) {
    return '\0';
  }
  return currentBuffer().content[currentBuffer().position];
}

char Lexer::peekChar(size_t offset) const {
  const auto &buffer = currentBuffer();
  size_t peekPos = buffer.position + offset;
  if (peekPos >= buffer.content.size()) {
    return '\0';
  }
  return buffer.content[peekPos];
}

void Lexer::advance() {
  if (!isAtBufferEnd()) {
    auto &buffer = currentBuffer();

    if (buffer.content[buffer.position] == '\n') {
      buffer.line++;
      buffer.column = 1;
    } else {
      buffer.column++;
    }

    buffer.position++;
    buffer.byteOffset++;
  }
}

void Lexer::skipWhitespace() {
  while (!isAtEnd() && isWhitespace(currentChar())) {
    advance();
  }
}

// Keyword lookup table generated from KEYWORD_LIST macro
static const std::unordered_map<std::string_view, TokenKind> keywordMap = {
#define KEYWORD_ENTRY(name, str) {str, TokenKind::name},
    KEYWORD_LIST(KEYWORD_ENTRY)
#undef KEYWORD_ENTRY
};

// Basic identifier/keyword lexing
Token Lexer::lexIdentifierOrKeyword() {
  const auto &buffer = currentBuffer();
  Position start(buffer.line, buffer.column, buffer.byteOffset);
  size_t startPos = buffer.position;

  while (!isAtEnd() && isIdentifierContinue(currentChar())) {
    advance();
  }

  size_t endPos = currentBuffer().position;
  std::string_view text(currentBuffer().content.data() + startPos,
                        endPos - startPos);

  // Check if the text is a keyword
  auto it = keywordMap.find(text);
  if (it != keywordMap.end()) {
    return Token(it->second, makeLocation(start));
  }

  // Intern the identifier text
  InternedString internedIdent = interner.intern(text);
  return Token(TokenKind::Ident, makeLocation(start), internedIdent);
}

// Enhanced number lexing with multiple bases, underscores, and type suffixes
// (Phase 2)
Token Lexer::lexNumber() {
  const auto &buffer = currentBuffer();
  Position start(buffer.line, buffer.column, buffer.byteOffset);
  size_t startPos = buffer.position;

  // Determine the base
  int base = 10;
  bool hasPrefix = false;

  if (currentChar() == '0' && !isAtBufferEnd()) {
    char next = peekChar(1);
    switch (next) {
    case 'x':
    case 'X':
      base = 16;
      hasPrefix = true;
      advance(); // skip '0'
      advance(); // skip 'x'/'X'
      break;
    case 'b':
    case 'B':
      base = 2;
      hasPrefix = true;
      advance(); // skip '0'
      advance(); // skip 'b'/'B'
      break;
    case 'o':
      base = 8;
      hasPrefix = true;
      advance(); // skip '0'
      advance(); // skip 'o'
      break;
    default:
      // Legacy octal or decimal starting with 0
      if (isDigit(next)) {
        base = 8; // Legacy octal
      } else {
        base = 10; // Just zero
      }
      break;
    }
  }

  // Parse the number with underscores
  __uint128_t value = 0;
  bool hasDigits = false;

  while (!isAtBufferEnd()) {
    char c = currentChar();

    if (c == '_') {
      // Skip underscore separators
      advance();
      continue;
    }

    // Check for floating point indicator
    if (c == '.') {
      // Look ahead to check if this is a range operator (.., ..<)
      if (buffer.position + 1 < buffer.content.size() &&
          buffer.content[buffer.position + 1] == '.') {
        // This is a range operator, not a decimal point
        // Return the integer and let the next token be the range operator
        break;
      }
      // This is a floating point number
      return lexFloat(start, base, value, hasDigits);
    } else if (c == 'e' || c == 'E' || (base == 16 && (c == 'p' || c == 'P'))) {
      // This is a floating point number
      return lexFloat(start, base, value, hasDigits);
    }

    // Check for float suffix (only for decimal base)
    if (base == 10 && (c == 'f' || c == 'F' || c == 'd' || c == 'D')) {
      // This is a floating point number with suffix
      return lexFloat(start, base, value, hasDigits);
    }

    int digit = -1;
    if (base <= 10) {
      if (c >= '0' && c < '0' + base) {
        digit = c - '0';
      }
    } else {
      // Hexadecimal
      if (c >= '0' && c <= '9') {
        digit = c - '0';
      } else if (c >= 'a' && c <= 'f') {
        digit = c - 'a' + 10;
      } else if (c >= 'A' && c <= 'F') {
        digit = c - 'A' + 10;
      }
    }

    if (digit == -1) {
      break; // Not a valid digit for this base
    }

    // Check for overflow before multiplying
    if (value > (__uint128_t(-1) / base)) {
      reportError(
          LexError::InvalidNumber,
          "Integer literal overflow: value too large for 128-bit integer");
      value = __uint128_t(-1); // Set to max value
    } else {
      __uint128_t newValue = value * base;
      if (newValue > __uint128_t(-1) - digit) {
        reportError(
            LexError::InvalidNumber,
            "Integer literal overflow: value too large for 128-bit integer");
        value = __uint128_t(-1); // Set to max value
      } else {
        value = newValue + digit;
      }
    }

    hasDigits = true;
    advance();
  }

  // Check if we have at least one digit
  if (!hasDigits || (hasPrefix && !hasDigits)) {
    reportError(LexError::InvalidNumber, "Invalid integer literal: no digits");
    return Token(TokenKind::Error, makeLocation(start));
  }

  // Parse type suffix
  IntegerType type = IntegerType::Auto;
  std::string_view suffix = parseTypeSuffix();

  if (!suffix.empty()) {
    type = parseIntegerTypeSuffix(suffix);
    if (type == IntegerType::Auto) {
      reportError(LexError::InvalidNumber,
                  "Invalid integer type suffix: " + std::string(suffix));
    }
  }

  return Token(TokenKind::IntLiteral, makeLocation(start), value, type);
}

// Enhanced floating-point lexing (Phase 3)
Token Lexer::lexFloat(const Position &start, int base, __uint128_t integerPart,
                      bool hasIntegerPart) {
  double value = static_cast<double>(integerPart);
  bool hasDecimalPart = false;
  bool hasExponent = false;

  // Handle decimal point and fractional part
  if (!isAtBufferEnd() && currentChar() == '.') {
    advance(); // consume '.'
    hasDecimalPart = true;

    double fractionalValue = 0.0;
    double fractionalDivisor = base;

    while (!isAtBufferEnd()) {
      char c = currentChar();

      if (c == '_') {
        // Skip underscore separators
        advance();
        continue;
      }

      int digit = -1;
      if (base <= 10) {
        if (c >= '0' && c < '0' + base) {
          digit = c - '0';
        }
      } else {
        // Hexadecimal
        if (c >= '0' && c <= '9') {
          digit = c - '0';
        } else if (c >= 'a' && c <= 'f') {
          digit = c - 'a' + 10;
        } else if (c >= 'A' && c <= 'F') {
          digit = c - 'A' + 10;
        }
      }

      if (digit == -1) {
        break;
      }

      fractionalValue += digit / fractionalDivisor;
      fractionalDivisor *= base;
      advance();
    }

    value += fractionalValue;
  }

  // Handle exponent (scientific notation)
  char expChar = (base == 16) ? 'p' : 'e';
  if (!isAtBufferEnd() &&
      (currentChar() == expChar || currentChar() == (expChar - 32))) {
    advance(); // consume 'e', 'E', 'p', or 'P'
    hasExponent = true;

    bool expNegative = false;
    if (!isAtBufferEnd() && (currentChar() == '+' || currentChar() == '-')) {
      expNegative = (currentChar() == '-');
      advance();
    }

    int exponentValue = 0;
    bool hasExpDigits = false;

    while (!isAtBufferEnd()) {
      char expChar = currentChar();

      if (expChar == '_') {
        // Skip underscore separators in exponent
        advance();
        continue;
      }

      if (!isDigit(expChar)) {
        break;
      }

      exponentValue = exponentValue * 10 + (expChar - '0');
      hasExpDigits = true;
      advance();
    }

    if (!hasExpDigits) {
      reportError(LexError::InvalidNumber, "Invalid exponent: no digits");
      return Token(TokenKind::Error, makeLocation(start));
    }

    if (expNegative) {
      exponentValue = -exponentValue;
    }

    // Apply exponent
    double expBase = (base == 16) ? 2.0 : 10.0;
    double expMultiplier = 1.0;
    if (exponentValue >= 0) {
      for (int i = 0; i < exponentValue; i++) {
        expMultiplier *= expBase;
      }
    } else {
      for (int i = 0; i < -exponentValue; i++) {
        expMultiplier /= expBase;
      }
    }
    value *= expMultiplier;
  }

  // Check that we have at least some part of the number
  if (!hasIntegerPart && !hasDecimalPart) {
    reportError(LexError::InvalidNumber,
                "Invalid floating-point literal: no digits");
    return Token(TokenKind::Error, makeLocation(start));
  }

  // Parse float type suffix
  FloatType type = FloatType::Auto;
  std::string_view suffix = parseTypeSuffix();

  if (!suffix.empty()) {
    type = parseFloatTypeSuffix(suffix);
    if (type == FloatType::Auto) {
      reportError(LexError::InvalidNumber,
                  "Invalid float type suffix: " + std::string(suffix));
    }
  }

  return Token(TokenKind::FloatLiteral, makeLocation(start), value, type);
}

// Basic string lexing (Phase 4) - supports multiline and interpolation
// detection (Phase 6)
Token Lexer::lexString() {
  const auto &buffer = currentBuffer();
  Position start(buffer.line, buffer.column, buffer.byteOffset);

  advance(); // consume opening quote

  // Check if this string contains interpolation
  if (hasInterpolation()) {
    // Start interpolated string parsing
    pushInterpolationContext();
    Token result = lexInterpolatedString();
    return result;
  } else {
    // Parse as regular string
    Token result = lexRegularString(start);
    return result;
  }
}

Token Lexer::lexRegularString(const Position &start) {
  // Scan string content and detect escapes
  const char *contentStart = &currentBuffer().content[currentBuffer().position];
  size_t sourceLength = 0;
  bool hasEscapes = false;
  size_t estimatedLength = 0;

  // Scan to find end and detect escapes
  while (!isAtBufferEnd() && currentChar() != '"') {
    if (currentChar() == '\\') {
      hasEscapes = true;
      advance(); // skip backslash
      if (!isAtBufferEnd()) {
        advance();            // skip escaped char
        estimatedLength += 1; // most escapes become 1 char
      }
      sourceLength += 2; // backslash + escaped char
    } else {
      advance();
      estimatedLength += 1;
      sourceLength += 1;
    }
  }

  if (isAtBufferEnd()) {
    reportError(LexError::UnterminatedString, "Unterminated string literal");
    return Token(TokenKind::Error, makeLocation(start));
  }

  advance(); // consume closing quote

  // Create processed string token using Option 1
  return createProcessedStringToken(contentStart, sourceLength, hasEscapes,
                                    estimatedLength, start,
                                    TokenKind::StringLiteral);
}

// Basic character lexing (Phase 4)
Token Lexer::lexCharacter() {
  const auto &buffer = currentBuffer();
  Position start(buffer.line, buffer.column, buffer.byteOffset);

  advance(); // consume opening quote

  if (isAtBufferEnd()) {
    reportError(LexError::UnterminatedString,
                "Unterminated character literal: EOF reached");
    return Token(TokenKind::Error, makeLocation(start));
  }

  uint32_t codepoint = 0;
  char c = currentChar();

  if (c == '\n') {
    reportError(LexError::UnterminatedString,
                "Unterminated character literal: newline in character");
    return Token(TokenKind::Error, makeLocation(start));
  }

  if (c == '\\') {
    // Handle escape sequence
    advance(); // consume backslash
    if (isAtBufferEnd()) {
      reportError(LexError::UnterminatedString,
                  "Unterminated character literal: escape at end");
      return Token(TokenKind::Error, makeLocation(start));
    }

    // Track error count before parsing escape sequence
    size_t errorCountBefore = logger.getErrorCount();
    codepoint = parseEscapeSequenceForChar();
    // If errors were reported during escape parsing, return error token
    if (logger.getErrorCount() > errorCountBefore) {
      // Still need to consume the closing quote if present
      if (!isAtBufferEnd() && currentChar() == '\'') {
        advance();
      }
      return Token(TokenKind::Error, makeLocation(start));
    }
  } else {
    // Regular character or UTF-8 sequence
    codepoint = parseUTF8Codepoint();
    if (codepoint == 0xFFFD) { // Replacement character indicates error
      reportError(LexError::InvalidUtf8,
                  "Invalid UTF-8 sequence in character literal");
      return Token(TokenKind::Error, makeLocation(start));
    }
  }

  if (isAtBufferEnd() || currentChar() != '\'') {
    reportError(LexError::UnterminatedString,
                "Unterminated character literal: missing closing quote");
    return Token(TokenKind::Error, makeLocation(start));
  }

  advance(); // consume closing quote

  return Token(TokenKind::CharLiteral, makeLocation(start), codepoint);
}

// Raw string lexing (Phase 4) - no escape processing, multiline allowed
Token Lexer::lexRawString() {
  const auto &buffer = currentBuffer();
  Position start(buffer.line, buffer.column, buffer.byteOffset);

  advance(); // consume 'r'
  advance(); // consume opening quote

  const char *contentStart = &currentBuffer().content[currentBuffer().position];
  size_t contentLength = 0;

  while (!isAtBufferEnd() && currentChar() != '"') {
    char c = currentChar();

    // In raw strings, no escape processing - everything is literal
    // Including newlines, backslashes, etc.
    if (!isValidUTF8Start(c)) {
      reportError(LexError::InvalidUtf8,
                  "Invalid UTF-8 sequence in raw string");
    }
    advance();
    contentLength++;
  }

  if (isAtBufferEnd()) {
    reportError(LexError::UnterminatedString,
                "Unterminated raw string literal: EOF reached");
    return Token(TokenKind::Error, makeLocation(start));
  }

  advance(); // consume closing quote

  // Raw strings: no escapes, just intern directly
  std::string_view rawContent(contentStart, contentLength);
  InternedString internedContent = interner.intern(rawContent);
  return Token(TokenKind::StringLiteral, makeLocation(start), internedContent);
}

// Phase 5: Comment handling
void Lexer::skipLineComment() {
  advance(); // consume second '/'

  // Skip until end of line or end of file
  while (!isAtBufferEnd() && currentChar() != '\n') {
    advance();
  }
  // Note: Don't advance past '\n' - let normal tokenization handle it
}

void Lexer::skipBlockComment() {
  advance(); // consume '*'

  size_t depth = 1; // Track nesting depth

  while (!isAtBufferEnd() && depth > 0) {
    char c = currentChar();

    if (c == '/' && peekChar(1) == '*') {
      // Nested block comment start
      advance(); // consume '/'
      advance(); // consume '*'
      depth++;
    } else if (c == '*' && peekChar(1) == '/') {
      // Block comment end
      advance(); // consume '*'
      advance(); // consume '/'
      depth--;
    } else {
      advance();
    }
  }

  // Check for unterminated block comment
  if (depth > 0) {
    reportError(LexError::UnterminatedComment,
                "Unterminated block comment: missing closing */");
  }
}

Token Lexer::lexSymbol() {
  // Additional symbol handling if needed
  reportError(LexError::InvalidCharacter, "Unknown symbol");
  return createErrorToken();
}

// Helper methods
bool Lexer::isAtBufferEnd() const {
  if (bufferStack.empty())
    return true;
  const auto &buffer = currentBuffer();
  return buffer.position >= buffer.content.size();
}

void Lexer::advancePosition() {
  // This is already handled in advance()
}

// Error reporting
void Lexer::reportError(LexError error, const std::string &message) {
  reportError(error, getCurrentLocation(), message);
}

void Lexer::reportError(LexError error, const Location &location,
                        const std::string &message) {
  logger.error(location, "Lexical error: {}", message);
}

Token Lexer::createErrorToken() {
  return Token(TokenKind::Error, getCurrentLocation());
}

// Phase 2: Number parsing helper implementations
std::string_view Lexer::parseTypeSuffix() {
  const auto &buffer = currentBuffer();
  size_t startPos = buffer.position;

  while (!isAtBufferEnd()) {
    char c = currentChar();

    // Type suffix characters: letters (for i8, u32, etc.)
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9')) {
      advance();
    } else {
      break;
    }
  }

  size_t endPos = currentBuffer().position;
  if (endPos > startPos) {
    return std::string_view(buffer.content.data() + startPos,
                            endPos - startPos);
  }
  return std::string_view();
}

IntegerType Lexer::parseIntegerTypeSuffix(const std::string_view &suffix) {
  // Modern suffixes (preferred)
  if (suffix == "i8")
    return IntegerType::I8;
  if (suffix == "u8")
    return IntegerType::U8;
  if (suffix == "i16")
    return IntegerType::I16;
  if (suffix == "u16")
    return IntegerType::U16;
  if (suffix == "i32")
    return IntegerType::I32;
  if (suffix == "u32")
    return IntegerType::U32;
  if (suffix == "i64")
    return IntegerType::I64;
  if (suffix == "u64")
    return IntegerType::U64;
  if (suffix == "i128")
    return IntegerType::I128;
  if (suffix == "u128")
    return IntegerType::U128;

  // Legacy C-style suffixes (for compatibility)
  if (suffix == "u" || suffix == "U")
    return IntegerType::U32;
  if (suffix == "l" || suffix == "L")
    return IntegerType::I64;
  if (suffix == "ul" || suffix == "uL" || suffix == "Ul" || suffix == "UL" ||
      suffix == "lu" || suffix == "lU" || suffix == "Lu" || suffix == "LU")
    return IntegerType::U64;
  if (suffix == "ll" || suffix == "LL")
    return IntegerType::I64;
  if (suffix == "ull" || suffix == "uLL" || suffix == "Ull" ||
      suffix == "ULL" || suffix == "llu" || suffix == "llU" ||
      suffix == "LLu" || suffix == "LLU")
    return IntegerType::U64;

  // Unknown suffix
  return IntegerType::Auto;
}

// Phase 3: Float type suffix parsing
FloatType Lexer::parseFloatTypeSuffix(const std::string_view &suffix) {
  if (suffix == "f" || suffix == "F")
    return FloatType::F32;
  if (suffix == "d" || suffix == "D")
    return FloatType::F64;

  // Unknown suffix
  return FloatType::Auto;
}

// Phase 4: String/Character parsing helpers
char Lexer::parseBasicEscapeSequence() {
  char c = currentChar();
  advance();

  switch (c) {
  case 'n':
    return '\n';
  case 'r':
    return '\r';
  case 't':
    return '\t';
  case '\\':
    return '\\';
  case '\'':
    return '\'';
  case '\"':
    return '\"';
  case '0':
    return '\0';
  case 'x': {
    // Hex escape sequence: \xNN
    if (isAtBufferEnd() || !isHexDigit(currentChar())) {
      reportError(LexError::InvalidEscape,
                  "Invalid hex escape: expected hex digit");
      return '\0';
    }

    int value = 0;
    for (int i = 0; i < 2 && !isAtBufferEnd() && isHexDigit(currentChar());
         i++) {
      char hexChar = currentChar();
      if (hexChar >= '0' && hexChar <= '9') {
        value = value * 16 + (hexChar - '0');
      } else if (hexChar >= 'a' && hexChar <= 'f') {
        value = value * 16 + (hexChar - 'a' + 10);
      } else if (hexChar >= 'A' && hexChar <= 'F') {
        value = value * 16 + (hexChar - 'A' + 10);
      }
      advance();
    }
    return static_cast<char>(value);
  }
  default:
    // Let caller handle Unicode escapes or report unknown escapes
    return c; // Indicates unhandled escape
  }
}

// Parse escape sequences for character literals, returning full Unicode
// codepoint
uint32_t Lexer::parseEscapeSequenceForChar() {
  char c = currentChar();

  switch (c) {
  case 'n':
    advance();
    return '\n';
  case 'r':
    advance();
    return '\r';
  case 't':
    advance();
    return '\t';
  case '\\':
    advance();
    return '\\';
  case '\'':
    advance();
    return '\'';
  case '\"':
    advance();
    return '\"';
  case '0':
    advance();
    return '\0';
  case 'x': {
    advance(); // consume 'x'
    // Hex escape sequence: \xNN
    if (isAtBufferEnd() || !isHexDigit(currentChar())) {
      reportError(LexError::InvalidEscape,
                  "Invalid hex escape: expected hex digit");
      return 0xFFFD;
    }

    int value = 0;
    for (int i = 0; i < 2 && !isAtBufferEnd() && isHexDigit(currentChar());
         i++) {
      char hexChar = currentChar();
      if (hexChar >= '0' && hexChar <= '9') {
        value = value * 16 + (hexChar - '0');
      } else if (hexChar >= 'a' && hexChar <= 'f') {
        value = value * 16 + (hexChar - 'a' + 10);
      } else if (hexChar >= 'A' && hexChar <= 'F') {
        value = value * 16 + (hexChar - 'A' + 10);
      }
      advance();
    }
    return static_cast<uint32_t>(static_cast<unsigned char>(value));
  }
  case 'u': {
    advance(); // consume 'u'
    // Unicode escape sequence: \uNNNN or \u{...}
    if (!isAtBufferEnd() && currentChar() == '{') {
      advance();                                   // consume '{'
      uint32_t codepoint = parseUnicodeEscape(-1); // Variable length
      if (isAtBufferEnd() || currentChar() != '}') {
        reportError(LexError::InvalidUnicodeEscape,
                    "Missing closing brace in Unicode escape");
        return 0xFFFD;
      }
      advance(); // consume '}'
      return codepoint;
    } else {
      // Fixed 4-digit Unicode escape
      return parseUnicodeEscape(4);
    }
  }
  case 'U': {
    advance(); // consume 'U'
    // Fixed 8-digit Unicode escape
    return parseUnicodeEscape(8);
  }
  default:
    advance(); // consume the unknown escape character
    reportError(LexError::InvalidEscape,
                "Unknown escape sequence: \\" + std::string(1, c));
    return static_cast<uint32_t>(static_cast<unsigned char>(c));
  }
}

uint32_t Lexer::parseUnicodeEscape(int digitCount) {
  uint32_t value = 0;
  int count = 0;

  while (!isAtBufferEnd() && (digitCount == -1 || count < digitCount)) {
    char c = currentChar();

    if (!isHexDigit(c)) {
      if (digitCount == -1) {
        // Variable length, stop at non-hex
        break;
      } else {
        // Fixed length, this is an error
        reportError(LexError::InvalidUnicodeEscape,
                    "Expected hex digit in Unicode escape");
        return 0xFFFD; // Replacement character
      }
    }

    int digit;
    if (c >= '0' && c <= '9') {
      digit = c - '0';
    } else if (c >= 'a' && c <= 'f') {
      digit = c - 'a' + 10;
    } else if (c >= 'A' && c <= 'F') {
      digit = c - 'A' + 10;
    } else {
      break;
    }

    value = value * 16 + digit;
    advance();
    count++;

    // Check for overflow
    if (value > 0x10FFFF) {
      reportError(LexError::InvalidUnicodeEscape,
                  "Unicode codepoint out of range");
      return 0xFFFD;
    }
  }

  if (digitCount != -1 && count < digitCount) {
    reportError(LexError::InvalidUnicodeEscape,
                "Incomplete Unicode escape sequence");
    return 0xFFFD;
  }

  if (count == 0) {
    reportError(LexError::InvalidUnicodeEscape,
                "Empty Unicode escape sequence");
    return 0xFFFD;
  }

  return value;
}

uint32_t Lexer::parseUTF8Codepoint() {
  if (isAtBufferEnd()) {
    return 0xFFFD;
  }

  unsigned char first = static_cast<unsigned char>(currentChar());
  advance();

  // ASCII character
  if (first <= 0x7F) {
    return static_cast<uint32_t>(first);
  }

  // Multi-byte UTF-8 sequence
  uint32_t codepoint = 0;
  int extraBytes = 0;

  if ((first & 0xE0) == 0xC0) {
    // 2-byte sequence
    codepoint = first & 0x1F;
    extraBytes = 1;
  } else if ((first & 0xF0) == 0xE0) {
    // 3-byte sequence
    codepoint = first & 0x0F;
    extraBytes = 2;
  } else if ((first & 0xF8) == 0xF0) {
    // 4-byte sequence
    codepoint = first & 0x07;
    extraBytes = 3;
  } else {
    // Invalid UTF-8 start byte
    return 0xFFFD;
  }

  // Read continuation bytes
  for (int i = 0; i < extraBytes; i++) {
    if (isAtBufferEnd()) {
      return 0xFFFD;
    }

    unsigned char cont = static_cast<unsigned char>(currentChar());
    if ((cont & 0xC0) != 0x80) {
      // Invalid continuation byte
      return 0xFFFD;
    }

    codepoint = (codepoint << 6) | (cont & 0x3F);
    advance();
  }

  // Validate the codepoint
  if (codepoint > 0x10FFFF || (codepoint >= 0xD800 && codepoint <= 0xDFFF)) {
    return 0xFFFD;
  }

  return codepoint;
}

bool Lexer::isValidUTF8Start(char c) const {
  unsigned char byte = static_cast<unsigned char>(c);
  // Valid UTF-8 start bytes: 0xxxxxxx (ASCII) or 110xxxxx, 1110xxxx, 11110xxx
  return (byte <= 0x7F) || ((byte & 0xE0) == 0xC0) || ((byte & 0xF0) == 0xE0) ||
         ((byte & 0xF8) == 0xF0);
}

// Character classification helpers
bool Lexer::isIdentifierStart(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Lexer::isIdentifierContinue(char c) {
  return isIdentifierStart(c) || (c >= '0' && c <= '9');
}

bool Lexer::isDigit(char c) { return c >= '0' && c <= '9'; }

bool Lexer::isHexDigit(char c) {
  return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
         (c >= 'A' && c <= 'F');
}

bool Lexer::isWhitespace(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

// Phase 6: String Interpolation Implementation
Token Lexer::lexInterpolatedString() {
  const auto &buffer = currentBuffer();
  Position start(buffer.line, buffer.column, buffer.byteOffset);

  // Scan string content until '{' or '"'
  InterpolatedScanResult result = scanInterpolatedStringContent(start);

  if (result.foundInterpolation) {
    // Found '{' - create LString token with content before '{'
    advance(); // consume '{' to enter expression mode
    enterExpressionMode();

    // Check for empty interpolation
    skipWhitespace();
    if (currentChar() == '}') {
      reportError(LexError::InvalidInterpolation,
                  "Empty interpolation '{}' is not allowed");
      return Token(TokenKind::Error, makeLocation(start));
    }

    return createProcessedStringToken(result.contentStart, result.sourceLength,
                                      result.hasEscapes, result.estimatedLength,
                                      start, TokenKind::LString);
  } else {
    // Reached end of string without interpolation - this should be an RString
    if (isAtBufferEnd()) {
      reportError(LexError::UnterminatedString,
                  "Unterminated string literal: EOF reached");
      return Token(TokenKind::Error, makeLocation(start));
    }

    advance(); // consume closing quote
    popInterpolationContext();

    return createProcessedStringToken(result.contentStart, result.sourceLength,
                                      result.hasEscapes, result.estimatedLength,
                                      start, TokenKind::RString);
  }
}

Token Lexer::continueStringAfterExpression() {
  // We've just finished parsing an expression and need to continue the string
  // The lexer should be positioned right after the '}'
  const auto &buffer = currentBuffer();
  Position start(buffer.line, buffer.column, buffer.byteOffset);

  // Scan string content until '{' or '"'
  InterpolatedScanResult result = scanInterpolatedStringContent(start);

  if (result.foundInterpolation) {
    // Found '{' - create StringLiteral token with content before '{'
    advance(); // consume '{' to enter expression mode
    enterExpressionMode();

    // Check for empty interpolation
    skipWhitespace();
    if (currentChar() == '}') {
      reportError(LexError::InvalidInterpolation,
                  "Empty interpolation '{}' is not allowed");
      return Token(TokenKind::Error, makeLocation(start));
    }

    return createProcessedStringToken(result.contentStart, result.sourceLength,
                                      result.hasEscapes, result.estimatedLength,
                                      start, TokenKind::StringLiteral);
  } else {
    // Reached end of string - create RString token
    if (isAtBufferEnd()) {
      reportError(LexError::UnterminatedString,
                  "Unterminated string literal: EOF reached");
      return Token(TokenKind::Error, makeLocation(start));
    }

    advance(); // consume closing quote
    popInterpolationContext();

    return createProcessedStringToken(result.contentStart, result.sourceLength,
                                      result.hasEscapes, result.estimatedLength,
                                      start, TokenKind::RString);
  }
}

bool Lexer::hasInterpolation() {
  // Scan ahead to detect if string contains unescaped '{'
  const auto &buffer = currentBuffer();
  size_t savedPos = buffer.position;
  size_t savedLine = buffer.line;
  size_t savedCol = buffer.column;
  size_t savedOffset = buffer.byteOffset;

  bool foundInterpolation = false;

  while (!isAtBufferEnd() && currentChar() != '"') {
    char c = currentChar();

    if (c == '\\') {
      // Handle escape sequences properly
      advance(); // consume backslash
      if (!isAtBufferEnd()) {
        char escapeChar = currentChar();
        advance(); // consume escape character

        // Handle Unicode escapes with braces: \u{...}
        if (escapeChar == 'u' && !isAtBufferEnd() && currentChar() == '{') {
          advance(); // consume '{'
          // Skip until closing '}'
          while (!isAtBufferEnd() && currentChar() != '}') {
            advance();
          }
          if (!isAtBufferEnd() && currentChar() == '}') {
            advance(); // consume '}'
          }
        }
      }
    } else if (c == '{') {
      foundInterpolation = true;
      break;
    } else {
      advance();
    }
  }

  // Restore position
  auto &mutableBuffer = currentBuffer();
  mutableBuffer.position = savedPos;
  mutableBuffer.line = savedLine;
  mutableBuffer.column = savedCol;
  mutableBuffer.byteOffset = savedOffset;

  return foundInterpolation;
}

void Lexer::pushInterpolationContext() {
  InterpolationContext context;
  context.active = true;
  context.inExpression = false;
  context.braceDepth = 0;
  interpolationStack.push_back(context);
}

void Lexer::popInterpolationContext() {
  if (!interpolationStack.empty()) {
    interpolationStack.pop_back();
  }
}

void Lexer::enterExpressionMode() {
  if (!interpolationStack.empty()) {
    currentInterpolationContext().inExpression = true;
    currentInterpolationContext().braceDepth = 0;
  }
}

void Lexer::exitExpressionMode() {
  if (!interpolationStack.empty()) {
    currentInterpolationContext().inExpression = false;
    currentInterpolationContext().braceDepth = 0;
  }
}

Lexer::InterpolationContext &Lexer::currentInterpolationContext() {
  return interpolationStack.back();
}

const Lexer::InterpolationContext &Lexer::currentInterpolationContext() const {
  return interpolationStack.back();
}

bool Lexer::inInterpolation() const {
  return !interpolationStack.empty() && interpolationStack.back().active;
}

bool Lexer::wouldCreateCycle(std::string_view filename) const {
  // Check if filename is already in the buffer stack
  for (const auto &buffer : bufferStack) {
    if (buffer.filename == filename) {
      return true;
    }
  }
  return false;
}

// Buffer access
Lexer::LexerBuffer &Lexer::currentBuffer() { return bufferStack.back(); }

const Lexer::LexerBuffer &Lexer::currentBuffer() const {
  return bufferStack.back();
}

// String processing implementation using Option 1 (stack+heap strategy)
Token Lexer::createProcessedStringToken(const char *contentStart,
                                        size_t sourceLength, bool hasEscapes,
                                        size_t estimatedLength,
                                        const Position &start,
                                        TokenKind tokenKind) {
  if (!hasEscapes) {
    // No escapes: intern directly
    std::string_view content(contentStart, sourceLength);
    InternedString internedContent = interner.intern(content);
    return Token(tokenKind, makeLocation(start), internedContent);
  }

  // Has escapes: use temporary buffer
  constexpr size_t STACK_BUFFER_SIZE = 512;

  if (estimatedLength <= STACK_BUFFER_SIZE) {
    // Small strings: use stack buffer
    char stackBuffer[STACK_BUFFER_SIZE];
    size_t actualLength =
        processEscapeSequences(contentStart, sourceLength, stackBuffer);

    std::string_view processedView(stackBuffer, actualLength);
    InternedString internedContent = interner.intern(processedView);
    return Token(tokenKind, makeLocation(start), internedContent);
    // stackBuffer automatically freed when function exits
  } else {
    // Large strings: use heap allocation
    std::unique_ptr<char[]> heapBuffer =
        std::make_unique<char[]>(estimatedLength + 1);
    size_t actualLength =
        processEscapeSequences(contentStart, sourceLength, heapBuffer.get());

    std::string_view processedView(heapBuffer.get(), actualLength);
    InternedString internedContent = interner.intern(processedView);
    return Token(tokenKind, makeLocation(start), internedContent);
    // heapBuffer automatically freed when unique_ptr goes out of scope
  }
}

// Helper function to scan interpolated string content until { or "
struct InterpolatedScanResult {
  const char *contentStart;
  size_t sourceLength;
  bool hasEscapes;
  size_t estimatedLength;
  bool foundInterpolation; // true if stopped at {, false if stopped at "
};

Lexer::InterpolatedScanResult
Lexer::scanInterpolatedStringContent(const Position &start) {
  const auto &buffer = currentBuffer();
  const char *contentStart = &buffer.content[buffer.position];
  size_t sourceLength = 0;
  bool hasEscapes = false;
  size_t estimatedLength = 0;
  bool foundInterpolation = false;

  // Scan until we hit { or "
  while (!isAtBufferEnd() && currentChar() != '"' && currentChar() != '{') {
    if (currentChar() == '\\') {
      hasEscapes = true;
      advance(); // skip backslash
      if (!isAtBufferEnd()) {
        advance();            // skip escaped char
        estimatedLength += 1; // most escapes become 1 char
      }
      sourceLength += 2; // backslash + escaped char
    } else {
      advance();
      estimatedLength += 1;
      sourceLength += 1;
    }
  }

  // Check what stopped us
  if (!isAtBufferEnd() && currentChar() == '{') {
    foundInterpolation = true;
  }

  return {contentStart, sourceLength, hasEscapes, estimatedLength,
          foundInterpolation};
}

size_t Lexer::processEscapeSequences(const char *source, size_t sourceLength,
                                     char *dest) {
  size_t writePos = 0;

  for (size_t i = 0; i < sourceLength; ++i) {
    if (source[i] == '\\' && i + 1 < sourceLength) {
      // Process escape sequence
      char next = source[i + 1];
      switch (next) {
      case 'n':
        dest[writePos++] = '\n';
        i++; // skip the escaped character
        break;
      case 't':
        dest[writePos++] = '\t';
        i++;
        break;
      case 'r':
        dest[writePos++] = '\r';
        i++;
        break;
      case '\\':
        dest[writePos++] = '\\';
        i++;
        break;
      case '"':
        dest[writePos++] = '"';
        i++;
        break;
      case '0':
        dest[writePos++] = '\0';
        i++;
        break;
      case '{':
        dest[writePos++] = '{';
        i++;
        break;
      case '}':
        dest[writePos++] = '}';
        i++;
        break;
      case 'u': {
        // Handle Unicode escapes \u{...}
        if (i + 2 < sourceLength && source[i + 2] == '{') {
          // Find closing }
          size_t closePos = i + 3;
          while (closePos < sourceLength && source[closePos] != '}') {
            closePos++;
          }
          if (closePos < sourceLength) {
            // Parse hex digits between { and }
            uint32_t codepoint = 0;
            bool valid = true;
            for (size_t j = i + 3; j < closePos; j++) {
              char c = source[j];
              if (c >= '0' && c <= '9') {
                codepoint = (codepoint << 4) | (c - '0');
              } else if (c >= 'a' && c <= 'f') {
                codepoint = (codepoint << 4) | (c - 'a' + 10);
              } else if (c >= 'A' && c <= 'F') {
                codepoint = (codepoint << 4) | (c - 'A' + 10);
              } else {
                valid = false;
                break;
              }
            }
            if (valid && codepoint <= 0x10FFFF) {
              // Encode as UTF-8
              if (codepoint < 0x80) {
                dest[writePos++] = static_cast<char>(codepoint);
              } else if (codepoint < 0x800) {
                dest[writePos++] = static_cast<char>(0xC0 | (codepoint >> 6));
                dest[writePos++] = static_cast<char>(0x80 | (codepoint & 0x3F));
              } else if (codepoint < 0x10000) {
                dest[writePos++] = static_cast<char>(0xE0 | (codepoint >> 12));
                dest[writePos++] =
                    static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
                dest[writePos++] = static_cast<char>(0x80 | (codepoint & 0x3F));
              } else {
                dest[writePos++] = static_cast<char>(0xF0 | (codepoint >> 18));
                dest[writePos++] =
                    static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
                dest[writePos++] =
                    static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
                dest[writePos++] = static_cast<char>(0x80 | (codepoint & 0x3F));
              }
              i = closePos; // skip past the }
            } else {
              // Invalid Unicode escape, copy literally
              dest[writePos++] = source[i];
            }
          } else {
            // Malformed escape, copy literally
            dest[writePos++] = source[i];
          }
        } else {
          // Not a valid \u{...} escape, copy literally
          dest[writePos++] = source[i];
        }
        break;
      }
      default:
        // Unknown escape sequence, copy both characters literally
        dest[writePos++] = source[i];     // backslash
        dest[writePos++] = source[i + 1]; // escaped char
        i++;
        break;
      }
    } else {
      // Regular character
      dest[writePos++] = source[i];
    }
  }

  return writePos;
}

} // namespace cxy
