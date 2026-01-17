#pragma once

#include "cxy/arena_allocator.hpp"
#include "cxy/ast/attributes.hpp"
#include "cxy/ast/node.hpp"
#include "cxy/diagnostics.hpp"
#include "cxy/lexer.hpp"
#include "cxy/token.hpp"
#include "cxy/types/registry.hpp"

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
   * @brief Construct a parser for the given lexer and dependencies.
   *
   * @param lexer Token source for parsing
   * @param arena Memory allocator for AST nodes
   * @param sourceManager Source manager for reading token text
   * @param interner String interner for caching identifier and string values
   * @param diagnostics Diagnostic logger for error reporting
   * @param typeRegistry Type registry for assigning types to AST nodes
   */
  explicit Parser(Lexer &lexer, ArenaAllocator &arena,
                  SourceManager &sourceManager, StringInterner &interner,
                  DiagnosticLogger &diagnostics, TypeRegistry &typeRegistry);

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
  ast::ASTNode *parseExpression(bool withoutStructLiterals = false);

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
  ast::ASTNode *parseRelationalExpression(bool withoutStructLiterals = false);

  /**
   * @brief Parse a range expression (.., ..<).
   *
   * range_expression ::=
   *   | shift_expression
   *   | range_expression '..' shift_expression
   *   | range_expression '..<' shift_expression
   *   | '..' shift_expression
   *   | shift_expression '..'
   *   | '..'
   *
   * @return Parsed range expression AST node, or nullptr on error
   */
  ast::ASTNode *parseRangeExpression(bool withoutStructLiterals = false);

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
  ast::ASTNode *parseEqualityExpression(bool withoutStructLiterals = false);

  /**
   * @brief Parse a bitwise AND expression (&).
   *
   * bitwise_and_expression ::=
   *   | equality_expression
   *   | bitwise_and_expression '&' equality_expression
   *
   * @return Parsed bitwise AND expression AST node, or nullptr on error
   */
  ast::ASTNode *parseBitwiseAndExpression(bool withoutStructLiterals = false);

  /**
   * @brief Parse a bitwise XOR expression (^).
   *
   * bitwise_xor_expression ::=
   *   | bitwise_and_expression
   *   | bitwise_xor_expression '^' bitwise_and_expression
   *
   * @return Parsed bitwise XOR expression AST node, or nullptr on error
   */
  ast::ASTNode *parseBitwiseXorExpression(bool withoutStructLiterals = false);

  /**
   * @brief Parse a bitwise OR expression (|).
   *
   * bitwise_or_expression ::=
   *   | bitwise_xor_expression
   *   | bitwise_or_expression '|' bitwise_xor_expression
   *
   * @return Parsed bitwise OR expression AST node, or nullptr on error
   */
  ast::ASTNode *parseBitwiseOrExpression(bool withoutStructLiterals = false);

  /**
   * @brief Parse a logical AND expression (&&).
   *
   * logical_and_expression ::=
   *   | bitwise_or_expression
   *   | logical_and_expression '&&' bitwise_or_expression
   *
   * @return Parsed logical AND expression AST node, or nullptr on error
   */
  ast::ASTNode *parseLogicalAndExpression(bool withoutStructLiterals = false);

  /**
   * @brief Parse a logical OR expression (||).
   *
   * logical_or_expression ::=
   *   | logical_and_expression
   *   | logical_or_expression '||' logical_and_expression
   *
   * @return Parsed logical OR expression AST node, or nullptr on error
   */
  ast::ASTNode *parseLogicalOrExpression(bool withoutStructLiterals = false);

  /**
   * @brief Parse a conditional expression (ternary ?:).
   *
   * conditional_expression ::=
   *   | logical_or_expression
   *   | logical_or_expression '?' expression ':' conditional_expression
   *
   * @return Parsed conditional expression AST node, or nullptr on error
   */
  ast::ASTNode *parseConditionalExpression(bool withoutStructLiterals = false);

  /**
   * @brief Parse an assignment expression (=, +=, -=, etc.).
   *
   * assignment_expression ::=
   *   | conditional_expression
   *   | conditional_expression assignment_operator assignment_expression
   *
   * @return Parsed assignment expression AST node, or nullptr on error
   */
  ast::ASTNode *parseAssignmentExpression(bool withoutStructLiterals = false);

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
  ast::ASTNode *parseShiftExpression(bool withoutStructLiterals = false);

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
  ast::ASTNode *parseAdditiveExpression(bool withoutStructLiterals = false);

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
  ast::ASTNode *parseMultiplicativeExpression(bool withoutStructLiterals = false);

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
  ast::ASTNode *parseUnaryExpression(bool withoutStructLiterals = false);

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
  ast::ASTNode *parseCastExpression(bool withoutStructLiterals = false);

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
  ast::ASTNode *parsePostfixExpression(bool withoutStructLiterals = false);

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
  ast::ASTNode *parsePrimaryExpression(bool withoutStructLiterals = false);

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
  ast::ASTNode *parseIdentifierExpression(bool withoutStructLiterals = false);

  /**
   * @brief Parse a macro call expression.
   *
   * macro_call ::=
   *   | identifier '!'                            # Bare macro call
   *   | identifier '!' '(' argument_list? ')'     # Function-like macro
   *
   * @return Parsed macro call AST node, or nullptr on error
   */
  ast::ASTNode *parseMacroCall();

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

  /**
   * @brief Parse a struct literal expression.
   *
   * struct_literal ::= [type] '{' struct_field_list? '}'
   * struct_field_list ::= struct_field (',' struct_field)*
   * struct_field ::= Ident ':' expression | Ident
   *
   * Supports both typed struct literals (Point { x: 1, y: 2 }) and
   * anonymous struct literals ({ x: 1, y: 2 }). Also supports shorthand
   * syntax where field name matches variable name.
   *
   * @param type Optional type node for typed struct literals (nullptr for
   * anonymous)
   * @return Parsed struct literal AST node, or nullptr on error
   */
  ast::ASTNode *parseStructLiteral(ast::ASTNode *type);

  /**
   * @brief Parse an interpolated string expression.
   *
   * interpolated_string ::= LString (expression | StringLiteral)* RString
   *
   * Supports nested expressions within {} and handles string parts between
   * interpolations.
   *
   * @return Parsed interpolated string AST node, or nullptr on error
   */
  ast::ASTNode *parseInterpolatedString();

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

  // Phase 4: Statement parsing interface

  /**
   * @brief Parse a statement.
   *
   * statement ::=
   *   | break_statement
   *   | continue_statement  
   *   | expression_statement
   *
   * @return Parsed statement AST node, or nullptr on error
   */
  ast::ASTNode *parseStatement();

  /**
   * @brief Parse an expression statement.
   *
   * expression_statement ::=
   *   | expression ';'?                  # Expression with optional semicolon
   *
   * @return Parsed expression statement AST node, or nullptr on error
   */
  ast::ASTNode *parseExpressionStatement();

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
  TypeRegistry &typeRegistry_;    ///< Type registry for AST node type assignment

  // Error tracking
  std::vector<ParseError> errors_; ///< Accumulated parse errors

  // Private parsing helpers

  /**
   * @brief Parse a break statement.
   *
   * break_statement ::=
   *   | 'break' ';'?                     # Break with optional semicolon
   *
   * @return Parsed break statement AST node, or nullptr on error
   */
  ast::ASTNode *parseBreakStatement();

  /**
   * @brief Parse a continue statement.
   *
   * continue_statement ::=
   *   | 'continue' ';'?                  # Continue with optional semicolon
   *
   * @return Parsed continue statement AST node, or nullptr on error
   */
  ast::ASTNode *parseContinueStatement();

  /**
   * @brief Parse a block statement.
   *
   * block_statement ::=
   *   | '{' statement* '}'               # Block with zero or more statements
   *
   * @return Parsed block statement AST node, or nullptr on error
   */
  ast::ASTNode *parseBlockStatement();

  /**
   * @brief Parse a defer statement.
   *
   * defer_statement ::=
   *   | 'defer' statement                # Defer execution until scope exit
   *
   * @return Parsed defer statement AST node, or nullptr on error
   */
  ast::ASTNode *parseDeferStatement();

  /**
   * @brief Parse a return statement.
   *
   * return_statement ::=
   *   | 'return' expression? ';'?        # Return with optional value and semicolon
   *
   * @return Parsed return statement AST node, or nullptr on error
   */
  ast::ASTNode *parseReturnStatement();

  /**
   * @brief Parse a yield statement.
   *
   * yield_statement ::=
   *   | 'yield' expression? ';'?         # Yield with optional value and semicolon
   *
   * @return Parsed yield statement AST node, or nullptr on error
   */
  ast::ASTNode *parseYieldStatement();

  /**
   * @brief Parse a variable declaration statement.
   *
   * variable_declaration ::=
   *   | ('var'|'const'|'auto') name_list (type_annotation | initializer | (type_annotation initializer)) ';'?
   *
   * name_list ::= identifier (',' identifier)* ','?
   * type_annotation ::= ':' type_expression
   * initializer ::= '=' expression
   *
   * @param singleVariable If true, only allow single variable declaration (for if conditions)
   * @return Parsed variable declaration AST node, or nullptr on error
   */
  ast::ASTNode *parseVariableDeclaration(bool singleVariable = false);

  /**
   * @brief Parse an if statement.
   *
   * if_statement ::=
   *   | 'if' condition if_body else_clause?
   *
   * condition ::= '(' condition_expr ')' | condition_expr
   * condition_expr ::= expression | single_variable_declaration
   * if_body ::= statement | block_statement
   * else_clause ::= 'else' if_statement | 'else' block_statement
   *
   * @return Parsed if statement AST node, or nullptr on error
   */
  ast::ASTNode *parseIfStatement();

  /**
   * @brief Parse a while statement.
   *
   * while_statement ::=
   *   | 'while' condition? while_body
   *
   * condition ::= '(' condition_expr ')' | condition_expr
   * condition_expr ::= expression | single_variable_declaration
   * while_body ::= statement | block_statement
   *
   * @return Parsed while statement AST node, or nullptr on error
   */
  ast::ASTNode *parseWhileStatement();

  /**
   * @brief Parse a for statement.
   *
   * for_statement ::= 'for' for_clause for_body
   * for_clause ::= '(' for_clause_core ')' | for_clause_core
   * for_clause_core ::= iterator_variable_list 'in' range_expression (',' condition_expression)?
   * iterator_variable_list ::= iterator_name (',' iterator_name)* ','?
   * iterator_name ::= identifier | '_'
   * for_body ::= statement | block_statement
   *
   * @return Parsed for statement AST node, or nullptr on error
   */
  ast::ASTNode *parseForStatement();

  /**
   * @brief Parse a switch statement.
   *
   * switch_statement ::= 'switch' switch_clause switch_body
   * switch_clause ::= '(' switch_clause_core ')' | switch_clause_core
   * switch_clause_core ::= (declaration_keyword identifier '=')? expression
   * switch_body ::= '{' case_list '}'
   *
   * @return Parsed switch statement AST node, or nullptr on error
   */
  ast::ASTNode *parseSwitchStatement();

  /**
   * @brief Parse a case statement for switch statements.
   *
   * case_statement ::= case_pattern '=>' case_body | default_case '=>' case_body
   * case_pattern ::= expression (',' expression)* ','?
   * default_case ::= '...'
   * case_body ::= statement | block_statement
   *
   * @return Parsed case statement AST node, or nullptr on error
   */
  ast::ASTNode *parseCaseStatement();

  /**
   * @brief Parse a match statement with type pattern matching.
   *
   * match_statement ::= 'match' match_clause match_body
   * match_clause ::= '(' expression ')' | expression
   * match_body ::= '{' match_case_list '}'
   * match_case_list ::= match_case*
   * match_case ::= match_pattern '=>' case_body
   * match_pattern ::= type_pattern (',' type_pattern)* ('as' identifier)?
   *                 | '...' ('as' identifier)?
   * type_pattern ::= type
   * case_body ::= statement | block_statement
   *
   * @return Parsed match statement AST node, or nullptr on error
   */
  ast::ASTNode *parseMatchStatement();

  /**
   * @brief Parse a match case statement for match statements.
   *
   * match_case ::= match_pattern '=>' case_body
   * match_pattern ::= type_pattern (',' type_pattern)* ('as' identifier)?
   *                 | '...' ('as' identifier)?
   * case_body ::= statement | block_statement
   *
   * @return Parsed match case AST node, or nullptr on error
   */
  ast::ASTNode *parseMatchCaseStatement();

  /**
   * @brief Parse attribute list that can appear before declarations/statements.
   *
   * attribute_list ::= attribute+
   * attribute ::= '@' attribute_spec | '@[' attribute_list_inner ']'
   * attribute_list_inner ::= attribute_spec (',' attribute_spec)* ','?
   *
   * @return AttributeListNode or nullptr on error
   */
  ast::ASTNode *parseAttributeList();

  /**
   * @brief Parse a single attribute specification.
   *
   * attribute_spec ::= identifier attribute_args?
   * attribute_args ::= '(' attribute_arg_list? ')' | '(' named_attribute_args ')'
   *
   * @return AttributeNode or nullptr on error
   */
  ast::ASTNode *parseAttribute();

  /**
   * @brief Parse attribute arguments (positional or named).
   *
   * attribute_arg_list ::= literal (',' literal)* ','?
   * named_attribute_args ::= named_attribute_arg (',' named_attribute_arg)* ','?
   * named_attribute_arg ::= identifier ':' literal
   *
   * @param attr AttributeNode to add arguments to
   * @return true if arguments parsed successfully, false on error
   */
  bool parseAttributeArguments(ast::AttributeNode *attr);

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

  /**
   * @brief Check if current token is a separator/terminator.
   *
   * Used by synchronize() to determine which tokens to skip over.
   * Separators and terminators don't start new constructs.
   *
   * @return True if current token should be skipped during synchronization
   */
  bool isSeparatorToken() const;

  /**
   * @brief Check if current token can start a statement.
   *
   * Used for optional expression parsing in return/yield statements
   * to determine when to stop parsing the optional expression.
   *
   * @return True if current token can start a statement
   */
  bool isStatementStart() const;
};

} // namespace cxy
