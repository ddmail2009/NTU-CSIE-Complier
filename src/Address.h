#ifndef _ADDRESS_H_
#define _ADDRESS_H_

#include "header.h"
#include <cstdlib>
#include <cstdarg>

// forward declaration
class Register;

// description variable address in memory, won't generate any code
class Address{
    public:
        Address(Register *r, int offset = 0):
            reg(r),
            _offset(offset),
            addrIsLabel(false) {
                this->setName();
        }

        Address(const char *format, ...){
            va_list args;
            va_start(args, format);
            vsprintf(this->addrName, format, args);
            va_end(args);
            addrIsLabel = true;
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
        bool isLabel() const{ return addrIsLabel; }
    private:
        char addrName[100];
        Register *reg;
        int _offset;
        bool addrIsLabel;

        void setName();
};

#endif
