#ifndef __Register_H
#define __Register_H

#include <vector>
#include "header.h"
// forward declaration
class Address;

class Register{
    public:
        Register(const char *name, DATA_TYPE type = INT_TYPE);

        const char *name() const;
        const DATA_TYPE type() const;
        bool isDirty();

        void operand(BINARY_OPERATOR op, const Register *left, const int value);
        void operand(BINARY_OPERATOR op, const Register *left, const double value);
        void operand(BINARY_OPERATOR op, const Register *left, const Register *right);
        void operand(UNARY_OPERATOR op, const Register *left, const Register *right);

        void load(int value);
        void load(double value);
        void load(const Register *from);
        void load(const Address &addr, bool loadWord = true);

        // save to the target, if it is a ast_node, it's a temporary value.
        void save();
        void save(const Address &addr);

        void setTarget(const AST_NODE *node);
        void setTarget(const Address &addr);
        bool fit(const AST_NODE *node) const;
        bool fit(const Address &addr) const;
        const void *target;
    private:
        char reg_name[10];
        DATA_TYPE reg_type;
        bool dirty;

        bool targetType;
};

// description variable address in memory, won't generate any code
class Address{
    public:
        Address(Register *reg, int offset = 0);

        Address(const char *format, ...);

        Address operator +(int i) const;
        Address operator -(int i) const;

        const char *getName() const{
            return addrName;
        }

        const int getOffset() const{
            return offset;
        }

        bool isLabel() const{
            return addrIsLabel;
        }
    private:
        void setName();
        char addrName[100];

        Register *reg;
        int offset;
        bool addrIsLabel;
};

class RegisterSystem{
    public:
        RegisterSystem();
        Register *getReg(DATA_TYPE type, bool isCaller = false);
        Register *getReg(const char *str);

        void saveCalleeReg(){
            Address address(getReg("$sp"));
            for(int i=0; i<8; i++)
                callerReg[i]->save(address + 4*i + 4);
        }
        void restoreCalleeReg(){
            Address address(getReg("$sp"));
            for(int i=0; i<8; i++)
                callerReg[i]->load(address + 4*i + 4);
        }

        Register *getFit(const Address &addr){
            for(int i=0; i<8; i++)
                if(calleeReg[i]->fit(addr)) return calleeReg[i];
            for(int i=0; i<8; i++)
                if(callerReg[i]->fit(addr)) return callerReg[i];
            for(int i=0; i<30; i++)
                if(floatReg[i]->fit(addr)) return floatReg[i];
            return NULL;
        }
    private:
        int findVacant(Register *arr[], int size);

        std::vector<Register*> registers;
        Register *calleeReg[8];
        Register *callerReg[10];
        Register *floatReg[30];
};

#endif
