#include "cxy/ast/printer.hpp"
#include "cxy/ast/expressions.hpp"
#include "cxy/ast/identifiers.hpp"
#include "cxy/ast/literals.hpp"
#include "cxy/flags.hpp"
#include "cxy/token.hpp"
#include <format>
#include <fstream>

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
  ConstASTVisitor::visit(node);
}

bool ASTPrinter::visitNode(const ASTNode *node) {
  printNodeStart(node);
  increaseIndent();
  currentDepth_++;
  isFirstChild_ = true;

  return true; // Continue to children
}

void ASTPrinter::visitNodePost(const ASTNode *node) {
  currentDepth_--;
  decreaseIndent();
  printNodeEnd();
  isFirstChild_ = false;
}

bool ASTPrinter::visitNoop(const ASTNode *node) {
  printNodeStartInline(node);
  return false; // No children for noop
}

bool ASTPrinter::visitBool(const BoolLiteralNode *node) {
  printNodeStartInline(node);
  *output_ << " " << (node->value ? "true" : "false");
  return false; // No children for literals
}

bool ASTPrinter::visitInt(const IntLiteralNode *node) {
  printNodeStartInline(node);
  *output_ << " " << static_cast<long long>(node->value);
  return false;
}

bool ASTPrinter::visitFloat(const FloatLiteralNode *node) {
  printNodeStartInline(node);
  *output_ << " " << node->value;
  return false;
}

bool ASTPrinter::visitString(const StringLiteralNode *node) {
  printNodeStartInline(node);
  *output_ << " \"" << node->value.view() << "\"";
  return false;
}

bool ASTPrinter::visitChar(const CharLiteralNode *node) {
  printNodeStartInline(node);
  if (node->value >= 32 && node->value <= 126) {
    *output_ << " '" << static_cast<char>(node->value) << "'";
  } else {
    *output_ << std::format(" '\\u{{{:04x}}}'", node->value);
  }
  return false;
}

bool ASTPrinter::visitNull(const NullLiteralNode *node) {
  printNodeStartInline(node);
  return false;
}

bool ASTPrinter::visitIdentifier(const IdentifierNode *node) {
  printNodeStartInline(node);
  *output_ << " " << node->name.view();
  return false;
}

bool ASTPrinter::visitQualifiedPath(const QualifiedPathNode *node) {
  printNodeStartInline(node);
  // Children will be printed automatically
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
  return true; // Continue to string parts
}

bool ASTPrinter::visitCast(const CastExpressionNode *node) {
  printNodeStartInline(node);
  return true; // Continue to target type and expression
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
  return true; // Continue to elements
}

bool ASTPrinter::visitTuple(const TupleExpressionNode *node) {
  printNodeStartInline(node);
  return true; // Continue to elements
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
  return true; // Continue to start and end
}

bool ASTPrinter::visitSpread(const SpreadExpressionNode *node) {
  printNodeStartInline(node);
  return true; // Continue to expression
}

void ASTPrinter::printNodeStart(const ASTNode *node) {
  if (needsIndent_) {
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

  needsIndent_ = !isCompactMode();
}

void ASTPrinter::printNodeEnd() {
  *output_ << ")";
  needsIndent_ = !isCompactMode();
}

void ASTPrinter::printNodeStartInline(const ASTNode *node) {
  if (shouldPrintInline(node) || isCompactMode()) {
    if (needsIndent_) {
      printIndent();
    } else if (!isFirstChild_) {
      printSpace();
    }

    *output_ << "(" << nodeKindToString(node->kind);

    // Print optional attributes inline
    if (config_.hasFlag(PrinterFlags::IncludeLocation)) {
      printLocation(node->location);
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

void ASTPrinter::printIndent() {
  if (!isCompactMode()) {
    printNewline();
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
  if (isCompactMode()) {
    return true;
  }

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

} // namespace cxy::ast
