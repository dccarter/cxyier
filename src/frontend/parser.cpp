#include "cxy/parser.hpp"
#include "cxy/ast/annotations.hpp"
#include "cxy/ast/attributes.hpp"
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

// Operator overload helper functions

std::string Parser::getBinaryOverloadOperatorName(TokenKind token) {
  switch (token) {
    case TokenKind::Plus: return "add";
    case TokenKind::Minus: return "sub";
    case TokenKind::Mult: return "mul";
    case TokenKind::Div: return "div";
    case TokenKind::Mod: return "mod";
    case TokenKind::Equal: return "eq";
    case TokenKind::NotEqual: return "ne";
    case TokenKind::Less: return "lt";
    case TokenKind::LessEqual: return "le";
    case TokenKind::Greater: return "gt";
    case TokenKind::GreaterEqual: return "ge";
    case TokenKind::LAnd: return "land";
    case TokenKind::LOr: return "lor";
    case TokenKind::BAnd: return "band";
    case TokenKind::BOr: return "bor";
    case TokenKind::BXor: return "bxor";
    case TokenKind::Shl: return "shl";
    case TokenKind::Shr: return "shr";
    case TokenKind::PlusEqual: return "addeq";
    case TokenKind::MinusEqual: return "subeq";
    case TokenKind::MultEqual: return "muleq";
    case TokenKind::DivEqual: return "diveq";
    case TokenKind::ModEqual: return "modeq";
    case TokenKind::BAndEqual: return "bandeq";
    case TokenKind::BXorEqual: return "bxoreq";
    case TokenKind::BOrEqual: return "boreq";
    case TokenKind::ShlEqual: return "shleq";
    case TokenKind::ShrEqual: return "shreq";
    default: return "";
  }
}



std::string Parser::getSpecialOverloadOperatorName(TokenKind firstToken, TokenKind secondToken, TokenKind thirdToken) {
  // Handle multi-token operators
  if (firstToken == TokenKind::LParen && secondToken == TokenKind::RParen) {
    return "call";
  }
  if (firstToken == TokenKind::LBracket && secondToken == TokenKind::RBracket) {
    if (thirdToken == TokenKind::Assign) {
      return "indexassign";
    }
    return "index";
  }

  if (firstToken == TokenKind::BAndDot) {
    return "redirect";
  }
  if (firstToken == TokenKind::DotDot) {
    return "range";
  }
  return "";
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

ast::ASTNode *Parser::parseExpression(bool withoutStructLiterals) {
  // Start at the top of the precedence hierarchy
  // Now includes assignment expressions
  return parseAssignmentExpression(withoutStructLiterals);
}

ast::ASTNode *Parser::parseAssignmentExpression(bool withoutStructLiterals) {
  // assignment_expression ::=
  //   | conditional_expression
  //   | conditional_expression assignment_operator assignment_expression

  ast::ASTNode *left = parseConditionalExpression(withoutStructLiterals);
  if (!left) {
    return nullptr;
  }

  // Check for assignment operators (right-associative)
  if (isAssignmentOperator(current().kind)) {
    Token opToken = current();
    advance(); // consume operator

    // Parse the right-hand side (right-associative)
    ast::ASTNode *right = parseAssignmentExpression(withoutStructLiterals);
    if (!right) {
      return nullptr; // Error already reported
    }

    // Create assignment expression node
    return ast::createAssignmentExpr(left, opToken.kind, right,
                                     opToken.location, arena_);
  }

  return left;
}

ast::ASTNode *Parser::parseConditionalExpression(bool withoutStructLiterals) {
  // conditional_expression ::=
  //   | logical_or_expression
  //   | logical_or_expression '?' expression ':' conditional_expression

  ast::ASTNode *condition = parseLogicalOrExpression(withoutStructLiterals);
  if (!condition) {
    return nullptr;
  }

  // Check for ternary operator
  if (check(TokenKind::Question)) {
    Token questionToken = current();
    advance(); // consume '?'

    // Parse the 'then' expression
    ast::ASTNode *thenExpr = parseExpression(withoutStructLiterals);
    if (!thenExpr) {
      return nullptr; // Error already reported
    }

    // Expect ':'
    if (!expect(TokenKind::Colon,
                "Expected ':' after then expression in ternary operator")) {
      return nullptr;
    }

    // Parse the 'else' expression (right-associative)
    ast::ASTNode *elseExpr = parseConditionalExpression(withoutStructLiterals);
    if (!elseExpr) {
      return nullptr; // Error already reported
    }

    // Create ternary expression node
    return ast::createTernaryExpr(condition, thenExpr, elseExpr,
                                  questionToken.location, arena_);
  }

  return condition;
}

ast::ASTNode *Parser::parseLogicalOrExpression(bool withoutStructLiterals) {
  // logical_or_expression ::=
  //   | logical_and_expression
  //   | logical_or_expression '||' logical_and_expression

  ast::ASTNode *left = parseLogicalAndExpression(withoutStructLiterals);
  if (!left) {
    return nullptr;
  }

  while (check(TokenKind::LOr)) {
    Token opToken = current();
    advance(); // consume operator

    ast::ASTNode *right = parseLogicalAndExpression(withoutStructLiterals);
    if (!right) {
      return nullptr; // Error already reported
    }

    // Create binary expression node
    left = ast::createBinaryExpr(left, opToken.kind, right, opToken.location,
                                 arena_);
  }

  return left;
}

ast::ASTNode *Parser::parseLogicalAndExpression(bool withoutStructLiterals) {
  // logical_and_expression ::=
  //   | bitwise_or_expression
  //   | logical_and_expression '&&' bitwise_or_expression

  ast::ASTNode *left = parseBitwiseOrExpression(withoutStructLiterals);
  if (!left) {
    return nullptr;
  }

  while (check(TokenKind::LAnd)) {
    Token opToken = current();
    advance(); // consume operator

    ast::ASTNode *right = parseBitwiseOrExpression(withoutStructLiterals);
    if (!right) {
      return nullptr; // Error already reported
    }

    // Create binary expression node
    left = ast::createBinaryExpr(left, opToken.kind, right, opToken.location,
                                 arena_);
  }

  return left;
}

ast::ASTNode *Parser::parseBitwiseOrExpression(bool withoutStructLiterals) {
  // bitwise_or_expression ::=
  //   | bitwise_xor_expression
  //   | bitwise_or_expression '|' bitwise_xor_expression

  ast::ASTNode *left = parseBitwiseXorExpression(withoutStructLiterals);
  if (!left) {
    return nullptr;
  }

  while (check(TokenKind::BOr)) {
    Token opToken = current();
    advance(); // consume operator

    ast::ASTNode *right = parseBitwiseXorExpression(withoutStructLiterals);
    if (!right) {
      return nullptr; // Error already reported
    }

    // Create binary expression node
    left = ast::createBinaryExpr(left, opToken.kind, right, opToken.location,
                                 arena_);
  }

  return left;
}

ast::ASTNode *Parser::parseBitwiseXorExpression(bool withoutStructLiterals) {
  // bitwise_xor_expression ::=
  //   | bitwise_and_expression
  //   | bitwise_xor_expression '^' bitwise_and_expression

  ast::ASTNode *left = parseBitwiseAndExpression(withoutStructLiterals);
  if (!left) {
    return nullptr;
  }

  while (check(TokenKind::BXor)) {
    Token opToken = current();
    advance(); // consume operator

    ast::ASTNode *right = parseBitwiseAndExpression(withoutStructLiterals);
    if (!right) {
      return nullptr; // Error already reported
    }

    // Create binary expression node
    left = ast::createBinaryExpr(left, opToken.kind, right, opToken.location,
                                 arena_);
  }

  return left;
}

ast::ASTNode *Parser::parseBitwiseAndExpression(bool withoutStructLiterals) {
  // bitwise_and_expression ::=
  //   | equality_expression
  //   | bitwise_and_expression '&' equality_expression

  ast::ASTNode *left = parseEqualityExpression(withoutStructLiterals);
  if (!left) {
    return nullptr;
  }

  while (check(TokenKind::BAnd)) {
    Token opToken = current();
    advance(); // consume operator

    ast::ASTNode *right = parseEqualityExpression(withoutStructLiterals);
    if (!right) {
      return nullptr; // Error already reported
    }

    // Create binary expression node
    left = ast::createBinaryExpr(left, opToken.kind, right, opToken.location,
                                 arena_);
  }

  return left;
}

ast::ASTNode *Parser::parseEqualityExpression(bool withoutStructLiterals) {
  // equality_expression ::=
  //   | relational_expression
  //   | equality_expression '==' relational_expression
  //   | equality_expression '!=' relational_expression

  ast::ASTNode *left = parseRelationalExpression(withoutStructLiterals);
  if (!left) {
    return nullptr;
  }

  while (check(TokenKind::Equal) || check(TokenKind::NotEqual)) {
    Token opToken = current();
    advance(); // consume operator

    ast::ASTNode *right = parseRelationalExpression(withoutStructLiterals);
    if (!right) {
      return nullptr; // Error already reported
    }

    // Create binary expression node
    left = ast::createBinaryExpr(left, opToken.kind, right, opToken.location,
                                 arena_);
  }

  return left;
}

ast::ASTNode *Parser::parseRelationalExpression(bool withoutStructLiterals) {
  // relational_expression ::=
  //   | range_expression
  //   | relational_expression '<' range_expression
  //   | relational_expression '<=' range_expression
  //   | relational_expression '>' range_expression
  //   | relational_expression '>=' range_expression

  ast::ASTNode *left = parseRangeExpression(withoutStructLiterals);
  if (!left) {
    return nullptr;
  }

  while (check(TokenKind::Less) || check(TokenKind::LessEqual) ||
         check(TokenKind::Greater) || check(TokenKind::GreaterEqual)) {
    Token opToken = current();
    advance(); // consume operator

    ast::ASTNode *right = parseRangeExpression(withoutStructLiterals);
    if (!right) {
      return nullptr;
    }

    left = ast::createBinaryExpr(left, opToken.kind, right, opToken.location,
                                 arena_);
  }

  return left;
}

ast::ASTNode *Parser::parseRangeExpression(bool withoutStructLiterals) {
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
      ast::ASTNode *end = parseShiftExpression(withoutStructLiterals);
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

    ast::ASTNode *end = parseShiftExpression(withoutStructLiterals);
    if (!end) {
      return nullptr;
    }
    return ast::createRangeExpr(nullptr, end, false, opToken.location, arena_);
  }

  // First, parse left side expression
  ast::ASTNode *left = parseShiftExpression(withoutStructLiterals);
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
      ast::ASTNode *right = parseShiftExpression(withoutStructLiterals);
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
    ast::ASTNode *right = parseShiftExpression(withoutStructLiterals);
    if (!right) {
      return nullptr;
    }
    return ast::createRangeExpr(left, right, false, opToken.location, arena_);
  }

  // No range operator found, just return the left side
  return left;
}

