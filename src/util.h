#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#ifndef _UTIL_H_
#define _UTIL_H_

typedef unsigned char byte;
typedef unsigned long size_t;
typedef enum {false, true} bool;

int writen(const void * /*buff*/, size_t /*offset*/, size_t /*n*/, FILE * /*file*/);
int readn(void * /*buff*/, size_t /*offset*/, size_t /*n*/, FILE * /*file*/);
int write0(FILE * /*file*/, size_t /*n*/);

long getFileLength(FILE *file);

#endif
