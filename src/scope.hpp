#pragma once 

#include <unordered_map>
#include <iostream>
#include "node.hpp"

using std::string;
using std::cout;
using std::endl;

class NIdentifier;
class NExpressionArgs;
class NArgs;

// A definition of a variable, function, or (in the future) class
class Definition {
public:
  virtual bool isFunction() { return false; }
  virtual bool isVariable() { return false; }
  virtual bool isArgument() { return false; }
  virtual Type* get_type() = 0;
};

class FunctionDefinition : public Definition {
public:
  NFunctionDeclStatement *stmt;
  FunctionDefinition(NFunctionDeclStatement *stmt) : stmt(stmt) {}
  virtual bool isFunction() { return true; }
  virtual bool argsMatch(NExpressionArgs *args);
  virtual void generateCallForArgs(NExpressionArgs *args);
  virtual bool hasCustomIterator() { return false; }
  virtual void generateItrCallForArgs(NIdentifier *id, NExpressionArgs *args) {
    throw std::runtime_error("This function is non-iterable");
  }

  virtual Type* get_type();
};

class VariableDefinition : public Definition {
public:
  Type *type;
  bool hasGeneratedHeader;

  VariableDefinition(Type *type) : type(type), hasGeneratedHeader(false) {}
  virtual bool isVariable() { return true; }
  virtual Type* get_type();
  virtual void set_type(Type *type) {
    this->type = type;
  }
};

// So we can modify the argument signature within a function block
class ArgumentDefinition : public VariableDefinition {
public:
  NArgs *arg;
  
  ArgumentDefinition(NArgs *arg);
  virtual void set_type(Type *type);

  virtual bool isVariable() { return true; }
  virtual bool isArgument() { return false; }
};

/**
 * Scope table stack
 * The head will be the top scope, tail will be the root scope
 */
class Scope {
protected:
  std::unordered_map<string, Definition *> table;
  void addStandardDefinitions();
public:
  Scope *next;

  Scope() {}
  Scope(Scope *next) : next(next) {}

  // Add a new definition to this scope table
  void addDefinition(string name, Definition *definition);

  // Recursively find a definition in this, or a parent scope
  virtual Definition * findDefinition(string name);

  // Compute the depth of the current scope
  int depth() {
    return next ? 1 + next->depth() : 0;
  }
};

class FunctionScope : public Scope {
public:
  FunctionScope();
  FunctionScope(Scope *next, NFunctionDeclStatement *stmt);

  // Doesn't search parent scopes, but does search a root scope
  virtual Definition * findDefinition(string name);
};

// Handy root scope class to handle built in functions
class RootScope : public Scope {
public:
  RootScope();
};

// To store a stack of the current functions. 
class FunctionStack {
public:
  FunctionStack *parent;
  int level;
  Type *type;
  NFunctionDeclStatement *stmt;

  // Specify a null stmt for main()
  FunctionStack(int level, FunctionStack *parent, Type *type)
    : level(level), parent(parent), type(type) {}

  virtual Type* get_type() {
    return type;
  }
  
  virtual Type* set_type(Type *type) {
    this->type = type;
  }
};

/*
 * The top scope - it will change when the parser enters a new block
 * Statements and expressions latch on to this, but it "moves up and down"
 */
extern Scope *currentScope;
extern FunctionStack *funcStack;
