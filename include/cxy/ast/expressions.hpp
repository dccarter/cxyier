#pragma once

#include "cxy/ast/node.hpp"
#include "cxy/token.hpp"
#include <format>

namespace cxy::ast {

/**
 * @brief Base class for all expression nodes.
 *
 * Expressions represent computations that produce values.
 * They form the core of the language's evaluation model.
 */
class ExpressionNode : public ASTNode {
public:
  explicit ExpressionNode(NodeKind k, Location loc, ArenaAllocator &arena)
      : ASTNode(k, loc, arena) {}
};

/**
 * @brief Unary expression node.
 *
 * Represents unary operations like `-x`, `!flag`, `++i`, `i++`, etc.
 * Can be either prefix or postfix operators.
 */
class UnaryExpressionNode : public ExpressionNode {
public:
  TokenKind op;     ///< Unary operator (tkMinus, tkBang, tkIncrement, etc.)
  bool isPrefix;    ///< true for prefix (++i), false for postfix (i++)
  ASTNode *operand; ///< Operand expression

  explicit UnaryExpressionNode(TokenKind operator_kind, bool is_prefix,
                               ASTNode *operand_expr, Location loc,
                               ArenaAllocator &arena)
      : ExpressionNode(astUnary, loc, arena), op(operator_kind),
        isPrefix(is_prefix), operand(operand_expr) {
    if (operand) {
      addChild(operand);
    }
  }

  std::format_context::iterator
  toString(std::format_context &ctx) const override {
    auto opStr = tokenKindToString(op);
    if (isPrefix) {
      return std::format_to(ctx.out(), "Unary({} {})", opStr,
                            operand ? std::format("{}", *operand) : "null");
    } else {
      return std::format_to(ctx.out(), "Unary({} {} [postfix])",
                            operand ? std::format("{}", *operand) : "null",
                            opStr);
    }
  }
};

/**
 * @brief Binary expression node.
 *
 * Represents binary operations like `a + b`, `x && y`, `arr[idx]`, etc.
 * Covers arithmetic, logical, comparison, and indexing operations.
 */
class BinaryExpressionNode : public ExpressionNode {
public:
  TokenKind op;   ///< Binary operator (tkPlus, tkAnd, tkEqual, etc.)
  ASTNode *left;  ///< Left operand expression
  ASTNode *right; ///< Right operand expression

  explicit BinaryExpressionNode(ASTNode *left_expr, TokenKind operator_kind,
                                ASTNode *right_expr, Location loc,
                                ArenaAllocator &arena)
      : ExpressionNode(astBinary, loc, arena), op(operator_kind),
        left(left_expr), right(right_expr) {
    if (left) {
      addChild(left);
    }
    if (right) {
      addChild(right);
    }
  }

  std::format_context::iterator
  toString(std::format_context &ctx) const override {
    return std::format_to(
        ctx.out(), "Binary({} {} {})", left ? std::format("{}", *left) : "null",
        tokenKindToString(op), right ? std::format("{}", *right) : "null");
  }
};

/**
 * @brief Ternary conditional expression node.
 *
 * Represents ternary conditional expressions like `condition ? then : else`.
 * The classic conditional operator found in C-style languages.
 */
class TernaryExpressionNode : public ExpressionNode {
public:
  ASTNode *condition; ///< Condition expression
  ASTNode *thenExpr;  ///< Expression when condition is true
  ASTNode *elseExpr;  ///< Expression when condition is false

  explicit TernaryExpressionNode(ASTNode *cond_expr, ASTNode *then_expr,
                                 ASTNode *else_expr, Location loc,
                                 ArenaAllocator &arena)
      : ExpressionNode(astTernary, loc, arena), condition(cond_expr),
        thenExpr(then_expr), elseExpr(else_expr) {
    if (condition) {
      addChild(condition);
    }
    if (thenExpr) {
      addChild(thenExpr);
    }
    if (elseExpr) {
      addChild(elseExpr);
    }
  }

