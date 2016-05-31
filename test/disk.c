#include "disk.h"

void print(disk_t *hd) {
    printf("\n");
    printf("bytesPerSector=%lu\n", hd->bytesPerSector);
    printf("totalSectors=%lu\n", hd->totalSectors);
    printf("sectorsPerCluster=%lu\n", hd->sectorsPerCluster);
    printf("reserved=%lu\n", hd->reserved);
    printf("fatNum=%lu\n", hd->fatNum);
    printf("fatSize=%lu\n", hd->fatSize);
    printf("fatPos=%lu,%lu\n", hd->fatPos[0], hd->fatPos[1]);
    printf("rootEntries=%lu\n", hd->rootEntries);
    printf("rootPos=%lu\n", hd->rootPos);
    printf("rootSize=%lu\n", hd->rootSize);
    printf("dataPos=%lu\n", hd->dataPos);
    printf("dataSize=%lu\n", hd->dataSize);
    printf("\n");
}

int main(void) {
    disk_t *hd, *another;

    another = checkDisk("dos1.flp");
    if (another == NULL) {
        printf("no such file.\n");
        exit(0);
    }
    print(another);

    return 0;
}
