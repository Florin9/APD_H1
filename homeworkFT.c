#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <pthread.h>


double *values;
double complex *result;
int N;
double PI;

typedef struct indices{
    int start;
    int finish;
} indices;

void *threadFunction(void *args) {

    indices ind = *(indices *) args;
    int i,j;
    for(i = ind.start; i < ind.finish; i++){
        for(j = 0; j < N; j++){
            result[i] += values[j] * cexp((-2*PI*I*i*j)/N);
        }
    }

    return NULL;
}

void readData(char *file) {
    FILE *fp = fopen(file, "rt");
    int ret = fscanf(fp, "%d", &N);
    if (ret != 1) {
        printf("Failed to read N\n");
        exit(1);
    }
    int i;
    values = malloc(N * sizeof(double));
    result = calloc(N, sizeof(double complex));

    for (i = 0; i < N; i++) {
        ret = fscanf(fp, "%lf", &values[i]);
        if (ret != 1) {
            printf("Eroare la citirea numarului %d\n", i);
            exit(1);
        }
    }
    fclose(fp);
}

void writeData(char *file) {
    FILE *fp = fopen(file, "w");
    fprintf(fp,"%d\n",N);
    int i;
    for (i = 0; i < N; i++) {
        fprintf(fp, "%lf %lf\n", creal(result[i]),cimag(result[i]));
    }
    fclose(fp);
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Not enough paramters: ./program input output P\n");
        exit(1);
    }
    PI = atan2(1, 1) * 4;
    int P = 0;
    int i;
    P = atoi(argv[3]);
    readData(argv[1]);

    pthread_t tid[P];

    indices limit[P];
    int nr = N/P;
    int rest = N - (N/P) * P;
    limit[0].start = 0;
    if(rest > 0) {
        limit[0].finish = nr + 1;
        rest--;
    } else {
        limit[0].finish = nr;
    }

    for (i = 1; i < P; i++){
        if(rest > 0){
            limit[i].start = limit[i-1].finish;
            limit[i].finish = limit[i].start + nr + 1;
            rest--;
        } else {
            limit[i].start = limit[i-1].finish;
            limit[i].finish = limit[i].start + nr;
        }
    }


    for (i = 0; i < P; i++) {
        pthread_create(&(tid[i]), NULL, threadFunction, &(limit[i]));
    }

    for (i = 0; i < P; i++) {
        pthread_join(tid[i], NULL);
    }

    writeData(argv[2]);
    free(result);
    free(values);
    return 0;
}
