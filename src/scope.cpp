#include <stdexcept>
#include "node.hpp"
#include "scope.hpp"

Scope *currentScope;
FunctionStack *funcStack;

void Scope::addDefinition(string name, Definition *definition) {
  if (table.find(name) != table.end()) {
    throw std::runtime_error("Definition '" + name +
                             "' previously declared as a " +
                             (table[name]->isFunction() ? "fun" : "var"));
  }
  table[name] = definition;
}

Definition * Scope::findDefinition(string name) {
  auto itr = table.find(name);
  if (itr != table.end()) {
    return (*itr).second;
  } else if (this->next != NULL) {
    return this->next->findDefinition(name);
  } else {
    return NULL;
  }
}

bool FunctionDefinition::argsMatch(NExpressionArgs *args) {
  NExpressionArgs *itr1 = args;
  NArgs *itr2 = stmt->args;

  for (; itr1 && itr2; itr1 = itr1->next, itr2 = itr2->next) {
    Type *itr1Type = itr1->expr->get_type();
    Type *itr2Type = itr2->get_type();
    if (itr1Type->cpp_type_string() != itr2Type->cpp_type_string()) {
      return false;
    }
  }
  return itr1 == NULL && itr2 == NULL;
}

void FunctionDefinition::generateCallForArgs(NExpressionArgs *args) {
  cout << stmt->id->name << '(';
  if (args) args->generate();
  cout << ')';
}

Type* VariableDefinition::get_type() {
  return type;
}

ArgumentDefinition::ArgumentDefinition(NArgs *arg)
    : VariableDefinition(arg->type), arg(arg) {
  hasGeneratedHeader = true;
}

void ArgumentDefinition::set_type(Type *type) {
  this->type = type;
  arg->type = type;
}

Type* FunctionDefinition::get_type() {
  return stmt->type;
}

class LenDefinition : public FunctionDefinition {
public:
  LenDefinition() : FunctionDefinition(NULL) {}

  virtual bool argsMatch(NExpressionArgs *args) {
    if (args == NULL && args->next != NULL) {
      return false;
    }
    // Throws up if the type has no len
    args->expr->get_type()->get_cpp_len_function();
    return true;
  }

  virtual void generateCallForArgs(NExpressionArgs *args) {
    Type *type = args->expr->get_type();
    std::cout << '(';
    args->expr->generate();
    std::cout << ')' << type->get_cpp_len_function();
  }

  virtual Type* get_type() {
    return new BasicType("int");
  }
};

class PrintDefinition : public FunctionDefinition {
public:
  PrintDefinition() : FunctionDefinition(NULL) {}

  virtual bool argsMatch(NExpressionArgs *args) {
    for (; args != NULL; args = args->next) {
      std::string type = args->expr->get_type()->cpp_type_string();
      if (type != "std::string" && type != "int") {
        return false;
      }
    }
    return true;
  }

  virtual void generateCallForArgs(NExpressionArgs *args) {
    cout << "std::cout";
    for (; args != NULL; args = args->next) {
      cout << " << ";
      args->expr->generate();
      // As per python, place spaces between the outputs
      if (args->next) {
        cout << " << ' '";
      }
    }
    cout << " << std::endl";
  }
};

class StrCastDefinition : public FunctionDefinition {
public:
  StrCastDefinition() : FunctionDefinition(NULL) {}

  virtual bool argsMatch(NExpressionArgs *args) {
    // We only expect one argument for a string cast
    if (args == NULL || args->next != NULL) return false;

    std::string type = args->expr->get_type()->cpp_type_string();
    return type == "int" || type == "std::string";
  }

  virtual void generateCallForArgs(NExpressionArgs *args) {
    std::string type = args->expr->get_type()->cpp_type_string();
    if (type == "std::string") {
      cout << "std::string(";
    } else if (type == "int") {
      cout << "std::to_string(";
    }
    args->expr->generate();
    cout << ')';
  }

  virtual Type* get_type() {
    return new StringType();
  }
};

class IntCastDefinition : public FunctionDefinition {
public:
  IntCastDefinition() : FunctionDefinition(NULL) {}

