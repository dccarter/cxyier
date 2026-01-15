#pragma once

#include "cxy/arena_allocator.hpp"
#include "cxy/ast/node.hpp"
#include "cxy/diagnostics.hpp"
#include "cxy/lexer.hpp"
#include "cxy/token.hpp"

#include <string>
#include <vector>

namespace cxy {

/**
 * @brief Parse error types for diagnostic reporting.
 */
enum class ParseErrorType {
  UnexpectedToken,   // Got unexpected token
  MissingToken,      // Expected token not found
  InvalidExpression, // Malformed expression
  InvalidStatement,  // Malformed statement
  InvalidDeclaration // Malformed declaration
};

/**
 * @brief Parse error information for error recovery and reporting.
 */
class ParseError {
public:
  ParseErrorType type;
  Location location;
  std::string message;
  std::vector<TokenKind> expectedTokens;
  Token actualToken;

  ParseError(ParseErrorType error_type, Location loc, std::string msg,
             Token actual = Token{})
      : type(error_type), location(loc), message(std::move(msg)),
        actualToken(actual) {}

  ParseError(ParseErrorType error_type, Location loc, std::string msg,
             std::vector<TokenKind> expected, Token actual)
      : type(error_type), location(loc), message(std::move(msg)),
        expectedTokens(std::move(expected)), actualToken(actual) {}
};

/**
 * @brief CXY recursive descent parser with LL(3) lookahead.
 *
 * The parser transforms a stream of tokens from the lexer into an Abstract
 * Syntax Tree (AST). It uses a sliding window of 4 tokens to make parsing
 * decisions and supports error recovery.
 */
class Parser {
public:
  /**
   * @brief Construct a parser with the given lexer and arena allocator.
   *
   * @param lexer Token source for parsing
   * @param arena Memory allocator for AST nodes
   * @param sourceManager Source manager for reading token text
   * @param interner String interner for caching identifier and string values
   * @param diagnostics Diagnostic logger for error reporting
   */
  explicit Parser(Lexer &lexer, ArenaAllocator &arena,
                  SourceManager &sourceManager, StringInterner &interner,
                  DiagnosticLogger &diagnostics);

  /**
   * @brief Initialize the parser by preloading the token buffer.
   *
   * Must be called before any parsing operations. Loads the initial
   * 4 tokens into the sliding window buffer.
   */
  void initialize();

  // Phase 2: Expression parsing interface

  /**
   * @brief Parse a complete expression.
   *
   * Follows Phase 2 grammar hierarchy.
   *
   * @return Parsed expression AST node, or nullptr on error
   */
  ast::ASTNode *parseExpression();

  /**
   * @brief Parse a relational expression (<, <=, >, >=).
   *
   * relational_expression ::=
   *   | shift_expression
   *   | relational_expression '<' shift_expression
   *   | relational_expression '<=' shift_expression
   *   | relational_expression '>' shift_expression
   *   | relational_expression '>=' shift_expression
   *
   * @return Parsed relational expression AST node, or nullptr on error
   */
  ast::ASTNode *parseRelationalExpression();

  /**
   * @brief Parse an equality expression (== and !=).
   *
   * equality_expression ::=
   *   | relational_expression
   *   | equality_expression '==' relational_expression
   *   | equality_expression '!=' relational_expression
   *
   * @return Parsed equality expression AST node, or nullptr on error
   */
  ast::ASTNode *parseEqualityExpression();

  /**
   * @brief Parse a bitwise AND expression (&).
   *
   * bitwise_and_expression ::=
   *   | equality_expression
   *   | bitwise_and_expression '&' equality_expression
   *
   * @return Parsed bitwise AND expression AST node, or nullptr on error
   */
  ast::ASTNode *parseBitwiseAndExpression();

  /**
   * @brief Parse a bitwise XOR expression (^).
   *
   * bitwise_xor_expression ::=
   *   | bitwise_and_expression
   *   | bitwise_xor_expression '^' bitwise_and_expression
   *
   * @return Parsed bitwise XOR expression AST node, or nullptr on error
   */
  ast::ASTNode *parseBitwiseXorExpression();

  /**
   * @brief Parse a bitwise OR expression (|).
   *
   * bitwise_or_expression ::=
   *   | bitwise_xor_expression
   *   | bitwise_or_expression '|' bitwise_xor_expression
   *
   * @return Parsed bitwise OR expression AST node, or nullptr on error
   */
  ast::ASTNode *parseBitwiseOrExpression();

  /**
   * @brief Parse a logical AND expression (&&).
   *
   * logical_and_expression ::=
   *   | bitwise_or_expression
   *   | logical_and_expression '&&' bitwise_or_expression
   *
   * @return Parsed logical AND expression AST node, or nullptr on error
   */
  ast::ASTNode *parseLogicalAndExpression();

