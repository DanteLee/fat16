#include "disk.h"
#include "dostime.h"

#ifndef _FS_H_
#define _FS_H_

#define is_ro(x) ((x) & 1)
#define is_hidden(x) ((x) & 2)
#define is_sys(x) ((x) & 4)
#define is_vol(x) ((x) & 8)
#define is_subdir(x) ((x) & 16)

#define P_NORMAL 0
#define P_RO 1
#define P_HIDDEN 2
#define P_SYS 4
#define P_VOL 8
#define P_SUBDIR 16

#define WHOLE_FILE 0xffffffff

#define BYTES_PER_CLUSTER (curr_disk->bytesPerSector * curr_disk->sectorsPerCluster)
#define ROOT_END ((curr_disk->rootPos + curr_disk->rootSize) * curr_disk->bytesPerSector)

disk_t *curr_disk;

void loadDisk(disk_t *);

int getUnusedCluster(bool set);
int freeCluster(int);
int *getChain(int, int);
void initFileBuff(void * /*buff*/, byte /*prop*/, char * /*name*/);
size_t appendToDir(size_t /*father*/, byte * /*son*/, bool /*inBoot*/);
#define GET_CLUSTER_POS(x) (x*BYTES_PER_CLUSTER+curr_disk->bytesPerSector*curr_disk->dataPos)

size_t find(char *, size_t);

int _create(char *, int /*mode*/);
int _delete(char *);
#define createFile(path) _create(path, P_NORMAL)
#define deleteFile(path) _delete(path)
#define createDir(path) _create(path, P_SUBDIR)
#define deleteDir(path) _delete(path)

int _read(size_t pos, void *buff, size_t n);
int _write(size_t pos, const void *buff, size_t n);
int readFile(char *, void *, size_t);
int writeFile(char *, const void *, size_t);
byte *readdir(char *);

#endif
