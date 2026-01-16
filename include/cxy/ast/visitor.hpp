#pragma once

#include "cxy/ast/kind.hpp"
#include "cxy/ast/node.hpp"

namespace cxy::ast {

// Forward declarations for all node types
class LiteralNode;
class BoolLiteralNode;
class IntLiteralNode;
class FloatLiteralNode;
class StringLiteralNode;
class CharLiteralNode;
class NullLiteralNode;

class IdentifierNode;
class QualifiedPathNode;

class ExpressionNode;
class UnaryExpressionNode;
class BinaryExpressionNode;
class TernaryExpressionNode;
class AssignmentExpressionNode;
class GroupExpressionNode;
class StmtExpressionNode;
class StringExpressionNode;
class CastExpressionNode;
class CallExpressionNode;
class IndexExpressionNode;
class PrimitiveTypeNode;
class ArrayExpressionNode;
class TupleExpressionNode;
class FieldExpressionNode;
class StructExpressionNode;
class MemberExpressionNode;
class MacroCallExpressionNode;
class ClosureExpressionNode;
class RangeExpressionNode;
class SpreadExpressionNode;

// Statement node forward declarations
class StatementNode;
class ExpressionStatementNode;
class BreakStatementNode;
class ContinueStatementNode;
class DeferStatementNode;
class ReturnStatementNode;
class YieldStatementNode;
class BlockStatementNode;
class IfStatementNode;
class ForStatementNode;
class WhileStatementNode;
class SwitchStatementNode;
class MatchStatementNode;
class CaseStatementNode;
class MatchCaseNode;

// Declaration node forward declarations
class DeclarationNode;
class VariableDeclarationNode;

/**
 * @brief Base visitor class for AST traversal.
 *
 * Implements the visitor pattern for AST nodes. Provides both generic
 * visit() methods and specific visit methods for each node type.
 * Supports both pre-order and post-order traversal.
 */
class ASTVisitor {
public:
  virtual ~ASTVisitor() = default;

  // Main entry point for visiting nodes
  virtual void visit(ASTNode *node);
  virtual void visit(const ASTNode *node);

  // Generic node visit methods (called for all nodes)
  virtual bool visitNode(ASTNode *node) { return true; }
  virtual bool visitNode(const ASTNode *node) { return true; }
  virtual void visitNodePost(ASTNode *node) {}
  virtual void visitNodePost(const ASTNode *node) {}

  // Special/Utility node visits
  virtual bool visitNoop(ASTNode *node) { return visitNode(node); }
  virtual void visitNoopPost(ASTNode *node) { visitNodePost(node); }

  // Literal node visits
  virtual bool visitBool(BoolLiteralNode *node);
  virtual void visitBoolPost(BoolLiteralNode *node);

  virtual bool visitInt(IntLiteralNode *node);
  virtual void visitIntPost(IntLiteralNode *node);

  virtual bool visitFloat(FloatLiteralNode *node);
  virtual void visitFloatPost(FloatLiteralNode *node);

  virtual bool visitString(StringLiteralNode *node);
  virtual void visitStringPost(StringLiteralNode *node);

  virtual bool visitChar(CharLiteralNode *node);
  virtual void visitCharPost(CharLiteralNode *node);

  virtual bool visitNull(NullLiteralNode *node);
  virtual void visitNullPost(NullLiteralNode *node);

  // Identifier node visits
  virtual bool visitIdentifier(IdentifierNode *node);
  virtual void visitIdentifierPost(IdentifierNode *node);

  virtual bool visitQualifiedPath(QualifiedPathNode *node);
  virtual void visitQualifiedPathPost(QualifiedPathNode *node);

  virtual bool visitPrimitiveType(PrimitiveTypeNode *node);
  virtual void visitPrimitiveTypePost(PrimitiveTypeNode *node);

  // Expression node visits
  virtual bool visitUnary(UnaryExpressionNode *node);
  virtual void visitUnaryPost(UnaryExpressionNode *node);

  virtual bool visitBinary(BinaryExpressionNode *node);
  virtual void visitBinaryPost(BinaryExpressionNode *node);

  virtual bool visitTernary(TernaryExpressionNode *node);
  virtual void visitTernaryPost(TernaryExpressionNode *node);

  virtual bool visitAssignment(AssignmentExpressionNode *node);
  virtual void visitAssignmentPost(AssignmentExpressionNode *node);

