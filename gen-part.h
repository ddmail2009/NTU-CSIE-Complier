#ifndef _Gen_Part_H
#define _Gen_Part_H

enum SystemCallCode {
  PRINT_INT = 1, // $a0 = integer
  PRINT_FLOAT,   // $f12 = float
  PRINT_DOUBLE,  // $f12 = double
  PRINT_STRING,  // $a0 = string
  READ_INT,      // integer (in $v0)
  READ_FLOAT,    // float (in $f0)
  READ_DOUBLE,   // double (in $f0)
  READ_STRING,   // $a0 = buffer, $a1 = length
  SBRK,          // $a0 = amount     address(in $v0)
  EXIT,
};

// this enum does nothing
enum ReadSysCallType {
  INT,
  //FLOAT, // C-- only have float, but we use double instead
  DOUBLE,
  STRING,
  CHAR
};

struct SysCallParameter {
  enum SystemCallCode type;
  /* for print system service */
  union {
    int ival;
    double fval;
    const char* str;
  } val;
  SysCallParameter(SystemCallCode t):type(t) { }
  SysCallParameter(SystemCallCode t, int i): type(t) { val.ival = i; }
  SysCallParameter(SystemCallCode t, double f): type(t) { val.fval = f; }
  SysCallParameter(SystemCallCode t, const char *str): type(t) { val.str = str; }
  void setInt(int i) { val.ival = i; }
  void setFloat(double d) { val.fval = d; }
};

struct Variable {
    // type can only be INT_TYPE, FLOAT_TYPE or CONST_STRING_TYPE
    DATA_TYPE _type;
    const char* idName; // string doesn't use the idName
    union {
        int ival;
        double fval;
        const char* str;
    } init;
    Variable(DATA_TYPE t): _type(t), idName(NULL) { }
    Variable(DATA_TYPE t, const char* str): _type(t), idName(str) { }
    DATA_TYPE type() const { return _type; }
    void setInt(int i) { init.ival = i; }
    void setFloat(double d) { init.fval = d; }
    void setString(const char* s) { init.str = s; }
    void setIdName(const char* id) { idName = id; }
    int getInt() const { return init.ival; }
    double getFloat() const { return init.fval; }
    const char* getString() const { return init.str; }
    const char* getId() const { return idName; }
};

int gen_head(const char *name);
void gen_prologue(const char *functionName);
void gen_epilogue(const char *functionName, int offset);
void genGlobalVariableWithInit(const Variable* g);
void genStackVariableWithInit(const SymbolTableEntry* entry, const Variable& var);
void genSysCall(const SysCallParameter& information);

void genConStmt(AST_NODE *node);
void genOpStmt(AST_NODE *node);
void genVariable(AST_NODE *node);
#endif