  std::format_context::iterator
  toString(std::format_context &ctx) const override {
    return std::format_to(ctx.out(), "Ternary({} ? {} : {})",
                          condition ? std::format("{}", *condition) : "null",
                          thenExpr ? std::format("{}", *thenExpr) : "null",
                          elseExpr ? std::format("{}", *elseExpr) : "null");
  }
};

/**
 * @brief Assignment expression node.
 *
 * Represents assignment operations like `x = y`, `a += b`, `ptr *= 2`, etc.
 * Covers simple assignment and compound assignment operators.
 */
class AssignmentExpressionNode : public ExpressionNode {
public:
  TokenKind op;    ///< Assignment operator (tkAssign, tkPlusAssign, etc.)
  ASTNode *target; ///< Target lvalue expression
  ASTNode *value;  ///< Value expression to assign

  explicit AssignmentExpressionNode(ASTNode *target_expr,
                                    TokenKind operator_kind,
                                    ASTNode *value_expr, Location loc,
                                    ArenaAllocator &arena)
      : ExpressionNode(astAssignment, loc, arena), op(operator_kind),
        target(target_expr), value(value_expr) {
    if (target) {
      addChild(target);
    }
    if (value) {
      addChild(value);
    }
  }

  std::format_context::iterator
  toString(std::format_context &ctx) const override {
    return std::format_to(ctx.out(), "Assignment({} {} {})",
                          target ? std::format("{}", *target) : "null",
                          tokenKindToString(op),
                          value ? std::format("{}", *value) : "null");
  }
};

// Helper functions for creating expression nodes

/**
 * @brief Create a unary expression node.
 */
inline UnaryExpressionNode *createUnaryExpr(TokenKind op, bool isPrefix,
                                            ASTNode *operand, Location loc,
                                            ArenaAllocator &arena) {
  return arena.construct<UnaryExpressionNode>(op, isPrefix, operand, loc,
                                              arena);
}

/**
 * @brief Create a binary expression node.
 */
inline BinaryExpressionNode *createBinaryExpr(ASTNode *left, TokenKind op,
                                              ASTNode *right, Location loc,
                                              ArenaAllocator &arena) {
  return arena.construct<BinaryExpressionNode>(left, op, right, loc, arena);
}

/**
 * @brief Create a ternary expression node.
 */
inline TernaryExpressionNode *createTernaryExpr(ASTNode *condition,
                                                ASTNode *thenExpr,
                                                ASTNode *elseExpr, Location loc,
                                                ArenaAllocator &arena) {
  return arena.construct<TernaryExpressionNode>(condition, thenExpr, elseExpr,
                                                loc, arena);
}

/**
 * @brief Create an assignment expression node.
 */
inline AssignmentExpressionNode *
createAssignmentExpr(ASTNode *target, TokenKind op, ASTNode *value,
                     Location loc, ArenaAllocator &arena) {
  return arena.construct<AssignmentExpressionNode>(target, op, value, loc,
                                                   arena);
}

/**
 * @brief Grouped/parenthesized expression node.
 *
 * Represents parenthesized expressions like `(a + b)`.
 * Important for preserving precedence and evaluation order.
 */
class GroupExpressionNode : public ExpressionNode {
public:
  ASTNode *expr; ///< The grouped expression

  explicit GroupExpressionNode(ASTNode *expression, Location loc,
                               ArenaAllocator &arena)
      : ExpressionNode(astGroup, loc, arena), expr(expression) {
    if (expr) {
      addChild(expr);
    }
  }

  std::format_context::iterator
  toString(std::format_context &ctx) const override {
    return std::format_to(ctx.out(), "Group({})",
                          expr ? std::format("{}", *expr) : "null");
  }
};

/**
 * @brief Statement expression node.
 *
 * Represents statement expressions like `({ statements... })`.
 * Allows statements to be used in expression contexts.
 */
class StmtExpressionNode : public ExpressionNode {
public:
  ASTNode *stmt; ///< The statement block

  explicit StmtExpressionNode(ASTNode *statement, Location loc,
                              ArenaAllocator &arena)
      : ExpressionNode(astStmt, loc, arena), stmt(statement) {
    if (stmt) {
      addChild(stmt);
    }
  }

