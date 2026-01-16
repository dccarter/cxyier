#include "cxy/parser.hpp"
#include "cxy/ast/declarations.hpp"
#include "cxy/ast/expressions.hpp"
#include "cxy/ast/identifiers.hpp"
#include "cxy/ast/literals.hpp"
#include "cxy/ast/statements.hpp"
#include "cxy/ast/types.hpp"
#include "cxy/types/primitive.hpp"

#include <cmath>
#include <format>
#include <functional>

namespace cxy {

Parser::Parser(Lexer &lexer, ArenaAllocator &arena,
               SourceManager &sourceManager, StringInterner &interner,
               DiagnosticLogger &diagnostics, TypeRegistry &typeRegistry)
    : lexer_(lexer), arena_(arena), sourceManager_(sourceManager),
      interner_(interner), diagnostics_(diagnostics), typeRegistry_(typeRegistry) {
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
  //   | range_expression
  //   | relational_expression '<' range_expression
  //   | relational_expression '<=' range_expression
  //   | relational_expression '>' range_expression
  //   | relational_expression '>=' range_expression

  ast::ASTNode *left = parseRangeExpression();
  if (!left) {
    return nullptr;
  }

  while (check(TokenKind::Less) || check(TokenKind::LessEqual) ||
         check(TokenKind::Greater) || check(TokenKind::GreaterEqual)) {
    Token opToken = current();
    advance(); // consume operator

    ast::ASTNode *right = parseRangeExpression();
    if (!right) {
      return nullptr;
    }

    left = ast::createBinaryExpr(left, opToken.kind, right, opToken.location,
                                 arena_);
  }

  return left;
}

ast::ASTNode *Parser::parseRangeExpression() {
  // range_expression ::=
  //   | shift_expression
  //   | range_expression '..' shift_expression
  //   | range_expression '..<' shift_expression
  //   | '..' shift_expression
  //   | shift_expression '..'
  //   | '..'

  // Handle open start ranges (..expr, ..<expr, ..)
  if (check(TokenKind::DotDot)) {
    Token opToken = current();
    advance(); // consume '..'

    // Check if there's a right side expression
    if (!isAtEnd() && !check(TokenKind::RBracket) && !check(TokenKind::Comma) &&
        !check(TokenKind::RParen) && !check(TokenKind::RBrace) &&
        !check(TokenKind::Semicolon)) {
      // ..expr (open start inclusive range)
      ast::ASTNode *end = parseShiftExpression();
      if (!end) {
        return nullptr;
      }
      return ast::createRangeExpr(nullptr, end, true, opToken.location, arena_);
    } else {
      // Just .. (full open range)
      return ast::createRangeExpr(nullptr, nullptr, true, opToken.location,
                                  arena_);
    }
  }

  if (check(TokenKind::DotDotLess)) {
    Token opToken = current();
    advance(); // consume '..<'

    ast::ASTNode *end = parseShiftExpression();
    if (!end) {
      return nullptr;
    }
    return ast::createRangeExpr(nullptr, end, false, opToken.location, arena_);
  }

  // First, parse left side expression
  ast::ASTNode *left = parseShiftExpression();
  if (!left) {
    return nullptr;
  }

  // Check for range operators after parsing left side
  if (check(TokenKind::DotDot)) {
    Token opToken = current();
    advance(); // consume '..'

    // Check if there's a right side expression
    if (!isAtEnd() && !check(TokenKind::RBracket) && !check(TokenKind::Comma) &&
        !check(TokenKind::RParen) && !check(TokenKind::RBrace) &&
        !check(TokenKind::Semicolon)) {
      // expr..expr (inclusive range)
      ast::ASTNode *right = parseShiftExpression();
      if (!right) {
        return nullptr;
      }
      return ast::createRangeExpr(left, right, true, opToken.location, arena_);
    } else {
      // expr.. (open end range)
      return ast::createRangeExpr(left, nullptr, true, opToken.location,
                                  arena_);
    }
  }

  if (check(TokenKind::DotDotLess)) {
    Token opToken = current();
    advance(); // consume '..<'

    // Must have a right side for exclusive ranges
    ast::ASTNode *right = parseShiftExpression();
    if (!right) {
      return nullptr;
    }
    return ast::createRangeExpr(left, right, false, opToken.location, arena_);
  }

  // No range operator found, just return the left side
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
  //   | '^' unary_expression

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
         check(TokenKind::Dot) || check(TokenKind::BAndDot)) {

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
  //   | struct_literal
  //   | spread_expression

  // Check for spread expression first
  if (check(TokenKind::Elipsis)) {
    Location loc = current().location;
    advance(); // consume '...'

    // Check for double spread (nested spread expressions are not allowed)
    if (check(TokenKind::Elipsis)) {
      ParseError error = createUnexpectedTokenError(
          TokenKind::Ident,
          "Cannot spread a spread expression - '...' after '...' is invalid");
      reportError(error);
      return nullptr;
    }

    // Parse the expression to spread
    ast::ASTNode *expr = parsePostfixExpression();
    if (!expr) {
      ParseError error = createUnexpectedTokenError(
          TokenKind::Ident, "Expected expression after '...'");
      reportError(error);
      return nullptr;
    }

    return ast::createSpreadExpr(expr, loc, arena_);
  }

  // Check for array literal
  if (check(TokenKind::LBracket)) {
    return parseArrayLiteral();
  }

  // Check for anonymous struct literal
  if (check(TokenKind::LBrace)) {
    return parseStructLiteral(nullptr);
  }

  // Check for parenthesized expression or tuple literal
  if (check(TokenKind::LParen)) {
    return parseTupleOrGroupedExpression();
  }

  // Try literal expression
  if (isLiteral(current().kind)) {
    return parseLiteralExpression();
  }

  // Try interpolated string expression
  if (check(TokenKind::LString)) {
    return parseInterpolatedString();
  }

  // Try identifier expression or macro call
  if (check(TokenKind::Ident)) {
    // Check for macro call (identifier followed by '!')
    if (lookahead(1).kind == TokenKind::LNot) {
      return parseMacroCall();
    } else {
      return parseIdentifierExpression();
    }
  }

  // No valid primary expression found
  std::vector<TokenKind> expected = {
      TokenKind::IntLiteral,    TokenKind::FloatLiteral, TokenKind::CharLiteral,
      TokenKind::StringLiteral, TokenKind::LString,      TokenKind::True,
      TokenKind::False,         TokenKind::Null,         TokenKind::Ident,
      TokenKind::LParen,        TokenKind::LBracket,     TokenKind::LBrace,
      TokenKind::Elipsis};

  ParseError error = createUnexpectedTokenError(
      expected, "Expected literal, identifier, parenthesized expression, "
                "array literal, struct literal, spread expression, or "
                "interpolated string");
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
  // typed_struct_literal ::= Ident '{' struct_field_list? '}'

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

  // Check for typed struct literal (identifier followed by '{')
  if (check(TokenKind::LBrace)) {
    // Create identifier node for the type
    InternedString name = identToken.value.stringValue;
    ast::ASTNode *typeNode =
        ast::createIdentifier(name, identToken.location, arena_);
    return parseStructLiteral(typeNode);
  }

  // Use the processed identifier value directly from the token
  InternedString name = identToken.value.stringValue;
  return ast::createIdentifier(name, identToken.location, arena_);
}

ast::ASTNode *Parser::parseMacroCall() {
  // macro_call ::=
  //   | identifier '!'                            # Bare macro call
  //   | identifier '!' '(' argument_list? ')'     # Function-like macro

  if (!check(TokenKind::Ident)) {
    ParseError error = createUnexpectedTokenError(TokenKind::Ident,
                                                  "Expected macro identifier");
    reportError(error);
    return nullptr;
  }

  Token identToken = current();
  advance(); // consume identifier

  if (!identToken.hasLiteralValue()) {
    ParseError error = createUnexpectedTokenError(
        TokenKind::Ident, "Macro identifier token missing value");
    reportError(error);
    return nullptr;
  }

  if (!expect(TokenKind::LNot, "Expected '!' after macro identifier")) {
    return nullptr;
  }

  // Create identifier node for macro name
  InternedString name = identToken.value.stringValue;
  ast::ASTNode *macroName =
      ast::createIdentifier(name, identToken.location, arena_);

  // Create macro call expression
  auto *macroCall =
      ast::createMacroCallExpr(macroName, identToken.location, arena_);

  // Check for function-like macro with parentheses
  if (check(TokenKind::LParen)) {
    advance(); // consume '('

    // Check for empty argument list
    if (check(TokenKind::RParen)) {
      advance(); // consume ')'
      return macroCall;
    }

    // Parse arguments
    ast::ASTNode *firstArg = parseExpression();
    if (!firstArg) {
      return nullptr;
    }
    macroCall->addArgument(firstArg);

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
      macroCall->addArgument(arg);
    }

    if (!expect(TokenKind::RParen, "Expected ')' after macro arguments")) {
      return nullptr;
    }
  }

