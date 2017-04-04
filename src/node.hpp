#pragma once 

#include <iostream>
#include <vector>
#include <stdexcept>
#include <unordered_map>
#include <cstring>

class NStatement;
class NExpression;
class NFunctionDeclStatement;
class Scope;
class Type;

extern Scope *currentScope;

#include "scope.hpp"
#include "type.hpp"

using std::cout;
using std::endl;
using std::string;

// Enum to denote operator types
typedef enum {
  N_LT,
  N_GT,
  N_EQ,
  N_NEQ,
  N_GTE,
  N_LTE,
  N_NOT,
  N_OR,
  N_AND,
  N_ADD,
  N_SUB,
  N_MUL,
  N_DIV,
  N_SL,
  N_SR,
  N_BA,
  N_BO,
  N_BX,
  N_BN,
} NOpType;

string NOpType_str(NOpType t);

class NStatement {
public:
  Scope *scope;

  NStatement() {
    scope = currentScope;
  }

  void printIndent(int level) {
    for (int i = 0; i < level; i++) {
      cout << "  ";
    }
  }

  virtual void generate(int level) {
    printIndent(level);
  }

  virtual void addToRootStmts() {
    extern std::vector<NStatement*> rootStmts;
    rootStmts.push_back(this);
  }
};

// A block ks defined as a collection of statements (inside curly braces)
class NBlock : public NStatement {
public:
  NStatement *stmt;
  NBlock *next;
  
  NBlock(NStatement *stmt, NBlock *next) : stmt(stmt), next(next) { }

  virtual void generate(int level) {
    stmt->generate(level);
    if (next) {
      next->generate(level);
    }
  }
};

class NExpression {
public:
  Scope *scope;

  NExpression() {
    scope = currentScope;
  }

  virtual Type* get_type() = 0;
  virtual void set_type(Type *type) {
    if (get_type()->cpp_type_string() != type->cpp_type_string()) {
      throw std::runtime_error("Cannot set this expression to a "
                               + type->cpp_type_string());
    }
  }
  virtual void generate() = 0;
  virtual void generate_itr_header(NIdentifier *id);
};

// Some expressions can be standalone statements (e.g. function calls)
class NExpressionStatement : public NStatement {
public:
  NExpression *expr;

  NExpressionStatement(NExpression *expr) : expr(expr), NStatement() {}
    
  virtual void generate(int level) {
    NStatement::generate(level);
    expr->generate();
    cout << ";\n";
  }
};

class NPassStatement : public NStatement {
public:
  NPassStatement() {}

  virtual void generate(int level) {
    // pass does nothing :)
  }
};

class NBreakStatement : public NStatement {
public:
  NBreakStatement() {}

  virtual void generate(int level) {
    NStatement::generate(level);
    cout << "break;\n";
  }
};

class NContinueStatement : public NStatement {
public:
  NContinueStatement() {}

  virtual void generate(int level) {
    NStatement::generate(level);
    cout << "continue;\n";
  }
};

class NIdentifier : public NExpression {
public:
  string name;
  NIdentifier(const string& name) : name(name) { }

  virtual void generate() {
    cout << name;
  }

  virtual Type* get_type();
  virtual void set_type(Type *type);
};

class NInteger : public NExpression {
public:
  long long value;
  BasicType *type;
  NInteger(long long value) : value(value) { type = new BasicType("int"); }

  Type* get_type() {
    return type; 
  }

  virtual void generate() {
    cout << value;
  }
};

class NString : public NExpression {
public:
  string value;
  StringType *type;
  NString(const char *value) {
    this->value = string(value + 1, strlen(value) - 2);
    type = new StringType();
  }

  Type* get_type() {
    return type;
  }

  virtual void generate() {
    cout << "std::string(\"" << value << "\")";
  }
  
  virtual void generate_itr_header(NIdentifier *id) {
    cout << "for (std::string " << id->name << " : ";
    cout << "javelin::string_itr(";
    generate();
    cout << ")) {" << endl;
  }
};

