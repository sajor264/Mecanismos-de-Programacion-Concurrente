#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/shm.h>

typedef struct Matrix{
    int size;
    int* data;
}Matrix;

struct Matrix* matrixA;
struct Matrix* matrixB;
struct Matrix* sharedMemory;

int sem, shm, shmData;

int createSem(int key){
    int semID;
    semID = semget(key, 1, 0666 | IPC_CREAT);
    semctl(semID, 0, SETVAL, 1);
    return semID;
}

void semWait(int semID){
    struct sembuf s;
    s.sem_num = 0;
    s.sem_op = -1;
    s.sem_flg = SEM_UNDO;
    semop(semID, &s, 1);
    return;
}

void semSignal(int semID){
    struct sembuf s;
    s.sem_num = 0;
    s.sem_op = 1;
    s.sem_flg = SEM_UNDO;
    semop(semID, &s, 1);
    return;
}

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

void *matrixMul(int col){
    usleep(100);
    int mSize = matrixA -> size;
    for (int i = 0; i < mSize; i++) {
        int sum = 0;
        for (int j = 0; j < mSize; j++) {
            sum += matrixA->data[i*mSize + j] * matrixB->data[j*mSize + col];
        }
        sharedMemory -> data[i*mSize + col] = sum; 
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
    FILE *stats = fopen ( "./MatrixP Files/Stats.txt", "w" );
    FILE *mat;

    char currentFile[26];
    char *line;
    int n, x;

    long avgSeconds = 0;
    long avgMicroseconds = 0;
    struct timeval startTime, endTime;

    sem = createSem(0x1234);

    shm = shmget((key_t)1234, sizeof(struct Matrix), 0666 | IPC_CREAT);
    sharedMemory = shmat(shm, NULL, 0);

    printf("\nEjecutando programa MatrixP...\n");
    printf("Ingrese el tamano de la matriz cuadrada:");
    scanf("%d", &n);

    for(int i = 0; i < 100; i++){

        matrixA = createMatrix(n);
        matrixB = createMatrix(n);
        sharedMemory -> size = n;
        shmData = shmget((key_t)4321, sizeof(int)*n*n, 0666 | IPC_CREAT);
        sharedMemory -> data = shmat(shmData, NULL, 0);

        gettimeofday(&startTime, NULL);

        for(int col = 0; col < n; col++){
            if(fork() == 0){
                matrixMul(col);
                exit(0);
            }
        }
        
        for(int col = 0; col < n; col++){
            wait(NULL);
        }

        gettimeofday(&endTime, NULL);
        long seconds = (endTime.tv_sec - startTime.tv_sec);
        long microSeconds = ((seconds * 1000000) + endTime.tv_usec) - (startTime.tv_usec);
        avgMicroseconds = avgMicroseconds + microSeconds;
        avgSeconds = avgSeconds + seconds;

        fprintf(stats, "Repeticion %d: %ld segundos(%ld microsegundos)\n", i, seconds, microSeconds);

        sprintf(currentFile, "./MatrixP Files/Mat_%d.txt", i);
        mat = fopen (currentFile, "w");
        writeMatrix(sharedMemory, mat);
        fclose(mat);

        shmctl(shmData, IPC_RMID, NULL); 
    }

    shmctl(shm, IPC_RMID, NULL);

    avgSeconds = avgSeconds/100;
    avgMicroseconds = avgMicroseconds/100;

    fprintf(stats, "--------------------------------------------------------------------------------\n");
    fprintf(stats, "Tiempo promedio de las multiplicaciones: %ld segundos(%ld microsegundos)\n", avgSeconds, avgMicroseconds);
    fclose(stats); 

    printf("\nDatos guardados exitosamente!\n\n\n");

    return 0;
}