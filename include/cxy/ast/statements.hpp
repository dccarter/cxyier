#pragma once

#include "cxy/ast/node.hpp"
#include "cxy/token.hpp"
#include <format>

namespace cxy::ast {

/**
 * @brief Base class for all statement nodes.
 *
 * Statements represent actions or control flow constructs.
 * They typically do not produce values (unlike expressions).
 */
class StatementNode : public ASTNode {
public:
  explicit StatementNode(NodeKind k, Location loc, ArenaAllocator &arena)
      : ASTNode(k, loc, arena) {}
};

/**
 * @brief Expression statement node.
 *
 * Represents an expression used as a statement (e.g., function calls, assignments).
 * The expression is evaluated but its value is discarded.
 */
class ExpressionStatementNode : public StatementNode {
public:
  ASTNode *expression; ///< The expression to evaluate

  explicit ExpressionStatementNode(ASTNode *expr, Location loc, ArenaAllocator &arena)
      : StatementNode(astExprStmt, loc, arena), expression(expr) {
    if (expression) {
      addChild(expression);
    }
  }

  std::format_context::iterator toString(std::format_context &ctx) const override {
    return std::format_to(ctx.out(), "ExprStmt({})", 
                         expression ? std::format("{}", *expression) : "null");
  }
};

/**
 * @brief Break statement node.
 *
 * Represents a break statement that exits from loops or switch statements.
 */
class BreakStatementNode : public StatementNode {
public:
  explicit BreakStatementNode(Location loc, ArenaAllocator &arena)
      : StatementNode(astBreakStmt, loc, arena) {}

  std::format_context::iterator toString(std::format_context &ctx) const override {
    return std::format_to(ctx.out(), "BreakStmt");
  }
};

/**
 * @brief Continue statement node.
 *
 * Represents a continue statement that skips to the next iteration of a loop.
 */
class ContinueStatementNode : public StatementNode {
public:
  explicit ContinueStatementNode(Location loc, ArenaAllocator &arena)
      : StatementNode(astContinueStmt, loc, arena) {}

  std::format_context::iterator toString(std::format_context &ctx) const override {
    return std::format_to(ctx.out(), "ContinueStmt");
  }
};

/**
 * @brief Defer statement node.
 *
 * Represents a defer statement that executes code when the current scope exits.
 * The deferred statement/block is executed in LIFO order.
 */
class DeferStatementNode : public StatementNode {
public:
  ASTNode *statement; ///< Statement or block to defer

  explicit DeferStatementNode(ASTNode *stmt, Location loc, ArenaAllocator &arena)
      : StatementNode(astDeferStmt, loc, arena), statement(stmt) {
    if (statement) {
      addChild(statement);
    }
  }

  std::format_context::iterator toString(std::format_context &ctx) const override {
    return std::format_to(ctx.out(), "DeferStmt({})", 
                         statement ? std::format("{}", *statement) : "null");
  }
};

/**
 * @brief Return statement node.
 *
 * Represents a return statement that exits from a function.
 * May include an optional expression for the return value.
 */
class ReturnStatementNode : public StatementNode {
public:
  ASTNode *expression = nullptr; ///< Optional return value expression

  explicit ReturnStatementNode(Location loc, ArenaAllocator &arena, ASTNode *expr = nullptr)
      : StatementNode(astReturnStmt, loc, arena), expression(expr) {
    if (expression) {
      addChild(expression);
    }
  }

  std::format_context::iterator toString(std::format_context &ctx) const override {
    if (expression) {
      return std::format_to(ctx.out(), "ReturnStmt({})", *expression);
    }
    return std::format_to(ctx.out(), "ReturnStmt");
  }
};

/**
 * @brief Yield statement node.
 *
 * Represents a yield statement for generators/coroutines.
 * Suspends execution and produces a value.
 */
class YieldStatementNode : public StatementNode {
public:
  ASTNode *expression = nullptr; ///< Optional yielded value expression

  explicit YieldStatementNode(Location loc, ArenaAllocator &arena, ASTNode *expr = nullptr)
      : StatementNode(astYieldStmt, loc, arena), expression(expr) {
    if (expression) {
      addChild(expression);
    }
  }

