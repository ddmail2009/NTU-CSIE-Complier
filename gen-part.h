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
  union {
    int ival;
    double fval;
  } val;
  SysCallParameter(SystemCallCode t):type(t) { }
};

int gen_head(const char *name);
void gen_prologue(const char *functionName);
void gen_epilogue(const char *functionName, int offset);
void genVariableWithInit(bool first, C_type c, const char* name, int a, double d);
void genStringLiteral(const char* str);
int getReg();
void genSysCall(const SysCallParameter* information);
