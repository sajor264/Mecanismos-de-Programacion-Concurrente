#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>

typedef struct Matrix{
    int size;
    int* data;
}Matrix;

struct Matrix* matrixA;
struct Matrix* matrixB;
struct Matrix* matrixRes;

void printMatrix(Matrix* m) {
    for(int i = 0; i < m->size; i++) {
        for(int j = 0; j < m->size; j++) {
            printf("%d\t", m->data[i*(m->size) + j]);
        }
        printf("\n");
    }
    printf("\n");
}

Matrix* createZeroMatrix(int mSize) {
    struct Matrix* matrix = malloc(sizeof(struct Matrix));
    matrix -> size = mSize;
    matrix -> data = malloc(sizeof(int)*mSize*mSize);
    for(int i = 0; i < mSize; i++) {
        for(int j = 0; j < mSize; j++) {
            matrix -> data[i*mSize + j] =  0;
        }
    }
    struct Matrix *m;
    m = matrix;
    return m;
}

Matrix* createMatrix(int mSize) {
    struct Matrix* matrix = malloc(sizeof(struct Matrix));
    matrix -> size = mSize;
    matrix -> data = malloc(sizeof(int)*mSize*mSize);
    for(int i = 0; i < mSize; i++) {
        for(int j = 0; j < mSize; j++) {
            matrix -> data[i*mSize + j] =  rand() % 1000;
        }
    }
    struct Matrix *m;
    m = matrix;
    return m;
}

void *matrixMul(void *param){
    usleep(100);
    long col = (long)param;
    int mSize = matrixA -> size;
    for (int i = 0; i < mSize; i++) {
        int sum = 0;
        for (int j = 0; j < mSize; j++) {
            sum += matrixA->data[i*mSize + j] * matrixB->data[j*mSize + col];
        }
        matrixRes -> data[i*mSize + col] = sum; 
    }
}

void writeMatrix(Matrix* m, FILE *file) {
    for(int i = 0; i < m->size; i++) {
        for(int j = 0; j < m->size; j++) {
            fprintf(file, "%d\t", m->data[i*(m->size) + j]);
        }
        fprintf(file, "\n");
    }
    fprintf(file, "\n");
}

int main(){
    FILE * stats = fopen ( "./MatrixT Files/Stats.txt", "w" );
    char currentFile[26];
    int n, x;
    srand (time(NULL));

    long avgSeconds = 0;
    long avgMicroseconds = 0;
    struct timeval startTime, endTime; 

    printf("\nEjecutando programa MatrixT...\n");
    printf("Ingrese el tamano de la matriz cuadrada:");
    scanf("%d", &n);

    for(int i = 0; i < 100; i++){
        matrixA = createMatrix(n);
        matrixB = createMatrix(n);
        matrixRes = createZeroMatrix(n);

        gettimeofday(&startTime, NULL);

        pthread_t threads[n];
        for(int col = 0; col < n; col++){
            x = pthread_create(&threads[col], NULL, matrixMul, (void*)(unsigned long) col);
            if (x != 0){
                printf("pthread %d failed.\n", col);
            }
        }

        for(int col = 0; col < n; col++){
            pthread_join(threads[col], NULL);
        }

        gettimeofday(&endTime, NULL);
        long seconds = (endTime.tv_sec - startTime.tv_sec);
        long microSeconds = ((seconds * 1000000) + endTime.tv_usec) - (startTime.tv_usec);

        sprintf(currentFile, "./MatrixT Files/Mat_%d.txt", i);
        FILE * mat = fopen ( currentFile, "w");
        writeMatrix(matrixRes, mat);
        fclose(mat);

        fprintf(stats, "Repeticion %d: %ld segundos(%ld microsegundos)\n", i, seconds, microSeconds);
        avgMicroseconds = avgMicroseconds + microSeconds;
        avgSeconds = avgSeconds + seconds;
    }

    fprintf(stats, "--------------------------------------------------------------------------------\n");
    avgSeconds = avgSeconds/100;
    avgMicroseconds = avgMicroseconds/100;
    fprintf(stats, "Tiempo promedio de las multiplicaciones: %ld segundos(%ld microsegundos)\n", avgSeconds, avgMicroseconds);
    fclose(stats); 

    printf("\nDatos guardados exitosamente!\n\n\n");

    return 0;
}