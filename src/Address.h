#ifndef _ADDRESS_H_
#define _ADDRESS_H_

#include "header.h"
#include <cstdlib>
#include <cstdarg>

#define LOADADDR 0
#define LOADWORD 1

// forward declaration
class Register;

// description variable address in memory, won't generate any code
class Address{
    public:
        Address(Register *r, int offset = 0):
            reg(r),
            _offset(offset){
                this->setName();
                this->loadType = LOADWORD;
                this->_volatile = false;
        }

        Address(const char *format, ...){
            va_list args;
            va_start(args, format);
            vsprintf(this->addrName, format, args);
            va_end(args);
            this->loadType = LOADWORD;
            this->_offset = 0;
            this->reg = NULL;
            this->_volatile = false;
        }

        bool operator ==(const Address &addr) const {
            return !strcmp(getName(), addr.getName()) ? true : false;
        }
        bool operator ==(const Address *addr) const{ return (*this) == *addr; }
        Address operator +(int i) const { return *this - (-i); }
        Address operator -(int i) const {
            Address tmp = *this;
            tmp._offset -= i;
            tmp.setName();
            return tmp;
        }

        const char *getName() const{ return addrName; }
        const int getOffset() const{ return _offset; }
        const char *getRegName() const;
        bool hasReg() const;

        int loadType, _volatile;
    private:
        char addrName[100];
        Register *reg;
        int _offset;

        void setName();
};

#endif