  virtual bool argsMatch(NExpressionArgs *args) {
    // We only expect one argument for a string cast
    if (args == NULL || args->next != NULL) return false;

    std::string type = args->expr->get_type()->cpp_type_string();
    return type == "int" || type == "std::string";
  }

  virtual void generateCallForArgs(NExpressionArgs *args) {
    std::string type = args->expr->get_type()->cpp_type_string();
    if (type == "std::string") {
      cout << "std::stoi(";
      args->expr->generate();
      cout << ')';
    } else if (type == "int") {
      args->expr->generate();
    }
  }

  virtual Type* get_type() {
    return new BasicType("int");
  }
};

class ExitDefinition : public FunctionDefinition {
public:
  ExitDefinition() : FunctionDefinition(NULL) {}

  virtual bool argsMatch(NExpressionArgs *args) {
    return args && args->expr && args->next == NULL &&
      args->expr->get_type()->cpp_type_string() == "int";
  }

  virtual void generateCallForArgs(NExpressionArgs *args) {
    cout << "exit(";
    args->generate(); // There will only be one of them
    cout << ')';
  }
};

class ModulusDefinition : public FunctionDefinition {
public:
  ModulusDefinition() : FunctionDefinition(NULL) {}

  virtual bool argsMatch(NExpressionArgs *args) {
    return args && args->expr && args->expr->get_type()->cpp_type_string() == "int"
      && args->next && args->next->expr &&
      args->next->expr->get_type()->cpp_type_string() == "int"
      && args->next->next == NULL;
  }

  virtual void generateCallForArgs(NExpressionArgs *args) {
    cout << "javelin::modulus(";
    args->generate(); // There will only be one of them
    cout << ')';
  }

  virtual Type* get_type() {
    return new BasicType("int");
  }
};

class RangeDefinition : public FunctionDefinition {
public:
  RangeDefinition() : FunctionDefinition(NULL) {}

  virtual bool argsMatch(NExpressionArgs *args) {
    return args && args->expr && args->expr->get_type()->cpp_type_string() == "int"
      && (args->next == NULL || args->next->expr->get_type()->cpp_type_string() == "int"
          && args->next->next == NULL);
  }

  virtual void generateCallForArgs(NExpressionArgs *args) {
    // TODO if being used in an assignment, generate a list
    throw std::runtime_error("range() does not support returning to a list.. yet");
  }

  virtual Type* get_type() {
    return new ListType(new BasicType("int"));
  }

  virtual bool hasCustomIterator() {
    return true;
  }

  virtual void generateItrCallForArgs(NIdentifier *id, NExpressionArgs *args) {
    cout << "for (int " << id->name << " = ";

    if (args->next) {
      // Double argument range
      args->expr->generate();
      args = args->next;
    } else {
      // Single argument range
      cout << '0';
    }

    cout << "; " << id->name << " < ";
    args->expr->generate();
    // Big hack to make it behave like the Python range iterable
    // Please don't ask
    cout << " || (" << id->name << "-- && false)"
         << "; " << id->name << "++) {" << endl;
  }
};

void Scope::addStandardDefinitions() {
  addDefinition("print", new PrintDefinition());
  addDefinition("exit", new ExitDefinition());
  addDefinition("str", new StrCastDefinition());
  addDefinition("int", new IntCastDefinition());
  addDefinition("javelin::modulus", new ModulusDefinition());
  addDefinition("range", new RangeDefinition());
  addDefinition("len", new LenDefinition());
}

FunctionScope::FunctionScope(Scope *next, NFunctionDeclStatement *stmt)
    : Scope(next) {
  addStandardDefinitions();

  addDefinition(stmt->id->name, new FunctionDefinition(stmt));

  NArgs *args = stmt->args;
  // Declare the args as local variables
  for (; args != NULL; args = args->next) {
    addDefinition(args->id->name, new ArgumentDefinition(args));
  }
}

Definition * FunctionScope::findDefinition(string name) {
  //return Scope::findDefinition(name);
  auto itr = table.find(name);
  if (itr != table.end()) {
    return (*itr).second;
  } else {
    return NULL;
  }
}

RootScope::RootScope() {
  addStandardDefinitions();
}
