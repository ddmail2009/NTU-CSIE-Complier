int dim = 2;

int a[2][2][2], b[2][2][2];
int c[2][2][2];

void print(int a[2][2][2])
{
    int i,j,k;
    for (i = 0; i < dim; i = i + 1) {
        for (j = 0; j < dim; j = j + 1) {
            for(k = 0; k < dim; k = k + 1) {
                write(a[i][j][k]);
                write(" ");
            }
            write("\n");
        }
        write("\n"); 
    }
}



void arraymult()
{
    int i,j,k;
    for (i = 0; i < dim; i = i + 1) {
        for (j = 0; j < dim; j = j + 1) {
            for (k = 0; k < dim; k = k + 1) {
                c[i][j][k] = a[i][j][k]*b[i][j][k];
            }
        }
    }
}


int main()
{
    int i,j,k;
    write("Enter matrix 1 of dim 2 x 2 x 2: \n");

    for (i = 0; i < dim; i = i + 1) {
        for (j = 0; j < dim; j = j + 1) {
            for (k=0; k < dim; k = k + 1) {
                a[i][j][k] = read();
            }
        }
    }
    print(a);
    write("Enter matrix 2 of dim 2 x 2 x 2 : \n");
    for (i = 0; i < dim; i = i + 1) {
        for (j = 0; j < dim; j = j + 1) {
            for (k = 0; k < dim; k = k + 1) {
                b[i][j][k] = read();
            }
        }
    }
    print(b);
    arraymult();
    print(c);

    return 0;
}