  std::format_context::iterator
  toString(std::format_context &ctx) const override {
    return std::format_to(ctx.out(), "StmtExpr({})",
                          stmt ? std::format("{}", *stmt) : "null");
  }
};

/**
 * @brief String interpolation expression node.
 *
 * Represents string interpolation like `"Hello ${name}!"`.
 * Contains mix of string parts and embedded expressions.
 */
class StringExpressionNode : public ExpressionNode {
public:
  ArenaVector<ASTNode *> parts; ///< String parts and expressions

  explicit StringExpressionNode(Location loc, ArenaAllocator &arena)
      : ExpressionNode(astStringExpr, loc, arena),
        parts(ArenaSTLAllocator<ASTNode *>(arena)) {}

  void addPart(ASTNode *part) {
    if (part) {
      parts.push_back(part);
      addChild(part);
    }
  }

  std::format_context::iterator
  toString(std::format_context &ctx) const override {
    auto out = std::format_to(ctx.out(), "StringExpr(");
    for (size_t i = 0; i < parts.size(); ++i) {
      if (i > 0)
        out = std::format_to(out, ", ");
      out = std::format_to(out, "{}",
                           parts[i] ? std::format("{}", *parts[i]) : "null");
    }
    return std::format_to(out, ")");
  }
};

/**
 * @brief Cast expression node.
 *
 * Represents type casts like `(int)value` or `value as Type`.
 * Supports both C-style and explicit cast syntax.
 */
class CastExpressionNode : public ExpressionNode {
public:
  ASTNode *expr;     ///< Expression being cast
  ASTNode *typeExpr; ///< Target type expression

  explicit CastExpressionNode(ASTNode *expression, ASTNode *target_type,
                              Location loc, ArenaAllocator &arena)
      : ExpressionNode(astCast, loc, arena), expr(expression),
        typeExpr(target_type) {
    if (expr) {
      addChild(expr);
    }
    if (typeExpr) {
      addChild(typeExpr);
    }
  }

  std::format_context::iterator
  toString(std::format_context &ctx) const override {
    return std::format_to(ctx.out(), "Cast({} as {})",
                          expr ? std::format("{}", *expr) : "null",
                          typeExpr ? std::format("{}", *typeExpr) : "null");
  }
};

/**
 * @brief Function call expression node.
 *
 * Represents function calls like `func(arg1, arg2)`.
 * Supports both regular and method calls.
 */
class CallExpressionNode : public ExpressionNode {
public:
  ASTNode *callee;                  ///< Function being called
  ArenaVector<ASTNode *> arguments; ///< Call arguments

  explicit CallExpressionNode(ASTNode *function, Location loc,
                              ArenaAllocator &arena)
      : ExpressionNode(astCall, loc, arena), callee(function),
        arguments(ArenaSTLAllocator<ASTNode *>(arena)) {
    if (callee) {
      addChild(callee);
    }
  }

  void addArgument(ASTNode *arg) {
    if (arg) {
      arguments.push_back(arg);
      addChild(arg);
    }
  }

  std::format_context::iterator
  toString(std::format_context &ctx) const override {
    auto out = std::format_to(ctx.out(), "Call({}, [",
                              callee ? std::format("{}", *callee) : "null");
    for (size_t i = 0; i < arguments.size(); ++i) {
      if (i > 0)
        out = std::format_to(out, ", ");
      out = std::format_to(
          out, "{}", arguments[i] ? std::format("{}", *arguments[i]) : "null");
    }
    return std::format_to(out, "])");
  }
};

/**
 * @brief Array/index access expression node.
 *
 * Represents array indexing like `arr[index]` or `map[key]`.
 * Used for subscript operations.
 */
class IndexExpressionNode : public ExpressionNode {
public:
  ASTNode *object; ///< Object being indexed
  ASTNode *index;  ///< Index expression

  explicit IndexExpressionNode(ASTNode *obj, ASTNode *idx, Location loc,
                               ArenaAllocator &arena)
      : ExpressionNode(astIndex, loc, arena), object(obj), index(idx) {
    if (object) {
      addChild(object);
    }
    if (index) {
      addChild(index);
    }
  }

  std::format_context::iterator
  toString(std::format_context &ctx) const override {
    return std::format_to(ctx.out(), "Index({}[{}])",
                          object ? std::format("{}", *object) : "null",
                          index ? std::format("{}", *index) : "null");
  }
};

/**
 * @brief Array literal expression node.
 *
 * Represents array literals like `[1, 2, 3]` or `[expr1, expr2]`.
 * Creates arrays from a list of expressions.
 */
class ArrayExpressionNode : public ExpressionNode {
public:
  ArenaVector<ASTNode *> elements; ///< Array elements

