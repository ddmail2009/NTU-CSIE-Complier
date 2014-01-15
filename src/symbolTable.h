#ifndef __SYMBOL_TABLE_H__
#define __SYMBOL_TABLE_H__
// This file is for reference only, you are not required to follow the implementation. //

#include "header.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "Register.h"
//SYMBOL_TABLE_PREINSERT_NAME
#define SYMBOL_TABLE_INT_NAME "int"
#define SYMBOL_TABLE_FLOAT_NAME "float"
#define SYMBOL_TABLE_VOID_NAME "void"
#define SYMBOL_TABLE_SYS_LIB_READ "read"
#define SYMBOL_TABLE_SYS_LIB_FREAD "fread"
#define HASH_TABLE_SIZE 256

enum SymbolAttributeKind {
  VARIABLE_ATTRIBUTE,
  TYPE_ATTRIBUTE,
  FUNCTION_SIGNATURE
};

enum TypeDescriptorKind {
  SCALAR_TYPE_DESCRIPTOR,
  ARRAY_TYPE_DESCRIPTOR,
};

class ArrayProperties{
  public:
    int dimension;
    int sizeInEachDimension[MAX_ARRAY_DIMENSION];
    DATA_TYPE elementType;

    /* cannot use constructor because ArrayProperties is in union */
    void init() {
      dimension = 0;
      for(int i = 0; i < MAX_ARRAY_DIMENSION; i++)
        sizeInEachDimension[i] = -1;
    }

    bool isCompatible(const ArrayProperties p, bool ignoreFirstDimSize = true) const{
      DATA_TYPE a = elementType, b = p.elementType;
      if(!(a == b || (a == INT_TYPE && b == FLOAT_TYPE) || (a == FLOAT_TYPE && b == INT_TYPE)))
        return false;
      if(dimension != p.dimension) return false;

      for(int i= ignoreFirstDimSize ? 1 : 0; i<dimension; i++)
        if(sizeInEachDimension[i] != p.sizeInEachDimension[i]) return false;
      return true;
    }
};

class TypeDescriptor {
  public:
    TypeDescriptor(TypeDescriptorKind typeKind, DATA_TYPE type);
    DATA_TYPE getDataType() const;
    void addDimension(int sizeInDimension);
    void addMissDimension();
    int getDimension() const;
    int getArrayDimensionSize(int idx) const;
    bool getMissDimension() const;
    bool isCompatible(const TypeDescriptor &b) const;
    void getSubArrayProperty(int dimension, TypeDescriptor **temp) const;
    TypeDescriptorKind getKind() const;
 

    TypeDescriptorKind kind;
    union {
      DATA_TYPE dataType;//kind: SCALAR_TYPE_DESCRIPTOR
      ArrayProperties arrayProperties;//kind: ARRAY_TYPE_DESCRIPTOR
    } properties;

    int size() const {
        if(getKind() == SCALAR_TYPE_DESCRIPTOR) return 4;
        else{
            int size = 1;
            for(int i=0; i<getDimension(); i++)
                size *= getArrayDimensionSize(i);
            return (size*4+3)/4*4;
        }
    }
};

struct Parameter {
  struct Parameter* next;
  TypeDescriptor* type;
  char* parameterName;

  Parameter(const char *name, TypeDescriptor *type){
    next = NULL;
    this->type = type;
    parameterName = new char[strlen(name)];
    strcpy(parameterName, name);
  }

  const char *getParamName(){
    return parameterName;
  }
};

class FunctionSignature {
  public:
    FunctionSignature(DATA_TYPE type): returnType(type){
      parametersCount = 0;
      parameterList = NULL;
    }

    void addParameter(Parameter *p){
      parametersCount += 1;
      Parameter *iter = parameterList;

      while(iter && iter->next != NULL) iter = iter->next;

      if(iter) iter->next = p;
      else parameterList = p;
    }

    DATA_TYPE getReturnType() const {
      return returnType;
    }

