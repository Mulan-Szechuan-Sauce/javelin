#pragma once 

#include <string>
#include <stdexcept>
using std::string;

class Type {
public:
  virtual string cpp_type_string() = 0;
  virtual Type* get_itr_type() {
    throw std::runtime_error("A " + cpp_type_string() + " is not iterable");
  }
  virtual bool isUnset() { return false; }
  virtual bool isVoid() { return false; }
  virtual bool isIndexible() { return false; }
  virtual bool isIndAssignible() { return false; }
  virtual std::string get_cpp_len_function() {
    throw std::runtime_error("A " + cpp_type_string() + " has no length");
  }
};

class UnsetType : public Type {
public:
  virtual bool isUnset() { return true; }
  virtual string cpp_type_string() {
    throw std::runtime_error("Undeclared type");
  }
};

class VoidType : public Type {
public:
  virtual string cpp_type_string() {
    return "void";
  }
  virtual bool isVoid() { return true; }
};

class BasicType : public Type {
  string type;
public:
  BasicType(string type) : type(type) {}
  virtual string cpp_type_string() {
    return type;
  };
};

class StringType : public Type {
public:
  StringType() {}
  
  virtual string cpp_type_string() {
    return "std::string";
  };

  virtual std::string get_cpp_len_function() {
    return ".length()";
  }
  virtual Type* get_itr_type() {
    return new StringType();
  };
  virtual bool isIndexible() { return true; }
};

class ListType : public Type {
  Type *itr_type;
public:
  ListType(Type *itr_type) : itr_type(itr_type) {}

  virtual string cpp_type_string() {
    return "std::vector<" + itr_type->cpp_type_string() + ">";
  };

  virtual Type* get_itr_type() {
    return itr_type;
  };

  virtual std::string get_cpp_len_function() {
    return ".length()";
  }
  virtual bool isIndexible() { return true; }
  virtual bool isIndAssignible() { return true; }
};
