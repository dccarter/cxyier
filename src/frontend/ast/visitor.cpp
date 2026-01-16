#include "cxy/ast/visitor.hpp"
#include "cxy/ast/attributes.hpp"
#include "cxy/ast/expressions.hpp"
#include "cxy/ast/identifiers.hpp"
#include "cxy/ast/literals.hpp"
#include "cxy/ast/statements.hpp"
#include "cxy/ast/types.hpp"

namespace cxy::ast {

// ASTVisitor implementation

void ASTVisitor::visit(ASTNode *node) {
  if (!node)
    return;

  if (dispatchVisit(node)) {
    // Visit children if the node visit returned true
    for (ASTNode *child : node->children) {
      visit(child);
    }
  }

  dispatchVisitPost(node);
}

void ASTVisitor::visit(const ASTNode *node) {
  if (!node)
    return;

  // For const nodes, we dispatch to const visitor methods
  // but we need to cast away const for the dispatch mechanism
  if (dispatchVisit(const_cast<ASTNode *>(node))) {
    for (const ASTNode *child : node->children) {
      visit(child);
    }
  }

  dispatchVisitPost(const_cast<ASTNode *>(node));
}

bool ASTVisitor::dispatchVisit(ASTNode *node) {
  switch (node->kind) {
  case astNoop:
    return visitNoop(node);

  // Literals
  case astBool:
    return visitBool(static_cast<BoolLiteralNode *>(node));
  case astInt:
    return visitInt(static_cast<IntLiteralNode *>(node));
  case astFloat:
    return visitFloat(static_cast<FloatLiteralNode *>(node));
  case astString:
    return visitString(static_cast<StringLiteralNode *>(node));
  case astChar:
    return visitChar(static_cast<CharLiteralNode *>(node));
  case astNull:
    return visitNull(static_cast<NullLiteralNode *>(node));

  // Identifiers
  case astIdentifier:
    return visitIdentifier(static_cast<IdentifierNode *>(node));
  case astQualifiedPath:
    return visitQualifiedPath(static_cast<QualifiedPathNode *>(node));
  case astPrimitiveType:
    return visitPrimitiveType(static_cast<PrimitiveTypeNode *>(node));

  // Expressions
  case astUnaryExpr:
    return visitUnary(static_cast<UnaryExpressionNode *>(node));
  case astBinaryExpr:
    return visitBinary(static_cast<BinaryExpressionNode *>(node));
  case astTernaryExpr:
    return visitTernary(static_cast<TernaryExpressionNode *>(node));
  case astAssignmentExpr:
    return visitAssignment(static_cast<AssignmentExpressionNode *>(node));
  case astGroupExpr:
    return visitGroup(static_cast<GroupExpressionNode *>(node));
  case astStmtExpr:
    return visitStmt(static_cast<StmtExpressionNode *>(node));
  case astStringExpr:
    return visitStringExpr(static_cast<StringExpressionNode *>(node));
  case astCastExpr:
    return visitCast(static_cast<CastExpressionNode *>(node));
  case astCallExpr:
    return visitCall(static_cast<CallExpressionNode *>(node));
  case astIndexExpr:
    return visitIndex(static_cast<IndexExpressionNode *>(node));
  case astArrayExpr:
    return visitArray(static_cast<ArrayExpressionNode *>(node));
  case astTupleExpr:
    return visitTuple(static_cast<TupleExpressionNode *>(node));
  case astFieldExpr:
    return visitField(static_cast<FieldExpressionNode *>(node));
  case astStructExpr:
    return visitStruct(static_cast<StructExpressionNode *>(node));
  case astMemberExpr:
    return visitMember(static_cast<MemberExpressionNode *>(node));
  case astMacroCallExpr:
    return visitMacroCall(static_cast<MacroCallExpressionNode *>(node));
  case astClosureExpr:
    return visitClosure(static_cast<ClosureExpressionNode *>(node));
  case astRangeExpr:
    return visitRange(static_cast<RangeExpressionNode *>(node));
  case astSpreadExpr:
    return visitSpread(static_cast<SpreadExpressionNode *>(node));

  // Statements
  case astExprStmt:
    return visitExprStmt(static_cast<ExpressionStatementNode *>(node));
  case astBreakStmt:
    return visitBreakStmt(static_cast<BreakStatementNode *>(node));
  case astContinueStmt:
    return visitContinueStmt(static_cast<ContinueStatementNode *>(node));
  case astDeferStmt:
    return visitDeferStmt(static_cast<DeferStatementNode *>(node));
  case astReturnStmt:
    return visitReturnStmt(static_cast<ReturnStatementNode *>(node));
  case astYieldStmt:
    return visitYieldStmt(static_cast<YieldStatementNode *>(node));
  case astBlockStmt:
    return visitBlockStmt(static_cast<BlockStatementNode *>(node));
  case astIfStmt:
    return visitIfStmt(static_cast<IfStatementNode *>(node));
  case astForStmt:
    return visitForStmt(static_cast<ForStatementNode *>(node));
  case astWhileStmt:
    return visitWhileStmt(static_cast<WhileStatementNode *>(node));
  case astSwitchStmt:
    return visitSwitchStmt(static_cast<SwitchStatementNode *>(node));
  case astMatchStmt:
    return visitMatchStmt(static_cast<MatchStatementNode *>(node));
  case astCaseStmt:
    return visitCaseStmt(static_cast<CaseStatementNode *>(node));

  default:
    // Fallback to generic visitNode for unknown types
    return visitNode(node);
  }
}

void ASTVisitor::dispatchVisitPost(ASTNode *node) {
  switch (node->kind) {
  case astNoop:
    visitNoopPost(node);
    break;

  // Literals
  case astBool:
    visitBoolPost(static_cast<BoolLiteralNode *>(node));
    break;
  case astInt:
    visitIntPost(static_cast<IntLiteralNode *>(node));
    break;
  case astFloat:
    visitFloatPost(static_cast<FloatLiteralNode *>(node));
    break;
  case astString:
    visitStringPost(static_cast<StringLiteralNode *>(node));
    break;
  case astChar:
    visitCharPost(static_cast<CharLiteralNode *>(node));
    break;
  case astNull:
    visitNullPost(static_cast<NullLiteralNode *>(node));
    break;

  // Identifiers
  case astIdentifier:
    visitIdentifierPost(static_cast<IdentifierNode *>(node));
    break;
  case astQualifiedPath:
    visitQualifiedPathPost(static_cast<QualifiedPathNode *>(node));
    break;
  case astPrimitiveType:
    visitPrimitiveTypePost(static_cast<PrimitiveTypeNode *>(node));
    break;

  // Expressions
  case astUnaryExpr:
    visitUnaryPost(static_cast<UnaryExpressionNode *>(node));
    break;
  case astBinaryExpr:
    visitBinaryPost(static_cast<BinaryExpressionNode *>(node));
    break;
  case astTernaryExpr:
    visitTernaryPost(static_cast<TernaryExpressionNode *>(node));
    break;
  case astAssignmentExpr:
    visitAssignmentPost(static_cast<AssignmentExpressionNode *>(node));
    break;
  case astGroupExpr:
    visitGroupPost(static_cast<GroupExpressionNode *>(node));
    break;
  case astStmtExpr:
    visitStmtPost(static_cast<StmtExpressionNode *>(node));
    break;
  case astStringExpr:
    visitStringExprPost(static_cast<StringExpressionNode *>(node));
    break;
  case astCastExpr:
    visitCastPost(static_cast<CastExpressionNode *>(node));
    break;
  case astCallExpr:
    visitCallPost(static_cast<CallExpressionNode *>(node));
    break;
  case astIndexExpr:
    visitIndexPost(static_cast<IndexExpressionNode *>(node));
    break;
  case astArrayExpr:
    visitArrayPost(static_cast<ArrayExpressionNode *>(node));
    break;
  case astTupleExpr:
    visitTuplePost(static_cast<TupleExpressionNode *>(node));
    break;
  case astFieldExpr:
    visitFieldPost(static_cast<FieldExpressionNode *>(node));
    break;
  case astStructExpr:
    visitStructPost(static_cast<StructExpressionNode *>(node));
    break;
  case astMemberExpr:
    visitMemberPost(static_cast<MemberExpressionNode *>(node));
    break;
  case astMacroCallExpr:
    visitMacroCallPost(static_cast<MacroCallExpressionNode *>(node));
    break;
  case astClosureExpr:
    visitClosurePost(static_cast<ClosureExpressionNode *>(node));
    break;
  case astRangeExpr:
    visitRangePost(static_cast<RangeExpressionNode *>(node));
    break;
  case astSpreadExpr:
    visitSpreadPost(static_cast<SpreadExpressionNode *>(node));
    break;

  // Statements
  case astExprStmt:
    visitExprStmtPost(static_cast<ExpressionStatementNode *>(node));
    break;
  case astBreakStmt:
    visitBreakStmtPost(static_cast<BreakStatementNode *>(node));
    break;
  case astContinueStmt:
    visitContinueStmtPost(static_cast<ContinueStatementNode *>(node));
    break;
  case astDeferStmt:
    visitDeferStmtPost(static_cast<DeferStatementNode *>(node));
    break;
  case astReturnStmt:
    visitReturnStmtPost(static_cast<ReturnStatementNode *>(node));
    break;
  case astYieldStmt:
    visitYieldStmtPost(static_cast<YieldStatementNode *>(node));
    break;
  case astBlockStmt:
    visitBlockStmtPost(static_cast<BlockStatementNode *>(node));
    break;
  case astIfStmt:
    visitIfStmtPost(static_cast<IfStatementNode *>(node));
    break;
  case astForStmt:
    visitForStmtPost(static_cast<ForStatementNode *>(node));
    break;
  case astWhileStmt:
    visitWhileStmtPost(static_cast<WhileStatementNode *>(node));
    break;
  case astSwitchStmt:
    visitSwitchStmtPost(static_cast<SwitchStatementNode *>(node));
    break;
  case astMatchStmt:
    visitMatchStmtPost(static_cast<MatchStatementNode *>(node));
    break;
  case astCaseStmt:
    visitCaseStmtPost(static_cast<CaseStatementNode *>(node));
    break;

  default:
    // Fallback to generic visitNodePost for unknown types
    visitNodePost(node);
    break;
  }
}

// ConstASTVisitor implementation

void ConstASTVisitor::visit(const ASTNode *node) {
  if (!node)
    return;

  if (dispatchVisit(node)) {
    // Visit children if the node visit returned true
    for (const ASTNode *child : node->children) {
      visit(child);
    }
  }

  dispatchVisitPost(node);
}

bool ConstASTVisitor::dispatchVisit(const ASTNode *node) {
  switch (node->kind) {
  case astNoop:
    return visitNoop(node);

  // Literals
  case astBool:
    return visitBool(static_cast<const BoolLiteralNode *>(node));
  case astInt:
    return visitInt(static_cast<const IntLiteralNode *>(node));
  case astFloat:
    return visitFloat(static_cast<const FloatLiteralNode *>(node));
  case astString:
    return visitString(static_cast<const StringLiteralNode *>(node));
  case astChar:
    return visitChar(static_cast<const CharLiteralNode *>(node));
  case astNull:
    return visitNull(static_cast<const NullLiteralNode *>(node));

  // Identifiers
  case astIdentifier:
    return visitIdentifier(static_cast<const IdentifierNode *>(node));
  case astQualifiedPath:
    return visitQualifiedPath(static_cast<const QualifiedPathNode *>(node));
  case astPrimitiveType:
    return visitPrimitiveType(static_cast<const PrimitiveTypeNode *>(node));

  // Expressions
  case astUnaryExpr:
    return visitUnary(static_cast<const UnaryExpressionNode *>(node));
  case astBinaryExpr:
    return visitBinary(static_cast<const BinaryExpressionNode *>(node));
  case astTernaryExpr:
    return visitTernary(static_cast<const TernaryExpressionNode *>(node));
  case astAssignmentExpr:
    return visitAssignment(static_cast<const AssignmentExpressionNode *>(node));
  case astGroupExpr:
    return visitGroup(static_cast<const GroupExpressionNode *>(node));
  case astStmtExpr:
    return visitStmt(static_cast<const StmtExpressionNode *>(node));
  case astStringExpr:
    return visitStringExpr(static_cast<const StringExpressionNode *>(node));
  case astCastExpr:
    return visitCast(static_cast<const CastExpressionNode *>(node));
  case astCallExpr:
    return visitCall(static_cast<const CallExpressionNode *>(node));
  case astIndexExpr:
    return visitIndex(static_cast<const IndexExpressionNode *>(node));
  case astArrayExpr:
    return visitArray(static_cast<const ArrayExpressionNode *>(node));
  case astTupleExpr:
    return visitTuple(static_cast<const TupleExpressionNode *>(node));
  case astFieldExpr:
    return visitField(static_cast<const FieldExpressionNode *>(node));
  case astStructExpr:
    return visitStruct(static_cast<const StructExpressionNode *>(node));
  case astMemberExpr:
    return visitMember(static_cast<const MemberExpressionNode *>(node));
  case astMacroCallExpr:
    return visitMacroCall(static_cast<const MacroCallExpressionNode *>(node));
  case astClosureExpr:
    return visitClosure(static_cast<const ClosureExpressionNode *>(node));
  case astRangeExpr:
    return visitRange(static_cast<const RangeExpressionNode *>(node));
  case astSpreadExpr:
    return visitSpread(static_cast<const SpreadExpressionNode *>(node));

  // Statements
  case astExprStmt:
    return visitExprStmt(static_cast<const ExpressionStatementNode *>(node));
  case astBreakStmt:
    return visitBreakStmt(static_cast<const BreakStatementNode *>(node));
  case astContinueStmt:
    return visitContinueStmt(static_cast<const ContinueStatementNode *>(node));
  case astDeferStmt:
    return visitDeferStmt(static_cast<const DeferStatementNode *>(node));
  case astReturnStmt:
    return visitReturnStmt(static_cast<const ReturnStatementNode *>(node));
  case astYieldStmt:
    return visitYieldStmt(static_cast<const YieldStatementNode *>(node));
  case astBlockStmt:
    return visitBlockStmt(static_cast<const BlockStatementNode *>(node));
  case astIfStmt:
    return visitIfStmt(static_cast<const IfStatementNode *>(node));
  case astForStmt:
    return visitForStmt(static_cast<const ForStatementNode *>(node));
  case astWhileStmt:
    return visitWhileStmt(static_cast<const WhileStatementNode *>(node));
  case astSwitchStmt:
    return visitSwitchStmt(static_cast<const SwitchStatementNode *>(node));
  case astMatchStmt:
    return visitMatchStmt(static_cast<const MatchStatementNode *>(node));
  case astCaseStmt:
    return visitCaseStmt(static_cast<const CaseStatementNode *>(node));

  default:
    // Fallback to generic visitNode for unknown types
    return visitNode(node);
  }
}

void ConstASTVisitor::dispatchVisitPost(const ASTNode *node) {
  switch (node->kind) {
  case astNoop:
    visitNoopPost(node);
    break;

  // Literals
  case astBool:
    visitBoolPost(static_cast<const BoolLiteralNode *>(node));
    break;
  case astInt:
    visitIntPost(static_cast<const IntLiteralNode *>(node));
    break;
  case astFloat:
    visitFloatPost(static_cast<const FloatLiteralNode *>(node));
    break;
  case astString:
    visitStringPost(static_cast<const StringLiteralNode *>(node));
    break;
  case astChar:
    visitCharPost(static_cast<const CharLiteralNode *>(node));
    break;
  case astNull:
    visitNullPost(static_cast<const NullLiteralNode *>(node));
    break;

  // Identifiers
  case astIdentifier:
    visitIdentifierPost(static_cast<const IdentifierNode *>(node));
    break;
  case astQualifiedPath:
    visitQualifiedPathPost(static_cast<const QualifiedPathNode *>(node));
    break;
  case astPrimitiveType:
    visitPrimitiveTypePost(static_cast<const PrimitiveTypeNode *>(node));
    break;

  // Expressions
  case astUnaryExpr:
    visitUnaryPost(static_cast<const UnaryExpressionNode *>(node));
    break;
  case astBinaryExpr:
    visitBinaryPost(static_cast<const BinaryExpressionNode *>(node));
    break;
  case astTernaryExpr:
    visitTernaryPost(static_cast<const TernaryExpressionNode *>(node));
    break;
  case astAssignmentExpr:
    visitAssignmentPost(static_cast<const AssignmentExpressionNode *>(node));
    break;
  case astGroupExpr:
    visitGroupPost(static_cast<const GroupExpressionNode *>(node));
    break;
  case astStmtExpr:
    visitStmtPost(static_cast<const StmtExpressionNode *>(node));
    break;
  case astStringExpr:
    visitStringExprPost(static_cast<const StringExpressionNode *>(node));
    break;
  case astCastExpr:
    visitCastPost(static_cast<const CastExpressionNode *>(node));
    break;
  case astCallExpr:
    visitCallPost(static_cast<const CallExpressionNode *>(node));
    break;
  case astIndexExpr:
    visitIndexPost(static_cast<const IndexExpressionNode *>(node));
    break;
  case astArrayExpr:
    visitArrayPost(static_cast<const ArrayExpressionNode *>(node));
    break;
  case astTupleExpr:
    visitTuplePost(static_cast<const TupleExpressionNode *>(node));
    break;
  case astFieldExpr:
    visitFieldPost(static_cast<const FieldExpressionNode *>(node));
    break;
  case astStructExpr:
    visitStructPost(static_cast<const StructExpressionNode *>(node));
    break;
  case astMemberExpr:
    visitMemberPost(static_cast<const MemberExpressionNode *>(node));
    break;
  case astMacroCallExpr:
    visitMacroCallPost(static_cast<const MacroCallExpressionNode *>(node));
    break;
  case astClosureExpr:
    visitClosurePost(static_cast<const ClosureExpressionNode *>(node));
    break;
  case astRangeExpr:
    visitRangePost(static_cast<const RangeExpressionNode *>(node));
    break;
  case astSpreadExpr:
    visitSpreadPost(static_cast<const SpreadExpressionNode *>(node));
    break;

  // Statements
  case astExprStmt:
    visitExprStmtPost(static_cast<const ExpressionStatementNode *>(node));
    break;
  case astBreakStmt:
    visitBreakStmtPost(static_cast<const BreakStatementNode *>(node));
    break;
  case astContinueStmt:
    visitContinueStmtPost(static_cast<const ContinueStatementNode *>(node));
    break;
  case astDeferStmt:
    visitDeferStmtPost(static_cast<const DeferStatementNode *>(node));
    break;
  case astReturnStmt:
    visitReturnStmtPost(static_cast<const ReturnStatementNode *>(node));
    break;
  case astYieldStmt:
    visitYieldStmtPost(static_cast<const YieldStatementNode *>(node));
    break;
  case astBlockStmt:
    visitBlockStmtPost(static_cast<const BlockStatementNode *>(node));
    break;
  case astIfStmt:
    visitIfStmtPost(static_cast<const IfStatementNode *>(node));
    break;
  case astForStmt:
    visitForStmtPost(static_cast<const ForStatementNode *>(node));
    break;
  case astWhileStmt:
    visitWhileStmtPost(static_cast<const WhileStatementNode *>(node));
    break;
  case astSwitchStmt:
    visitSwitchStmtPost(static_cast<const SwitchStatementNode *>(node));
    break;
  case astMatchStmt:
    visitMatchStmtPost(static_cast<const MatchStatementNode *>(node));
    break;
  case astCaseStmt:
    visitCaseStmtPost(static_cast<const CaseStatementNode *>(node));
    break;

  default:
    // Fallback to generic visitNodePost for unknown types
    visitNodePost(node);
    break;
  }
}

// Default implementations for ASTVisitor

bool ASTVisitor::visitBool(BoolLiteralNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitBoolPost(BoolLiteralNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitInt(IntLiteralNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitIntPost(IntLiteralNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitFloat(FloatLiteralNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitFloatPost(FloatLiteralNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitString(StringLiteralNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitStringPost(StringLiteralNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitChar(CharLiteralNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitCharPost(CharLiteralNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitNull(NullLiteralNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitNullPost(NullLiteralNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitIdentifier(IdentifierNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitIdentifierPost(IdentifierNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitQualifiedPath(QualifiedPathNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitQualifiedPathPost(QualifiedPathNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitPrimitiveType(PrimitiveTypeNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitPrimitiveTypePost(PrimitiveTypeNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitUnary(UnaryExpressionNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitUnaryPost(UnaryExpressionNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitBinary(BinaryExpressionNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitBinaryPost(BinaryExpressionNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitTernary(TernaryExpressionNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitTernaryPost(TernaryExpressionNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitAssignment(AssignmentExpressionNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitAssignmentPost(AssignmentExpressionNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitGroup(GroupExpressionNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitGroupPost(GroupExpressionNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitStmt(StmtExpressionNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitStmtPost(StmtExpressionNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitStringExpr(StringExpressionNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitStringExprPost(StringExpressionNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitCast(CastExpressionNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitCastPost(CastExpressionNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitCall(CallExpressionNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitCallPost(CallExpressionNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitIndex(IndexExpressionNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitIndexPost(IndexExpressionNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitArray(ArrayExpressionNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitArrayPost(ArrayExpressionNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitTuple(TupleExpressionNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitTuplePost(TupleExpressionNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitStruct(StructExpressionNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitStructPost(StructExpressionNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitMember(MemberExpressionNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitMemberPost(MemberExpressionNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitMacroCall(MacroCallExpressionNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitMacroCallPost(MacroCallExpressionNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitClosure(ClosureExpressionNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitClosurePost(ClosureExpressionNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitRange(RangeExpressionNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitRangePost(RangeExpressionNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitSpread(SpreadExpressionNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitSpreadPost(SpreadExpressionNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

// Default implementations for ConstASTVisitor

bool ConstASTVisitor::visitBool(const BoolLiteralNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitBoolPost(const BoolLiteralNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitInt(const IntLiteralNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitIntPost(const IntLiteralNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitFloat(const FloatLiteralNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitFloatPost(const FloatLiteralNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitString(const StringLiteralNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitStringPost(const StringLiteralNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitChar(const CharLiteralNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitCharPost(const CharLiteralNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitNull(const NullLiteralNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitNullPost(const NullLiteralNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitIdentifier(const IdentifierNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitIdentifierPost(const IdentifierNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitQualifiedPath(const QualifiedPathNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitQualifiedPathPost(const QualifiedPathNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitPrimitiveType(const PrimitiveTypeNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitPrimitiveTypePost(const PrimitiveTypeNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitUnary(const UnaryExpressionNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitUnaryPost(const UnaryExpressionNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitBinary(const BinaryExpressionNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitBinaryPost(const BinaryExpressionNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitTernary(const TernaryExpressionNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitTernaryPost(const TernaryExpressionNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitAssignment(const AssignmentExpressionNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitAssignmentPost(
    const AssignmentExpressionNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitGroup(const GroupExpressionNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitGroupPost(const GroupExpressionNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitStmt(const StmtExpressionNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitStmtPost(const StmtExpressionNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitStringExpr(const StringExpressionNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitStringExprPost(const StringExpressionNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitCast(const CastExpressionNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitCastPost(const CastExpressionNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitCall(const CallExpressionNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitCallPost(const CallExpressionNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitIndex(const IndexExpressionNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitIndexPost(const IndexExpressionNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitArray(const ArrayExpressionNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitArrayPost(const ArrayExpressionNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitTuple(const TupleExpressionNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitTuplePost(const TupleExpressionNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitStruct(const StructExpressionNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitStructPost(const StructExpressionNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitMember(const MemberExpressionNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitMemberPost(const MemberExpressionNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitMacroCall(const MacroCallExpressionNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitMacroCallPost(const MacroCallExpressionNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitClosure(const ClosureExpressionNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitClosurePost(const ClosureExpressionNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitRange(const RangeExpressionNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitRangePost(const RangeExpressionNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitSpread(const SpreadExpressionNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitSpreadPost(const SpreadExpressionNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ASTVisitor::visitField(FieldExpressionNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitFieldPost(FieldExpressionNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ConstASTVisitor::visitField(const FieldExpressionNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitFieldPost(const FieldExpressionNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

// Statement visitor implementations - ASTVisitor

bool ASTVisitor::visitExprStmt(ExpressionStatementNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitExprStmtPost(ExpressionStatementNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitBreakStmt(BreakStatementNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitBreakStmtPost(BreakStatementNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitContinueStmt(ContinueStatementNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitContinueStmtPost(ContinueStatementNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitDeferStmt(DeferStatementNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitDeferStmtPost(DeferStatementNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitReturnStmt(ReturnStatementNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitReturnStmtPost(ReturnStatementNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitYieldStmt(YieldStatementNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitYieldStmtPost(YieldStatementNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitBlockStmt(BlockStatementNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitBlockStmtPost(BlockStatementNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitIfStmt(IfStatementNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitIfStmtPost(IfStatementNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitForStmt(ForStatementNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitForStmtPost(ForStatementNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitWhileStmt(WhileStatementNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitWhileStmtPost(WhileStatementNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitSwitchStmt(SwitchStatementNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitSwitchStmtPost(SwitchStatementNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitMatchStmt(MatchStatementNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitMatchStmtPost(MatchStatementNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitCaseStmt(CaseStatementNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitCaseStmtPost(CaseStatementNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

// Statement visitor implementations - ConstASTVisitor

bool ConstASTVisitor::visitExprStmt(const ExpressionStatementNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitExprStmtPost(const ExpressionStatementNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitBreakStmt(const BreakStatementNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitBreakStmtPost(const BreakStatementNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitContinueStmt(const ContinueStatementNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitContinueStmtPost(const ContinueStatementNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitDeferStmt(const DeferStatementNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitDeferStmtPost(const DeferStatementNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitReturnStmt(const ReturnStatementNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitReturnStmtPost(const ReturnStatementNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitYieldStmt(const YieldStatementNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitYieldStmtPost(const YieldStatementNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitBlockStmt(const BlockStatementNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitBlockStmtPost(const BlockStatementNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitIfStmt(const IfStatementNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitIfStmtPost(const IfStatementNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitForStmt(const ForStatementNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitForStmtPost(const ForStatementNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitWhileStmt(const WhileStatementNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitWhileStmtPost(const WhileStatementNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitSwitchStmt(const SwitchStatementNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitSwitchStmtPost(const SwitchStatementNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitMatchStmt(const MatchStatementNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitMatchStmtPost(const MatchStatementNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitCaseStmt(const CaseStatementNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitCaseStmtPost(const CaseStatementNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

} // namespace cxy::ast
