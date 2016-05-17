#include "util.h"

int writen(const void *buff, size_t offset, size_t n, FILE *file) {
    int s;
    long start = ftell(file);
    byte *zero = (byte *)malloc(sizeof(byte) * n);
    memset(zero, 0, n);

    if (fseek(file, (long)offset, SEEK_SET) < 0) return -1;
    fwrite(zero, 1, n, file);
    fseek(file, -n, SEEK_CUR);
    free(zero);
    if ((s = fwrite(buff, 1, n, file)) < 0) return -1;

    fseek(file, start, SEEK_SET);

    return s;
}

int readn(void * buff, size_t offset, size_t n, FILE * file) {
    int s;
    long start = ftell(file);

    if (fseek(file, (long)offset, SEEK_SET) < 0) return -1;
    if (s = fread(buff, 1, n, file) < 0) return -1;

    fseek(file, start, SEEK_SET);

    return s;
}

int write0(FILE * file, size_t n) {
    byte buff[64] = {0};
    size_t last = n;

    while (last > 64) {
        fwrite(buff, 1, 64, file);
        last -= 64;
    }
    fwrite(buff, 1, last, file);
    last -= last;

    return n - last;
}

long getFileLength(FILE *file) {
    long start = ftell(file), len = -1;

    fseek(file, 0, SEEK_END);
    len = ftell(file);
    fseek(file, start, SEEK_SET);

    return len;
}
