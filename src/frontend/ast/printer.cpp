#include "cxy/ast/printer.hpp"
#include "cxy/ast/annotations.hpp"
#include "cxy/ast/attributes.hpp"
#include "cxy/ast/declarations.hpp"
#include "cxy/ast/expressions.hpp"
#include "cxy/ast/identifiers.hpp"
#include "cxy/ast/kind.hpp"
#include "cxy/ast/literals.hpp"
#include "cxy/ast/statements.hpp"
#include "cxy/ast/types.hpp"
#include "cxy/flags.hpp"
#include "cxy/token.hpp"
#include <format>
#include <fstream>
#include <iostream>

namespace cxy::ast {

ASTPrinter::ASTPrinter(const PrinterConfig &config)
    : config_(config), output_(&defaultStream_) {}

ASTPrinter::ASTPrinter(ArenaAllocator &arena, const PrinterConfig &config)
    : config_(config), output_(&defaultStream_), arena_(&arena) {}

std::string ASTPrinter::print(const ASTNode *root) {
  defaultStream_.str("");
  defaultStream_.clear();
  output_ = &defaultStream_;
  printToStream(root, defaultStream_);
  return defaultStream_.str();
}

void ASTPrinter::print(const ASTNode *root, std::ostream &out) {
  printToStream(root, out);
}

void ASTPrinter::printToStream(const ASTNode *root, std::ostream &out) {
  output_ = &out;
  resetState();

  if (!root) {
    *output_ << "(Null)";
    return;
  }

  visit(root);
}

void ASTPrinter::setConfig(const PrinterConfig &config) { config_ = config; }

void ASTPrinter::visit(const ASTNode *node) {
  if (!node)
    return;

  // Update statistics and check depth first
  updateStatistics(node);

  if (!shouldPrintNode(node)) {
    return; // Skip this node and its children
  }

  if (config_.maxDepth > 0 && currentDepth_ >= config_.maxDepth) {
    *output_ << "...";
    return; // Skip children due to depth limit
  }

  // Call the base visitor implementation for actual printing
  bool needsIndent = needsIndent_;
  bool isFirstChild = isFirstChild_;

  if (dispatchVisit(node)) {
    // Visit children if the node visit returned true
    increaseIndent();
    isFirstChild_ = !isCompactMode();
    for (const ASTNode *child : node->children) {
      visit(child);
    }
    decreaseIndent();
  }

  dispatchVisitPost(node);

  needsIndent_= needsIndent;
  isFirstChild_ = isFirstChild;
}

bool ASTPrinter::visitNode(const ASTNode *node) {
  printNodeStart(node);
  increaseIndent();
  currentDepth_++;
  isFirstChild_ = true;

  return true; // Continue to children
}

void ASTPrinter::visitNodePost(const ASTNode *node) {
  // Always decrement depth for all nodes to maintain balance
  currentDepth_--;

  // Only handle non-inline nodes (inline nodes handle their own closing)
  if (!shouldPrintInline(node)) {
    printNodeEnd();
  }
  isFirstChild_ = false;
}

bool ASTPrinter::visitNoop(const ASTNode *node) {
  printNodeStartInline(node);
  *output_ << ")";
  return false; // No children for noop
}

bool ASTPrinter::visitBool(const BoolLiteralNode *node) {
  printNodeStartInline(node);
  *output_ << " " << (node->value ? "true" : "false") << ")";
  return false;
}

bool ASTPrinter::visitInt(const IntLiteralNode *node) {
  printNodeStartInline(node);
  *output_ << " " << static_cast<long long>(node->value) << ")";
  return false;
}

bool ASTPrinter::visitFloat(const FloatLiteralNode *node) {
  printNodeStartInline(node);
  *output_ << " " << node->value << ")";
  return false;
}

bool ASTPrinter::visitString(const StringLiteralNode *node) {
  printNodeStartInline(node);
  *output_ << " \"" << node->value.view() << "\")";
  return false;
}

bool ASTPrinter::visitChar(const CharLiteralNode *node) {
  printNodeStartInline(node);
  if (node->value >= 32 && node->value <= 126) {
    *output_ << " '" << static_cast<char>(node->value) << "'";
  } else {
    *output_ << std::format(" '\\u{{{:04x}}}'", node->value);
  }
  *output_ << ")";
  return false;
}

bool ASTPrinter::visitNull(const NullLiteralNode *node) {
  printNodeStartInline(node);
  *output_ << ")";
  return false;
}

bool ASTPrinter::visitIdentifier(const IdentifierNode *node) {
  printNodeStartInline(node);
  *output_ << " " << node->name.view() << ")";
  return false;
}

bool ASTPrinter::visitPrimitiveType(const PrimitiveTypeNode *node) {
  *output_ << " (Type " << tokenKindToString(node->typeKind);
  if (shouldPrintInline(node) || isCompactMode()) {
    if (config_.hasFlag(PrinterFlags::IncludeAttributes)) {
      printAttributes(node);
    }
    *output_ << ")";
  }
  return false;
}

bool ASTPrinter::visitArrayType(const ArrayTypeNode *node) {
    printNodeStartInline(node);
    // if (node->size != 0)
    //     *output_ << " " << node->size;
    // Children will be printed automatically
    return true;
}

bool ASTPrinter::visitQualifiedPath(const QualifiedPathNode *node) {
  printNodeStartInline(node);
  // Children will be printed automatically
  return true;
}

bool ASTPrinter::visitPathSegment(const PathSegmentNode *node) {
  printNodeStartInline(node);
  *output_ << " " << node->name.view();
  // Generic arguments will be printed as children
  return true;
}

bool ASTPrinter::visitUnary(const UnaryExpressionNode *node) {
  printNodeStartInline(node);
  *output_ << " " << tokenKindToString(node->op);
  if (!node->isPrefix) {
    *output_ << " [postfix]";
  }
  return true; // Continue to operand
}

bool ASTPrinter::visitBinary(const BinaryExpressionNode *node) {
  printNodeStartInline(node);
  *output_ << " " << tokenKindToString(node->op);
  return true; // Continue to left and right operands
}

bool ASTPrinter::visitTernary(const TernaryExpressionNode *node) {
  printNodeStartInline(node);
  return true; // Continue to condition, trueExpr, falseExpr
}

bool ASTPrinter::visitAssignment(const AssignmentExpressionNode *node) {
  printNodeStartInline(node);
  *output_ << " " << tokenKindToString(node->op);
  return true; // Continue to target and value
}

bool ASTPrinter::visitGroup(const GroupExpressionNode *node) {
  printNodeStartInline(node);
  return true; // Continue to grouped expression
}

bool ASTPrinter::visitStmt(const StmtExpressionNode *node) {
  printNodeStartInline(node);
  return true; // Continue to statement
}

bool ASTPrinter::visitStringExpr(const StringExpressionNode *node) {
  printNodeStartInline(node);

  // Print parts specially - string literals as raw strings, expressions as AST
  // nodes
  for (size_t i = 0; i < node->parts.size(); ++i) {
    *output_ << " ";

    ASTNode *part = node->parts[i];
    if (part && part->kind == astString) {
      // Print string literals as raw quoted strings
      auto *stringLiteral = static_cast<const StringLiteralNode *>(part);
      *output_ << "\"" << stringLiteral->value.view() << "\"";
    } else if (part) {
      // Print expressions as AST nodes
      visit(part);
    }
  }

  if (shouldPrintInline(node) || isCompactMode()) {
    if (config_.hasFlag(PrinterFlags::IncludeAttributes)) {
      printAttributes(node);
    }
    *output_ << ")";
  }
  return false; // Don't continue to children since we handled them manually
}

bool ASTPrinter::visitCast(const CastExpressionNode *node) {
  printNodeStartInline(node);
  const char *op = node->isRetype ? "!:" : "as";
  *output_ << " " << op;
  return true; // Continue to expression and type
}

bool ASTPrinter::visitCall(const CallExpressionNode *node) {
  printNodeStartInline(node);
  return true; // Continue to callee and arguments
}

bool ASTPrinter::visitIndex(const IndexExpressionNode *node) {
  printNodeStartInline(node);
  return true; // Continue to array and index
}

bool ASTPrinter::visitArray(const ArrayExpressionNode *node) {
  printNodeStartInline(node);
  return true; // Continue to array elements
}

bool ASTPrinter::visitTuple(const TupleExpressionNode *node) {
  printNodeStartInline(node);
  return true; // Continue to tuple elements
}

bool ASTPrinter::visitField(const FieldExpressionNode *node) {
  printNodeStartInline(node);
  return true; // Continue to name, value, and default value
}

bool ASTPrinter::visitStruct(const StructExpressionNode *node) {
  printNodeStartInline(node);
  return true; // Continue to fields
}

bool ASTPrinter::visitMember(const MemberExpressionNode *node) {
  printNodeStartInline(node);
  *output_ << " " << (node->isArrow ? "&." : ".");
  return true; // Continue to object and member
}

bool ASTPrinter::visitMacroCall(const MacroCallExpressionNode *node) {
  printNodeStartInline(node);
  return true; // Continue to macro and arguments
}

bool ASTPrinter::visitClosure(const ClosureExpressionNode *node) {
  printNodeStartInline(node);
  return true; // Continue to parameters and body
}

bool ASTPrinter::visitRange(const RangeExpressionNode *node) {
  printNodeStartInline(node);
  const char *op = node->isInclusive ? ".." : "..<";
  *output_ << " " << op;
  return true; // Continue to start and end
}

bool ASTPrinter::visitSpread(const SpreadExpressionNode *node) {
  printNodeStartInline(node);
  return true; // Continue to expression
}

void ASTPrinter::printNodeEnd() {
  *output_ << ")";
  needsIndent_ = !isCompactMode();
}

void ASTPrinter::printNodeStart(const ASTNode *node) {
  if (needsIndent_) {
    printNewline();
    printIndent();
  } else if (!isFirstChild_) {
    printSpace();
  }

  *output_ << "(" << nodeKindToString(node->kind);

  // Print optional attributes
  if (config_.hasFlag(PrinterFlags::IncludeLocation)) {
    printLocation(node->location);
  }

  if (config_.hasFlag(PrinterFlags::IncludeTypes) && node->type) {
    printType(node->type);
  }

  if (config_.hasFlag(PrinterFlags::IncludeFlags) && node->flags != flgNone) {
    printFlags(node->flags);
  }

  if (config_.hasFlag(PrinterFlags::IncludeMetadata)) {
    printMetadata(node);
  }

  if (config_.hasFlag(PrinterFlags::IncludeAttributes)) {
    printAttributes(node);
  }

  needsIndent_ = !isCompactMode();
}

void ASTPrinter::printNodeEndInline(const ASTNode *node) { *output_ << ")"; }

void ASTPrinter::printNodeEndInlineWithAttributes(const ASTNode *node) {
  // Print attributes at the end for inline nodes
  if (config_.hasFlag(PrinterFlags::IncludeAttributes)) {
    printAttributes(node);
  }
  *output_ << ")";
}

void ASTPrinter::printNodeStartInline(const ASTNode *node) {
  if (shouldPrintInline(node) || isCompactMode()) {
    if (needsIndent_) {
      printNewline();
      printIndent();
    } else if (!isFirstChild_) {
      printSpace();
    }

    *output_ << "(" << nodeKindToString(node->kind);

    // Print optional attributes inline
    if (config_.hasFlag(PrinterFlags::IncludeLocation)) {
      printLocation(node->location);
    }

    if (config_.hasFlag(PrinterFlags::IncludeTypes) && node->type) {
      printType(node->type);
    }

    if (config_.hasFlag(PrinterFlags::IncludeFlags) && node->flags != flgNone) {
      printFlags(node->flags);
    }

    if (config_.hasFlag(PrinterFlags::IncludeMetadata)) {
      printMetadata(node);
    }

    if (config_.hasFlag(PrinterFlags::IncludeAttributes)) {
      printAttributes(node);
    }

    needsIndent_ = false;
  } else {
    printNodeStart(node);
  }
}

void ASTPrinter::printLocation(const Location &loc) {
  if (!loc.filename.empty()) {
    *output_ << " @" << loc.start.row << ":" << loc.start.column;
    if (loc.start.row != loc.end.row || loc.start.column != loc.end.column) {
      *output_ << "-" << loc.end.row << ":" << loc.end.column;
    }
  }
}

void ASTPrinter::printType(const Type *type) {
  if (type) {
    *output_ << " [type=Type]";
  }
}

void ASTPrinter::printFlags(Flags flags) {
  *output_ << " [flags=" << flagsToString(flags) << "]";
}

void ASTPrinter::printMetadata(const ASTNode *node) {
  if (!node->metadata.empty()) {
    *output_ << " [metadata=" << node->metadata.size() << " entries]";
  }
}

void ASTPrinter::printAttributes(const ASTNode *node) {
  if (!node->attrs.empty()) {
    *output_ << " [";
    for (size_t i = 0; i < node->attrs.size(); ++i) {
      if (i > 0) {
        *output_ << " ";
      }

      if (node->attrs[i] && node->attrs[i]->kind == astAttribute) {
        // Cast to AttributeNode to access name and args
        auto *attr = static_cast<const AttributeNode *>(node->attrs[i]);

        if (attr->hasParameters()) {
          *output_ << "(" << attr->name.view();
          for (size_t j = 0; j < attr->args.size(); ++j) {
              *output_ << " ";

            ASTNode *arg = attr->args[j];
            if (arg->kind == astFieldExpr) {
              // Named parameter: name value
              *output_ << "(";
              auto *field = static_cast<const FieldExpressionNode *>(arg);
              if (field->name) {
                printAttributeArgument(field->name);
                *output_ << " ";
              }
              if (field->value) {
                printAttributeArgument(field->value);
              }
              *output_ << ")";
            } else {
              // Positional parameter: just the value
              printAttributeArgument(arg);
            }
          }
          *output_ << ")";
        } else {
            *output_ << attr->name.view();
        }
      } else if (node->attrs[i]) {
        // Fallback for non-attribute nodes
        *output_ << nodeKindToString(node->attrs[i]->kind);
      } else {
        *output_ << "null";
      }
    }
    *output_ << "]";
  }
}

void ASTPrinter::printAttributeArgument(const ASTNode *arg) {
  if (!arg) {
    *output_ << "null";
    return;
  }

  switch (arg->kind) {
  case astBool: {
    auto *boolNode = static_cast<const BoolLiteralNode *>(arg);
    *output_ << (boolNode->value ? "true" : "false");
    break;
  }
  case astInt: {
    auto *intNode = static_cast<const IntLiteralNode *>(arg);
    *output_ << static_cast<long long>(intNode->value);
    break;
  }
  case astFloat: {
    auto *floatNode = static_cast<const FloatLiteralNode *>(arg);
    *output_ << floatNode->value;
    break;
  }
  case astString: {
    auto *stringNode = static_cast<const StringLiteralNode *>(arg);
    *output_ << "\"" << stringNode->value.view() << "\"";
    break;
  }
  case astChar: {
    auto *charNode = static_cast<const CharLiteralNode *>(arg);
    if (charNode->value >= 32 && charNode->value <= 126) {
      *output_ << "'" << static_cast<char>(charNode->value) << "'";
    } else {
      *output_ << std::format("'\\u{{{:04x}}}'", charNode->value);
    }
    break;
  }
  case astIdentifier: {
    auto *identNode = static_cast<const IdentifierNode *>(arg);
    *output_ << identNode->name.view();
    break;
  }
  case astNull:
    *output_ << "null";
    break;
  default:
    *output_ << nodeKindToString(arg->kind);
    break;
  }
}

void ASTPrinter::printIndent() {
  if (!isCompactMode()) {
    for (uint32_t i = 0; i < indentLevel_ * config_.indentSize; ++i) {
      *output_ << " ";
    }
  }
}

void ASTPrinter::increaseIndent() { indentLevel_++; }

void ASTPrinter::decreaseIndent() {
  if (indentLevel_ > 0) {
    indentLevel_--;
  }
}

void ASTPrinter::printSpace() {
  if (!needsIndent_) {
    *output_ << " ";
  }
}

void ASTPrinter::printNewline() {
  if (!isCompactMode()) {
    *output_ << "\n";
  }
}

bool ASTPrinter::shouldPrintNode(const ASTNode *node) const {
  if (config_.nodeFilter) {
    return config_.nodeFilter(node);
  }
  return true;
}

bool ASTPrinter::shouldPrintInline(const ASTNode *node) const {
  // Print literals and simple nodes inline
  switch (node->kind) {
  case astBool:
  case astInt:
  case astFloat:
  case astString:
  case astChar:
  case astNull:
  case astIdentifier:
  case astNoop:
    return true;
  default:
    return false;
  }
}

bool ASTPrinter::isCompactMode() const {
  return config_.hasFlag(PrinterFlags::CompactMode);
}

void ASTPrinter::resetState() {
  currentDepth_ = 0;
  indentLevel_ = 0;
  nodesVisited_ = 0;
  maxDepthReached_ = 0;
  needsIndent_ = false;
  isFirstChild_ = true;
}

void ASTPrinter::updateStatistics(const ASTNode *node) {
  nodesVisited_++;
  if (currentDepth_ > maxDepthReached_) {
    maxDepthReached_ = currentDepth_;
  }
}

// Utility functions
std::string printAST(const ASTNode *root, const PrinterConfig &config) {
  ASTPrinter printer(config);
  return printer.print(root);
}

void printASTToFile(const ASTNode *root, const std::string &filename,
                    const PrinterConfig &config) {
  std::ofstream file(filename);
  if (!file) {
    throw std::runtime_error("Failed to open file: " + filename);
  }

  ASTPrinter printer(config);
  printer.print(root, file);
}

// Statement visitor implementations

bool ASTPrinter::visitExprStmt(const ExpressionStatementNode *node) {
  printNodeStartInline(node);
  return true;
}

bool ASTPrinter::visitBreakStmt(const BreakStatementNode *node) {
  printNodeStartInline(node);
  return true;
}

bool ASTPrinter::visitContinueStmt(const ContinueStatementNode *node) {
  printNodeStartInline(node);
  return true;
}

bool ASTPrinter::visitDeferStmt(const DeferStatementNode *node) {
  printNodeStartInline(node);
  return true;
}

bool ASTPrinter::visitReturnStmt(const ReturnStatementNode *node) {
  printNodeStartInline(node);
  return true;
}

bool ASTPrinter::visitYieldStmt(const YieldStatementNode *node) {
  printNodeStartInline(node);
  return true;
}

bool ASTPrinter::visitBlockStmt(const BlockStatementNode *node) {
  printNodeStartInline(node);
  return true;
}

bool ASTPrinter::visitIfStmt(const IfStatementNode *node) {
  printNodeStartInline(node);
  return true;
}

bool ASTPrinter::visitForStmt(const ForStatementNode *node) {
  printNodeStartInline(node);

  // Print Variables section with label
  increaseIndent();
  printNewline();
  if (isCompactMode())
      printSpace();
  else
      printIndent();
  *output_ << "(Variables";
  for (auto *var : node->variables) {
      *output_ << " ";
    if (var && var->kind == astIdentifier) {
      auto *identNode = static_cast<const IdentifierNode *>(var);
      *output_ << identNode->name.view();
    } else {
      *output_ << "(Error)";
    }
  }
  *output_ << ")";
  if (isCompactMode())
      printSpace();
  visit(node->range);
  if (isCompactMode())
      printSpace();
  visit(node->condition);
  if (isCompactMode())
      printSpace();
  visit(node->body);


  // Return true to let default visitor handle range, condition, body
  return false;
}

bool ASTPrinter::visitWhileStmt(const WhileStatementNode *node) {
  printNodeStartInline(node);
  return true;
}

bool ASTPrinter::visitSwitchStmt(const SwitchStatementNode *node) {
  printNodeStartInline(node);
  return true;
}

bool ASTPrinter::visitMatchStmt(const MatchStatementNode *node) {
  printNodeStartInline(node);
  return true;
}

bool ASTPrinter::visitCaseStmt(const CaseStatementNode *node) {
  printNodeStartInline(node);
  if (node->isDefault) {
    *output_ << " default";
  }
  return true; // Continue to values and statement children
}

bool ASTPrinter::visitMatchCase(const MatchCaseNode *node) {
  printNodeStartInline(node);
  if (node->isDefault) {
    *output_ << " default";
  }
  return true; // Continue to types, binding and statement children
}

bool ASTPrinter::visitVariableDeclaration(const VariableDeclarationNode *node) {
  printNodeStartInline(node);
  return true;
}

bool ASTPrinter::visitFuncDeclaration(const FuncDeclarationNode *node) {
  printNodeStartInline(node);
  return true;
}

bool ASTPrinter::visitFuncParamDeclaration(const FuncParamDeclarationNode *node) {
  printNodeStartInline(node);
  return true;
}

bool ASTPrinter::visitMethodDeclaration(const MethodDeclarationNode *node) {
  printNodeStartInline(node);
  return true;
}

bool ASTPrinter::visitFieldDeclaration(const FieldDeclarationNode *node) {
  printNodeStartInline(node);
  return true;
}

bool ASTPrinter::visitStructDeclaration(const StructDeclarationNode *node) {
  printNodeStartInline(node);
  return true;
}

bool ASTPrinter::visitClassDeclaration(const ClassDeclarationNode *node) {
  printNodeStartInline(node);
  return true;
}

bool ASTPrinter::visitAnnotation(const AnnotationNode *node) {
  printNodeStartInline(node);
  *output_ << " " << node->name.view();
  return true; // Continue to visit children (the value)
}

} // namespace cxy::ast