    int getParameterCount() const{
      return parametersCount;
    }

    Parameter * getParams() const{
      return parameterList;
    }

    void print() const{
      Parameter *iter = parameterList;
      while(iter){
        printf("Name[%s]type[%d], ", iter->parameterName, iter->type->getDataType());
        iter = iter->next;
      }
      puts("");
    }


    int parametersCount;
    Parameter *parameterList;
    DATA_TYPE returnType;
};

class SymbolAttribute {
  public:
    SymbolAttribute(SymbolAttributeKind attrkind, TypeDescriptor* t) {
      attributeKind = attrkind;
      assert(attributeKind == VARIABLE_ATTRIBUTE || attributeKind == TYPE_ATTRIBUTE);
      attr.typeDescriptor = t;
    }

    SymbolAttribute(SymbolAttributeKind attrkind, FunctionSignature *f) {
      attributeKind = attrkind;
      assert(attributeKind == FUNCTION_SIGNATURE);
      attr.functionSignature = f;
    }

    TypeDescriptor *getTypeDes(){
      assert(attributeKind == VARIABLE_ATTRIBUTE || attributeKind == TYPE_ATTRIBUTE);
      return attr.typeDescriptor;
    }

    FunctionSignature* getFuncSig(){
      assert(attributeKind == FUNCTION_SIGNATURE);
      return attr.functionSignature;
    }

    SymbolAttributeKind getKind(){
      return attributeKind;
    }

    DATA_TYPE getDataType(){
      switch(attributeKind){
        case VARIABLE_ATTRIBUTE: case TYPE_ATTRIBUTE:
          return getTypeDes()->getDataType();
        case FUNCTION_SIGNATURE:
          return getFuncSig()->getReturnType();
        default:
          printf("Unknown attributeKind\n");
      }
      return NONE_TYPE;
    }


    SymbolAttributeKind attributeKind;
    union {
      TypeDescriptor* typeDescriptor;
      FunctionSignature* functionSignature;
    } attr;
};

class SymbolTableEntry {
  public:
    struct SymbolTableEntry* nextInHashChain;
    struct SymbolTableEntry* prevInHashChain;
    struct SymbolTableEntry* nextInSameLevel;
    struct SymbolTableEntry* sameNameInOuterLevel;
    char name[1024];
    SymbolAttribute* attribute;
    int nestingLevel;
    // offset is for local variable allocation $fp + offset
    // address contains offset and the base register
    const Address *address;

    SymbolTableEntry(int level, const char *n = NULL, SymbolAttribute *attr = NULL):
        nextInHashChain(NULL),
        prevInHashChain(NULL),
        nextInSameLevel(NULL),
        sameNameInOuterLevel(NULL),
        attribute(attr),
        nestingLevel(level),
        address(NULL){
            assert(n != NULL);
            strcpy(name, n);
    }
private:
    int getArrayOffset(AST_NODE *node){
        int offset = 0;
        if(node->getIDKind() == ARRAY_ID){
            TypeDescriptor *type = node->getSymbol()->attribute->getTypeDes();
            AST_NODE *dim = node->child;
            int dimension = 0;
            while(dim){
                offset += dim->getConIntValue();
                dim = dim->rightSibling;
                if(!dim) break;

                offset += type->getArrayDimensionSize(dimension);
                dimension += 1;
            }
        }
        return offset;
    }
};

struct SymbolTable {
  SymbolTableEntry* hashTable[HASH_TABLE_SIZE];
  SymbolTableEntry** scopeDisplay;
  int currentLevel;
  int scopeDisplayElementCount;
};


void initializeSymbolTable();
void symbolTableEnd();
SymbolTableEntry* retrieveSymbol(char* symbolName);
SymbolTableEntry* enterSymbol(char* symbolName, SymbolAttribute* attribute);
void removeSymbol(char* symbolName);
int declaredLocally(char* symbolName);
void openScope();
void closeScope();

#endif
