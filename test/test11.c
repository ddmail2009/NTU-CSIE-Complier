float delta = 1.0;

int floor(float num) {
    int temp;
    temp = num;
    return temp;
}


int ceil(float num) {
    int temp;
    float dumb;
    if (num > 0) {
        temp = num + delta;
    } else {
        temp = num - delta;
    }
    return temp;
}


int main() {
    float num;
    float midp;

    num = 3.2; 

    write(ceil(num));
    write("\n");
    write(floor(num));
    write("\n");
    midp = (ceil(num) + floor(num))/2.0;
    write(midp);
    write("\n");

    return 0;
}