  explicit ArrayExpressionNode(Location loc, ArenaAllocator &arena)
      : ExpressionNode(astArray, loc, arena),
        elements(ArenaSTLAllocator<ASTNode *>(arena)) {}

  void addElement(ASTNode *element) {
    if (element) {
      elements.push_back(element);
      addChild(element);
    }
  }

  std::format_context::iterator
  toString(std::format_context &ctx) const override {
    auto out = std::format_to(ctx.out(), "Array([");
    for (size_t i = 0; i < elements.size(); ++i) {
      if (i > 0)
        out = std::format_to(out, ", ");
      out = std::format_to(
          out, "{}", elements[i] ? std::format("{}", *elements[i]) : "null");
    }
    return std::format_to(out, "])");
  }
};

/**
 * @brief Tuple expression node.
 *
 * Represents tuple literals like `(a, b, c)`.
 * Fixed-size heterogeneous collections.
 */
class TupleExpressionNode : public ExpressionNode {
public:
  ArenaVector<ASTNode *> elements; ///< Tuple elements

  explicit TupleExpressionNode(Location loc, ArenaAllocator &arena)
      : ExpressionNode(astTuple, loc, arena),
        elements(ArenaSTLAllocator<ASTNode *>(arena)) {}

  void addElement(ASTNode *element) {
    if (element) {
      elements.push_back(element);
      addChild(element);
    }
  }

  std::format_context::iterator
  toString(std::format_context &ctx) const override {
    auto out = std::format_to(ctx.out(), "Tuple((");
    for (size_t i = 0; i < elements.size(); ++i) {
      if (i > 0)
        out = std::format_to(out, ", ");
      out = std::format_to(
          out, "{}", elements[i] ? std::format("{}", *elements[i]) : "null");
    }
    return std::format_to(out, "))");
  }
};

/**
 * @brief Struct literal expression node.
 *
 * Represents struct literals like `Point { x: 1, y: 2 }`.
 * Named field initialization.
 */
class StructExpressionNode : public ExpressionNode {
public:
  struct Field {
    std::string name;
    ASTNode *value;
  };

  ASTNode *typeExpr;         ///< Optional type expression
  ArenaVector<Field> fields; ///< Named fields

  explicit StructExpressionNode(ASTNode *type, Location loc,
                                ArenaAllocator &arena)
      : ExpressionNode(astStruct, loc, arena), typeExpr(type),
        fields(ArenaSTLAllocator<Field>(arena)) {
    if (typeExpr) {
      addChild(typeExpr);
    }
  }

  void addField(const std::string &name, ASTNode *value) {
    fields.push_back({name, value});
    if (value) {
      addChild(value);
    }
  }

  std::format_context::iterator
  toString(std::format_context &ctx) const override {
    auto out = std::format_to(ctx.out(), "Struct({} {{ ",
                              typeExpr ? std::format("{}", *typeExpr) : "");
    for (size_t i = 0; i < fields.size(); ++i) {
      if (i > 0)
        out = std::format_to(out, ", ");
      out = std::format_to(out, "{}: {}", fields[i].name,
                           fields[i].value ? std::format("{}", *fields[i].value)
                                           : "null");
    }
    return std::format_to(out, " }})");
  }
};

/**
 * @brief Member access expression node.
 *
 * Represents member access like `obj.field` or `ptr->member`.
 * Used for accessing struct/object members.
 */
class MemberExpressionNode : public ExpressionNode {
public:
  ASTNode *object;    ///< Object being accessed
  std::string member; ///< Member name
  bool isArrow;       ///< true for ->, false for .

  explicit MemberExpressionNode(ASTNode *obj, const std::string &member_name,
                                bool arrow, Location loc, ArenaAllocator &arena)
      : ExpressionNode(astMember, loc, arena), object(obj), member(member_name),
        isArrow(arrow) {
    if (object) {
      addChild(object);
    }
  }