class NBinaryOperator : public NExpression {
public:
  // Kk here. This has no type. Only the sides have types
  NOpType op;
  NExpression *lhs;
  NExpression *rhs;
  NBinaryOperator(NExpression *lhs, NOpType op, NExpression *rhs) :
    lhs(lhs), rhs(rhs), op(op) { }

  Type* get_type() {
    Type *lhs_type = lhs->get_type();
    Type *rhs_type;

    // Propogate the types, for implicit typing
    if (lhs_type->isUnset()) {
      rhs_type = lhs_type = rhs->get_type();
      lhs->set_type(rhs_type);
    } else {
      rhs->set_type(lhs_type);
      rhs_type = lhs_type;
    }
    return lhs_type;
  }

  void set_type(Type *type) {
    lhs->set_type(type);
    rhs->set_type(type);
  }

  virtual void generate() {
    lhs->generate();
    cout << " " << NOpType_str(op) << " ";
    rhs->generate();
  }
};

class NUnaryOperator : public NExpression {
public:
  NOpType op;
  NExpression *rhs;
  NUnaryOperator(NOpType op, NExpression *rhs) : rhs(rhs), op(op) { }

  Type* get_type() {
    return rhs->get_type();
  }

  virtual void generate() {
    cout << NOpType_str(op);
    rhs->generate();
  }
};

class NAssignment : public NStatement {
public:
  NIdentifier *lhs;
  NExpression *rhs;
  NAssignment(NIdentifier *lhs, NExpression *rhs);

  virtual void generate(int level);

  virtual void addToRootStmts() {
    // We want it in both lists!
    NStatement::addToRootStmts();

    extern std::vector<NAssignment*> rootAssignStmts;
    rootAssignStmts.push_back(this);
  }
};

class NWhileStatement : public NStatement {
public:
  NExpression *expr;
  NStatement *stmt;

  NWhileStatement(NExpression *expr, NStatement *stmt) :
    expr(expr), stmt(stmt) {}
  
  virtual void generate(int level) {
    NStatement::generate(level);
    cout << "while (";
    expr->generate();
    cout << ") {" << endl;
    stmt->generate(level + 1);
    printIndent(level);
    cout << "}" << endl;
  }
};

class NForStatement : public NStatement {
public:
  NIdentifier *itr_name;
  NExpression *iterable;
  NStatement *stmt;

  NForStatement(NIdentifier *itr_name, NExpression *iterable, NStatement *stmt);
  
  virtual void generate(int level) {
    NStatement::generate(level);
    Type *itr_type = iterable->get_type()->get_itr_type();

    iterable->generate_itr_header(itr_name);
    stmt->generate(level + 1);
    printIndent(level);
    cout << "}" << endl;
  }
};

class NElseStatement : public NStatement {
public:
  NStatement *stmt;

  NElseStatement(NStatement *stmt) : stmt(stmt) {}

  virtual void generate(int level) {
    NStatement::generate(level);
    cout << "else {" << endl;
    stmt->generate(level + 1);
    printIndent(level);
    cout << "}" << endl;
  }
};

class NElifStatement : public NStatement {
public:
  NExpression *expr;
  NStatement *stmt;

  // At most, _one_ of these should be non-null
  NElifStatement *elifStmt;
  NElseStatement *elseStmt;

  NElifStatement(NExpression *expr, NStatement *stmt,
                 NElifStatement *elifStmt, NElseStatement *elseStmt) :
    expr(expr), stmt(stmt), elifStmt(elifStmt), elseStmt(elseStmt) {}

  virtual void generate(int level) {
    NStatement::generate(level);
    cout << "else if (";
    expr->generate();
    cout << ") {" << endl;
    stmt->generate(level + 1);
    printIndent(level);
    cout << "}" << endl;

    if (elifStmt) {
      elifStmt->generate(level);
    } else if (elseStmt) { // huhu
      elseStmt->generate(level);
    }
  }
};

class NIfStatement : public NStatement {
public:
  NExpression *expr;
  NStatement *stmt;

  // At most, _one_ of these should be non-null
  NElifStatement *elifStmt;
  NElseStatement *elseStmt;

  NIfStatement(NExpression *expr, NStatement *stmt,
               NElifStatement *elifStmt, NElseStatement *elseStmt) :
    expr(expr), stmt(stmt), elifStmt(elifStmt), elseStmt(elseStmt) {}

