#include "fs.h"

void printDisk(disk_t *hd) {
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

void printDir(byte *buff) {
    char name[9], ext[4];
    while (*buff != 0xff) {
        if (!is_subdir(*(buff+0xb)) && *buff != 0xe5 && *buff != 0) {
            strncpy(name, buff, 8);
            memcpy(ext, buff+8, 3);
            name[8] = '\0';
            ext[4] = '\0';
            printf("%s.%s\n", name, ext);
        } else {
            strncpy(name, buff, 8);
            name[8] = '\0';
            printf("%s\n", name);
        }
        buff += 32;
    }
}

int main() {
    disk_t *disk;
    char *text1 = "this is a test.", result[512] = {0};
    char *text2 = "file in root.";
    byte *dirbuff;

    // 格式化
    disk = createDisk("disk.flp");
    fdisk(disk);

    printDisk(disk);

    loadDisk(disk);

    createDir("/mydir");

    createFile("/root.txt");
    createFile("/mydir/test.txt");

    writeFile("/root.txt", text2, strlen(text2));
    writeFile("/mydir/test.txt", text1, strlen(text1));

    readFile("/mydir/test.txt", result, strlen(text1));
    printf("%s\n", result);

    dirbuff = readdir("/");
    printDir(dirbuff);

    destroyDisk(disk);

    return 0;
}