  virtual bool visitGroup(GroupExpressionNode *node);
  virtual void visitGroupPost(GroupExpressionNode *node);

  virtual bool visitStmt(StmtExpressionNode *node);
  virtual void visitStmtPost(StmtExpressionNode *node);

  virtual bool visitStringExpr(StringExpressionNode *node);
  virtual void visitStringExprPost(StringExpressionNode *node);

  virtual bool visitCast(CastExpressionNode *node);
  virtual void visitCastPost(CastExpressionNode *node);

  virtual bool visitCall(CallExpressionNode *node);
  virtual void visitCallPost(CallExpressionNode *node);

  virtual bool visitIndex(IndexExpressionNode *node);
  virtual void visitIndexPost(IndexExpressionNode *node);

  virtual bool visitArray(ArrayExpressionNode *node);
  virtual void visitArrayPost(ArrayExpressionNode *node);

  virtual bool visitTuple(TupleExpressionNode *node);
  virtual void visitTuplePost(TupleExpressionNode *node);

  virtual bool visitField(FieldExpressionNode *node);
  virtual void visitFieldPost(FieldExpressionNode *node);

  virtual bool visitStruct(StructExpressionNode *node);
  virtual void visitStructPost(StructExpressionNode *node);

  virtual bool visitMember(MemberExpressionNode *node);
  virtual void visitMemberPost(MemberExpressionNode *node);

  virtual bool visitMacroCall(MacroCallExpressionNode *node);
  virtual void visitMacroCallPost(MacroCallExpressionNode *node);

  virtual bool visitClosure(ClosureExpressionNode *node);
  virtual void visitClosurePost(ClosureExpressionNode *node);

  virtual bool visitRange(RangeExpressionNode *node);
  virtual void visitRangePost(RangeExpressionNode *node);

  virtual bool visitSpread(SpreadExpressionNode *node);
  virtual void visitSpreadPost(SpreadExpressionNode *node);

  // Statement node visits
  virtual bool visitExprStmt(ExpressionStatementNode *node);
  virtual void visitExprStmtPost(ExpressionStatementNode *node);

  virtual bool visitBreakStmt(BreakStatementNode *node);
  virtual void visitBreakStmtPost(BreakStatementNode *node);

  virtual bool visitContinueStmt(ContinueStatementNode *node);
  virtual void visitContinueStmtPost(ContinueStatementNode *node);

  virtual bool visitDeferStmt(DeferStatementNode *node);
  virtual void visitDeferStmtPost(DeferStatementNode *node);

  virtual bool visitReturnStmt(ReturnStatementNode *node);
  virtual void visitReturnStmtPost(ReturnStatementNode *node);

  virtual bool visitYieldStmt(YieldStatementNode *node);
  virtual void visitYieldStmtPost(YieldStatementNode *node);

  virtual bool visitBlockStmt(BlockStatementNode *node);
  virtual void visitBlockStmtPost(BlockStatementNode *node);

  virtual bool visitIfStmt(IfStatementNode *node);
  virtual void visitIfStmtPost(IfStatementNode *node);

  virtual bool visitForStmt(ForStatementNode *node);
  virtual void visitForStmtPost(ForStatementNode *node);

  virtual bool visitWhileStmt(WhileStatementNode *node);
  virtual void visitWhileStmtPost(WhileStatementNode *node);

  virtual bool visitSwitchStmt(SwitchStatementNode *node);
  virtual void visitSwitchStmtPost(SwitchStatementNode *node);

  virtual bool visitMatchStmt(MatchStatementNode *node);
  virtual void visitMatchStmtPost(MatchStatementNode *node);

  virtual bool visitCaseStmt(CaseStatementNode *node);
  virtual void visitCaseStmtPost(CaseStatementNode *node);

  virtual bool visitMatchCase(MatchCaseNode *node);
  virtual void visitMatchCasePost(MatchCaseNode *node);

  // Declaration node visits
  virtual bool visitVariableDeclaration(VariableDeclarationNode *node);
  virtual void visitVariableDeclarationPost(VariableDeclarationNode *node);

private:
  // Internal dispatch method
  bool dispatchVisit(ASTNode *node);
  void dispatchVisitPost(ASTNode *node);
};

/**
 * @brief Const-only visitor for read-only AST traversal.
 *
 * Similar to ASTVisitor but only provides const methods.
 * Useful for analysis passes that don't modify the AST.
 */
class ConstASTVisitor {
public:
  virtual ~ConstASTVisitor() = default;

