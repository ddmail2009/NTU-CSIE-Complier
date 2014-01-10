#include <stdio.h>
struct name{
    char n[100];
};

void isPtrSame(name *ptr, name *aptr){
    printf("%d\n", ptr == aptr);
}


int main(){
    name a;
    name *aptr = &a;

    name b;
    name *bptr = &b;

    isPtrSame(aptr, bptr);
    bptr = aptr;
    isPtrSame(aptr,bptr);
    
}