  virtual void generate(int level) {
    NStatement::generate(level);
    cout << "if (";
    expr->generate();
    cout << ") {" << endl;
    stmt->generate(level + 1);
    printIndent(level);
    cout << "}" << endl;

    if (elifStmt) {
      elifStmt->generate(level);
    } else if (elseStmt) {
      elseStmt->generate(level);
    }
  }
};

class NArgs {
public:
  NIdentifier *id;
  NArgs *next;
  Type *type;

  NArgs(NIdentifier *id, Type *type, NArgs *next)
    : id(id), type(type), next(next) {}

  virtual Type* get_type() {
    return type->isUnset() ? id->get_type() : type;
  }

  virtual void generate() {
    cout << get_type()->cpp_type_string() << " " << id->name;

    if (next) {
      cout << ',';
      next->generate();
    }
  }
};

class NExpressionArgs {
public:
  NExpression *expr;
  NExpressionArgs *next;

  NExpressionArgs(NExpression *expr, NExpressionArgs *next) :
    expr(expr), next(next) {}

  void generate() {
    expr->generate();
    if (next) {
      cout << ',';
      next->generate();
    }
  }
};

// Homogenously typed list (a restriction of our Python subset)
class NList : public NExpression {
public:
  NExpressionArgs *contents;
  ListType *type;

  NList(NExpressionArgs *contents);

  Type* get_type() {
    return type;
  }

  virtual void generate();
  virtual void generate_itr_header(NIdentifier *id) {
    Type *t = type->get_itr_type();
    cout << "for (" << t->cpp_type_string() << ' ' << id->name << " : ";
    generate();
    cout << ") {" << endl;
  }
};

class NListIndex : public NExpression {
public:
  NExpression *list_expr;
  NExpression *index;

  NListIndex(NExpression *list_expr, NExpression *index) :
      list_expr(list_expr), index(index) { }

  virtual Type* get_type() {
    return list_expr->get_type()->get_itr_type();
  }

  virtual void generate() {
    list_expr->generate();
    cout << "[";
    index->generate();
    cout << "]";
  }
};

class NFunctionDeclStatement : public NStatement {
public:
  NIdentifier *id;
  NArgs *args;
  Type *type;
  NStatement *stmt;

  NFunctionDeclStatement(NIdentifier *id, NArgs *args, Type *type,
                         NStatement *stmt);

  virtual void generate(int level) {
    NStatement::generate(level);

    if (level == 0) {
      cout << type->cpp_type_string() << ' ' << id->name << '(';
      if (args) args->generate();
      cout << ")";
    } else {
      // Lambdas don't capture by reference, by default.
      cout << "auto " << id->name << " = [] (";
      if (args) args->generate();
      cout << ")";
      if (! type->isVoid()) {
        cout << " -> " << type->cpp_type_string() << ' ';
      }
    }

    cout << " {\n";
    stmt->generate(level + 1);
    printIndent(level);
    cout << "}";
    if (level > 0) cout << ';'; // For lambdas
    cout << endl;
  }

  // Generate the header - generally at the top of the file
  void generateHeader() {
    NStatement::generate(0);

    cout << type->cpp_type_string() << ' ' << id->name << '(';
    if (args) args->generate();
    cout << ");\n";
  }

  virtual void addToRootStmts() {
    // Don't store it in the rootStmts.. we treat functions specially
    extern std::vector<NFunctionDeclStatement*> rootFuncStmts;
    rootFuncStmts.push_back(this);
  }
};

class NReturn : public NStatement {
public:
  NExpression *expr;

  NReturn(NExpression *expr);

  virtual void generate(int level) {
    NStatement::generate(level);
    cout << "return ";
    if (expr) expr->generate();
    cout << ';' << endl;
  }
};

class NFunctionCallExpression : public NExpression {
public:
  NIdentifier *id;
  NExpressionArgs *args;

  NFunctionCallExpression(NIdentifier *id, NExpressionArgs *args);

  virtual Type* get_type();
  virtual void generate();
  virtual void generate_itr_header(NIdentifier *id);
};
