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
  } val;
  SysCallParameter(SystemCallCode t):type(t) { }
  void setInt(int i) { val.ival = i; }
  void setFloat(double d) { val.fval = d; }
};

struct GlobalVariable {
    // type can only be INT_TYPE, FLOAT_TYPE or CONST_STRING_TYPE
    enum DATA_TYPE type;
    const char* idName; // string doesn't use the idName
    union {
        int ival;
        double fval;
        const char* str;
    } init;
    GlobalVariable(DATA_TYPE t): type(t), idName(NULL) { }
    DATA_TYPE type() { return type; }
    void setInt(int i) { init.ival = i; }
    void setFloat(double d) { init.fval = d; }
    void setString(const char* s) { init.str = s; }
    void setIdName(const char* id) { idName = id; }
    int getInt() { return init.ival; }
    double getFloat() { return init.fval; }
    const char* getString() { return init.str; }
    const char* getId() { return idName; }
};

int gen_head(const char *name);
void gen_prologue(const char *functionName);
void gen_epilogue(const char *functionName, int offset);
void genGlobalVariableWithInit(const GlobalVariable* g);
int getReg();
void genSysCall(const SysCallParameter* information);
