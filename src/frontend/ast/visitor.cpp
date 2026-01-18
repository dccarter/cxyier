#include "cxy/ast/visitor.hpp"
#include "cxy/ast/annotations.hpp"
#include "cxy/ast/attributes.hpp"
#include "cxy/ast/declarations.hpp"
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
  case astPathSegment:
    return visitPathSegment(static_cast<PathSegmentNode *>(node));
  case astPrimitiveType:
    return visitPrimitiveType(static_cast<PrimitiveTypeNode *>(node));
  case astArrayType:
    return visitArrayType(static_cast<ArrayTypeNode *>(node));
  case astFunctionType:
    return visitFunctionType(static_cast<FunctionTypeNode *>(node));
  case astUnionType:
    return visitUnionType(static_cast<UnionTypeNode *>(node));
  case astTupleType:
    return visitTupleType(static_cast<TupleTypeNode *>(node));
  case astResultType:
    return visitResultType(static_cast<ResultTypeNode *>(node));
  case astOptionalType:
    return visitOptionalType(static_cast<OptionalTypeNode *>(node));
  case astReferenceType:
    return visitReferenceType(static_cast<ReferenceTypeNode *>(node));
  case astPointerType:
    return visitPointerType(static_cast<PointerTypeNode *>(node));

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
  case astMatchCase:
    return visitMatchCase(static_cast<MatchCaseNode *>(node));

  // Declarations
  case astVariableDeclaration:
    return visitVariableDeclaration(static_cast<VariableDeclarationNode *>(node));
  case astFuncDeclaration:
    return visitFuncDeclaration(static_cast<FuncDeclarationNode *>(node));
  case astFuncParamDeclaration:
    return visitFuncParamDeclaration(static_cast<FuncParamDeclarationNode *>(node));
  case astMethodDeclaration:
    return visitMethodDeclaration(static_cast<MethodDeclarationNode *>(node));
  case astTypeDeclaration:
    return visitTypeDeclaration(static_cast<TypeDeclarationNode *>(node));
  case astEnumOptionDeclaration:
    return visitEnumOptionDeclaration(static_cast<EnumOptionDeclarationNode *>(node));
  case astEnumDeclaration:
    return visitEnumDeclaration(static_cast<EnumDeclarationNode *>(node));
  case astFieldDeclaration:
    return visitFieldDeclaration(static_cast<FieldDeclarationNode *>(node));
  case astStructDeclaration:
    return visitStructDeclaration(static_cast<StructDeclarationNode *>(node));
  case astClassDeclaration:
    return visitClassDeclaration(static_cast<ClassDeclarationNode *>(node));
  case astExternDeclaration:
    return visitExternDeclaration(static_cast<ExternDeclarationNode *>(node));
  case astModuleDeclaration:
    return visitModuleDeclaration(static_cast<ModuleDeclarationNode *>(node));
  case astImportDeclaration:
    return visitImportDeclaration(static_cast<ImportDeclarationNode *>(node));
  case astImportItem:
    return visitImportItem(static_cast<ImportItemNode *>(node));
  case astTypeParameterDeclaration:
    return visitTypeParameterDeclaration(static_cast<TypeParameterDeclarationNode *>(node));
  case astGenericDeclaration:
    return visitGenericDeclaration(static_cast<GenericDeclarationNode *>(node));
  case astTestDeclaration:
    return visitTestDeclaration(static_cast<TestDeclarationNode *>(node));
  case astMacroDeclaration:
    return visitMacroDeclaration(static_cast<MacroDeclarationNode *>(node));
  case astAnnotation:
    return visitAnnotation(static_cast<AnnotationNode *>(node));
  case astAnnotationList:
    return visitAnnotationList(static_cast<AnnotationListNode *>(node));

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
  case astPathSegment:
    visitPathSegmentPost(static_cast<PathSegmentNode *>(node));
    break;
  case astPrimitiveType:
    visitPrimitiveTypePost(static_cast<PrimitiveTypeNode *>(node));
    break;
  case astArrayType:
    visitArrayTypePost(static_cast<ArrayTypeNode *>(node));
    break;
  case astFunctionType:
    visitFunctionTypePost(static_cast<FunctionTypeNode *>(node));
    break;
  case astUnionType:
    visitUnionTypePost(static_cast<UnionTypeNode *>(node));
    break;
  case astTupleType:
    visitTupleTypePost(static_cast<TupleTypeNode *>(node));
    break;
  case astResultType:
    visitResultTypePost(static_cast<ResultTypeNode *>(node));
    break;
  case astOptionalType:
    visitOptionalTypePost(static_cast<OptionalTypeNode *>(node));
    break;
  case astReferenceType:
    visitReferenceTypePost(static_cast<ReferenceTypeNode *>(node));
    break;
  case astPointerType:
    visitPointerTypePost(static_cast<PointerTypeNode *>(node));
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
  case astMatchCase:
    visitMatchCasePost(static_cast<MatchCaseNode *>(node));
    break;

  // Declarations
  case astVariableDeclaration:
    visitVariableDeclarationPost(static_cast<VariableDeclarationNode *>(node));
    break;
  case astFuncDeclaration:
    visitFuncDeclarationPost(static_cast<FuncDeclarationNode *>(node));
    break;
  case astFuncParamDeclaration:
    visitFuncParamDeclarationPost(static_cast<FuncParamDeclarationNode *>(node));
    break;
  case astMethodDeclaration:
    visitMethodDeclarationPost(static_cast<MethodDeclarationNode *>(node));
    break;
  case astTypeDeclaration:
    visitTypeDeclarationPost(static_cast<TypeDeclarationNode *>(node));
    break;
  case astEnumOptionDeclaration:
    visitEnumOptionDeclarationPost(static_cast<EnumOptionDeclarationNode *>(node));
    break;
  case astEnumDeclaration:
    visitEnumDeclarationPost(static_cast<EnumDeclarationNode *>(node));
    break;
  case astFieldDeclaration:
    visitFieldDeclarationPost(static_cast<FieldDeclarationNode *>(node));
    break;
  case astStructDeclaration:
    visitStructDeclarationPost(static_cast<StructDeclarationNode *>(node));
    break;
  case astClassDeclaration:
    visitClassDeclarationPost(static_cast<ClassDeclarationNode *>(node));
    break;
  case astExternDeclaration:
    visitExternDeclarationPost(static_cast<ExternDeclarationNode *>(node));
    break;
  case astModuleDeclaration:
    visitModuleDeclarationPost(static_cast<ModuleDeclarationNode *>(node));
    break;
  case astImportDeclaration:
    visitImportDeclarationPost(static_cast<ImportDeclarationNode *>(node));
    break;
  case astImportItem:
    visitImportItemPost(static_cast<ImportItemNode *>(node));
    break;
  case astTypeParameterDeclaration:
    visitTypeParameterDeclarationPost(static_cast<TypeParameterDeclarationNode *>(node));
    break;
  case astGenericDeclaration:
    visitGenericDeclarationPost(static_cast<GenericDeclarationNode *>(node));
    break;
  case astTestDeclaration:
    visitTestDeclarationPost(static_cast<TestDeclarationNode *>(node));
    break;
  case astMacroDeclaration:
    visitMacroDeclarationPost(static_cast<MacroDeclarationNode *>(node));
    break;
  case astAnnotation:
    visitAnnotationPost(static_cast<AnnotationNode *>(node));
    break;
  case astAnnotationList:
    visitAnnotationListPost(static_cast<AnnotationListNode *>(node));
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
  case astPathSegment:
    return visitPathSegment(static_cast<const PathSegmentNode *>(node));
  case astPrimitiveType:
    return visitPrimitiveType(static_cast<const PrimitiveTypeNode *>(node));
  case astArrayType:
    return visitArrayType(static_cast<const ArrayTypeNode *>(node));
  case astFunctionType:
    return visitFunctionType(static_cast<const FunctionTypeNode *>(node));
  case astUnionType:
    return visitUnionType(static_cast<const UnionTypeNode *>(node));
  case astTupleType:
    return visitTupleType(static_cast<const TupleTypeNode *>(node));
  case astResultType:
    return visitResultType(static_cast<const ResultTypeNode *>(node));
  case astOptionalType:
    return visitOptionalType(static_cast<const OptionalTypeNode *>(node));
  case astReferenceType:
    return visitReferenceType(static_cast<const ReferenceTypeNode *>(node));
  case astPointerType:
    return visitPointerType(static_cast<const PointerTypeNode *>(node));

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
  case astMatchCase:
    return visitMatchCase(static_cast<const MatchCaseNode *>(node));

  // Declarations
  case astVariableDeclaration:
    return visitVariableDeclaration(static_cast<const VariableDeclarationNode *>(node));
  case astFuncDeclaration:
    return visitFuncDeclaration(static_cast<const FuncDeclarationNode *>(node));
  case astFuncParamDeclaration:
    return visitFuncParamDeclaration(static_cast<const FuncParamDeclarationNode *>(node));
  case astMethodDeclaration:
    return visitMethodDeclaration(static_cast<const MethodDeclarationNode *>(node));
  case astTypeDeclaration:
    return visitTypeDeclaration(static_cast<const TypeDeclarationNode *>(node));
  case astEnumOptionDeclaration:
    return visitEnumOptionDeclaration(static_cast<const EnumOptionDeclarationNode *>(node));
  case astEnumDeclaration:
    return visitEnumDeclaration(static_cast<const EnumDeclarationNode *>(node));
  case astFieldDeclaration:
    return visitFieldDeclaration(static_cast<const FieldDeclarationNode *>(node));
  case astStructDeclaration:
    return visitStructDeclaration(static_cast<const StructDeclarationNode *>(node));
  case astClassDeclaration:
    return visitClassDeclaration(static_cast<const ClassDeclarationNode *>(node));
  case astExternDeclaration:
    return visitExternDeclaration(static_cast<const ExternDeclarationNode *>(node));
  case astModuleDeclaration:
    return visitModuleDeclaration(static_cast<const ModuleDeclarationNode *>(node));
  case astImportDeclaration:
    return visitImportDeclaration(static_cast<const ImportDeclarationNode *>(node));
  case astImportItem:
    return visitImportItem(static_cast<const ImportItemNode *>(node));
  case astTypeParameterDeclaration:
    return visitTypeParameterDeclaration(static_cast<const TypeParameterDeclarationNode *>(node));
  case astGenericDeclaration:
    return visitGenericDeclaration(static_cast<const GenericDeclarationNode *>(node));
  case astTestDeclaration:
    return visitTestDeclaration(static_cast<const TestDeclarationNode *>(node));
  case astMacroDeclaration:
    return visitMacroDeclaration(static_cast<const MacroDeclarationNode *>(node));
  case astAnnotation:
    return visitAnnotation(static_cast<const AnnotationNode *>(node));
  case astAnnotationList:
    return visitAnnotationList(static_cast<const AnnotationListNode *>(node));

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
  case astPathSegment:
    visitPathSegmentPost(static_cast<const PathSegmentNode *>(node));
    break;
  case astPrimitiveType:
    visitPrimitiveTypePost(static_cast<const PrimitiveTypeNode *>(node));
    break;
  case astArrayType:
    visitArrayTypePost(static_cast<const ArrayTypeNode *>(node));
    break;
  case astFunctionType:
    visitFunctionTypePost(static_cast<const FunctionTypeNode *>(node));
    break;
  case astUnionType:
    visitUnionTypePost(static_cast<const UnionTypeNode *>(node));
    break;
  case astTupleType:
    visitTupleTypePost(static_cast<const TupleTypeNode *>(node));
    break;
  case astResultType:
    visitResultTypePost(static_cast<const ResultTypeNode *>(node));
    break;
  case astOptionalType:
    visitOptionalTypePost(static_cast<const OptionalTypeNode *>(node));
    break;
  case astReferenceType:
    visitReferenceTypePost(static_cast<const ReferenceTypeNode *>(node));
    break;
  case astPointerType:
    visitPointerTypePost(static_cast<const PointerTypeNode *>(node));
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
  case astMatchCase:
    visitMatchCasePost(static_cast<const MatchCaseNode *>(node));
    break;

  // Declarations
  case astVariableDeclaration:
    visitVariableDeclarationPost(static_cast<const VariableDeclarationNode *>(node));
    break;
  case astFuncDeclaration:
    visitFuncDeclarationPost(static_cast<const FuncDeclarationNode *>(node));
    break;
  case astFuncParamDeclaration:
    visitFuncParamDeclarationPost(static_cast<const FuncParamDeclarationNode *>(node));
    break;
  case astMethodDeclaration:
    visitMethodDeclarationPost(static_cast<const MethodDeclarationNode *>(node));
    break;
  case astTypeDeclaration:
    visitTypeDeclarationPost(static_cast<const TypeDeclarationNode *>(node));
    break;
  case astEnumOptionDeclaration:
    visitEnumOptionDeclarationPost(static_cast<const EnumOptionDeclarationNode *>(node));
    break;
  case astEnumDeclaration:
    visitEnumDeclarationPost(static_cast<const EnumDeclarationNode *>(node));
    break;
  case astFieldDeclaration:
    visitFieldDeclarationPost(static_cast<const FieldDeclarationNode *>(node));
    break;
  case astStructDeclaration:
    visitStructDeclarationPost(static_cast<const StructDeclarationNode *>(node));
    break;
  case astClassDeclaration:
    visitClassDeclarationPost(static_cast<const ClassDeclarationNode *>(node));
    break;
  case astExternDeclaration:
    visitExternDeclarationPost(static_cast<const ExternDeclarationNode *>(node));
    break;
  case astModuleDeclaration:
    visitModuleDeclarationPost(static_cast<const ModuleDeclarationNode *>(node));
    break;
  case astImportDeclaration:
    visitImportDeclarationPost(static_cast<const ImportDeclarationNode *>(node));
    break;
  case astImportItem:
    visitImportItemPost(static_cast<const ImportItemNode *>(node));
    break;
  case astTypeParameterDeclaration:
    visitTypeParameterDeclarationPost(static_cast<const TypeParameterDeclarationNode *>(node));
    break;
  case astGenericDeclaration:
    visitGenericDeclarationPost(static_cast<const GenericDeclarationNode *>(node));
    break;
  case astTestDeclaration:
    visitTestDeclarationPost(static_cast<const TestDeclarationNode *>(node));
    break;
  case astMacroDeclaration:
    visitMacroDeclarationPost(static_cast<const MacroDeclarationNode *>(node));
    break;
  case astAnnotation:
    visitAnnotationPost(static_cast<const AnnotationNode *>(node));
    break;
  case astAnnotationList:
    visitAnnotationListPost(static_cast<const AnnotationListNode *>(node));
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

