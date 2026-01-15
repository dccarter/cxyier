#include "cxy/parser.hpp"
#include "cxy/ast/expressions.hpp"
#include "cxy/ast/identifiers.hpp"
#include "cxy/ast/literals.hpp"
#include "cxy/ast/types.hpp"

#include <cmath>
#include <format>

namespace cxy {

Parser::Parser(Lexer &lexer, ArenaAllocator &arena,
               SourceManager &sourceManager, StringInterner &interner,
               DiagnosticLogger &diagnostics)
    : lexer_(lexer), arena_(arena), sourceManager_(sourceManager),
      interner_(interner), diagnostics_(diagnostics) {
  // Initialize token buffer to empty state
  for (int i = 0; i < 4; ++i) {
    tokens_[i] = Token{};
  }
}

void Parser::initialize() {
  // Preload the lookahead buffer for LL(3) parsing
  tokens_[0] =
      Token(TokenKind::Error, Location{}); // No previous token initially
  tokens_[1] = lexer_.nextToken();         // Current token
  tokens_[2] = lexer_.nextToken();         // Lookahead 1
  tokens_[3] = lexer_.nextToken();         // Lookahead 2
}

void Parser::advance() {
  // Shift buffer: drop previous, advance current, read new lookahead
  tokens_[0] = tokens_[1];         // previous = current
  tokens_[1] = tokens_[2];         // current = lookahead1
  tokens_[2] = tokens_[3];         // lookahead1 = lookahead2
  tokens_[3] = lexer_.nextToken(); // lookahead2 = next from lexer
}

bool Parser::checkAny(const std::vector<TokenKind> &kinds) const {
  TokenKind currentKind = current().kind;
  for (TokenKind kind : kinds) {
    if (currentKind == kind) {
      return true;
    }
  }
  return false;
}

bool Parser::match(TokenKind kind) {
  if (check(kind)) {
    advance();
    return true;
  }
  return false;
}

bool Parser::expect(TokenKind kind, const std::string &errorMessage) {
  if (check(kind)) {
    advance();
    return true;
  }

  // Create error message
  std::string msg =
      errorMessage.empty()
          ? std::format("Expected '{}', got '{}'", tokenKindToString(kind),
                        tokenKindToString(current().kind))
          : errorMessage;

  ParseError error(ParseErrorType::MissingToken, current().location, msg,
                   {kind}, current());
  reportError(error);
  return false;
}

void Parser::reportError(const ParseError &error) {
  errors_.push_back(error);

  // Report error through DiagnosticLogger
  diagnostics_.error(error.message, error.location);
}

ParseError Parser::createUnexpectedTokenError(TokenKind expected,
                                              const std::string &message) {
  std::string msg =
      message.empty()
          ? std::format("Expected '{}', got '{}'", tokenKindToString(expected),
                        tokenKindToString(current().kind))
          : message;
  return ParseError(ParseErrorType::UnexpectedToken, current().location, msg,
                    {expected}, current());
}

ParseError
Parser::createUnexpectedTokenError(const std::vector<TokenKind> &expected,
                                   const std::string &message) {
  std::string msg = message;
  if (msg.empty()) {
    msg = "Expected one of: ";
    for (size_t i = 0; i < expected.size(); ++i) {
      if (i > 0)
        msg += ", ";
      msg += std::format("'{}'", tokenKindToString(expected[i]));
    }
    msg += std::format(", got '{}'", tokenKindToString(current().kind));
  }
  return ParseError(ParseErrorType::UnexpectedToken, current().location, msg,
                    expected, current());
}

// Phase 2: Expression parsing with operator precedence

ast::ASTNode *Parser::parseExpression() {
  // Start at the top of the precedence hierarchy
  // Now includes assignment expressions
  return parseAssignmentExpression();
}

ast::ASTNode *Parser::parseAssignmentExpression() {
  // assignment_expression ::=
  //   | conditional_expression
  //   | conditional_expression assignment_operator assignment_expression

  ast::ASTNode *left = parseConditionalExpression();
  if (!left) {
    return nullptr;
  }

  // Check for assignment operators (right-associative)
  if (isAssignmentOperator(current().kind)) {
    Token opToken = current();
    advance(); // consume operator

    // Parse the right-hand side (right-associative)
    ast::ASTNode *right = parseAssignmentExpression();
    if (!right) {
      return nullptr; // Error already reported
    }

    // Create assignment expression node
    return ast::createAssignmentExpr(left, opToken.kind, right,
                                     opToken.location, arena_);
  }

  return left;
}

ast::ASTNode *Parser::parseConditionalExpression() {
  // conditional_expression ::=
  //   | logical_or_expression
  //   | logical_or_expression '?' expression ':' conditional_expression

  ast::ASTNode *condition = parseLogicalOrExpression();
  if (!condition) {
    return nullptr;
  }

  // Check for ternary operator
  if (check(TokenKind::Question)) {
    Token questionToken = current();
    advance(); // consume '?'

    // Parse the 'then' expression
    ast::ASTNode *thenExpr = parseExpression();
    if (!thenExpr) {
      return nullptr; // Error already reported
    }

    // Expect ':'
    if (!expect(TokenKind::Colon,
                "Expected ':' after then expression in ternary operator")) {
      return nullptr;
    }

    // Parse the 'else' expression (right-associative)
    ast::ASTNode *elseExpr = parseConditionalExpression();
    if (!elseExpr) {
      return nullptr; // Error already reported
    }

    // Create ternary expression node
    return ast::createTernaryExpr(condition, thenExpr, elseExpr,
                                  questionToken.location, arena_);
  }

  return condition;
}

ast::ASTNode *Parser::parseLogicalOrExpression() {
  // logical_or_expression ::=
  //   | logical_and_expression
  //   | logical_or_expression '||' logical_and_expression

  ast::ASTNode *left = parseLogicalAndExpression();
  if (!left) {
    return nullptr;
  }

  while (check(TokenKind::LOr)) {
    Token opToken = current();
    advance(); // consume operator

    ast::ASTNode *right = parseLogicalAndExpression();
    if (!right) {
      return nullptr; // Error already reported
    }

    // Create binary expression node
    left = ast::createBinaryExpr(left, opToken.kind, right, opToken.location,
                                 arena_);
  }

  return left;
}

ast::ASTNode *Parser::parseLogicalAndExpression() {
  // logical_and_expression ::=
  //   | bitwise_or_expression
  //   | logical_and_expression '&&' bitwise_or_expression

  ast::ASTNode *left = parseBitwiseOrExpression();
  if (!left) {
    return nullptr;
  }

  while (check(TokenKind::LAnd)) {
    Token opToken = current();
    advance(); // consume operator

    ast::ASTNode *right = parseBitwiseOrExpression();
    if (!right) {
      return nullptr; // Error already reported
    }

    // Create binary expression node
    left = ast::createBinaryExpr(left, opToken.kind, right, opToken.location,
                                 arena_);
  }

  return left;
}

ast::ASTNode *Parser::parseBitwiseOrExpression() {
  // bitwise_or_expression ::=
  //   | bitwise_xor_expression
  //   | bitwise_or_expression '|' bitwise_xor_expression

  ast::ASTNode *left = parseBitwiseXorExpression();
  if (!left) {
    return nullptr;
  }

  while (check(TokenKind::BOr)) {
    Token opToken = current();
    advance(); // consume operator

    ast::ASTNode *right = parseBitwiseXorExpression();
    if (!right) {
      return nullptr; // Error already reported
    }

    // Create binary expression node
    left = ast::createBinaryExpr(left, opToken.kind, right, opToken.location,
                                 arena_);
  }

  return left;
}

ast::ASTNode *Parser::parseBitwiseXorExpression() {
  // bitwise_xor_expression ::=
  //   | bitwise_and_expression
  //   | bitwise_xor_expression '^' bitwise_and_expression

  ast::ASTNode *left = parseBitwiseAndExpression();
  if (!left) {
    return nullptr;
  }

  while (check(TokenKind::BXor)) {
    Token opToken = current();
    advance(); // consume operator

    ast::ASTNode *right = parseBitwiseAndExpression();
    if (!right) {
      return nullptr; // Error already reported
    }

    // Create binary expression node
    left = ast::createBinaryExpr(left, opToken.kind, right, opToken.location,
                                 arena_);
  }

  return left;
}

ast::ASTNode *Parser::parseBitwiseAndExpression() {
  // bitwise_and_expression ::=
  //   | equality_expression
  //   | bitwise_and_expression '&' equality_expression

  ast::ASTNode *left = parseEqualityExpression();
  if (!left) {
    return nullptr;
  }

  while (check(TokenKind::BAnd)) {
    Token opToken = current();
    advance(); // consume operator

    ast::ASTNode *right = parseEqualityExpression();
    if (!right) {
      return nullptr; // Error already reported
    }

    // Create binary expression node
    left = ast::createBinaryExpr(left, opToken.kind, right, opToken.location,
                                 arena_);
  }

  return left;
}

ast::ASTNode *Parser::parseEqualityExpression() {
  // equality_expression ::=
  //   | relational_expression
  //   | equality_expression '==' relational_expression
  //   | equality_expression '!=' relational_expression

  ast::ASTNode *left = parseRelationalExpression();
  if (!left) {
    return nullptr;
  }

  while (check(TokenKind::Equal) || check(TokenKind::NotEqual)) {
    Token opToken = current();
    advance(); // consume operator

    ast::ASTNode *right = parseRelationalExpression();
    if (!right) {
      return nullptr; // Error already reported
    }

    // Create binary expression node
    left = ast::createBinaryExpr(left, opToken.kind, right, opToken.location,
                                 arena_);
  }

  return left;
}

ast::ASTNode *Parser::parseRelationalExpression() {
  // relational_expression ::=
  //   | shift_expression
  //   | relational_expression '<' shift_expression
  //   | relational_expression '<=' shift_expression
  //   | relational_expression '>' shift_expression
  //   | relational_expression '>=' shift_expression

  ast::ASTNode *left = parseShiftExpression();
  if (!left) {
    return nullptr;
  }

  while (check(TokenKind::Less) || check(TokenKind::LessEqual) ||
         check(TokenKind::Greater) || check(TokenKind::GreaterEqual)) {
    Token opToken = current();
    advance(); // consume operator

    ast::ASTNode *right = parseShiftExpression();
    if (!right) {
      return nullptr;
    }

    left = ast::createBinaryExpr(left, opToken.kind, right, opToken.location,
                                 arena_);
  }

  return left;
}

ast::ASTNode *Parser::parseShiftExpression() {
  // shift_expression ::=
  //   | additive_expression
  //   | shift_expression '<<' additive_expression
  //   | shift_expression '>>' additive_expression

  ast::ASTNode *left = parseAdditiveExpression();
  if (!left) {
    return nullptr;
  }

  while (check(TokenKind::Shl) || check(TokenKind::Shr)) {
    Token opToken = current();
    advance(); // consume operator

    ast::ASTNode *right = parseAdditiveExpression();
    if (!right) {
      return nullptr;
    }

    left = ast::createBinaryExpr(left, opToken.kind, right, opToken.location,
                                 arena_);
  }

  return left;
}

ast::ASTNode *Parser::parseAdditiveExpression() {
  // additive_expression ::=
  //   | multiplicative_expression
  //   | additive_expression '+' multiplicative_expression
  //   | additive_expression '-' multiplicative_expression

  ast::ASTNode *left = parseMultiplicativeExpression();
  if (!left) {
    return nullptr;
  }

  while (check(TokenKind::Plus) || check(TokenKind::Minus)) {
    Token opToken = current();
    advance(); // consume operator

    ast::ASTNode *right = parseMultiplicativeExpression();
    if (!right) {
      return nullptr;
    }

    left = ast::createBinaryExpr(left, opToken.kind, right, opToken.location,
                                 arena_);
  }

  return left;
}

ast::ASTNode *Parser::parseMultiplicativeExpression() {
  // multiplicative_expression ::=
  //   | cast_expression
  //   | multiplicative_expression '*' cast_expression
  //   | multiplicative_expression '/' cast_expression
  //   | multiplicative_expression '%' cast_expression

  ast::ASTNode *left = parseCastExpression();
  if (!left) {
    return nullptr;
  }

  while (check(TokenKind::Mult) || check(TokenKind::Div) ||
         check(TokenKind::Mod)) {
    Token opToken = current();
    advance(); // consume operator

    ast::ASTNode *right = parseCastExpression();
    if (!right) {
      return nullptr;
    }

    left = ast::createBinaryExpr(left, opToken.kind, right, opToken.location,
                                 arena_);
  }

  return left;
}

ast::ASTNode *Parser::parseUnaryExpression() {
  // unary_expression ::=
  //   | postfix_expression
  //   | '++' unary_expression
  //   | '--' unary_expression
  //   | '+' unary_expression
  //   | '-' unary_expression
  //   | '!' unary_expression
  //   | '~' unary_expression
  //   | '&' unary_expression
  //   | '&&' unary_expression

  // Check for prefix operators
  if (check(TokenKind::PlusPlus) || check(TokenKind::MinusMinus) ||
      check(TokenKind::Plus) || check(TokenKind::Minus) ||
      check(TokenKind::LNot) || check(TokenKind::BNot) ||
      check(TokenKind::BAnd) || check(TokenKind::LAnd) ||
      check(TokenKind::BXor)) {

    Token opToken = current();
    advance(); // consume operator

    ast::ASTNode *operand = parseUnaryExpression(); // right-associative
    if (!operand) {
      return nullptr;
    }

    return ast::createUnaryExpr(opToken.kind, true, operand, opToken.location,
                                arena_);
  }

  // No prefix operator, delegate to postfix expression
  return parsePostfixExpression();
}

ast::ASTNode *Parser::parseCastExpression() {
  // cast_expression ::=
  //   | postfix_expression
  //   | cast_expression 'as' type_expression
  //   | cast_expression '!:' type_expression

  ast::ASTNode *left = parseUnaryExpression();
  if (!left) {
    return nullptr;
  }

  // Handle cast operators (left-associative)
  while (check(TokenKind::As) || check(TokenKind::BangColon)) {
    bool isRetype = check(TokenKind::BangColon);
    Token opToken = current();
    advance(); // consume 'as' or '!:'

    ast::ASTNode *typeExpr = parseTypeExpression();
    if (!typeExpr) {
      return nullptr;
    }

    left =
        ast::createCastExpr(left, typeExpr, isRetype, opToken.location, arena_);
  }

  return left;
}

ast::ASTNode *Parser::parseTypeExpression() {
  // type_expression ::= primitive_type
  // primitive_type ::= 'i8' | 'i16' | 'i32' | 'i64' | 'i128'
  //                  | 'u8' | 'u16' | 'u32' | 'u64' | 'u128'
  //                  | 'f32' | 'f64' | 'bool' | 'char' | 'void'
  //                  | 'auto' | 'string'

  if (!isPrimitiveType(current().kind)) {
    reportError(ParseError(ParseErrorType::UnexpectedToken, current().location,
                           "Expected type name"));
    return nullptr;
  }

  Token typeToken = current();
  advance(); // consume type token

  return ast::createPrimitiveType(typeToken.kind, typeToken.location, arena_);
}

ast::ASTNode *Parser::parsePostfixExpression() {
  // postfix_expression ::=
  //   | primary_expression
  //   | postfix_expression '++'
  //   | postfix_expression '--'
  //   | postfix_expression '(' argument_list? ')'

  ast::ASTNode *expr = parsePrimaryExpression();
  if (!expr) {
    return nullptr;
  }

  while (check(TokenKind::PlusPlus) || check(TokenKind::MinusMinus) ||
         check(TokenKind::LParen) || check(TokenKind::LBracket) ||
         check(TokenKind::Dot) || check(TokenKind::BAndDot) ||
         check(TokenKind::FloatLiteral)) {

    if (check(TokenKind::LBracket)) {
      // Array indexing
      Location indexLoc = current().location;
      advance(); // consume '['

      ast::ASTNode *indexExpr = parseExpression();
      if (!indexExpr) {
        return nullptr;
      }

      if (!expect(TokenKind::RBracket, "Expected ']' after array index")) {
        return nullptr;
      }

      expr = ast::createIndexExpr(expr, indexExpr, indexLoc, arena_);
    } else if (check(TokenKind::Dot)) {
      // Member access
      Location memberLoc = current().location;
      advance(); // consume '.'

      ast::ASTNode *memberExpr = parsePrimaryExpression();
      if (!memberExpr) {
        return nullptr;
      }

      expr = ast::createMemberExpr(expr, memberExpr, false, memberLoc, arena_);
    } else if (check(TokenKind::BAndDot)) {
      // Overloaded member access (&.)
      Location memberLoc = current().location;
      advance(); // consume '&.'

      // For &. operator, only identifiers are allowed (not full expressions)
      if (!check(TokenKind::Ident)) {
        reportError(ParseError(ParseErrorType::UnexpectedToken,
                               current().location,
                               "Expected identifier after '&.' operator"));
        return nullptr;
      }

      ast::ASTNode *memberExpr = parseIdentifierExpression();
      if (!memberExpr) {
        return nullptr;
      }

      expr = ast::createMemberExpr(expr, memberExpr, true, memberLoc, arena_);
    } else if (check(TokenKind::LParen)) {
      // Function call
      Location callLoc = current().location;
      advance(); // consume '('

      auto *callExpr = ast::createCallExpr(expr, callLoc, arena_);

      // Check for empty argument list
      if (check(TokenKind::RParen)) {
        advance(); // consume ')'
        expr = callExpr;
        continue;
      }

      // Parse arguments
      ast::ASTNode *firstArg = parseExpression();
      if (!firstArg) {
        return nullptr;
      }
      callExpr->addArgument(firstArg);

      // Parse remaining arguments
      while (check(TokenKind::Comma)) {
        advance(); // consume ','

        // Allow trailing comma
        if (check(TokenKind::RParen)) {
          break;
        }

        ast::ASTNode *arg = parseExpression();
        if (!arg) {
          return nullptr;
        }
        callExpr->addArgument(arg);
      }

      if (!expect(TokenKind::RParen, "Expected ')' after function arguments")) {
        return nullptr;
      }

      expr = callExpr;
    } else {
      // Postfix increment/decrement
      Token opToken = current();
      advance(); // consume operator

      expr = ast::createUnaryExpr(opToken.kind, false, expr, opToken.location,
                                  arena_);
    }
  }

  return expr;
}

ast::ASTNode *Parser::parsePrimaryExpression() {
  // primary_expression ::=
  //   | literal_expression
  //   | identifier_expression
  //   | '(' expression ')'
  //   | array_literal

  // Check for array literal first
  if (check(TokenKind::LBracket)) {
    return parseArrayLiteral();
  }

  // Check for parenthesized expression or tuple literal
  if (check(TokenKind::LParen)) {
    return parseTupleOrGroupedExpression();
  }

  // Try literal expression
  if (isLiteral(current().kind)) {
    return parseLiteralExpression();
  }

  // Try identifier expression
  if (check(TokenKind::Ident)) {
    return parseIdentifierExpression();
  }

  // No valid primary expression found
  std::vector<TokenKind> expected = {
      TokenKind::IntLiteral,    TokenKind::FloatLiteral, TokenKind::CharLiteral,
      TokenKind::StringLiteral, TokenKind::True,         TokenKind::False,
      TokenKind::Null,          TokenKind::Ident,        TokenKind::LParen,
      TokenKind::LBracket};

  ParseError error = createUnexpectedTokenError(
      expected, "Expected literal, identifier, parenthesized expression, or "
                "array literal");
  reportError(error);
  return nullptr;
}

ast::ASTNode *Parser::parseLiteralExpression() {
  // literal_expression ::=
  //   | integer_literal
  //   | float_literal
  //   | character_literal
  //   | string_literal
  //   | boolean_literal
  //   | null_literal

  switch (current().kind) {
  case TokenKind::IntLiteral:
    return parseIntegerLiteral();
  case TokenKind::FloatLiteral:
    return parseFloatLiteral();
  case TokenKind::CharLiteral:
    return parseCharacterLiteral();
  case TokenKind::StringLiteral:
    return parseStringLiteral();
  case TokenKind::True:
  case TokenKind::False:
    return parseBooleanLiteral();
  case TokenKind::Null:
    return parseNullLiteral();
  default:
    // This should not happen if called correctly
    ParseError error(ParseErrorType::InvalidExpression, current().location,
                     std::format("Expected literal, got '{}'",
                                 tokenKindToString(current().kind)),
                     current());
    reportError(error);
    return nullptr;
  }
}

ast::ASTNode *Parser::parseIdentifierExpression() {
  // identifier_expression ::= Ident

  if (!check(TokenKind::Ident)) {
    ParseError error =
        createUnexpectedTokenError(TokenKind::Ident, "Expected identifier");
    reportError(error);
    return nullptr;
  }

  Token identToken = current();
  advance(); // consume identifier

  // Check that identifier token has processed value
  if (!identToken.hasLiteralValue()) {
    ParseError error = createUnexpectedTokenError(
        TokenKind::Ident, "Identifier token missing value");
    reportError(error);
    return nullptr;
  }

  // Use the processed identifier value directly from the token
  InternedString name = identToken.value.stringValue;
  return ast::createIdentifier(name, identToken.location, arena_);
}

// Private literal parsing helpers

ast::ASTNode *Parser::parseIntegerLiteral() {
  if (!check(TokenKind::IntLiteral)) {
    ParseError error = createUnexpectedTokenError(TokenKind::IntLiteral,
                                                  "Expected integer literal");
    reportError(error);
    return nullptr;
  }

  Token token = current();
  advance(); // consume integer

  if (!token.hasLiteralValue()) {
    ParseError error(ParseErrorType::InvalidExpression, token.location,
                     "Integer literal has no value", token);
    reportError(error);
    return nullptr;
  }

  // Convert token's unsigned value to signed for AST node
  __int128 value = static_cast<__int128>(token.getIntValue());
  return ast::createIntLiteral(value, token.location, arena_);
}

ast::ASTNode *Parser::parseFloatLiteral() {
  if (!check(TokenKind::FloatLiteral)) {
    ParseError error = createUnexpectedTokenError(TokenKind::FloatLiteral,
                                                  "Expected float literal");
    reportError(error);
    return nullptr;
  }

  Token token = current();
  advance(); // consume float

  if (!token.hasLiteralValue()) {
    ParseError error(ParseErrorType::InvalidExpression, token.location,
                     "Float literal has no value", token);
    reportError(error);
    return nullptr;
  }

  double value = token.getFloatValue();
  return ast::createFloatLiteral(value, token.location, arena_);
}

ast::ASTNode *Parser::parseCharacterLiteral() {
  if (!check(TokenKind::CharLiteral)) {
    ParseError error = createUnexpectedTokenError(TokenKind::CharLiteral,
                                                  "Expected character literal");
    reportError(error);
    return nullptr;
  }

  Token token = current();
  advance(); // consume character

  if (!token.hasLiteralValue()) {
    ParseError error(ParseErrorType::InvalidExpression, token.location,
                     "Character literal has no value", token);
    reportError(error);
    return nullptr;
  }

  uint32_t value = token.getCharValue();
  return ast::createCharLiteral(value, token.location, arena_);
}

ast::ASTNode *Parser::parseStringLiteral() {
  if (!check(TokenKind::StringLiteral)) {
    ParseError error = createUnexpectedTokenError(TokenKind::StringLiteral,
                                                  "Expected string literal");
    reportError(error);
    return nullptr;
  }

  Token token = current();
  advance(); // consume string

  // String tokens now contain processed InternedString values
  if (!token.hasLiteralValue()) {
    ParseError error = createUnexpectedTokenError(TokenKind::StringLiteral,
                                                  "String token missing value");
    reportError(error);
    return nullptr;
  }

  // Get the processed string value directly from the token
  InternedString value = token.value.stringValue;
  return ast::createStringLiteral(value, token.location, arena_);
}

ast::ASTNode *Parser::parseBooleanLiteral() {
  bool value;
  Token token = current();

  if (check(TokenKind::True)) {
    value = true;
    advance();
  } else if (check(TokenKind::False)) {
    value = false;
    advance();
  } else {
    std::vector<TokenKind> expected = {TokenKind::True, TokenKind::False};
    ParseError error =
        createUnexpectedTokenError(expected, "Expected 'true' or 'false'");
    reportError(error);
    return nullptr;
  }

  return ast::createBoolLiteral(value, token.location, arena_);
}

ast::ASTNode *Parser::parseNullLiteral() {
  if (!check(TokenKind::Null)) {
    ParseError error =
        createUnexpectedTokenError(TokenKind::Null, "Expected 'null'");
    reportError(error);
    return nullptr;
  }

  Token token = current();
  advance(); // consume 'null'

  return ast::createNullLiteral(token.location, arena_);
}

ast::ASTNode *Parser::parseArrayLiteral() {
  // array_literal ::= '[' array_element_list? ']'
  // array_element_list ::= expression (',' expression)*

  if (!expect(TokenKind::LBracket, "Expected '[' to start array literal")) {
    return nullptr;
  }

  Location startLoc = previous().location;
  auto *arrayExpr = ast::createArrayExpr(startLoc, arena_);

  // Check for empty array
  if (check(TokenKind::RBracket)) {
    advance(); // consume ']'
    return arrayExpr;
  }

  // Parse first element
  ast::ASTNode *firstElement = parseExpression();
  if (!firstElement) {
    return nullptr;
  }
  arrayExpr->addElement(firstElement);

  // Parse remaining elements
  while (check(TokenKind::Comma)) {
    advance(); // consume ','

    // Allow trailing comma
    if (check(TokenKind::RBracket)) {
      break;
    }

    ast::ASTNode *element = parseExpression();
    if (!element) {
      return nullptr;
    }
    arrayExpr->addElement(element);
  }

  if (!expect(TokenKind::RBracket, "Expected ']' after array elements")) {
    return nullptr;
  }

  return arrayExpr;
}

ast::ASTNode *Parser::parseTupleOrGroupedExpression() {
  // tuple_literal ::= '(' expression (',' expression)+ ')'
  // grouped_expression ::= '(' expression ')'

  if (!expect(TokenKind::LParen,
              "Expected '(' to start tuple or grouped expression")) {
    return nullptr;
  }

  Location startLoc = previous().location;

  // Check for empty parentheses (error case)
  if (check(TokenKind::RParen)) {
    reportError(ParseError(ParseErrorType::InvalidExpression,
                           current().location,
                           "Empty parentheses not allowed"));
    return nullptr;
  }

  // Parse first expression
  ast::ASTNode *firstExpr = parseExpression();
  if (!firstExpr) {
    return nullptr;
  }

  // Check if this is a tuple (has comma) or grouped expression
  if (check(TokenKind::Comma)) {
    // It's a tuple - create tuple expression and parse remaining elements
    auto *tupleExpr = ast::createTupleExpr(startLoc, arena_);
    tupleExpr->addElement(firstExpr);

    while (check(TokenKind::Comma)) {
      advance(); // consume ','

      // Allow trailing comma
      if (check(TokenKind::RParen)) {
        break;
      }

      ast::ASTNode *element = parseExpression();
      if (!element) {
        return nullptr;
      }
      tupleExpr->addElement(element);
    }

    if (!expect(TokenKind::RParen, "Expected ')' after tuple elements")) {
      return nullptr;
    }

    return tupleExpr;
  } else {
    // It's a grouped expression - return the inner expression directly
    if (!expect(TokenKind::RParen, "Expected ')' after expression")) {
      return nullptr;
    }

    return firstExpr;
  }
}

// Error recovery

void Parser::synchronize() {
  // Skip tokens until we find a synchronization point
  while (!isAtEnd() && !isSynchronizationPoint()) {
    advance();
  }
}

bool Parser::isSynchronizationPoint() const {
  // Synchronization points for error recovery:
  // - Statement boundaries: ';', '}', newlines
  // - Expression boundaries: ',', ')', ']', '}'
  // - Declaration boundaries: keywords like 'func', 'var', 'struct'
  TokenKind kind = current().kind;

  // Statement/block boundaries
  if (kind == TokenKind::Semicolon || kind == TokenKind::RBrace) {
    return true;
  }

  // Expression boundaries
  if (kind == TokenKind::Comma || kind == TokenKind::RParen ||
      kind == TokenKind::RBracket) {
    return true;
  }

  // Declaration keywords (for future phases)
  if (kind == TokenKind::Func || kind == TokenKind::Var ||
      kind == TokenKind::Const || kind == TokenKind::Struct ||
      kind == TokenKind::Enum || kind == TokenKind::Type) {
    return true;
  }

  return false;
}

} // namespace cxy