  /**
   * @brief Parse a logical OR expression (||).
   *
   * logical_or_expression ::=
   *   | logical_and_expression
   *   | logical_or_expression '||' logical_and_expression
   *
   * @return Parsed logical OR expression AST node, or nullptr on error
   */
  ast::ASTNode *parseLogicalOrExpression();

  /**
   * @brief Parse a conditional expression (ternary ?:).
   *
   * conditional_expression ::=
   *   | logical_or_expression
   *   | logical_or_expression '?' expression ':' conditional_expression
   *
   * @return Parsed conditional expression AST node, or nullptr on error
   */
  ast::ASTNode *parseConditionalExpression();

  /**
   * @brief Parse an assignment expression (=, +=, -=, etc.).
   *
   * assignment_expression ::=
   *   | conditional_expression
   *   | conditional_expression assignment_operator assignment_expression
   *
   * @return Parsed assignment expression AST node, or nullptr on error
   */
  ast::ASTNode *parseAssignmentExpression();

  /**
   * @brief Parse a shift expression (<< and >>).
   *
   * shift_expression ::=
   *   | additive_expression
   *   | shift_expression '<<' additive_expression
   *   | shift_expression '>>' additive_expression
   *
   * @return Parsed shift expression AST node, or nullptr on error
   */
  ast::ASTNode *parseShiftExpression();

  /**
   * @brief Parse an additive expression (+ and -).
   *
   * additive_expression ::=
   *   | multiplicative_expression
   *   | additive_expression '+' multiplicative_expression
   *   | additive_expression '-' multiplicative_expression
   *
   * @return Parsed additive expression AST node, or nullptr on error
   */
  ast::ASTNode *parseAdditiveExpression();

  /**
   * @brief Parse a multiplicative expression (*, /, %).
   *
   * multiplicative_expression ::=
   *   | unary_expression
   *   | multiplicative_expression '*' unary_expression
   *   | multiplicative_expression '/' unary_expression
   *   | multiplicative_expression '%' unary_expression
   *
   * @return Parsed multiplicative expression AST node, or nullptr on error
   */
  ast::ASTNode *parseMultiplicativeExpression();

  /**
   * @brief Parse a unary expression (prefix operators).
   *
   * unary_expression ::=
   *   | postfix_expression
   *   | '++' unary_expression
   *   | '--' unary_expression
   *   | '+' unary_expression
   *   | '-' unary_expression
   *   | '!' unary_expression
   *   | '~' unary_expression
   *   | '&' unary_expression
   *   | '&&' unary_expression
   *
   * @return Parsed unary expression AST node, or nullptr on error
   */
  ast::ASTNode *parseUnaryExpression();

  /**
   * @brief Parse a cast expression (as, !: operators).
   *
   * cast_expression ::=
   *   | postfix_expression
   *   | cast_expression 'as' type_expression
   *   | cast_expression '!:' type_expression
   *
   * @return Parsed cast expression AST node, or nullptr on error
   */
  ast::ASTNode *parseCastExpression();

  /**
   * @brief Parse a type expression (primitive types).
   *
   * type_expression ::= primitive_type
   *
   * @return Parsed type expression AST node, or nullptr on error
   */
  ast::ASTNode *parseTypeExpression();

  /**
   * @brief Parse a postfix expression (postfix operators).
   *
   * postfix_expression ::=
   *   | primary_expression
   *   | postfix_expression '++'
   *   | postfix_expression '--'
   *
   * @return Parsed postfix expression AST node, or nullptr on error
   */
  ast::ASTNode *parsePostfixExpression();

  /**
   * @brief Parse a primary expression (literals, identifiers, parenthesized).
   *
   * primary_expression ::=
   *   | literal_expression
   *   | identifier_expression
   *   | '(' expression ')'
   *
   * @return Parsed primary expression AST node, or nullptr on error
   */
  ast::ASTNode *parsePrimaryExpression();

  /**
   * @brief Parse a literal expression of any type.
   *
   * literal_expression ::=
   *   | integer_literal
   *   | float_literal
   *   | character_literal
   *   | string_literal
   *   | boolean_literal
   *   | null_literal
   *
   * @return Parsed literal AST node, or nullptr on error
   */
  ast::ASTNode *parseLiteralExpression();

  /**
   * @brief Parse an identifier expression.
   *
   * identifier_expression ::= Ident
   *
   * @return Parsed identifier AST node, or nullptr on error
   */
  ast::ASTNode *parseIdentifierExpression();

  /**
   * @brief Parse an array literal expression.
   *
   * array_literal ::= '[' array_element_list? ']'
   * array_element_list ::= expression (',' expression)*
   *
   * @return Parsed array literal AST node, or nullptr on error
   */
  ast::ASTNode *parseArrayLiteral();

  /**
   * @brief Parse a tuple literal or grouped expression.
   *
   * tuple_literal ::= '(' expression (',' expression)+ ')'
   * grouped_expression ::= '(' expression ')'
   *
   * Disambiguates between tuples and grouped expressions based on comma
   * presence.
   *
   * @return Parsed tuple or grouped expression AST node, or nullptr on error
   */
  ast::ASTNode *parseTupleOrGroupedExpression();

