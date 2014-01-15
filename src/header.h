#ifndef __HEADER_H__
#define __HEADER_H__

#include <assert.h>
#include <stdio.h>
#include <string.h>
#define MAX_ARRAY_DIMENSION 10

typedef enum DATA_TYPE{
    INT_TYPE,
    FLOAT_TYPE,
    VOID_TYPE,
    INT_PTR_TYPE,//for parameter passing
    FLOAT_PTR_TYPE,//for parameter passing
    CONST_STRING_TYPE,//for "const string"
    NONE_TYPE,//for nodes like PROGRAM_NODE which has no type
    ERROR_TYPE
} DATA_TYPE;

typedef enum IDENTIFIER_KIND{
    NORMAL_ID, //function Name, uninitialized scalar variable
    ARRAY_ID, //ID_NODE->child = dim
    WITH_INIT_ID, //ID_NODE->child = initial value
} IDENTIFIER_KIND;

typedef enum BINARY_OPERATOR{
    BINARY_OP_ADD,
    BINARY_OP_SUB,
    BINARY_OP_MUL,
    BINARY_OP_DIV,
    BINARY_OP_EQ,
    BINARY_OP_GE,
    BINARY_OP_LE,
    BINARY_OP_NE,
    BINARY_OP_GT,
    BINARY_OP_LT,
    BINARY_OP_AND,
    BINARY_OP_OR
} BINARY_OPERATOR;

typedef enum UNARY_OPERATOR{
    UNARY_OP_POSITIVE,
    UNARY_OP_NEGATIVE,
    UNARY_OP_LOGICAL_NEGATION
} UNARY_OPERATOR;

//C_type= type of constant ex: 1, 3.3, "const string"
//do not modify, or lexer might break
typedef enum C_type {
    INTEGERC,
    FLOATC,
    STRINGC
} C_type;

typedef enum STMT_KIND{
    WHILE_STMT,
    FOR_STMT,
    ASSIGN_STMT, //TODO:for simpler implementation, assign_expr also uses this
    IF_STMT,
    FUNCTION_CALL_STMT,
    RETURN_STMT,
} STMT_KIND;

typedef enum EXPR_KIND{
    BINARY_OPERATION,
    UNARY_OPERATION
} EXPR_KIND;

typedef enum DECL_KIND{
    VARIABLE_DECL,
    TYPE_DECL,
    FUNCTION_DECL,
    FUNCTION_PARAMETER_DECL
} DECL_KIND;

typedef enum AST_TYPE{
/*0*/    PROGRAM_NODE,
/*1*/    DECLARATION_NODE,
/*2*/    IDENTIFIER_NODE,
/*3*/    PARAM_LIST_NODE,
/*4*/    NUL_NODE,
/*5*/    BLOCK_NODE,
/*6*/    VARIABLE_DECL_LIST_NODE,
/*7*/    STMT_LIST_NODE,
/*8*/    STMT_NODE,
/*9*/    EXPR_NODE,
/*10*/    CONST_VALUE_NODE, //ex:1, 2, "constant string"
/*11*/    NONEMPTY_ASSIGN_EXPR_LIST_NODE,
/*12*/    NONEMPTY_RELOP_EXPR_LIST_NODE
} AST_TYPE;

//*************************
// AST_NODE's semantic value
//*************************

typedef struct STMTSemanticValue{
    STMT_KIND kind;
} STMTSemanticValue;

typedef struct EXPRSemanticValue{
    EXPR_KIND kind;

    int isConstEval;

    union{
        int iValue;
        float fValue;
    } constEvalValue;

    union{
        BINARY_OPERATOR binaryOp;
        UNARY_OPERATOR unaryOp;
    } op;

    BINARY_OPERATOR binaryOp() const{
        assert(kind == BINARY_OPERATION);
        return op.binaryOp;
    }

    UNARY_OPERATOR unaryOp() const{
        assert(kind == UNARY_OPERATION);
        return op.unaryOp;
    }
} EXPRSemanticValue;

typedef struct DECLSemanticValue{
    DECL_KIND kind;
} DECLSemanticValue;