  std::format_context::iterator toString(std::format_context &ctx) const override {
    if (expression) {
      return std::format_to(ctx.out(), "YieldStmt({})", *expression);
    }
    return std::format_to(ctx.out(), "YieldStmt");
  }
};

/**
 * @brief Block statement node.
 *
 * Represents a block of statements enclosed in braces.
 * Creates a new scope for variable declarations and control flow.
 */
class BlockStatementNode : public StatementNode {
public:
  ArenaVector<ASTNode *> statements; ///< List of statements in the block

  explicit BlockStatementNode(Location loc, ArenaAllocator &arena)
      : StatementNode(astBlockStmt, loc, arena), 
        statements(ArenaSTLAllocator<ASTNode *>(arena)) {}

  void addStatement(ASTNode *stmt) {
    if (stmt) {
      statements.push_back(stmt);
      addChild(stmt);
    }
  }

  std::format_context::iterator toString(std::format_context &ctx) const override {
    auto it = std::format_to(ctx.out(), "BlockStmt(");
    for (size_t i = 0; i < statements.size(); ++i) {
      if (i > 0) it = std::format_to(it, ", ");
      it = std::format_to(it, "{}", *statements[i]);
    }
    return std::format_to(it, ")");
  }
};

/**
 * @brief If statement node.
 *
 * Represents an if statement with optional else clause.
 * Supports both single statements and block statements.
 */
class IfStatementNode : public StatementNode {
public:
  ASTNode *condition;     ///< Boolean condition expression
  ASTNode *thenStatement; ///< Statement to execute if condition is true
  ASTNode *elseStatement = nullptr; ///< Optional else statement/block

  explicit IfStatementNode(ASTNode *cond, ASTNode *thenStmt, Location loc, 
                          ArenaAllocator &arena, ASTNode *elseStmt = nullptr)
      : StatementNode(astIfStmt, loc, arena), condition(cond), 
        thenStatement(thenStmt), elseStatement(elseStmt) {
    if (condition) addChild(condition);
    if (thenStatement) addChild(thenStatement);
    if (elseStatement) addChild(elseStatement);
  }

  std::format_context::iterator toString(std::format_context &ctx) const override {
    auto it = std::format_to(ctx.out(), "IfStmt({}, {}",
                            condition ? std::format("{}", *condition) : "null",
                            thenStatement ? std::format("{}", *thenStatement) : "null");
    if (elseStatement) {
      it = std::format_to(it, ", {}", *elseStatement);
    }
    return std::format_to(it, ")");
  }
};

/**
 * @brief For statement node.
 *
 * Represents Cxy for loops with syntax: `for a, b in pairs {}` or `for item in collection, item.isValid {}`
 * Components: variable declarations, range expression, optional condition, and body.
 */
class ForStatementNode : public StatementNode {
public:
  ArenaVector<ASTNode *> variables; ///< Variable declarations (list for destructuring)
  ASTNode *range;                   ///< Range expression to iterate over
  ASTNode *condition = nullptr;     ///< Optional condition filter
  ASTNode *body;                    ///< Loop body statement/block

  explicit ForStatementNode(ASTNode *rangeExpr, ASTNode *bodyStmt, 
                           Location loc, ArenaAllocator &arena, ASTNode *cond = nullptr)
      : StatementNode(astForStmt, loc, arena), 
        variables(ArenaSTLAllocator<ASTNode *>(arena)),
        range(rangeExpr), condition(cond), body(bodyStmt) {
    if (range) addChild(range);
    if (condition) addChild(condition);
    if (body) addChild(body);
  }

  void addVariable(ASTNode *var) {
    if (var) {
      variables.push_back(var);
      addChild(var);
    }
  }