  std::format_context::iterator
  toString(std::format_context &ctx) const override {
    return std::format_to(ctx.out(), "Member({}{}{})",
                          object ? std::format("{}", *object) : "null",
                          isArrow ? "->" : ".", member);
  }
};

/**
 * @brief Macro call expression node.
 *
 * Represents macro invocations like `debug!("msg")` or `vec![1, 2, 3]`.
 * Distinguished from regular function calls.
 */
class MacroCallExpressionNode : public ExpressionNode {
public:
  std::string macroName;            ///< Macro name
  ArenaVector<ASTNode *> arguments; ///< Macro arguments

  explicit MacroCallExpressionNode(const std::string &name, Location loc,
                                   ArenaAllocator &arena)
      : ExpressionNode(astMacroCall, loc, arena), macroName(name),
        arguments(ArenaSTLAllocator<ASTNode *>(arena)) {}

  void addArgument(ASTNode *arg) {
    if (arg) {
      arguments.push_back(arg);
      addChild(arg);
    }
  }

  std::format_context::iterator
  toString(std::format_context &ctx) const override {
    auto out = std::format_to(ctx.out(), "MacroCall({}![", macroName);
    for (size_t i = 0; i < arguments.size(); ++i) {
      if (i > 0)
        out = std::format_to(out, ", ");
      out = std::format_to(
          out, "{}", arguments[i] ? std::format("{}", *arguments[i]) : "null");
    }
    return std::format_to(out, "])");
  }
};

/**
 * @brief Closure expression node.
 *
 * Represents closures/lambdas like `|x, y| x + y` or `|| 42`.
 * Anonymous function expressions with capture semantics.
 */
class ClosureExpressionNode : public ExpressionNode {
public:
  ArenaVector<ASTNode *> parameters; ///< Parameter declarations
  ASTNode *body;                     ///< Closure body expression/block

  explicit ClosureExpressionNode(ASTNode *closure_body, Location loc,
                                 ArenaAllocator &arena)
      : ExpressionNode(astClosure, loc, arena),
        parameters(ArenaSTLAllocator<ASTNode *>(arena)), body(closure_body) {
    if (body) {
      addChild(body);
    }
  }

  void addParameter(ASTNode *param) {
    if (param) {
      parameters.push_back(param);
      addChild(param);
    }
  }

  std::format_context::iterator
  toString(std::format_context &ctx) const override {
    auto out = std::format_to(ctx.out(), "Closure(|");
    for (size_t i = 0; i < parameters.size(); ++i) {
      if (i > 0)
        out = std::format_to(out, ", ");
      out = std::format_to(out, "{}",
                           parameters[i] ? std::format("{}", *parameters[i])
                                         : "null");
    }
    return std::format_to(out, "| {})",
                          body ? std::format("{}", *body) : "null");
  }
};

/**
 * @brief Range expression node.
 *
 * Represents range expressions like `1..10`, `start..=end`, or `..`.
 * Used for iteration and slice operations.
 */
class RangeExpressionNode : public ExpressionNode {
public:
  ASTNode *start;   ///< Start expression (nullable)
  ASTNode *end;     ///< End expression (nullable)
  bool isInclusive; ///< true for ..=, false for ..

  explicit RangeExpressionNode(ASTNode *start_expr, ASTNode *end_expr,
                               bool inclusive, Location loc,
                               ArenaAllocator &arena)
      : ExpressionNode(astRange, loc, arena), start(start_expr), end(end_expr),
        isInclusive(inclusive) {
    if (start) {
      addChild(start);
    }
    if (end) {
      addChild(end);
    }
  }

  std::format_context::iterator
  toString(std::format_context &ctx) const override {
    std::string op = isInclusive ? "..=" : "..";
    return std::format_to(ctx.out(), "Range({}{}{})",
                          start ? std::format("{}", *start) : "", op,
                          end ? std::format("{}", *end) : "");
  }
};

/**
 * @brief Spread expression node.
 *
 * Represents spread/splat operations like `...array` or `*args`.
 * Used in function calls and collection literals.
 */
class SpreadExpressionNode : public ExpressionNode {
public:
  ASTNode *expr; ///< Expression being spread

