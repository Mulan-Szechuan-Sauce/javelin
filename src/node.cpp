#include <string>

#include "type.hpp"
#include "node.hpp"

void NExpression::generate_itr_header(NIdentifier *id) {
  Type *t = get_type();
  if (t->isIndexible()) {
    t = ((ListType *)t)->get_itr_type();
  }
  // Default for header
  cout << "for (" << t->cpp_type_string() << ' ' << id->name << " : ";
  generate();
  cout << ") {" << endl;
}

NFunctionDeclStatement::
NFunctionDeclStatement(NIdentifier *id, NArgs *args, Type *type,
                       NStatement *stmt) :
    id(id), args(args), type(type), stmt(stmt) {
  scope->addDefinition(id->name, new FunctionDefinition(this));
}

NReturn::NReturn(NExpression *expr) : expr(expr) {
  if (funcStack == NULL) {
    throw std::runtime_error("Return statement outside of function");
  }

  Type *type = expr ? expr->get_type() : new VoidType();
  Type *declaredType = funcStack->get_type();

  if (declaredType->isUnset()) {
    // Type implication on first return
    NFunctionDeclStatement *funcDecl = funcStack->stmt;
    funcDecl->type = type;
    funcStack->set_type(type);
  } else {
    if (expr == NULL && ! declaredType->isVoid()) {
      throw std::runtime_error("Cannot return a value on a void function");
    } else if (expr != NULL) {
      expr->set_type(declaredType);
    }
  }
}

NFunctionCallExpression::
NFunctionCallExpression(NIdentifier *id, NExpressionArgs *args):
    id(id), args(args) {
  Definition *def = scope->findDefinition(id->name);
  if (def == NULL) {
    throw std::runtime_error(id->name + " function is undefined");
  }
  if ( ! def->isFunction()) {
    throw std::runtime_error(id->name + " is not defined as a function");
  }
  if ( ! ((FunctionDefinition *)def)->argsMatch(args)) {
    throw std::runtime_error("Type mismatch");
  }
}

Type* NFunctionCallExpression::get_type() {
  Definition *d = scope->findDefinition(id->name);
  if ( ! d->isFunction()) {
    throw std::runtime_error(id->name + " is not a function");
  }
  return ((FunctionDefinition *)d)->get_type();
}

NAssignment::
NAssignment(NIdentifier *lhs, NExpression *rhs) : lhs(lhs), rhs(rhs) {
  Type *type = rhs->get_type();
  Definition *def = scope->findDefinition(lhs->name);
  VariableDefinition *vdef = (VariableDefinition *)def;

  // FIXME accomodate conflicting function & class names

  if (def == NULL) {
    scope->addDefinition(lhs->name, new VariableDefinition(type));
  } else if (vdef->get_type()->isUnset()) {
    // For argument type implication
    lhs->set_type(type);
    vdef->set_type(type);
  // Already defined as a var
  } else if (type->cpp_type_string() != vdef->type->cpp_type_string()) {
    throw std::runtime_error(lhs->name +
                             " was previously declared as a '" +
                             vdef->type->cpp_type_string() + "'");
  }
}

void NAssignment::generate(int level) {
  NStatement::generate(level);

  // Only declare the type if it's not a root statement
  // Otherwise, a header is declared at the top of the file
  // Also, don't declare the type if the variable is already declared
  Definition *def = scope->findDefinition(lhs->name);
  VariableDefinition *vdef = (VariableDefinition *)def;

  if (! vdef->hasGeneratedHeader) {
    // Earthquake, ignore it
    cout << lhs->get_type()->cpp_type_string() << " ";
                vdef->hasGeneratedHeader = true;
  }
  lhs->generate();
  cout << " = ";
  rhs->generate();
  cout << ";" << endl;
}

Type* NIdentifier::get_type() {
  auto vd = (VariableDefinition *)scope->findDefinition(name);
  if ( ! vd) throw std::runtime_error("Type for " + name + " not found");
  return vd->type;
}

void NIdentifier::set_type(Type *type) {
  auto vd = (VariableDefinition *)scope->findDefinition(name);
  if (vd->get_type()->isUnset()) {
    vd->set_type(type);
  } else if (vd->get_type()->cpp_type_string() != type->cpp_type_string()) {
    throw std::runtime_error(name + " is already a "
                             + vd->type->cpp_type_string());
  }
}

void NFunctionCallExpression::generate() {
  // This will exist, since we check its existance in the constructor
  FunctionDefinition *def = (FunctionDefinition *)scope->findDefinition(id->name);
  def->generateCallForArgs(args);
}

void NFunctionCallExpression::generate_itr_header(NIdentifier *id) {
  FunctionDefinition *def = (FunctionDefinition *)scope->findDefinition(this->id->name);
  if (def->hasCustomIterator()) {
    def->generateItrCallForArgs(id, args);
  } else {
    NExpression::generate_itr_header(id);
  }
}

NForStatement::NForStatement(NIdentifier *itr_name, NExpression *iterable, NStatement *stmt) :
    itr_name(itr_name), iterable(iterable), stmt(stmt) {
  Type *itr_type = iterable->get_type()->get_itr_type();
  scope->addDefinition(itr_name->name, new VariableDefinition(itr_type));
}

NList::NList(NExpressionArgs *contents) : contents(contents) {
  // Validate the type of the list - for now, you must have at least one element.
  if (contents == NULL) {
    throw std::runtime_error("No type associated with list declaration");
  } else {
    type = new ListType(contents->expr->get_type());
  }

  Type *itr_type = type->get_itr_type();
  for (contents = contents->next; contents != NULL; contents = contents->next) {
    Type *content_type = contents->expr->get_type();
    if (content_type->cpp_type_string() != itr_type->cpp_type_string()) {
      throw std::runtime_error("Type mismatch in " + itr_type->cpp_type_string() + " list");
    }
  }
}

void NList::generate() {
  cout << '{';
  if (contents != NULL) contents->generate();
  cout << '}';
}

string NOpType_str(NOpType t) {
  switch (t) {
  case N_LT:  return "<";
  case N_GT:  return ">";
  case N_EQ:  return "==";
  case N_NEQ: return "!=";
  case N_GTE: return ">=";
  case N_LTE: return "<=";
  case N_NOT: return "!";
  case N_AND: return "&&";
  case N_OR:  return "||";
  case N_ADD: return "+";
  case N_SUB: return "-";
  case N_MUL: return "*";
  case N_DIV: return "/";
  case N_SL: return "<<";
  case N_SR: return ">>";
  case N_BA: return "&";
  case N_BO: return "|";
  case N_BN: return "~";
  case N_BX: return "^";
  default:    return "<type string undeclared>";
  }
}
