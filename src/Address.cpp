#include "Address.h"
#include "Register.h"

void Address::setName() {
    if(!isLabel())
        sprintf(addrName, "%d(%s)", _offset, reg->name());
}