  explicit SpreadExpressionNode(ASTNode *expression, Location loc,
                                ArenaAllocator &arena)
      : ExpressionNode(astSpread, loc, arena), expr(expression) {
    if (expr) {
      addChild(expr);
    }
  }

  std::format_context::iterator
  toString(std::format_context &ctx) const override {
    return std::format_to(ctx.out(), "Spread(...{})",
                          expr ? std::format("{}", *expr) : "null");
  }
};

// Additional helper functions for creating expression nodes

/**
 * @brief Create a grouped expression node.
 */
inline GroupExpressionNode *createGroupExpr(ASTNode *expr, Location loc,
                                            ArenaAllocator &arena) {
  return arena.construct<GroupExpressionNode>(expr, loc, arena);
}

/**
 * @brief Create a statement expression node.
 */
inline StmtExpressionNode *createStmtExpr(ASTNode *stmt, Location loc,
                                          ArenaAllocator &arena) {
  return arena.construct<StmtExpressionNode>(stmt, loc, arena);
}

/**
 * @brief Create a string interpolation expression node.
 */
inline StringExpressionNode *createStringExpr(Location loc,
                                              ArenaAllocator &arena) {
  return arena.construct<StringExpressionNode>(loc, arena);
}

/**
 * @brief Create a cast expression node.
 */
inline CastExpressionNode *createCastExpr(ASTNode *expr, ASTNode *type,
                                          Location loc, ArenaAllocator &arena) {
  return arena.construct<CastExpressionNode>(expr, type, loc, arena);
}

/**
 * @brief Create a call expression node.
 */
inline CallExpressionNode *createCallExpr(ASTNode *callee, Location loc,
                                          ArenaAllocator &arena) {
  return arena.construct<CallExpressionNode>(callee, loc, arena);
}

/**
 * @brief Create an index expression node.
 */
inline IndexExpressionNode *createIndexExpr(ASTNode *object, ASTNode *index,
                                            Location loc,
                                            ArenaAllocator &arena) {
  return arena.construct<IndexExpressionNode>(object, index, loc, arena);
}

/**
 * @brief Create an array expression node.
 */
inline ArrayExpressionNode *createArrayExpr(Location loc,
                                            ArenaAllocator &arena) {
  return arena.construct<ArrayExpressionNode>(loc, arena);
}

/**
 * @brief Create a tuple expression node.
 */
inline TupleExpressionNode *createTupleExpr(Location loc,
                                            ArenaAllocator &arena) {
  return arena.construct<TupleExpressionNode>(loc, arena);
}

/**
 * @brief Create a struct expression node.
 */
inline StructExpressionNode *createStructExpr(ASTNode *type, Location loc,
                                              ArenaAllocator &arena) {
  return arena.construct<StructExpressionNode>(type, loc, arena);
}

/**
 * @brief Create a member expression node.
 */
inline MemberExpressionNode *createMemberExpr(ASTNode *object,
                                              const std::string &member,
                                              bool isArrow, Location loc,
                                              ArenaAllocator &arena) {
  return arena.construct<MemberExpressionNode>(object, member, isArrow, loc,
                                               arena);
}

/**
 * @brief Create a macro call expression node.
 */
inline MacroCallExpressionNode *createMacroCallExpr(const std::string &name,
                                                    Location loc,
                                                    ArenaAllocator &arena) {
  return arena.construct<MacroCallExpressionNode>(name, loc, arena);
}

/**
 * @brief Create a closure expression node.
 */
inline ClosureExpressionNode *createClosureExpr(ASTNode *body, Location loc,
                                                ArenaAllocator &arena) {
  return arena.construct<ClosureExpressionNode>(body, loc, arena);
}

/**
 * @brief Create a range expression node.
 */
inline RangeExpressionNode *createRangeExpr(ASTNode *start, ASTNode *end,
                                            bool inclusive, Location loc,
                                            ArenaAllocator &arena) {
  return arena.construct<RangeExpressionNode>(start, end, inclusive, loc,
                                              arena);
}

/**
 * @brief Create a spread expression node.
 */
inline SpreadExpressionNode *createSpreadExpr(ASTNode *expr, Location loc,
                                              ArenaAllocator &arena) {
  return arena.construct<SpreadExpressionNode>(expr, loc, arena);
}

} // namespace cxy::ast