  std::format_context::iterator toString(std::format_context &ctx) const override {
    auto it = std::format_to(ctx.out(), "ForStmt([");
    for (size_t i = 0; i < variables.size(); ++i) {
      if (i > 0) it = std::format_to(it, ", ");
      it = std::format_to(it, "{}", variables[i] ? std::format("{}", *variables[i]) : "null");
    }
    it = std::format_to(it, "] in {}",
                       range ? std::format("{}", *range) : "null");
    
    if (condition) {
      it = std::format_to(it, ", {}", *condition);
    }
    
    return std::format_to(it, ", {})", body ? std::format("{}", *body) : "null");
  }
};

/**
 * @brief While statement node.
 *
 * Represents a while loop with a condition and body.
 */
class WhileStatementNode : public StatementNode {
public:
  ASTNode *condition; ///< Loop condition expression
  ASTNode *body;      ///< Loop body statement/block

  explicit WhileStatementNode(ASTNode *cond, ASTNode *bodyStmt, Location loc, ArenaAllocator &arena)
      : StatementNode(astWhileStmt, loc, arena), condition(cond), body(bodyStmt) {
    if (condition) addChild(condition);
    if (body) addChild(body);
  }

  std::format_context::iterator toString(std::format_context &ctx) const override {
    return std::format_to(ctx.out(), "WhileStmt({}, {})",
                         condition ? std::format("{}", *condition) : "null",
                         body ? std::format("{}", *body) : "null");
  }
};

/**
 * @brief Switch statement node.
 *
 * Represents a switch statement with a discriminant and case list.
 */
class SwitchStatementNode : public StatementNode {
public:
  ASTNode *discriminant;              ///< Expression to switch on
  ArenaVector<ASTNode *> cases;       ///< List of case statements

  explicit SwitchStatementNode(ASTNode *disc, Location loc, ArenaAllocator &arena)
      : StatementNode(astSwitchStmt, loc, arena), discriminant(disc),
        cases(ArenaSTLAllocator<ASTNode *>(arena)) {
    if (discriminant) addChild(discriminant);
  }

  void addCase(ASTNode *caseStmt) {
    if (caseStmt) {
      cases.push_back(caseStmt);
      addChild(caseStmt);
    }
  }

  std::format_context::iterator toString(std::format_context &ctx) const override {
    auto it = std::format_to(ctx.out(), "SwitchStmt({}, [",
                            discriminant ? std::format("{}", *discriminant) : "null");
    for (size_t i = 0; i < cases.size(); ++i) {
      if (i > 0) it = std::format_to(it, ", ");
      it = std::format_to(it, "{}", *cases[i]);
    }
    return std::format_to(it, "])");
  }
};

/**
 * @brief Match statement node.
 *
 * Represents a pattern matching statement (more powerful than switch).
 * Similar to Rust's match or ML's case expressions.
 */
class MatchStatementNode : public StatementNode {
public:
  ASTNode *discriminant;              ///< Expression to match against
  ArenaVector<ASTNode *> patterns;    ///< List of pattern match arms

  explicit MatchStatementNode(ASTNode *disc, Location loc, ArenaAllocator &arena)
      : StatementNode(astMatchStmt, loc, arena), discriminant(disc),
        patterns(ArenaSTLAllocator<ASTNode *>(arena)) {
    if (discriminant) addChild(discriminant);
  }

  void addPattern(ASTNode *pattern) {
    if (pattern) {
      patterns.push_back(pattern);
      addChild(pattern);
    }
  }

  std::format_context::iterator toString(std::format_context &ctx) const override {
    auto it = std::format_to(ctx.out(), "MatchStmt({}, [",
                            discriminant ? std::format("{}", *discriminant) : "null");
    for (size_t i = 0; i < patterns.size(); ++i) {
      if (i > 0) it = std::format_to(it, ", ");
      it = std::format_to(it, "{}", *patterns[i]);
    }
    return std::format_to(it, "])");
  }
};

/**
 * @brief Case statement node.
 *
 * Represents a single case in a switch statement.
 * Includes the case value(s) and the associated statements.
 */
class CaseStatementNode : public StatementNode {
public:
  ArenaVector<ASTNode *> values;      ///< Case values (empty for default case)
  ArenaVector<ASTNode *> statements;  ///< Statements to execute for this case
  bool isDefault = false;             ///< true if this is the default case