ast::ASTNode *Parser::parseShiftExpression(bool withoutStructLiterals) {
  // shift_expression ::=
  //   | additive_expression
  //   | shift_expression '<<' additive_expression
  //   | shift_expression '>>' additive_expression

  ast::ASTNode *left = parseAdditiveExpression(withoutStructLiterals);
  if (!left) {
    return nullptr;
  }

  while (check(TokenKind::Shl) || check(TokenKind::Shr)) {
    Token opToken = current();
    advance(); // consume operator

    ast::ASTNode *right = parseAdditiveExpression(withoutStructLiterals);
    if (!right) {
      return nullptr;
    }

    left = ast::createBinaryExpr(left, opToken.kind, right, opToken.location,
                                 arena_);
  }

  return left;
}

ast::ASTNode *Parser::parseAdditiveExpression(bool withoutStructLiterals) {
  // additive_expression ::=
  //   | multiplicative_expression
  //   | additive_expression '+' multiplicative_expression
  //   | additive_expression '-' multiplicative_expression

  ast::ASTNode *left = parseMultiplicativeExpression(withoutStructLiterals);
  if (!left) {
    return nullptr;
  }

  while (check(TokenKind::Plus) || check(TokenKind::Minus)) {
    Token opToken = current();
    advance(); // consume operator

    ast::ASTNode *right = parseMultiplicativeExpression(withoutStructLiterals);
    if (!right) {
      return nullptr;
    }

    left = ast::createBinaryExpr(left, opToken.kind, right, opToken.location,
                                 arena_);
  }

  return left;
}

ast::ASTNode *Parser::parseMultiplicativeExpression(bool withoutStructLiterals) {
  // multiplicative_expression ::=
  //   | cast_expression
  //   | multiplicative_expression '*' cast_expression
  //   | multiplicative_expression '/' cast_expression
  //   | multiplicative_expression '%' cast_expression

  ast::ASTNode *left = parseCastExpression(withoutStructLiterals);
  if (!left) {
    return nullptr;
  }

  while (check(TokenKind::Mult) || check(TokenKind::Div) ||
         check(TokenKind::Mod)) {
    Token opToken = current();
    advance(); // consume operator

    ast::ASTNode *right = parseCastExpression(withoutStructLiterals);
    if (!right) {
      return nullptr;
    }

    left = ast::createBinaryExpr(left, opToken.kind, right, opToken.location,
                                 arena_);
  }

  return left;
}

ast::ASTNode *Parser::parseUnaryExpression(bool withoutStructLiterals) {
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

    ast::ASTNode *operand = parseUnaryExpression(withoutStructLiterals); // right-associative
    if (!operand) {
      return nullptr;
    }

    return ast::createUnaryExpr(opToken.kind, true, operand, opToken.location,
                                arena_);
  }

  // No prefix operator, delegate to postfix expression
  return parsePostfixExpression(withoutStructLiterals);
}