  // Main entry point for visiting nodes
  virtual void visit(const ASTNode *node);

  // Generic node visit methods
  virtual bool visitNode(const ASTNode *node) { return true; }
  virtual void visitNodePost(const ASTNode *node) {}

  // Special/Utility node visits
  virtual bool visitNoop(const ASTNode *node) { return visitNode(node); }
  virtual void visitNoopPost(const ASTNode *node) { visitNodePost(node); }

  // Literal node visits
  virtual bool visitBool(const BoolLiteralNode *node);
  virtual void visitBoolPost(const BoolLiteralNode *node);

  virtual bool visitInt(const IntLiteralNode *node);
  virtual void visitIntPost(const IntLiteralNode *node);

  virtual bool visitFloat(const FloatLiteralNode *node);
  virtual void visitFloatPost(const FloatLiteralNode *node);

  virtual bool visitString(const StringLiteralNode *node);
  virtual void visitStringPost(const StringLiteralNode *node);

  virtual bool visitChar(const CharLiteralNode *node);
  virtual void visitCharPost(const CharLiteralNode *node);

  virtual bool visitNull(const NullLiteralNode *node);
  virtual void visitNullPost(const NullLiteralNode *node);

  // Identifier node visits
  virtual bool visitIdentifier(const IdentifierNode *node);
  virtual void visitIdentifierPost(const IdentifierNode *node);

  virtual bool visitQualifiedPath(const QualifiedPathNode *node);
  virtual void visitQualifiedPathPost(const QualifiedPathNode *node);

  virtual bool visitPrimitiveType(const PrimitiveTypeNode *node);
  virtual void visitPrimitiveTypePost(const PrimitiveTypeNode *node);

  // Expression node visits
  virtual bool visitUnary(const UnaryExpressionNode *node);
  virtual void visitUnaryPost(const UnaryExpressionNode *node);

  virtual bool visitBinary(const BinaryExpressionNode *node);
  virtual void visitBinaryPost(const BinaryExpressionNode *node);

  virtual bool visitTernary(const TernaryExpressionNode *node);
  virtual void visitTernaryPost(const TernaryExpressionNode *node);

  virtual bool visitAssignment(const AssignmentExpressionNode *node);
  virtual void visitAssignmentPost(const AssignmentExpressionNode *node);

  virtual bool visitGroup(const GroupExpressionNode *node);
  virtual void visitGroupPost(const GroupExpressionNode *node);

  virtual bool visitStmt(const StmtExpressionNode *node);
  virtual void visitStmtPost(const StmtExpressionNode *node);

  virtual bool visitStringExpr(const StringExpressionNode *node);
  virtual void visitStringExprPost(const StringExpressionNode *node);

  virtual bool visitCast(const CastExpressionNode *node);
  virtual void visitCastPost(const CastExpressionNode *node);

  virtual bool visitCall(const CallExpressionNode *node);
  virtual void visitCallPost(const CallExpressionNode *node);

  virtual bool visitIndex(const IndexExpressionNode *node);
  virtual void visitIndexPost(const IndexExpressionNode *node);

  virtual bool visitArray(const ArrayExpressionNode *node);
  virtual void visitArrayPost(const ArrayExpressionNode *node);

  virtual bool visitTuple(const TupleExpressionNode *node);
  virtual void visitTuplePost(const TupleExpressionNode *node);

  virtual bool visitField(const FieldExpressionNode *node);
  virtual void visitFieldPost(const FieldExpressionNode *node);

  virtual bool visitStruct(const StructExpressionNode *node);
  virtual void visitStructPost(const StructExpressionNode *node);

  virtual bool visitMember(const MemberExpressionNode *node);
  virtual void visitMemberPost(const MemberExpressionNode *node);

  virtual bool visitMacroCall(const MacroCallExpressionNode *node);
  virtual void visitMacroCallPost(const MacroCallExpressionNode *node);

  virtual bool visitClosure(const ClosureExpressionNode *node);
  virtual void visitClosurePost(const ClosureExpressionNode *node);

  virtual bool visitRange(const RangeExpressionNode *node);
  virtual void visitRangePost(const RangeExpressionNode *node);

  virtual bool visitSpread(const SpreadExpressionNode *node);
  virtual void visitSpreadPost(const SpreadExpressionNode *node);