  explicit CaseStatementNode(Location loc, ArenaAllocator &arena, bool defaultCase = false)
      : StatementNode(astCaseStmt, loc, arena), isDefault(defaultCase),
        values(ArenaSTLAllocator<ASTNode *>(arena)),
        statements(ArenaSTLAllocator<ASTNode *>(arena)) {}

  void addValue(ASTNode *value) {
    if (value && !isDefault) {
      values.push_back(value);
      addChild(value);
    }
  }

  void addStatement(ASTNode *stmt) {
    if (stmt) {
      statements.push_back(stmt);
      addChild(stmt);
    }
  }

  std::format_context::iterator toString(std::format_context &ctx) const override {
    auto it = std::format_to(ctx.out(), "CaseStmt(");
    
    if (isDefault) {
      it = std::format_to(it, "default");
    } else {
      it = std::format_to(it, "[");
      for (size_t i = 0; i < values.size(); ++i) {
        if (i > 0) it = std::format_to(it, ", ");
        it = std::format_to(it, "{}", *values[i]);
      }
      it = std::format_to(it, "]");
    }
    
    it = std::format_to(it, ", [");
    for (size_t i = 0; i < statements.size(); ++i) {
      if (i > 0) it = std::format_to(it, ", ");
      it = std::format_to(it, "{}", *statements[i]);
    }
    return std::format_to(it, "])");
  }
};

// Statement creation helpers
inline ExpressionStatementNode *createExprStatement(ASTNode *expr, Location loc, ArenaAllocator &arena) {
  return arena.construct<ExpressionStatementNode>(expr, loc, arena);
}

inline BreakStatementNode *createBreakStatement(Location loc, ArenaAllocator &arena) {
  return arena.construct<BreakStatementNode>(loc, arena);
}

inline ContinueStatementNode *createContinueStatement(Location loc, ArenaAllocator &arena) {
  return arena.construct<ContinueStatementNode>(loc, arena);
}

inline DeferStatementNode *createDeferStatement(ASTNode *stmt, Location loc, ArenaAllocator &arena) {
  return arena.construct<DeferStatementNode>(stmt, loc, arena);
}

inline ReturnStatementNode *createReturnStatement(Location loc, ArenaAllocator &arena, ASTNode *expr = nullptr) {
  return arena.construct<ReturnStatementNode>(loc, arena, expr);
}

inline YieldStatementNode *createYieldStatement(Location loc, ArenaAllocator &arena, ASTNode *expr = nullptr) {
  return arena.construct<YieldStatementNode>(loc, arena, expr);
}

inline BlockStatementNode *createBlockStatement(Location loc, ArenaAllocator &arena) {
  return arena.construct<BlockStatementNode>(loc, arena);
}

inline IfStatementNode *createIfStatement(ASTNode *condition, ASTNode *thenStmt, Location loc, 
                                         ArenaAllocator &arena, ASTNode *elseStmt = nullptr) {
  return arena.construct<IfStatementNode>(condition, thenStmt, loc, arena, elseStmt);
}

inline ForStatementNode *createForStatement(ASTNode *range, ASTNode *body, 
                                           Location loc, ArenaAllocator &arena, ASTNode *condition = nullptr) {
  return arena.construct<ForStatementNode>(range, body, loc, arena, condition);
}

inline WhileStatementNode *createWhileStatement(ASTNode *condition, ASTNode *body, Location loc, ArenaAllocator &arena) {
  return arena.construct<WhileStatementNode>(condition, body, loc, arena);
}

inline SwitchStatementNode *createSwitchStatement(ASTNode *discriminant, Location loc, ArenaAllocator &arena) {
  return arena.construct<SwitchStatementNode>(discriminant, loc, arena);
}

inline MatchStatementNode *createMatchStatement(ASTNode *discriminant, Location loc, ArenaAllocator &arena) {
  return arena.construct<MatchStatementNode>(discriminant, loc, arena);
}

inline CaseStatementNode *createCaseStatement(Location loc, ArenaAllocator &arena, bool isDefault = false) {
  return arena.construct<CaseStatementNode>(loc, arena, isDefault);
}

} // namespace cxy::ast