ast::ASTNode *Parser::parseCastExpression(bool withoutStructLiterals) {
  // cast_expression ::=
  //   | postfix_expression
  //   | cast_expression 'as' type_expression
  //   | cast_expression '!:' type_expression

  ast::ASTNode *left = parseUnaryExpression(withoutStructLiterals);
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

ast::ASTNode *Parser::parsePostfixExpression(bool withoutStructLiterals) {
  // postfix_expression ::=
  //   | primary_expression
  //   | postfix_expression '++'
  //   | postfix_expression '--'
  //   | postfix_expression '(' argument_list? ')'

  ast::ASTNode *expr = parsePrimaryExpression(withoutStructLiterals);
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

      ast::ASTNode *memberExpr = parsePrimaryExpression(withoutStructLiterals);
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

      ast::ASTNode *memberExpr = parseIdentifierExpression(true);
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

ast::ASTNode *Parser::parsePrimaryExpression(bool withoutStructLiterals) {
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
    ast::ASTNode *expr = parsePostfixExpression(withoutStructLiterals);
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
  if (check(TokenKind::LBrace) && !withoutStructLiterals) {
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
      return parseIdentifierExpression(withoutStructLiterals);
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

ast::ASTNode *Parser::parseIdentifierExpression(bool withoutStructLiterals) {
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
  if (check(TokenKind::LBrace) && !withoutStructLiterals) {
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

  // Only advance past separators/terminators, not structure starters
  if (!isAtEnd() && isSeparatorToken()) {
    advance();
  }
}

bool Parser::isSeparatorToken() const {
  TokenKind kind = current().kind;
  // These are tokens we want to skip over during synchronization
  // Separators and terminators that don't start new constructs
  return kind == TokenKind::Comma ||
         kind == TokenKind::Semicolon ||
         kind == TokenKind::RBrace ||
         kind == TokenKind::RParen ||
         kind == TokenKind::RBracket;
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
  // Check for attributes at the beginning
  ast::AttributeListNode *attributes = nullptr;
  if (check(TokenKind::At)) {
    attributes = static_cast<ast::AttributeListNode*>(parseAttributeList());
    if (!attributes) {
      return nullptr; // Error parsing attributes
    }
  }

  // Dispatch based on current token
  ast::ASTNode *stmt = nullptr;
  switch (current().kind) {
  case TokenKind::Break:
    stmt = parseBreakStatement();
    break;
  case TokenKind::Continue:
    stmt = parseContinueStatement();
    break;
  case TokenKind::LBrace:
    stmt = parseBlockStatement();
    break;
  case TokenKind::Defer:
    stmt = parseDeferStatement();
    break;
  case TokenKind::Return:
    stmt = parseReturnStatement();
    break;
  case TokenKind::Yield:
    stmt = parseYieldStatement();
    break;
  case TokenKind::Var:
  case TokenKind::Const:
  case TokenKind::Auto:
    stmt = parseVariableDeclaration(false, false);
    break;
  case TokenKind::If:
    stmt = parseIfStatement();
    break;
  case TokenKind::While:
    stmt = parseWhileStatement();
    break;
  case TokenKind::For:
    stmt = parseForStatement();
    break;
  case TokenKind::Switch:
    stmt = parseSwitchStatement();
    break;
  case TokenKind::Match:
    stmt = parseMatchStatement();
    break;
  default:
    // Fall back to expression statement
    stmt = parseExpressionStatement();
    break;
  }

  // Attach attributes to the statement if both exist
  if (attributes && stmt) {
    for (auto *attr : attributes->attributes) {
      stmt->addAttribute(attr);
    }
  }

  return stmt;
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

ast::ASTNode *Parser::parseVariableDeclaration(bool singleVariable, bool isExtern) {
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

  // Parse additional names (comma-separated) - only if not single variable
  if (!singleVariable) {
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
  } else if (check(TokenKind::Comma)) {
    // Single variable mode - comma not allowed
    reportError(ParseError(ParseErrorType::UnexpectedToken, current().location,
                           "Only single variable declarations allowed in this context"));
    return nullptr;
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

  // Extern variable validation
  if (isExtern) {
    if (!typeExpr) {
      reportError(ParseError(ParseErrorType::InvalidDeclaration, current().location,
                             "External variable declarations must have explicit type annotations"));
      return nullptr;
    }
    if (initializer) {
      reportError(ParseError(ParseErrorType::InvalidDeclaration, current().location,
                             "External variable declarations cannot have initializers"));
      return nullptr;
    }
  }

  // Validate constraint: either type or initializer must be present
  // In single variable mode (if conditions), initializer is always required
  if (!typeExpr && !initializer) {
    reportError(ParseError(ParseErrorType::UnexpectedToken, current().location,
                           "Variable declaration must have either type annotation or initializer"));
    return nullptr;
  }

  if (singleVariable && !initializer) {
    reportError(ParseError(ParseErrorType::UnexpectedToken, current().location,
                           "Variable declarations in if conditions must have an initializer"));
    return nullptr;
  }

  // Check for optional semicolon
  if (check(TokenKind::Semicolon)) {
    advance(); // consume semicolon
  }

  return decl;
}

// Phase 5.2: Declaration parsing implementation

ast::ASTNode *Parser::parseDeclaration() {
  // Check for attributes first
  ast::AttributeListNode *attributes = nullptr;
  if (check(TokenKind::At)) {
    attributes = static_cast<ast::AttributeListNode*>(parseAttributeList());
    if (!attributes) {
      return nullptr; // Error parsing attributes
    }
  }

  // Check for visibility modifiers after attributes
  Flags visibilityFlags = flgNone;
  if (check(TokenKind::Pub)) {
    advance(); // consume 'pub'
    visibilityFlags |= flgPublic;
  } else if (check(TokenKind::Extern)) {
    advance(); // consume 'extern'
    visibilityFlags |= flgExtern;
  }

  // Dispatch based on current token
  ast::ASTNode *decl = nullptr;
  switch (current().kind) {
  case TokenKind::Var:
  case TokenKind::Const:
  case TokenKind::Auto:
    decl = parseVariableDeclaration(false, (visibilityFlags & flgExtern) != 0);
    break;
  case TokenKind::Func:
    decl = parseFunctionDeclaration((visibilityFlags & flgExtern) != 0);
    break;
  case TokenKind::Enum:
    // Enums cannot be extern
    if (visibilityFlags & flgExtern) {
      reportError(ParseError(ParseErrorType::InvalidDeclaration, current().location,
                             "Enums cannot be extern - they define types, not external symbols"));
      return nullptr;
    }
    decl = parseEnumDeclaration();
    break;
  case TokenKind::Struct:
  case TokenKind::Class:
    // Structs and classes cannot be extern
    if (visibilityFlags & flgExtern) {
      reportError(ParseError(ParseErrorType::InvalidDeclaration, current().location,
                             "Structs and classes cannot be extern - they define types, not external symbols"));
      return nullptr;
    }
    decl = parseStructOrClassDeclaration();
    break;
  default:
    reportError(ParseError(ParseErrorType::UnexpectedToken, current().location,
                           "Expected declaration"));
    return nullptr;
  }

  // Attach attributes to the declaration if both exist
  if (attributes && decl) {
    for (auto *attr : attributes->attributes) {
      decl->addAttribute(attr);
    }
  }

  // Set visibility flags on the declaration
  if (visibilityFlags != flgNone && decl) {
    decl->flags |= visibilityFlags;
  }

  return decl;
}

// Phase 5.2: Function declaration parsing implementation

ast::ASTNode *Parser::parseFunctionParamDeclaration() {
  Location startLoc = current().location;

  // Check for variadic modifier
  bool isVariadic = false;
  if (check(TokenKind::Elipsis)) {
    isVariadic = true;
    advance(); // consume '...'
  }

  // Parse parameter name
  if (!check(TokenKind::Ident)) {
    reportError(ParseError(ParseErrorType::UnexpectedToken, current().location,
                           "Expected parameter name"));
    return nullptr;
  }

  auto *param = ast::createFuncParamDeclaration(startLoc, arena_);
  Token paramNameToken = current();
  advance(); // consume parameter name
  auto *paramNameNode = ast::createIdentifier(paramNameToken.value.stringValue, paramNameToken.location, arena_);
  param->setName(paramNameNode);

  // Parse type using parseTypeExpression
  auto *typeExpr = parseTypeExpression();
  if (!typeExpr) {
    reportError(ParseError(ParseErrorType::UnexpectedToken, current().location,
                           "Expected parameter type"));
    return nullptr;
  }
  param->setType(typeExpr);

  // Parse default value if present
  if (check(TokenKind::Assign)) {
    advance(); // consume '='
    auto *defaultExpr = parseExpression();
    if (!defaultExpr) {
      return nullptr;
    }
    param->setDefaultValue(defaultExpr);
  }

  // Set variadic flag if this is a variadic parameter
  if (isVariadic) {
    param->flags |= flgVariadic;
  }

  return param;
}

ast::ASTNode *Parser::parseFunctionDeclaration(bool isExtern) {
  Location startLoc = current().location;

  // Consume 'func' keyword
  if (!expect(TokenKind::Func)) {
    return nullptr;
  }

  // Parse function name or operator overload
  if (!check(TokenKind::Ident) && !check(TokenKind::Quote)) {
    reportError(ParseError(ParseErrorType::UnexpectedToken, current().location,
                           "Expected function name or operator overload after 'func'"));
    return nullptr;
  }

  // Create function declaration
  auto *funcDecl = ast::createFuncDeclaration(startLoc, arena_);

  // Parse function name or operator overload
  if (check(TokenKind::Quote)) {
    // Parse operator overload: `operator`
    advance(); // consume opening backtick

    if (isAtEnd()) {
      reportError(ParseError(ParseErrorType::UnexpectedToken, current().location,
                             "Expected operator after '`'"));
      return nullptr;
    }

    Location operatorLoc = current().location;
    std::string operatorName;
    TokenKind operatorToken = TokenKind::Error;

    // Try special operators first (multi-token)
    if (check(TokenKind::LParen) && lookahead().kind == TokenKind::RParen) {
      // Handle () operator
      operatorName = getSpecialOverloadOperatorName(TokenKind::LParen, TokenKind::RParen);
      operatorToken = TokenKind::CallOverride;
      advance(); // consume '('
      advance(); // consume ')'
    } else if (check(TokenKind::LBracket)) {
      // Handle [] or []= operators
      advance(); // consume '['
      if (check(TokenKind::RBracket)) {
        advance(); // consume ']'
        if (check(TokenKind::Assign)) {
          // []= operator
          operatorName = getSpecialOverloadOperatorName(TokenKind::LBracket, TokenKind::RBracket, TokenKind::Assign);
          operatorToken = TokenKind::IndexAssignOvd;
          advance(); // consume '='
        } else {
          // [] operator
          operatorName = getSpecialOverloadOperatorName(TokenKind::LBracket, TokenKind::RBracket);
          operatorToken = TokenKind::IndexOverride;
        }
      } else {
        reportError(ParseError(ParseErrorType::UnexpectedToken, current().location,
                               "Expected ']' after '['"));
        return nullptr;
      }
    } else if (check(TokenKind::BAndDot)) {
      // Handle &. redirect operator
      operatorName = getSpecialOverloadOperatorName(TokenKind::BAndDot);
      operatorToken = current().kind;
      advance(); // consume '&.'
    } else if (check(TokenKind::DotDot)) {
      // Handle .. range operator
      operatorName = getSpecialOverloadOperatorName(TokenKind::DotDot);
      operatorToken = current().kind;
      advance(); // consume '..'
    } else {
      // Check for increment/decrement operators first
      if (current().kind == TokenKind::PlusPlus) {
        operatorName = "inc";
        operatorToken = TokenKind::PlusPlus;
        advance(); // consume '++'
      } else if (current().kind == TokenKind::MinusMinus) {
        operatorName = "dec";
        operatorToken = TokenKind::MinusMinus;
        advance(); // consume '--'
      } else {
        // Try binary operators
        operatorName = getBinaryOverloadOperatorName(current().kind);
        if (!operatorName.empty()) {
          operatorToken = current().kind;
          advance(); // consume operator
        } else {
          reportError(ParseError(ParseErrorType::UnexpectedToken, current().location,
                                 "Invalid operator for overload"));
          return nullptr;
        }
      }
    }

    if (!expect(TokenKind::Quote)) {
      return nullptr; // Expected closing backtick
    }

    // Create identifier node with operator name and set operator token
    InternedString opName = interner_.intern(operatorName);
    auto *nameNode = ast::createIdentifier(opName, operatorLoc, arena_);
    funcDecl->setName(nameNode);
    funcDecl->setOperatorToken(operatorToken);

  } else {
    // Parse regular identifier
    Token nameToken = current();
    advance(); // consume identifier

    if (!nameToken.hasLiteralValue()) {
      reportError(ParseError(ParseErrorType::UnexpectedToken, nameToken.location,
                             "Function name token missing value"));
      return nullptr;
    }

    auto *nameNode = ast::createIdentifier(nameToken.value.stringValue, nameToken.location, arena_);
    funcDecl->setName(nameNode);
  }

  // Check for generic parameters after function name
  ArenaVector<ast::ASTNode*> genericParams{ArenaSTLAllocator<ast::ASTNode*>(arena_)};
  bool hasGenericParams = false;

  if (check(TokenKind::Less)) {
    // Extern function validation: cannot be generic
    if (isExtern) {
      reportError(ParseError(ParseErrorType::InvalidDeclaration, current().location,
                             "External function declarations cannot have generic parameters"));
      return nullptr;
    }

    genericParams = parseGenericParameters();
    if (genericParams.empty() && check(TokenKind::Greater)) {
      // Error occurred during parsing, but we might have consumed '<'
      return nullptr;
    }
    hasGenericParams = !genericParams.empty();
  }

  // State tracking for validation
  bool hasDefaultParam = false;
  bool hasVariadicParam = false;

  // Parse parameter list if present
  if (check(TokenKind::LParen)) {
    advance(); // consume '('

    // Parse parameters
    while (!check(TokenKind::RParen) && !isAtEnd()) {
      // Check for variadic (must be last)
      if (hasVariadicParam) {
        reportError(ParseError(ParseErrorType::InvalidDeclaration, current().location,
                               "Variadic parameter must be the last parameter"));
        return nullptr;
      }

      bool currentIsVariadic = check(TokenKind::Elipsis);

      auto *param = parseFunctionParamDeclaration();
      if (!param) {
        return nullptr;
      }

      auto *paramDecl = static_cast<ast::FuncParamDeclarationNode*>(param);
      bool hasDefault = (paramDecl->defaultValue != nullptr);

      // Validate parameter ordering constraints
      if (hasDefaultParam && !hasDefault) {
        reportError(ParseError(ParseErrorType::InvalidDeclaration, current().location,
                               "Non-default parameter cannot follow default parameter"));
        return nullptr;
      }

      if (hasDefault) {
        hasDefaultParam = true;
      }

      if (currentIsVariadic) {
        hasVariadicParam = true;
      }

      funcDecl->addParameter(param);

      // Check for comma or end of parameters
      if (check(TokenKind::Comma)) {
        advance(); // consume ','
        // Optional trailing comma support
        if (check(TokenKind::RParen)) {
          break;
        }
      } else if (!check(TokenKind::RParen)) {
        reportError(ParseError(ParseErrorType::UnexpectedToken, current().location,
                               "Expected ',' or ')' in parameter list"));
        return nullptr;
      }
    }

    if (!expect(TokenKind::RParen)) {
      return nullptr;
    }
  }

  // Parse return type if present (direct type, no arrow)
  // If the next token is not a function body start, try to parse return type
  if (!check(TokenKind::FatArrow) && !check(TokenKind::LBrace) && !isAtEnd()) {
    auto *returnTypeExpr = parseTypeExpression();
    if (!returnTypeExpr) {
      return nullptr; // Error parsing return type
    }
    funcDecl->setReturnType(returnTypeExpr);
  }

  // Extern function validation: must have return type
  if (isExtern && !funcDecl->returnType) {
    reportError(ParseError(ParseErrorType::InvalidDeclaration, current().location,
                           "External function declarations must have explicit return types"));
    return nullptr;
  }

  // Parse function body if present
  if (check(TokenKind::FatArrow)) {
    // Extern function validation: cannot have body
    if (isExtern) {
      reportError(ParseError(ParseErrorType::InvalidDeclaration, current().location,
                             "External function declarations cannot have function bodies"));
      return nullptr;
    }

    advance(); // consume '=>'
    // Expression body
    auto *bodyExpr = parseExpression();
    if (!bodyExpr) {
      return nullptr;
    }
    funcDecl->setBody(bodyExpr);
  } else if (check(TokenKind::LBrace)) {
    // Extern function validation: cannot have body
    if (isExtern) {
      reportError(ParseError(ParseErrorType::InvalidDeclaration, current().location,
                             "External function declarations cannot have function bodies"));
      return nullptr;
    }

    // Block body
    auto *bodyBlock = parseBlockStatement();
    if (!bodyBlock) {
      return nullptr;
    }
    funcDecl->setBody(bodyBlock);
  }

  // Check for prohibited unary operator overloads
  if (funcDecl->isOperatorOverload()) {
    TokenKind opKind = funcDecl->operatorToken;
    size_t paramCount = funcDecl->parameters.size();

    // Prohibit unary use of certain operators
    if (paramCount == 0 && (opKind == TokenKind::BAnd || opKind == TokenKind::BXor ||
                           opKind == TokenKind::LAnd || opKind == TokenKind::LNot ||
                           opKind == TokenKind::BNot)) {
      reportError(ParseError(ParseErrorType::InvalidDeclaration, current().location,
                             "This operator cannot be overloaded as a unary operator"));
      return nullptr;
    }
  }

  // Set variadic flag on function if it has variadic parameters
  if (hasVariadicParam) {
    funcDecl->flags |= flgVariadic;
  }

  // If we have generic parameters, wrap in GenericDeclarationNode
  if (hasGenericParams) {
    auto *genericDecl = ast::createGenericDeclaration(startLoc, arena_);

    // Move generic parameters vector directly to avoid copying
    genericDecl->parameters = std::move(genericParams);

    // Add children for proper AST parent-child relationships
    for (auto *param : genericDecl->parameters) {
      if (param) {
        genericDecl->addChild(param);
      }
    }

    // Set the function declaration as the wrapped declaration
    genericDecl->setDeclaration(funcDecl);

    return genericDecl;
  }

  return funcDecl;
}

// Phase 4.5: If statement parsing implementation

ast::ASTNode *Parser::parseIfStatement() {
  Location startLoc = current().location;

  // Consume 'if' keyword
  if (!expect(TokenKind::If)) {
    return nullptr;
  }

  // Parse condition (with or without parentheses)
  bool hasParentheses = false;
  ast::ASTNode *condition = nullptr;

  if (check(TokenKind::LParen)) {
    hasParentheses = true;
    advance(); // consume '('

    // Parse condition expression or variable declaration
    if (check(TokenKind::Var) || check(TokenKind::Const) || check(TokenKind::Auto)) {
      condition = parseVariableDeclaration(true, false); // single variable only
    } else {
      condition = parseExpression();
    }

    if (!condition) {
      return nullptr; // Error already reported
    }

    if (!expect(TokenKind::RParen)) {
      return nullptr;
    }
  } else {
    // Parse bare condition (expression or variable declaration)
    if (check(TokenKind::Var) || check(TokenKind::Const) || check(TokenKind::Auto)) {
      condition = parseVariableDeclaration(true, false); // single variable only
    } else {
      condition = parseExpression(true);
    }

    if (!condition) {
      return nullptr; // Error already reported
    }
  }

  // Parse if body
  ast::ASTNode *thenStatement = nullptr;

  if (hasParentheses) {
    // With parentheses: single statement or block allowed
    if (check(TokenKind::LBrace)) {
      thenStatement = parseBlockStatement();
    } else {
      thenStatement = parseStatement();
    }
  } else {
    // Without parentheses: block required
    if (!check(TokenKind::LBrace)) {
      reportError(ParseError(ParseErrorType::UnexpectedToken, current().location,
                             "Block statement required when if condition has no parentheses"));
      return nullptr;
    }
    thenStatement = parseBlockStatement();
  }

  if (!thenStatement) {
    return nullptr; // Error already reported
  }

  // Parse optional else clause
  ast::ASTNode *elseStatement = nullptr;
  if (check(TokenKind::Else)) {
    advance(); // consume 'else'

    if (check(TokenKind::If)) {
      // else if - parse another if statement
      elseStatement = parseIfStatement();
    } else {
      // else clause - follow same rules as then clause
      if (hasParentheses) {
        // With parentheses in main condition: single statement or block allowed
        if (check(TokenKind::LBrace)) {
          elseStatement = parseBlockStatement();
        } else {
          elseStatement = parseStatement();
        }
      } else {
        // Without parentheses in main condition: block required
        if (!check(TokenKind::LBrace)) {
          reportError(ParseError(ParseErrorType::UnexpectedToken, current().location,
                                 "Expected block statement after 'else'"));
          return nullptr;
        }
        elseStatement = parseBlockStatement();
      }
    }

    if (!elseStatement) {
      return nullptr; // Error already reported
    }
  }

  // Create if statement node
  return ast::createIfStatement(condition, thenStatement, startLoc, arena_, elseStatement);
}

ast::ASTNode *Parser::parseWhileStatement() {
  Location startLoc = current().location;

  // Consume 'while' keyword
  if (!expect(TokenKind::While)) {
    return nullptr;
  }

  // Parse optional condition (with or without parentheses)
  bool hasParentheses = false;
  ast::ASTNode *condition = nullptr;

  if (check(TokenKind::LBrace)) {
    // Infinite loop: while { }
    condition = nullptr;
  } else if (check(TokenKind::LParen)) {
    hasParentheses = true;
    advance(); // consume '('

    // Parse condition expression or variable declaration
    if (check(TokenKind::Var) || check(TokenKind::Const) || check(TokenKind::Auto)) {
      condition = parseVariableDeclaration(true, false); // single variable only
    } else {
      condition = parseExpression();
    }

    if (!condition) {
      return nullptr; // Error already reported
    }

    if (!expect(TokenKind::RParen)) {
      return nullptr;
    }
  } else {
    // Parse bare condition (expression or variable declaration)
    if (check(TokenKind::Var) || check(TokenKind::Const) || check(TokenKind::Auto)) {
      condition = parseVariableDeclaration(true, false); // single variable only
    } else {
      condition = parseExpression(true); // withoutStructLiterals = true
    }

    if (!condition) {
      return nullptr; // Error already reported
    }
  }

  // Parse while body
  ast::ASTNode *body = nullptr;

  if (hasParentheses) {
    // With parentheses: single statement or block allowed
    if (check(TokenKind::LBrace)) {
      body = parseBlockStatement();
    } else {
      body = parseStatement();
    }
  } else {
    // Without parentheses or infinite loop: block required
    if (!check(TokenKind::LBrace)) {
      reportError(ParseError(ParseErrorType::UnexpectedToken, current().location,
                             "Block statement required for while loop"));
      return nullptr;
    }
    body = parseBlockStatement();
  }

  if (!body) {
    return nullptr; // Error already reported
  }

  // Create while statement node
  return ast::createWhileStatement(condition, body, startLoc, arena_);
}

ast::ASTNode *Parser::parseForStatement() {
  Location startLoc = current().location;

  // Consume 'for' keyword
  if (!expect(TokenKind::For)) {
    return nullptr;
  }

  // Check for parentheses
  bool hasParentheses = false;
  if (check(TokenKind::LParen)) {
    hasParentheses = true;
    advance(); // consume '('
  }

  // Parse iterator variable list
  std::vector<ast::ASTNode *> variables;

  do {
    if (!check(TokenKind::Ident)) {
      reportError(ParseError(ParseErrorType::UnexpectedToken, current().location,
                             "Expected identifier in for loop variable list"));
      return nullptr;
    }

    // Create identifier for the iterator variable
    Location varLoc = current().location;
    InternedString varName = current().value.stringValue;
    auto *identifier = ast::createIdentifier(varName, varLoc, arena_);
    variables.push_back(identifier);
    advance();

    // Check for comma
    if (check(TokenKind::Comma)) {
      advance();
      // Allow trailing comma before 'in'
      if (check(TokenKind::In)) {
        break;
      }
    } else {
      break;
    }
  } while (true);

  // Expect 'in' keyword
  if (!expect(TokenKind::In)) {
    reportError(ParseError(ParseErrorType::UnexpectedToken, current().location,
                           "Expected 'in' keyword in for loop"));
    return nullptr;
  }

  // Parse range expression
  ast::ASTNode *range = parseExpression(hasParentheses ? false : true); // withoutStructLiterals for bare form
  if (!range) {
    return nullptr; // Error already reported
  }

  // Check for optional condition (after comma)
  ast::ASTNode *condition = nullptr;
  if (check(TokenKind::Comma)) {
    advance(); // consume ','
    condition = parseExpression(hasParentheses ? false : true); // withoutStructLiterals for bare form
    if (!condition) {
      return nullptr; // Error already reported
    }
  }

  // Close parentheses if opened
  if (hasParentheses) {
    if (!expect(TokenKind::RParen)) {
      return nullptr;
    }
  }

  // Parse for body
  ast::ASTNode *body = nullptr;

  if (hasParentheses) {
    // With parentheses: single statement or block allowed
    if (check(TokenKind::LBrace)) {
      body = parseBlockStatement();
    } else {
      body = parseStatement();
    }
  } else {
    // Without parentheses: block required
    if (!check(TokenKind::LBrace)) {
      reportError(ParseError(ParseErrorType::UnexpectedToken, current().location,
                             "Block statement required for for loop"));
      return nullptr;
    }
    body = parseBlockStatement();
  }

  if (!body) {
    return nullptr; // Error already reported
  }

  // Create for statement node
  auto *forStmt = ast::createForStatement(range, body, startLoc, arena_, condition);

  // Add all variables to the for statement
  for (auto *var : variables) {
    forStmt->addVariable(var);
  }

  return forStmt;
}

ast::ASTNode *Parser::parseSwitchStatement() {
  Location startLoc = current().location;

  // Consume 'switch' keyword
  if (!expect(TokenKind::Switch)) {
    return nullptr;
  }

  // Parse discriminant (with or without parentheses)
  bool hasParentheses = false;
  ast::ASTNode *discriminant = nullptr;

  if (check(TokenKind::LParen)) {
    hasParentheses = true;
    advance(); // consume '('
  }

  // Parse discriminant expression or variable declaration
  if (check(TokenKind::Var) || check(TokenKind::Const) || check(TokenKind::Auto)) {
    discriminant = parseVariableDeclaration(true, false); // single variable only
  } else {
    discriminant = parseExpression(hasParentheses? false : true); // withoutStructLiterals = true
  }

  if (!discriminant) {
    return nullptr; // Error already reported
  }

  // Close parentheses if opened
  if (hasParentheses) {
    if (!expect(TokenKind::RParen)) {
      return nullptr;
    }
  }

  // Expect opening brace for switch body
  if (!expect(TokenKind::LBrace)) {
    reportError(ParseError(ParseErrorType::UnexpectedToken, current().location,
                           "Expected '{' to open switch body"));
    return nullptr;
  }

  // Create switch statement node
  auto *switchStmt = ast::createSwitchStatement(discriminant, startLoc, arena_);

  // Parse case list
  while (!check(TokenKind::RBrace) && !check(TokenKind::EoF)) {
    auto *caseStmt = parseCaseStatement();
    if (!caseStmt) {
      return nullptr; // Error already reported
    }
    switchStmt->addCase(caseStmt);
  }

  // Expect closing brace
  if (!expect(TokenKind::RBrace)) {
    reportError(ParseError(ParseErrorType::UnexpectedToken, current().location,
                           "Expected '}' to close switch body"));
    return nullptr;
  }

  return switchStmt;
}

ast::ASTNode *Parser::parseCaseStatement() {
  Location startLoc = current().location;
  bool isDefault = false;

  // Check for default case
  if (check(TokenKind::Elipsis)) {
    advance(); // consume '...'
    isDefault = true;
  }

  // Create case statement node
  auto *caseStmt = ast::createCaseStatement(startLoc, arena_, isDefault);

  if (!isDefault) {
    // Parse case values (expression list)
    do {
      auto *value = parseExpression();
      if (!value) {
        return nullptr; // Error already reported
      }
      caseStmt->addValue(value);

      // Check for comma
      if (check(TokenKind::Comma)) {
        advance();
        // Allow trailing comma before '=>'
        if (check(TokenKind::FatArrow)) {
          break;
        }
      } else {
        break;
      }
    } while (true);
  }

  // Expect '=>' arrow
  if (!expect(TokenKind::FatArrow)) {
    reportError(ParseError(ParseErrorType::UnexpectedToken, current().location,
                           "Expected '=>' after case pattern"));
    return nullptr;
  }

  // Parse case body (single statement or block)
  ast::ASTNode *body = nullptr;
  if (check(TokenKind::LBrace)) {
    body = parseBlockStatement();
  } else {
    body = parseStatement();
  }

  if (!body) {
    return nullptr; // Error already reported
  }

  caseStmt->addStatement(body);
  return caseStmt;
}

ast::ASTNode *Parser::parseMatchStatement() {
  Location startLoc = current().location;

  // Consume 'match' keyword
  if (!expect(TokenKind::Match)) {
    return nullptr;
  }

  // Parse discriminant (with or without parentheses)
  bool hasParentheses = false;
  ast::ASTNode *discriminant = nullptr;

  if (check(TokenKind::LParen)) {
    hasParentheses = true;
    advance(); // consume '('
  }

  // Parse discriminant expression
  discriminant = parseExpression(hasParentheses? false : true); // withoutStructLiterals = true
  if (!discriminant) {
    return nullptr; // Error already reported
  }

  // Close parentheses if opened
  if (hasParentheses) {
    if (!expect(TokenKind::RParen)) {
      return nullptr;
    }
  }

  // Expect opening brace for match body
  if (!expect(TokenKind::LBrace)) {
    reportError(ParseError(ParseErrorType::UnexpectedToken, current().location,
                           "Expected '{' to open match body"));
    return nullptr;
  }

  // Create match statement node
  auto *matchStmt = ast::createMatchStatement(discriminant, startLoc, arena_);

  // Parse match case list
  while (!check(TokenKind::RBrace) && !check(TokenKind::EoF)) {
    auto *matchCase = parseMatchCaseStatement();
    if (!matchCase) {
      return nullptr; // Error already reported
    }
    matchStmt->addPattern(matchCase);
  }

  // Expect closing brace
  if (!expect(TokenKind::RBrace)) {
    reportError(ParseError(ParseErrorType::UnexpectedToken, current().location,
                           "Expected '}' to close match body"));
    return nullptr;
  }

  return matchStmt;
}

ast::ASTNode *Parser::parseMatchCaseStatement() {
  Location startLoc = current().location;
  bool isDefault = false;

  // Check for default case
  if (check(TokenKind::Elipsis)) {
    advance(); // consume '...'
    isDefault = true;
  }

  // Create match case node
  auto *matchCase = ast::createMatchCase(startLoc, arena_, isDefault);

  if (!isDefault) {
    // Parse type patterns (expression list)
    do {
      auto *type = parseTypeExpression();
      if (!type) {
        return nullptr; // Error already reported
      }
      matchCase->addType(type);

      // Check for comma
      if (check(TokenKind::Comma)) {
        advance();
        // Allow trailing comma before 'as' or '=>'
        if (check(TokenKind::As) || check(TokenKind::FatArrow)) {
          break;
        }
      } else {
        break;
      }
    } while (true);
  }

  // Check for optional variable binding (as identifier)
  if (check(TokenKind::As)) {
    advance(); // consume 'as'

    if (!check(TokenKind::Ident)) {
      reportError(ParseError(ParseErrorType::UnexpectedToken, current().location,
                             "Expected identifier after 'as'"));
      return nullptr;
    }

    auto *binding = parseIdentifierExpression();
    if (!binding) {
      return nullptr; // Error already reported
    }

    matchCase->setBinding(binding);
  }

  // Expect '=>' arrow
  if (!expect(TokenKind::FatArrow)) {
    reportError(ParseError(ParseErrorType::UnexpectedToken, current().location,
                           "Expected '=>' after match pattern"));
    return nullptr;
  }

  // Parse case body (single statement or block)
  ast::ASTNode *body = nullptr;
  if (check(TokenKind::LBrace)) {
    body = parseBlockStatement();
  } else {
    body = parseStatement();
  }

  if (!body) {
    return nullptr; // Error already reported
  }

  matchCase->addStatement(body);
  return matchCase;
}

ast::ASTNode *Parser::parseAttributeList() {
  Location startLoc = current().location;
  auto *attrList = ast::createAttributeList(startLoc, arena_);

  // Parse attributes until we don't see '@' anymore
  while (check(TokenKind::At)) {
    advance(); // consume '@'

    // Check for list syntax @[attr1, attr2, ...]
    if (check(TokenKind::LBracket)) {
      advance(); // consume '['

      // Parse comma-separated attributes
      do {
        auto *attr = parseAttribute();
        if (!attr) {
          return nullptr; // Error already reported
        }
        attrList->addAttribute(static_cast<ast::AttributeNode*>(attr));

        // Check for comma
        if (check(TokenKind::Comma)) {
          advance();
          // Allow trailing comma before ']'
          if (check(TokenKind::RBracket)) {
            break;
          }
        } else {
          break;
        }
      } while (!check(TokenKind::RBracket) && !check(TokenKind::EoF));

      // Expect closing bracket
      if (!expect(TokenKind::RBracket)) {
        reportError(ParseError(ParseErrorType::UnexpectedToken, current().location,
                               "Expected ']' to close attribute list"));
        return nullptr;
      }
      break; // Only one @[...] list allowed
    } else {
      // Parse single attribute @attr
      auto *attr = parseAttribute();
      if (!attr) {
        return nullptr; // Error already reported
      }
      attrList->addAttribute(static_cast<ast::AttributeNode*>(attr));
    }
  }

  return attrList->hasAttributes() ? attrList : nullptr;
}

ast::ASTNode *Parser::parseAttribute() {
  Location startLoc = current().location;

  // Expect identifier for attribute name
  if (!check(TokenKind::Ident)) {
    reportError(ParseError(ParseErrorType::UnexpectedToken, current().location,
                           "Expected attribute name after '@'"));
    return nullptr;
  }

  Token nameToken = current();
  advance(); // consume attribute name

  // Create attribute node
  auto *attr = ast::createAttribute(interner_.intern(nameToken.value.stringValue.view()),
                                   startLoc, arena_);

  // Check for arguments
  if (check(TokenKind::LParen)) {
    if (!parseAttributeArguments(attr)) {
      return nullptr; // Error already reported
    }
  }

  return attr;
}

bool Parser::parseAttributeArguments(ast::AttributeNode *attr) {
  // Expect opening parenthesis
  if (!expect(TokenKind::LParen)) {
    return false;
  }

  // Handle empty argument list
  if (check(TokenKind::RParen)) {
    advance(); // consume ')'
    return true;
  }

  bool isNamedArgs = false;

  // Parse arguments
  do {
    // Check for named argument syntax (identifier ':' literal)
    if (check(TokenKind::Ident) && lookahead(1).kind == TokenKind::Colon) {
      isNamedArgs = true;

      // Parse named argument: name : value
      Token nameToken = current();
      advance(); // consume name
      advance(); // consume ':'

      // Parse literal value
      auto *value = parseLiteralExpression();
      if (!value) {
        reportError(ParseError(ParseErrorType::UnexpectedToken, current().location,
                               "Expected literal value in named attribute argument"));
        return false;
      }

      // Create field expression for named argument
      auto *field = ast::createFieldExpr(
        ast::createIdentifier(interner_.intern(nameToken.value.stringValue.view()),
                             nameToken.location, arena_),
        value, nameToken.location, arena_);
      attr->addArg(field);
    } else {
      // Parse positional literal argument
      if (isNamedArgs) {
        reportError(ParseError(ParseErrorType::UnexpectedToken, current().location,
                               "Cannot mix positional and named arguments in attribute"));
        return false;
      }

      auto *literal = parseLiteralExpression();
      if (!literal) {
        reportError(ParseError(ParseErrorType::UnexpectedToken, current().location,
                               "Expected literal argument in attribute"));
        return false;
      }
      attr->addArg(literal);
    }

    // Check for comma
    if (check(TokenKind::Comma)) {
      advance();
      // Allow trailing comma before ')'
      if (check(TokenKind::RParen)) {
        break;
      }
    } else {
      break;
    }
  } while (!check(TokenKind::RParen) && !check(TokenKind::EoF));

  // Expect closing parenthesis
  if (!expect(TokenKind::RParen)) {
    reportError(ParseError(ParseErrorType::UnexpectedToken, current().location,
                           "Expected ')' to close attribute arguments"));
    return false;
  }

  return true;
}

ast::ASTNode *Parser::parseGenericParameter() {
  Location startLoc = current().location;

  // Check for variadic modifier
  bool isVariadic = false;
  if (check(TokenKind::Elipsis)) {
    isVariadic = true;
    advance(); // consume '...'
  }

  // Parse parameter name (identifier)
  if (!check(TokenKind::Ident)) {
    reportError(ParseError(ParseErrorType::UnexpectedToken, current().location,
                           "Expected identifier for generic parameter name"));
    return nullptr;
  }

  Token nameToken = current();
  advance(); // consume identifier

  // Create generic parameter node
  auto *param = ast::createTypeParameterDeclaration(startLoc, arena_);

  // Set parameter name
  auto *nameNode = ast::createIdentifier(nameToken.value.stringValue, nameToken.location, arena_);
  param->setName(nameNode);

  // Set variadic flag if needed
  if (isVariadic) {
    param->flags |= flgVariadic;
  }

  // Parse optional constraint (':' type_expression)
  if (check(TokenKind::Colon)) {
    advance(); // consume ':'

    auto *constraintExpr = parseTypeExpression();
    if (!constraintExpr) {
      return nullptr; // Error parsing constraint
    }
    param->setConstraint(constraintExpr);
  }

  // Parse optional default value ('=' type_expression)
  if (check(TokenKind::Assign)) {
    advance(); // consume '='

    auto *defaultExpr = parseTypeExpression();
    if (!defaultExpr) {
      return nullptr; // Error parsing default type
    }
    param->setDefaultValue(defaultExpr);
  }

  return param;
}

ArenaVector<ast::ASTNode*> Parser::parseGenericParameters() {
  ArenaVector<ast::ASTNode*> params{ArenaSTLAllocator<ast::ASTNode*>(arena_)};

  // Expect '<' to start generic parameters
  if (!expect(TokenKind::Less)) {
    return params; // Return empty vector on error
  }

  // State tracking for validation
  bool hasDefaultParam = false;
  bool hasVariadicParam = false;

  // Parse parameters
  while (!check(TokenKind::Greater) && !isAtEnd()) {
    // Check for variadic (must be last)
    if (hasVariadicParam) {
      reportError(ParseError(ParseErrorType::InvalidDeclaration, current().location,
                             "Variadic generic parameter must be the last parameter"));
      return ArenaVector<ast::ASTNode*>(ArenaSTLAllocator<ast::ASTNode*>(arena_)); // Return empty vector
    }

    bool currentIsVariadic = check(TokenKind::Elipsis);

    auto *param = parseGenericParameter();
    if (!param) {
      return ArenaVector<ast::ASTNode*>(ArenaSTLAllocator<ast::ASTNode*>(arena_)); // Return empty vector on error
    }

    auto *paramDecl = static_cast<ast::TypeParameterDeclarationNode*>(param);
    bool hasDefault = (paramDecl->defaultValue != nullptr);

    // Validate parameter ordering constraints
    if (hasDefaultParam && !hasDefault) {
      reportError(ParseError(ParseErrorType::InvalidDeclaration, current().location,
                             "Non-defaulted generic parameter cannot follow defaulted parameter"));
      return ArenaVector<ast::ASTNode*>(ArenaSTLAllocator<ast::ASTNode*>(arena_)); // Return empty vector
    }

    if (hasDefault) {
      hasDefaultParam = true;
    }

    if (currentIsVariadic) {
      hasVariadicParam = true;
    }

    params.push_back(param);

    // Check for comma or end of parameters
    if (check(TokenKind::Comma)) {
      advance(); // consume ','
      // Optional trailing comma support
      if (check(TokenKind::Greater)) {
        break;
      }
    } else if (!check(TokenKind::Greater)) {
      reportError(ParseError(ParseErrorType::UnexpectedToken, current().location,
                             "Expected ',' or '>' in generic parameter list"));
      return ArenaVector<ast::ASTNode*>(ArenaSTLAllocator<ast::ASTNode*>(arena_)); // Return empty vector
    }
  }

  if (!expect(TokenKind::Greater)) {
    return ArenaVector<ast::ASTNode*>(ArenaSTLAllocator<ast::ASTNode*>(arena_)); // Return empty vector on error
  }

  return params;
}

ast::ASTNode *Parser::parseEnumDeclaration() {
  Location startLoc = current().location;

  // Consume 'enum' keyword
  if (!expect(TokenKind::Enum)) {
    return nullptr;
  }

  // Parse enum name
  if (!check(TokenKind::Ident)) {
    reportError(ParseError(ParseErrorType::UnexpectedToken, current().location,
                           "Expected enum name after 'enum'"));
    return nullptr;
  }

  // Create enum declaration
  auto *enumDecl = ast::createEnumDeclaration(startLoc, arena_);

  // Set enum name
  Token nameToken = current();
  advance(); // consume identifier
  auto *nameNode = ast::createIdentifier(nameToken.value.stringValue, nameToken.location, arena_);
  enumDecl->setName(nameNode);

  // Parse optional backing type
  if (check(TokenKind::Colon)) {
    advance(); // consume ':'

    auto *backingTypeExpr = parseTypeExpression();
    if (!backingTypeExpr) {
      return nullptr; // Error parsing backing type
    }
    enumDecl->setBase(backingTypeExpr);
  }

  // Parse enum body
  if (!expect(TokenKind::LBrace)) {
    return nullptr;
  }

  // Parse enum options
  while (!check(TokenKind::RBrace) && !isAtEnd()) {
    auto *option = parseEnumOption();
    if (!option) {
      return nullptr; // Error parsing option
    }

    enumDecl->addOption(option);

    // Check for comma or end of options
    if (check(TokenKind::Comma)) {
      advance(); // consume ','
      // Optional trailing comma support
      if (check(TokenKind::RBrace)) {
        break;
      }
    } else if (!check(TokenKind::RBrace)) {
      reportError(ParseError(ParseErrorType::UnexpectedToken, current().location,
                             "Expected ',' or '}' in enum option list"));
      return nullptr;
    }
  }

  if (!expect(TokenKind::RBrace)) {
    return nullptr;
  }

  return enumDecl;
}

ast::ASTNode *Parser::parseEnumOption() {
  Location startLoc = current().location;

  // Parse attributes if present
  ast::AttributeListNode *attributes = nullptr;
  if (check(TokenKind::At)) {
    attributes = static_cast<ast::AttributeListNode*>(parseAttributeList());
    if (!attributes) {
      return nullptr; // Error parsing attributes
    }
  }

  // Parse option name
  if (!check(TokenKind::Ident)) {
    reportError(ParseError(ParseErrorType::UnexpectedToken, current().location,
                           "Expected identifier for enum option name"));
    return nullptr;
  }

  Token nameToken = current();
  advance(); // consume identifier

  // Create enum option node
  auto *option = ast::createEnumOptionDeclaration(startLoc, arena_);

  // Set option name
  auto *nameNode = ast::createIdentifier(nameToken.value.stringValue, nameToken.location, arena_);
  option->setName(nameNode);

  // Parse optional value assignment
  if (check(TokenKind::Assign)) {
    advance(); // consume '='

    auto *valueExpr = parseExpression();
    if (!valueExpr) {
      return nullptr; // Error parsing value expression
    }
    option->setValue(valueExpr);
  }

  // Attach attributes to the option if present
  if (attributes) {
    for (auto *attr : attributes->attributes) {
      option->addAttribute(attr);
    }
  }

  return option;
}

// Struct and Class Declaration Parsing

ast::ASTNode *Parser::parseStructOrClassDeclaration() {
  Location startLoc = current().location;
  bool isClass = check(TokenKind::Class);

  // Consume 'struct' or 'class' keyword
  if (!expect(isClass ? TokenKind::Class : TokenKind::Struct)) {
    return nullptr;
  }

  // Parse name
  if (!check(TokenKind::Ident)) {
    reportError(ParseError(ParseErrorType::UnexpectedToken, current().location,
                           isClass ? "Expected class name" : "Expected struct name"));
    return nullptr;
  }

  Token nameToken = current();
  advance(); // consume name

  if (!nameToken.hasLiteralValue()) {
    reportError(ParseError(ParseErrorType::UnexpectedToken, nameToken.location,
                           "Name token missing value"));
    return nullptr;
  }

  InternedString name = nameToken.value.stringValue;
  auto *nameNode = ast::createIdentifier(name, nameToken.location, arena_);

  // Check for generic parameters
  ArenaVector<ast::ASTNode*> genericParams{ArenaSTLAllocator<ast::ASTNode*>(arena_)};
  if (check(TokenKind::Less)) {
    genericParams = parseGenericParameters();
    if (genericParams.empty()) {
      return nullptr; // Error already reported
    }
  }

  // Check for inheritance
  ast::ASTNode *baseType = nullptr;
  if (check(TokenKind::Colon)) {
    baseType = parseInheritanceClause();
    if (!baseType) {
      return nullptr; // Error already reported
    }
  }

  // Parse body
  if (!expect(TokenKind::LBrace)) {
    return nullptr;
  }

  // Create declaration node
  ast::ASTNode *decl;
  if (isClass) {
    auto *classDecl = ast::createClassDeclaration(startLoc, arena_);
    classDecl->setName(nameNode);
    if (baseType) {
      classDecl->setBase(baseType);
    }
    decl = classDecl;
  } else {
    auto *structDecl = ast::createStructDeclaration(startLoc, arena_);
    structDecl->setName(nameNode);
    decl = structDecl;
  }

  // Parse annotations first (if any) - they must appear at the top
  if (check(TokenKind::Quote)) {
    auto *annotationList = ast::createAnnotationList(startLoc, arena_);

    while (check(TokenKind::Quote)) {
      auto *annotation = static_cast<ast::AnnotationNode*>(parseAnnotationDeclaration());
      if (!annotation) {
        return nullptr; // Error already reported
      }
      annotationList->addAnnotation(annotation);
    }

    // Add annotation list to the declaration
    if (isClass) {
      static_cast<ast::ClassDeclarationNode*>(decl)->addAnnotation(annotationList);
    } else {
      static_cast<ast::StructDeclarationNode*>(decl)->addAnnotation(annotationList);
    }
  }

  // Parse regular members after annotations
  while (!check(TokenKind::RBrace) && !check(TokenKind::EoF)) {
    auto *member = parseStructOrClassMember();
    if (!member) {
      return nullptr; // Error already reported
    }

    // Add member to appropriate declaration type
    if (isClass) {
      static_cast<ast::ClassDeclarationNode*>(decl)->addMember(member);
    } else {
      static_cast<ast::StructDeclarationNode*>(decl)->addMember(member);
    }
  }

  if (!expect(TokenKind::RBrace)) {
    return nullptr;
  }

  // Wrap in generic declaration if needed
  if (!genericParams.empty()) {
    auto *genericDecl = ast::createGenericDeclaration(startLoc, arena_);

    // Move parameters to generic declaration
    for (auto *param : genericParams) {
      genericDecl->addParameter(param);
    }

    genericDecl->setDeclaration(decl);
    return genericDecl;
  }

  return decl;
}

ast::ASTNode *Parser::parseStructOrClassMember() {
  // Check for attributes
  ast::AttributeListNode *attributes = nullptr;
  if (check(TokenKind::At)) {
    attributes = static_cast<ast::AttributeListNode*>(parseAttributeList());
    if (!attributes) {
      return nullptr; // Error parsing attributes
    }
  }

  // Check for visibility modifier
  bool isPrivate = false;
  if (check(TokenKind::Priv)) {
    isPrivate = true;
    advance(); // consume 'priv'
  }

  // Check member type
  if (check(TokenKind::Func)) {
    // Parse method
    auto *method = parseFunctionDeclaration(false); // not extern
    if (!method) {
      return nullptr;
    }

    // Set visibility flags
    if (isPrivate) {
      method->flags &= ~flgPublic; // Remove public flag
    } else {
      method->flags |= flgPublic; // Set public flag (default)
    }

    // Attach attributes
    if (attributes) {
      for (auto *attr : attributes->attributes) {
        method->addAttribute(attr);
      }
    }

    return method;
  } else if (check(TokenKind::Ident)) {
    // Parse field
    auto *field = parseFieldDeclaration(isPrivate);
    if (!field) {
      return nullptr;
    }

    // Attach attributes
    if (attributes) {
      for (auto *attr : attributes->attributes) {
        field->addAttribute(attr);
      }
    }

    return field;
  } else {
    reportError(ParseError(ParseErrorType::UnexpectedToken, current().location,
                           "Expected member declaration (field or method)"));
    return nullptr;
  }
}

ast::ASTNode *Parser::parseFieldDeclaration(bool isPrivate) {
  Location startLoc = current().location;

  // Parse field name
  if (!check(TokenKind::Ident)) {
    reportError(ParseError(ParseErrorType::UnexpectedToken, current().location,
                           "Expected field name"));
    return nullptr;
  }

  Token nameToken = current();
  advance(); // consume name

  if (!nameToken.hasLiteralValue()) {
    reportError(ParseError(ParseErrorType::UnexpectedToken, nameToken.location,
                           "Field name token missing value"));
    return nullptr;
  }

  InternedString fieldName = nameToken.value.stringValue;
  auto *nameNode = ast::createIdentifier(fieldName, nameToken.location, arena_);

  // Create field declaration
  auto *fieldDecl = ast::createFieldDeclaration(startLoc, arena_);
  fieldDecl->setName(nameNode);

  // Set visibility flags
  if (isPrivate) {
    fieldDecl->flags &= ~flgPublic; // Remove public flag
  } else {
    fieldDecl->flags |= flgPublic; // Set public flag (default)
  }

  // Check for assignment first (inferred type)
  if (check(TokenKind::Assign)) {
    advance(); // consume '='

    auto *defaultValue = parseExpression();
    if (!defaultValue) {
      return nullptr;
    }
    fieldDecl->setDefaultValue(defaultValue);
  } else {
    // Type is required when no initializer
    auto *type = parseTypeExpression();
    if (!type) {
      return nullptr;
    }
    fieldDecl->setType(type);

    // Check for optional default value after type
    if (check(TokenKind::Assign)) {
      advance(); // consume '='

      auto *defaultValue = parseExpression();
      if (!defaultValue) {
        return nullptr;
      }
      fieldDecl->setDefaultValue(defaultValue);
    }
  }

  // Optional semicolon
  if (check(TokenKind::Semicolon)) {
    advance();
  }

  return fieldDecl;
}

ast::ASTNode *Parser::parseAnnotationDeclaration() {
  Location startLoc = current().location;

  // Consume backtick
  if (!expect(TokenKind::Quote)) {
    return nullptr;
  }

  // Parse annotation name
  if (!check(TokenKind::Ident)) {
    reportError(ParseError(ParseErrorType::UnexpectedToken, current().location,
                           "Expected annotation name"));
    return nullptr;
  }

  Token nameToken = current();
  advance(); // consume name

  if (!nameToken.hasLiteralValue()) {
    reportError(ParseError(ParseErrorType::UnexpectedToken, nameToken.location,
                           "Annotation name token missing value"));
    return nullptr;
  }

  InternedString annotationName = nameToken.value.stringValue;

  // Expect '='
  if (!expect(TokenKind::Assign)) {
    return nullptr;
  }

  // Parse value expression
  auto *value = parseExpression();
  if (!value) {
    return nullptr;
  }

  // Create annotation
  auto *annotation = ast::createAnnotation(annotationName, value, startLoc, arena_);

  return annotation;
}

ast::ASTNode *Parser::parseInheritanceClause() {
  // Consume ':'
  if (!expect(TokenKind::Colon)) {
    return nullptr;
  }

  // Parse base type expression
  auto *baseType = parseTypeExpression();
  if (!baseType) {
    reportError(ParseError(ParseErrorType::UnexpectedToken, current().location,
                           "Expected base type"));
    return nullptr;
  }

  return baseType;
}

} // namespace cxy