  // Statement node visits
  virtual bool visitExprStmt(const ExpressionStatementNode *node);
  virtual void visitExprStmtPost(const ExpressionStatementNode *node);

  virtual bool visitBreakStmt(const BreakStatementNode *node);
  virtual void visitBreakStmtPost(const BreakStatementNode *node);

  virtual bool visitContinueStmt(const ContinueStatementNode *node);
  virtual void visitContinueStmtPost(const ContinueStatementNode *node);

  virtual bool visitDeferStmt(const DeferStatementNode *node);
  virtual void visitDeferStmtPost(const DeferStatementNode *node);

  virtual bool visitReturnStmt(const ReturnStatementNode *node);
  virtual void visitReturnStmtPost(const ReturnStatementNode *node);

  virtual bool visitYieldStmt(const YieldStatementNode *node);
  virtual void visitYieldStmtPost(const YieldStatementNode *node);

  virtual bool visitBlockStmt(const BlockStatementNode *node);
  virtual void visitBlockStmtPost(const BlockStatementNode *node);

  virtual bool visitIfStmt(const IfStatementNode *node);
  virtual void visitIfStmtPost(const IfStatementNode *node);

  virtual bool visitForStmt(const ForStatementNode *node);
  virtual void visitForStmtPost(const ForStatementNode *node);

  virtual bool visitWhileStmt(const WhileStatementNode *node);
  virtual void visitWhileStmtPost(const WhileStatementNode *node);

  virtual bool visitSwitchStmt(const SwitchStatementNode *node);
  virtual void visitSwitchStmtPost(const SwitchStatementNode *node);

  virtual bool visitMatchStmt(const MatchStatementNode *node);
  virtual void visitMatchStmtPost(const MatchStatementNode *node);

  virtual bool visitCaseStmt(const CaseStatementNode *node);
  virtual void visitCaseStmtPost(const CaseStatementNode *node);

  virtual bool visitMatchCase(const MatchCaseNode *node);
  virtual void visitMatchCasePost(const MatchCaseNode *node);

  // Declaration node visits
  virtual bool visitVariableDeclaration(const VariableDeclarationNode *node);
  virtual void visitVariableDeclarationPost(const VariableDeclarationNode *node);

protected:
  // Internal dispatch method
  bool dispatchVisit(const ASTNode *node);
  void dispatchVisitPost(const ASTNode *node);
};

/**
 * @brief Simple function-based visitor for quick AST traversal.
 *
 * Takes a function that will be called for each node during traversal.
 * Useful for simple operations that don't require a full visitor class.
 *
 * @param root Root node to start traversal from
 * @param fn Function to call for each node (return false to skip children)
 */
template <typename Func> void walkAST(ASTNode *root, Func &&fn) {
  if (!root)
    return;

  if (fn(root)) {
    for (ASTNode *child : root->children) {
      walkAST(child, fn);
    }
  }
}

/**
 * @brief Const version of simple function-based visitor.
 */
template <typename Func> void walkAST(const ASTNode *root, Func &&fn) {
  if (!root)
    return;

  if (fn(root)) {
    for (const ASTNode *child : root->children) {
      walkAST(child, fn);
    }
  }
}

/**
 * @brief Collect all nodes of a specific type from the AST.
 *
 * @tparam T Node type to collect
 * @param root Root node to start search from
 * @param arena Arena for result vector allocation
 * @return Vector containing all nodes of type T
 */
template <typename T>
ArenaVector<T *> collectNodes(ASTNode *root, ArenaAllocator &arena) {
  ArenaVector<T *> result{ArenaSTLAllocator<T *>(arena)};

  walkAST(root, [&result](ASTNode *node) {
    if (auto *typed = dynamic_cast<T *>(node)) {
      result.push_back(typed);
    }
    return true;
  });

  return result;
}

/**
 * @brief Find the first node of a specific type in the AST.
 *
 * @tparam T Node type to find
 * @param root Root node to start search from
 * @return First node of type T, or nullptr if not found
 */
template <typename T> T *findNode(ASTNode *root) {
  T *result = nullptr;

  walkAST(root, [&result](ASTNode *node) {
    if (auto *typed = dynamic_cast<T *>(node)) {
      result = typed;
      return false; // Stop traversal
    }
    return true;
  });

  return result;
}

} // namespace cxy::ast