bool ASTVisitor::visitPathSegment(PathSegmentNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitPathSegmentPost(PathSegmentNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitPrimitiveType(PrimitiveTypeNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitPrimitiveTypePost(PrimitiveTypeNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitArrayType(ArrayTypeNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitArrayTypePost(ArrayTypeNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitFunctionType(FunctionTypeNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitFunctionTypePost(FunctionTypeNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitUnionType(UnionTypeNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitUnionTypePost(UnionTypeNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitTupleType(TupleTypeNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitTupleTypePost(TupleTypeNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitResultType(ResultTypeNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitResultTypePost(ResultTypeNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitOptionalType(OptionalTypeNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitOptionalTypePost(OptionalTypeNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitReferenceType(ReferenceTypeNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitReferenceTypePost(ReferenceTypeNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitPointerType(PointerTypeNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitPointerTypePost(PointerTypeNode *node) {
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

bool ConstASTVisitor::visitPathSegment(const PathSegmentNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitPathSegmentPost(const PathSegmentNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitPrimitiveType(const PrimitiveTypeNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitPrimitiveTypePost(const PrimitiveTypeNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitArrayType(const ArrayTypeNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitArrayTypePost(const ArrayTypeNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitFunctionType(const FunctionTypeNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitFunctionTypePost(const FunctionTypeNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitUnionType(const UnionTypeNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitUnionTypePost(const UnionTypeNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitTupleType(const TupleTypeNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitTupleTypePost(const TupleTypeNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitResultType(const ResultTypeNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitResultTypePost(const ResultTypeNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitOptionalType(const OptionalTypeNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitOptionalTypePost(const OptionalTypeNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitReferenceType(const ReferenceTypeNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitReferenceTypePost(const ReferenceTypeNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitPointerType(const PointerTypeNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitPointerTypePost(const PointerTypeNode *node) {
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

// Declaration node implementations for ASTVisitor

bool ASTVisitor::visitVariableDeclaration(VariableDeclarationNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitVariableDeclarationPost(VariableDeclarationNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

// Function declaration implementations for ASTVisitor

bool ASTVisitor::visitFuncDeclaration(FuncDeclarationNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitFuncDeclarationPost(FuncDeclarationNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitFuncParamDeclaration(FuncParamDeclarationNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitFuncParamDeclarationPost(FuncParamDeclarationNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitMethodDeclaration(MethodDeclarationNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitMethodDeclarationPost(MethodDeclarationNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitTypeDeclaration(TypeDeclarationNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitTypeDeclarationPost(TypeDeclarationNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitEnumOptionDeclaration(EnumOptionDeclarationNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitEnumOptionDeclarationPost(EnumOptionDeclarationNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitEnumDeclaration(EnumDeclarationNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitEnumDeclarationPost(EnumDeclarationNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitFieldDeclaration(FieldDeclarationNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitFieldDeclarationPost(FieldDeclarationNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitStructDeclaration(StructDeclarationNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitStructDeclarationPost(StructDeclarationNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitClassDeclaration(ClassDeclarationNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitClassDeclarationPost(ClassDeclarationNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitExternDeclaration(ExternDeclarationNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitExternDeclarationPost(ExternDeclarationNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitModuleDeclaration(ModuleDeclarationNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitModuleDeclarationPost(ModuleDeclarationNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitImportDeclaration(ImportDeclarationNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitImportDeclarationPost(ImportDeclarationNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitImportItem(ImportItemNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitImportItemPost(ImportItemNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitTypeParameterDeclaration(TypeParameterDeclarationNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitTypeParameterDeclarationPost(TypeParameterDeclarationNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitGenericDeclaration(GenericDeclarationNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitGenericDeclarationPost(GenericDeclarationNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitTestDeclaration(TestDeclarationNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitTestDeclarationPost(TestDeclarationNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitMacroDeclaration(MacroDeclarationNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitMacroDeclarationPost(MacroDeclarationNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitAnnotation(AnnotationNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitAnnotationPost(AnnotationNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

bool ASTVisitor::visitAnnotationList(AnnotationListNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitAnnotationListPost(AnnotationListNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

// Match case implementations for ASTVisitor

bool ASTVisitor::visitMatchCase(MatchCaseNode *node) {
  return visitNode(static_cast<ASTNode *>(node));
}

void ASTVisitor::visitMatchCasePost(MatchCaseNode *node) {
  visitNodePost(static_cast<ASTNode *>(node));
}

// Declaration node implementations for ConstASTVisitor

bool ConstASTVisitor::visitVariableDeclaration(const VariableDeclarationNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitVariableDeclarationPost(const VariableDeclarationNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

// Function declaration implementations for ConstASTVisitor

bool ConstASTVisitor::visitFuncDeclaration(const FuncDeclarationNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitFuncDeclarationPost(const FuncDeclarationNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitFuncParamDeclaration(const FuncParamDeclarationNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitFuncParamDeclarationPost(const FuncParamDeclarationNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitMethodDeclaration(const MethodDeclarationNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitMethodDeclarationPost(const MethodDeclarationNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitTypeDeclaration(const TypeDeclarationNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitTypeDeclarationPost(const TypeDeclarationNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitEnumOptionDeclaration(const EnumOptionDeclarationNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitEnumOptionDeclarationPost(const EnumOptionDeclarationNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitEnumDeclaration(const EnumDeclarationNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitEnumDeclarationPost(const EnumDeclarationNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitFieldDeclaration(const FieldDeclarationNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitFieldDeclarationPost(const FieldDeclarationNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitStructDeclaration(const StructDeclarationNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitStructDeclarationPost(const StructDeclarationNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitClassDeclaration(const ClassDeclarationNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitClassDeclarationPost(const ClassDeclarationNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitExternDeclaration(const ExternDeclarationNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitExternDeclarationPost(const ExternDeclarationNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitModuleDeclaration(const ModuleDeclarationNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitModuleDeclarationPost(const ModuleDeclarationNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitImportDeclaration(const ImportDeclarationNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitImportDeclarationPost(const ImportDeclarationNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitImportItem(const ImportItemNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitImportItemPost(const ImportItemNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitTypeParameterDeclaration(const TypeParameterDeclarationNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitTypeParameterDeclarationPost(const TypeParameterDeclarationNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitGenericDeclaration(const GenericDeclarationNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitGenericDeclarationPost(const GenericDeclarationNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitTestDeclaration(const TestDeclarationNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitTestDeclarationPost(const TestDeclarationNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitMacroDeclaration(const MacroDeclarationNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitMacroDeclarationPost(const MacroDeclarationNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitAnnotation(const AnnotationNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitAnnotationPost(const AnnotationNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

bool ConstASTVisitor::visitAnnotationList(const AnnotationListNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitAnnotationListPost(const AnnotationListNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

// Match case implementations for ConstASTVisitor

bool ConstASTVisitor::visitMatchCase(const MatchCaseNode *node) {
  return visitNode(static_cast<const ASTNode *>(node));
}

void ConstASTVisitor::visitMatchCasePost(const MatchCaseNode *node) {
  visitNodePost(static_cast<const ASTNode *>(node));
}

} // namespace cxy::ast