  // Token buffer access methods

  /**
   * @brief Get the current token being processed.
   *
   * @return Current token (tokens_[1])
   */
  Token current() const { return tokens_[1]; }

  /**
   * @brief Get a lookahead token at the specified offset.
   *
   * @param offset Lookahead offset (1 or 2 for LL(3))
   * @return Lookahead token (tokens_[1 + offset])
   */
  Token lookahead(int offset = 1) const {
    if (offset < 1 || offset > 2)
      return Token{};
    return tokens_[1 + offset];
  }

  /**
   * @brief Get the previous token for error reporting.
   *
   * @return Previous token (tokens_[0])
   */
  Token previous() const { return tokens_[0]; }

  /**
   * @brief Advance to the next token in the stream.
   *
   * Shifts the token buffer: previous <- current, current <- lookahead1,
   * lookahead1 <- lookahead2, lookahead2 <- lexer.nextToken()
   */
  void advance();

  // Token consumption and expectation utilities

  /**
   * @brief Check if the current token matches the expected kind.
   *
   * @param kind Expected token kind
   * @return True if current token matches
   */
  bool check(TokenKind kind) const { return current().kind == kind; }

  /**
   * @brief Check if the current token matches any of the expected kinds.
   *
   * @param kinds Vector of expected token kinds
   * @return True if current token matches any of them
   */
  bool checkAny(const std::vector<TokenKind> &kinds) const;

  /**
   * @brief Consume the current token if it matches the expected kind.
   *
   * @param kind Expected token kind
   * @return True if token was consumed, false otherwise
   */
  bool match(TokenKind kind);

  /**
   * @brief Consume the current token, expecting it to be of the given kind.
   *
   * Reports an error if the token doesn't match and returns false.
   *
   * @param kind Expected token kind
   * @param errorMessage Custom error message if expectation fails
   * @return True if token was consumed, false on error
   */
  bool expect(TokenKind kind, const std::string &errorMessage = "");

  // Error handling and recovery

  /**
   * @brief Report a parse error with diagnostic information.
   *
   * @param error Parse error information
   */
  void reportError(const ParseError &error);

  /**
   * @brief Create a parse error for an unexpected token.
   *
   * @param expected Expected token kind(s)
   * @param message Custom error message
   * @return ParseError object
   */
  ParseError createUnexpectedTokenError(TokenKind expected,
                                        const std::string &message = "");
  ParseError createUnexpectedTokenError(const std::vector<TokenKind> &expected,
                                        const std::string &message = "");

  /**
   * @brief Check if we're at the end of the token stream.
   *
   * @return True if current token is EOF
   */
  bool isAtEnd() const { return current().isEof(); }

private:
  // LL(3) sliding window buffer
  Token
      tokens_[4]; ///< Token buffer: [previous, current, lookahead1, lookahead2]

  // Core parser state
  Lexer &lexer_;                  ///< Token source
  ArenaAllocator &arena_;         ///< Memory allocator for AST nodes
  SourceManager &sourceManager_;  ///< Source manager for reading token text
  StringInterner &interner_;      ///< String interner for caching strings
  DiagnosticLogger &diagnostics_; ///< Diagnostic logger for error reporting

  // Error tracking
  std::vector<ParseError> errors_; ///< Accumulated parse errors

  // Private parsing helpers

  /**
   * @brief Parse an integer literal token into an AST node.
   *
   * @return IntLiteralNode or nullptr on error
   */
  ast::ASTNode *parseIntegerLiteral();

  /**
   * @brief Parse a float literal token into an AST node.
   *
   * @return FloatLiteralNode or nullptr on error
   */
  ast::ASTNode *parseFloatLiteral();

  /**
   * @brief Parse a character literal token into an AST node.
   *
   * @return CharLiteralNode or nullptr on error
   */
  ast::ASTNode *parseCharacterLiteral();

  /**
   * @brief Parse a string literal token into an AST node.
   *
   * @return StringLiteralNode or nullptr on error
   */
  ast::ASTNode *parseStringLiteral();

  /**
   * @brief Parse a boolean literal token into an AST node.
   *
   * @return BoolLiteralNode or nullptr on error
   */
  ast::ASTNode *parseBooleanLiteral();

  /**
   * @brief Parse a null literal token into an AST node.
   *
   * @return NullLiteralNode or nullptr on error
   */
  ast::ASTNode *parseNullLiteral();

  // Error recovery helpers

  /**
   * @brief Synchronize parser state after an error.
   *
   * Advances tokens until a synchronization point is found.
   * Used for error recovery to continue parsing.
   */
  void synchronize();

  /**
   * @brief Check if current token is a synchronization point.
   *
   * @return True if we should stop error recovery here
   */
  bool isSynchronizationPoint() const;
};

} // namespace cxy
