int result;
int fact(int n){
    if(n == 1){
        return n;
    } else {
        return (n*fact(n-1));
    }
}

int main(){
    int n;
    fact(2);
}
