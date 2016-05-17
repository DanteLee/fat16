#include "disk.h"

disk * createDisk(char * path) {
    disk *hd = (disk *)malloc(sizeof(disk));

    if (access(path, F_OK) == 0) {
        hd->diskfile = fopen(path, "rb+");
        fseek(hd->diskfile, 0, SEEK_SET);
        if (hd->diskfile == NULL) {
            free(hd);
            return NULL;
        }

        return genInfo(hd);
    }
}

disk * checkDisk(char * path) {
    disk *hd = (disk *)malloc(sizeof(disk));

    if (access(path, F_OK) == 0) {
        hd->diskfile = fopen(path, "rb+");
        fseek(hd->diskfile, 0, SEEK_SET);
        if (hd->diskfile == NULL) {
            free(hd);
            return NULL;
        }

        return chkInfo(hd);
    }

    return NULL;
}

void destroyDisk(disk *hd) {
    fclose(hd->diskfile);
    free(hd);
}

static disk * calcInfo(disk *hd) {
    hd->rootSize = (hd->rootEntries * 32) / 512;
    hd->dataSize = (((hd->totalSectors - hd->reserved - hd->rootSize) * hd->bytesPerSector * 16)
        - hd->fatNum * 576) / (((hd->fatNum) + 16) * hd->bytesPerSector);
    hd->fatSize = ((hd->dataSize * hd->bytesPerSector / 32 + 18) * 2) / hd->bytesPerSector;
    hd->fatPos = (size_t *)malloc(sizeof(size_t)*2);
    hd->fatPos[0] = hd->reserved;
    hd->fatPos[1] = hd->fatPos[0] + hd->fatSize;
    hd->rootPos = hd->fatPos[1] + hd->fatSize;
    hd->dataPos = hd->rootPos + hd->rootSize;
}

disk * genInfo(disk *hd) {
    if (hd == NULL) return NULL;
    long len = getFileLength(hd->diskfile);

    // 填充基本格式化信息
    hd->bytesPerSector = 512;
    hd->totalSectors = (size_t)(len / 512);
    hd->sectorsPerCluster = 1;
    hd->reserved = 1;
    hd->fatNum = 2;
    hd->rootEntries = 512;

    // 计算信息
    calcInfo(hd);

    return hd;
}

disk * chkInfo(disk *hd) {
    if (hd == NULL) return NULL;
    long len = getFileLength(hd->diskfile);

    // 填充基本格式化信息
    readn(&(hd->bytesPerSector), 0x0b, 2, hd->diskfile);
    readn(&(hd->totalSectors), 0x13, 2, hd->diskfile);
    readn(&(hd->sectorsPerCluster), 0x0d, 1, hd->diskfile);
    readn(&(hd->reserved), 0x0e, 2, hd->diskfile);
    readn(&(hd->fatNum), 0x10, 1, hd->diskfile);
    readn(&(hd->rootEntries), 0x11, 2, hd->diskfile);

    // 计算信息
    calcInfo(hd);

    return hd;
}

disk * fdisk(disk *hd) {
    size_t data;

    // dbr
    fwrite(formatedDBR, 1, 512, hd->diskfile);
    data = hd->bytesPerSector;
    writen(&data, 0x0b, 2, hd->diskfile);
    data = hd->totalSectors;
    writen(&data, 0x13, 2, hd->diskfile);
    data = hd->sectorsPerCluster;
    writen(&data, 0x0d, 1, hd->diskfile);
    data = hd->reserved;
    writen(&data, 0x0e, 2, hd->diskfile);
    data = hd->fatNum;
    writen(&data, 0x10, 1, hd->diskfile);
    data = hd->fatSize;
    writen(&data, 0x16, 2, hd->diskfile);
    data = hd->rootEntries;
    writen(&data, 0x11, 2, hd->diskfile);

    // fat
    write0(hd->diskfile, hd->fatSize * hd->bytesPerSector * 2);

    // root
    write0(hd->diskfile, hd->rootSize * hd->bytesPerSector);

    return hd;
}