typedef struct IdentifierSemanticValue{
    char *identifierName;
    struct SymbolTableEntry *symbolTableEntry;
    IDENTIFIER_KIND kind;

    const char* name() const{
        return identifierName;
    }
} IdentifierSemanticValue;

typedef struct TypeSpecSemanticValue{
    char *typeName;
} TypeSpecSemanticValue;

//don't modify or lexer may break
typedef struct CON_Type{
    C_type  const_type;
    union {
        int     intval;
        double  fval;
        char    *sc; 
    } const_u;

    C_type type() {
        return const_type;
    }
    int Int() {
        assert(type() == INTEGERC);
        return const_u.intval;
    }
    double Double() {
        assert(type() == FLOATC);
        return const_u.fval;
    }
    char* Char() {
        assert(type() == INTEGERC);
        return const_u.sc;
    }
} CON_Type;

/* add the part needed in this homework */
enum RegisterType {
    GENERAL,
    FLOATING
};


// forward declaration
class Register;

class AST_NODE {
    public:
        struct AST_NODE *child;
        struct AST_NODE *parent;
        struct AST_NODE *rightSibling;
        struct AST_NODE *leftmostSibling;
        AST_TYPE nodeType;
        DATA_TYPE dataType;
        int linenumber;

        // store temporaries value in register no. ?
        int place;
        union {
            IdentifierSemanticValue identifierSemanticValue;
            STMTSemanticValue stmtSemanticValue;
            DECLSemanticValue declSemanticValue;
            EXPRSemanticValue exprSemanticValue;
            CON_Type *const1;
        } semantic_value;

        void setSymbolTableEntry(SymbolTableEntry *entry){
            assert(type() == IDENTIFIER_NODE);
            semantic_value.identifierSemanticValue.symbolTableEntry = entry;
        }

        AST_TYPE type() const {
            return nodeType;
        }

        DATA_TYPE getDataType() const;

        void setDataType(DATA_TYPE type){
            dataType = type;
        }

        STMT_KIND getStmtType() const{
            assert(type() == STMT_NODE);
            return semantic_value.stmtSemanticValue.kind;
        }

        C_type getConType() const{
            assert(type() == CONST_VALUE_NODE);
            return semantic_value.const1->type();
        }

        int getConIntValue() const {
            assert(getConType() == INTEGERC);
            return semantic_value.const1->const_u.intval;
        }

        double getConFloatValue() const{
            assert(getConType() == FLOATC);
            return semantic_value.const1->const_u.fval;
        }

        const char* getCharPtrValue() {
            assert(getConType() == STRINGC);
            return semantic_value.const1->const_u.sc;
        }

        DECL_KIND getDeclKind() {
            assert(type() == DECLARATION_NODE);
            return semantic_value.declSemanticValue.kind;
        }

        IDENTIFIER_KIND getIDKind() {
            assert(type() == IDENTIFIER_NODE);
            return semantic_value.identifierSemanticValue.kind;
        }

        EXPR_KIND getExprKind() {
            assert(type() == EXPR_NODE);
            return semantic_value.exprSemanticValue.kind;
        }

        BINARY_OPERATOR getBinaryOp() const{
            assert(type() == EXPR_NODE);
            return semantic_value.exprSemanticValue.binaryOp();
        }

        UNARY_OPERATOR getUnaryOp() const{
            assert(type() == EXPR_NODE);
            return semantic_value.exprSemanticValue.unaryOp();
        }

        const char* getIDName() const{
            assert(type() == IDENTIFIER_NODE);
            return semantic_value.identifierSemanticValue.name();
        }

        SymbolTableEntry *getSymbol() const{
            assert(type() == IDENTIFIER_NODE);
            return semantic_value.identifierSemanticValue.symbolTableEntry;
        }

        // thie method is in Register.cpp
        void setRegister(Register *tmp, bool autoload = false);
        Register *getTempReg(int option = 0);
        const char *getTempName();
        Register *reg;
};

#define RegforceCaller  0x00000001
#define RegDisableload  0x00000010
#define RegReset        0x00000100

struct SymbolTable;

AST_NODE *Allocate(AST_TYPE type);
void semanticAnalysis(AST_NODE *root);

#endif
