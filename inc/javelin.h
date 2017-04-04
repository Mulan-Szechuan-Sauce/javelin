#include <iostream>
#include <string>
#include <vector>
#include <iterator>
namespace javelin {
  int modulus (int a, int b) {
    // To make modulus behave the same was as Python3 modulus
    return ((a % b) + b) % b;
  }

  // To allow iterating strings as strings, instead of chars
  class string_itr {
    std::string val;

    class iterator : public std::iterator<std::input_iterator_tag, std::string, std::string, const std::string*, std::string> {
      std::string val;
      size_t index;
    public:
      iterator(std::string val, size_t index) : val(val), index(index) {}
      iterator& operator++() {
        index++;
        return *this;
      }
      iterator operator++(int) {
        iterator retval = *this;
        ++(*this);
        return retval;
      }
      bool operator==(iterator other) const {
        return index == other.index;
      }
      bool operator!=(iterator other) const {
        return !(*this == other);
      }
      reference operator*() const {
        return std::string(1, val[index]);
      }
    };

  public:

    string_itr(std::string val) : val(val) {}

    iterator begin() {
      return iterator(val, 0);
    }
    iterator end() {
      return iterator(val, val.length());
    }
  };
};