  return macroCall;
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
  auto *node = ast::createIntLiteral(value, token.location, arena_);
  
  // Assign type from token information
  IntegerKind intKind = token.getIntType();
  node->type = typeRegistry_.integerType(intKind);
  
  return node;
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
  auto *node = ast::createFloatLiteral(value, token.location, arena_);
  
  // Assign type from token information
  FloatKind floatKind = token.getFloatType();
  node->type = typeRegistry_.floatType(floatKind);
  
  return node;
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
  auto *node = ast::createCharLiteral(value, token.location, arena_);
  
  // Assign character type
  node->type = typeRegistry_.charType();
  
  return node;
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
  auto *node = ast::createStringLiteral(value, token.location, arena_);
  
  // Assign string type (using CharType as placeholder for now)
  // TODO: Implement proper string type in type system
  node->type = typeRegistry_.charType();
  
  return node;
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

  auto *node = ast::createBoolLiteral(value, token.location, arena_);
  
  // Assign boolean type
  node->type = typeRegistry_.boolType();
  
  return node;
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

ast::ASTNode *Parser::parseStructLiteral(ast::ASTNode *type) {
  // struct_literal ::= [type] '{' struct_field_list? '}'
  // struct_field_list ::= struct_field (',' struct_field)*
  // struct_field ::= Ident ':' expression | Ident

  if (!expect(TokenKind::LBrace, "Expected '{' to start struct literal")) {
    return nullptr;
  }

  Location startLoc = previous().location;
  auto *structExpr = ast::createStructExpr(type, startLoc, arena_);

  // Handle empty struct literal
  if (check(TokenKind::RBrace)) {
    advance(); // consume '}'

    // Anonymous structs cannot be empty
    if (!type) {
      ParseError error = createUnexpectedTokenError(
          TokenKind::Ident, "Anonymous struct literals cannot be empty");
      reportError(error);
      return nullptr;
    }

    return structExpr;
  }

  // Parse field list
  do {
    // Each field must start with an identifier
    if (!check(TokenKind::Ident)) {
      ParseError error = createUnexpectedTokenError(
          TokenKind::Ident, "Expected field name in struct literal");
      reportError(error);
      return nullptr;
    }

    Token fieldNameToken = current();
    advance(); // consume field name

    if (!fieldNameToken.hasLiteralValue()) {
      ParseError error = createUnexpectedTokenError(
          TokenKind::Ident, "Field name token missing value");
      reportError(error);
      return nullptr;
    }

    InternedString fieldName = fieldNameToken.value.stringValue;
    ast::ASTNode *nameNode =
        ast::createIdentifier(fieldName, fieldNameToken.location, arena_);
    ast::ASTNode *valueNode;

    // Check for explicit value or shorthand syntax
    if (check(TokenKind::Colon)) {
      advance(); // consume ':'

      // Parse field value expression
      valueNode = parseExpression();
      if (!valueNode) {
        return nullptr;
      }
    } else {
      // Shorthand syntax - field name is also the variable name
      valueNode =
          ast::createIdentifier(fieldName, fieldNameToken.location, arena_);
    }

    // Create field node and add to struct
    auto *fieldExpr = ast::createFieldExpr(nameNode, valueNode,
                                           fieldNameToken.location, arena_);
    structExpr->addField(fieldExpr);

    // Check for comma or end
    if (check(TokenKind::Comma)) {
      advance(); // consume ','

      // Allow trailing comma
      if (check(TokenKind::RBrace)) {
        break;
      }
    } else if (check(TokenKind::RBrace)) {
      break;
    } else {
      ParseError error =
          createUnexpectedTokenError({TokenKind::Comma, TokenKind::RBrace},
                                     "Expected ',' or '}' after struct field");
      reportError(error);
      return nullptr;
    }
  } while (!isAtEnd() && !check(TokenKind::RBrace));

  if (!expect(TokenKind::RBrace, "Expected '}' to end struct literal")) {
    return nullptr;
  }

  return structExpr;
}

// Error recovery

ast::ASTNode *Parser::parseInterpolatedString() {
  if (!check(TokenKind::LString)) {
    ParseError error = createUnexpectedTokenError(
        TokenKind::LString, "Expected interpolated string");
    reportError(error);
    return nullptr;
  }

  Location startLoc = current().location;
  Token lToken = current();
  advance(); // consume LString

  // Create StringExpression to hold all parts
  auto *stringExpr = ast::createStringExpr(startLoc, arena_);

  // Add initial string part from LString if it has content
  if (lToken.hasValue && !lToken.value.stringValue.view().empty()) {
    InternedString initialPart = lToken.value.stringValue;
    auto *initialLiteral =
        ast::createStringLiteral(initialPart, lToken.location, arena_);
    stringExpr->addPart(initialLiteral);
  }

  // Parse the sequence of string parts and expressions
  while (true) {
    if (check(TokenKind::RString)) {
      // End of interpolated string
      Token rToken = current();
      advance(); // consume RString

      // Add final string part if it has content
      if (rToken.hasValue && !rToken.value.stringValue.view().empty()) {
        InternedString finalPart = rToken.value.stringValue;
        auto *finalLiteral =
            ast::createStringLiteral(finalPart, rToken.location, arena_);
        stringExpr->addPart(finalLiteral);
      }
      break;
    } else if (check(TokenKind::StringLiteral)) {
      // Middle string part between expressions
      Token strToken = current();
      advance(); // consume StringLiteral

      if (strToken.hasValue && !strToken.value.stringValue.view().empty()) {
        InternedString stringPart = strToken.value.stringValue;
        auto *stringLiteral =
            ast::createStringLiteral(stringPart, strToken.location, arena_);
        stringExpr->addPart(stringLiteral);
      }
    } else {
      // Parse expression within the interpolation
      ast::ASTNode *expr = parseExpression();
      if (!expr) {
        return nullptr;
      }
      stringExpr->addPart(expr);
    }
  }

  return stringExpr;
}

void Parser::synchronize() {
  // Skip tokens until we reach a synchronization point
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

bool Parser::isStatementStart() const {
  // Check if current token can start a statement
  TokenKind kind = current().kind;
  
  // Statement keywords
  if (kind == TokenKind::Break || kind == TokenKind::Continue ||
      kind == TokenKind::Defer || kind == TokenKind::Return ||
      kind == TokenKind::Yield) {
    return true;
  }
  
  // Block statement
  if (kind == TokenKind::LBrace) {
    return true;
  }
  
  // Control flow keywords (for future phases)
  if (kind == TokenKind::If || kind == TokenKind::While ||
      kind == TokenKind::For || kind == TokenKind::Match) {
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

// Phase 4: Statement parsing implementation

ast::ASTNode *Parser::parseStatement() {
  // Dispatch based on current token
  switch (current().kind) {
  case TokenKind::Break:
    return parseBreakStatement();
  case TokenKind::Continue:
    return parseContinueStatement();
  case TokenKind::LBrace:
    return parseBlockStatement();
  case TokenKind::Defer:
    return parseDeferStatement();
  case TokenKind::Return:
    return parseReturnStatement();
  case TokenKind::Yield:
    return parseYieldStatement();
  case TokenKind::Var:
  case TokenKind::Const:
  case TokenKind::Auto:
    return parseVariableDeclaration();
  default:
    // Fall back to expression statement
    return parseExpressionStatement();
  }
}

ast::ASTNode *Parser::parseBreakStatement() {
  Location startLoc = current().location;
  
  // Consume 'break' keyword
  if (!expect(TokenKind::Break)) {
    return nullptr;
  }

  // Check for optional semicolon
  if (check(TokenKind::Semicolon)) {
    advance(); // consume semicolon
  }

  // Create break statement node
  return ast::createBreakStatement(startLoc, arena_);
}

ast::ASTNode *Parser::parseContinueStatement() {
  Location startLoc = current().location;
  
  // Consume 'continue' keyword
  if (!expect(TokenKind::Continue)) {
    return nullptr;
  }

  // Check for optional semicolon
  if (check(TokenKind::Semicolon)) {
    advance(); // consume semicolon
  }

  // Create continue statement node
  return ast::createContinueStatement(startLoc, arena_);
}

ast::ASTNode *Parser::parseBlockStatement() {
  Location startLoc = current().location;
  
  // Consume opening brace
  if (!expect(TokenKind::LBrace)) {
    return nullptr;
  }

  // Create block statement node
  auto *blockStmt = ast::createBlockStatement(startLoc, arena_);
  if (!blockStmt) {
    return nullptr;
  }

  // Parse statements until closing brace
  while (!check(TokenKind::RBrace) && !isAtEnd()) {
    auto *stmt = parseStatement();
    if (stmt) {
      blockStmt->addStatement(stmt);
    } else {
      // Error in statement parsing - try to recover
      synchronize();
      // Continue parsing other statements in the block
    }
  }

  // Consume closing brace
  if (!expect(TokenKind::RBrace)) {
    ParseError error = createUnexpectedTokenError(TokenKind::RBrace,
                                                  "Expected '}' to close block statement");
    reportError(error);
    return nullptr;
  }

  return blockStmt;
}

ast::ASTNode *Parser::parseDeferStatement() {
  Location startLoc = current().location;
  
  // Consume 'defer' keyword
  if (!expect(TokenKind::Defer)) {
    return nullptr;
  }

  // Parse the statement to defer
  auto *stmt = parseStatement();
  if (!stmt) {
    ParseError error = createUnexpectedTokenError(TokenKind::LBrace,
                                                  "Expected statement after 'defer'");
    reportError(error);
    return nullptr;
  }

  // Create defer statement node
  return ast::createDeferStatement(stmt, startLoc, arena_);
}

ast::ASTNode *Parser::parseReturnStatement() {
  Location startLoc = current().location;
  
  // Consume 'return' keyword
  if (!expect(TokenKind::Return)) {
    return nullptr;
  }

  // Parse optional expression
  ast::ASTNode *expr = nullptr;
  if (!check(TokenKind::Semicolon) && !isAtEnd() && !isStatementStart()) {
    expr = parseExpression();
    // Note: expr can be nullptr if parsing fails, but we continue
  }

  // Check for optional semicolon
  if (check(TokenKind::Semicolon)) {
    advance(); // consume semicolon
  }

  // Create return statement node
  return ast::createReturnStatement(startLoc, arena_, expr);
}

ast::ASTNode *Parser::parseYieldStatement() {
  Location startLoc = current().location;
  
  // Consume 'yield' keyword
  if (!expect(TokenKind::Yield)) {
    return nullptr;
  }

  // Parse optional expression
  ast::ASTNode *expr = nullptr;
  if (!check(TokenKind::Semicolon) && !isAtEnd() && !isStatementStart()) {
    expr = parseExpression();
    // Note: expr can be nullptr if parsing fails, but we continue
  }

  // Check for optional semicolon
  if (check(TokenKind::Semicolon)) {
    advance(); // consume semicolon
  }

  // Create yield statement node
  return ast::createYieldStatement(startLoc, arena_, expr);
}

ast::ASTNode *Parser::parseExpressionStatement() {
  Location startLoc = current().location;

  // Parse the expression
  ast::ASTNode *expr = parseExpression();
  if (!expr) {
    return nullptr; // Error already reported by parseExpression
  }

  // Check for optional semicolon
  if (check(TokenKind::Semicolon)) {
    advance(); // consume semicolon
  }

  // Create expression statement node
  return ast::createExprStatement(expr, startLoc, arena_);
}

// Phase 5.1: Variable declaration parsing implementation

ast::ASTNode *Parser::parseVariableDeclaration() {
  Location startLoc = current().location;
  bool isConst = false;
  
  // Parse declaration keyword
  if (check(TokenKind::Const)) {
    isConst = true;
    advance(); // consume 'const'
  } else if (check(TokenKind::Var)) {
    advance(); // consume 'var'
  } else if (check(TokenKind::Auto)) {
    advance(); // consume 'auto'
  } else {
    reportError(ParseError(ParseErrorType::UnexpectedToken, current().location,
                           "Expected 'var', 'const', or 'auto'"));
    return nullptr;
  }

  // Create variable declaration node
  auto *decl = ast::createVariableDeclaration(startLoc, arena_, isConst);

  // Parse name list - first name is required
  if (!check(TokenKind::Ident)) {
    reportError(ParseError(ParseErrorType::UnexpectedToken, current().location,
                           "Expected identifier"));
    return nullptr;
  }

  // Parse first identifier
  InternedString firstName = current().value.stringValue;
  auto *firstNameNode = ast::createIdentifier(firstName, current().location, arena_);
  decl->addName(firstNameNode);
  advance(); // consume first identifier

  // Parse additional names (comma-separated)
  while (check(TokenKind::Comma)) {
    advance(); // consume comma

    // Check for trailing comma (optional)
    if (!check(TokenKind::Ident)) {
      // Trailing comma - stop parsing names
      break;
    }

    // Parse next identifier
    InternedString name = current().value.stringValue;
    auto *nameNode = ast::createIdentifier(name, current().location, arena_);
    decl->addName(nameNode);
    advance(); // consume identifier
  }

  // Parse optional type annotation
  ast::ASTNode *typeExpr = nullptr;
  if (check(TokenKind::Colon)) {
    advance(); // consume ':'

    typeExpr = parseTypeExpression();
    if (!typeExpr) {
      return nullptr; // Error already reported
    }
    decl->setType(typeExpr);
  }

  // Parse optional initializer
  ast::ASTNode *initializer = nullptr;
  if (check(TokenKind::Assign)) {
    advance(); // consume '='

    initializer = parseExpression();
    if (!initializer) {
      return nullptr; // Error already reported
    }
    decl->setInitializer(initializer);
  }

  // Validate constraint: either type or initializer must be present
  if (!typeExpr && !initializer) {
    reportError(ParseError(ParseErrorType::UnexpectedToken, current().location,
                           "Variable declaration must have either type annotation or initializer"));
    return nullptr;
  }

  // Check for optional semicolon
  if (check(TokenKind::Semicolon)) {
    advance(); // consume semicolon
  }

  return decl;
}

} // namespace cxy
