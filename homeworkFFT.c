#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <pthread.h>

typedef double complex cpx;

double *values;
cpx *result;
cpx *buff;
int N;
int P;
double PI;

typedef struct indices {
    cpx *buf;
    cpx *out;
    int step;
    int nrIt;
} indices;

void fft1(cpx buf[], cpx out[], int n, int step) {
    if (step < n) {
        fft1(out, buf, n, step * 2);
        fft1(out + step, buf + step, n, step * 2);

        for (int i = 0; i < n; i += 2 * step) {
            cpx t = cexp(-I * PI * i / n) * out[i + step];
            buf[i / 2] = out[i] + t;
            buf[(i + n) / 2] = out[i] - t;
        }
    }
}

void *fftHelper(void *args) {
    indices *ind1 = (indices *) args;
    cpx * buf = ind1->buf;
    cpx * out = ind1->out;
    int step = ind1->step;

    fft1(buf, out, N, step);

    return NULL;
}

void fft2() {
    cpx * buf = buff;
    cpx * out = result;
    int step = 1;
    int i;
    int n = N;


    if (step < n) {
        pthread_t tid[P];

        indices limit[P];
        limit[0].buf = out;
        limit[0].out = buf;
        limit[0].step = step * 2;
        limit[1].buf = out + step;
        limit[1].out = buf + step;
        limit[1].step = step * 2;

        for (i = 0; i < P; i++) {
            pthread_create(&(tid[i]), NULL, fftHelper, &(limit[i]));
        }

        for (i = 0; i < P; i++) {
            pthread_join(tid[i], NULL);
        }

        for (int i = 0; i < n; i += 2 * step) {
            cpx t = cexp(-I * PI * i / n) * out[i + step];
            buf[i / 2] = out[i] + t;
            buf[(i + n) / 2] = out[i] - t;
        }
    }

}

void fft4() {
    cpx * buf = buff;
    cpx * out = result;
    int step = 1;
    int step2 = 2;
    int step3 = 4;

    int i;
    int n = N;

    if (step < n) {
        pthread_t tid[P];

        indices limit[P];
        limit[0].buf = buf;
        limit[0].out = out;
        limit[0].step = step3;
        limit[1].buf = buf + step2;
        limit[1].out = out + step2;
        limit[1].step = step3;
        limit[2].buf = buf + step;
        limit[2].out = out + step;
        limit[2].step = step3;
        limit[3].buf = buf + step + step2;
        limit[3].out = out + step + step2;
        limit[3].step = step3;

        for (i = 0; i < P; i++) {
            pthread_create(&(tid[i]), NULL, fftHelper, &(limit[i]));
        }

        for (i = 0; i < P; i++) {
            pthread_join(tid[i], NULL);
        }
        // out si buf inversat
        step = 1;
        out = buff;
        buf = result;
        for (int i = 0; i < n; i += 2 * step2) {
            cpx t = cexp(-I * PI * i / n) * out[i + step2];
            buf[i / 2] = out[i] + t;
            buf[(i + n) / 2] = out[i] - t;
        }
        //out si buf inversat + s

        out = buff + step;
        buf = result + step;
        for (int i = 0; i < n; i += 2 * step2) {
            cpx t = cexp(-I * PI * i / n) * out[i + step2];
            buf[i / 2] = out[i] + t;
            buf[(i + n) / 2] = out[i] - t;
        }
        //out si buf initial
        out = result;
        buf = buff;
        for (int i = 0; i < n; i += 2 * step) {
            cpx t = cexp(-I * PI * i / n) * out[i + step];
            buf[i / 2] = out[i] + t;
            buf[(i + n) / 2] = out[i] - t;
        }
    }

}

void *threadFunction1(void *args) {

    cpx * buf = buff;
    cpx * out = result;
    fft1(buf, out, N, 1);

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
    result = calloc(N, sizeof(cpx));
    buff = calloc(N, sizeof(cpx));

    for (i = 0; i < N; i++) {
        ret = fscanf(fp, "%lf", &values[i]);
        if (ret != 1) {
            printf("Eroare la citirea numarului %d\n", i);
            exit(1);
        }
        buff[i] = values[i];
        result[i] = buff[i];

    }
    fclose(fp);
}

void writeData(char *file) {
    FILE *fp = fopen(file, "w");
    fprintf(fp, "%d\n", N);
    int i;
    for (i = 0; i < N; i++) {
        fprintf(fp, "%lf %lf\n", creal(buff[i]), cimag(buff[i]));
    }
    fclose(fp);
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Not enough paramters: ./program input output P\n");
        exit(1);
    }
    PI = atan2(1, 1) * 4;

    P = atoi(argv[3]);
    readData(argv[1]);

    pthread_t tid[P];

    indices limit[P];

    if (P == 1) {
        pthread_create(&(tid[0]), NULL, threadFunction1, &(limit[0]));
        pthread_join(tid[0], NULL);
    } else if (P == 2) {
        fft2();
    } else {
        fft4();
    }

    writeData(argv[2]);

    free(values);
    free(result);
    free(buff);

    return 0;
}
