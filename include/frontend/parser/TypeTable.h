#ifndef OBW_TYPETABLE_H
#define OBW_TYPETABLE_H

#include <string>
#include <vector>

enum BasicTypes {
  TKIND_INT,
  TKIND_REAL,
  TKIND_BOOL,
  TKIND_STRING,
  TKIND_LIST,
  TKIND_ARRAY,
  TKIND_FUNCTION,
  TKIND_NULL
};

/**
* Defined field in a new type/class
*/
struct TypeField {
  std::string name;
  int fieldIndex;
};

/**
* One record in a TypeTable
*/
struct TypeRecord {
  std::string name;               // class name
  std::vector<TypeField> fields;  // class fields

};

/**
* Info about newly defined types
* a.k.a. classes (in OBW context)
*/
class TypeTable {

};

#endif
