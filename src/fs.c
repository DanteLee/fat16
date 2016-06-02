#include "fs.h"

void loadDisk(disk_t *disk) {
    curr_disk = disk;
}

int getUnusedCluster(bool set) {
    size_t start = curr_disk->fatPos[0]*curr_disk->bytesPerSector + 4;
    size_t cp = start;
    size_t end = (curr_disk->fatPos[0]+curr_disk->fatSize)*curr_disk->bytesPerSector - 16;
    int fat = 0;
    bool flag = false;

    while (cp < end) {
        readn(&fat, cp, 2, curr_disk->diskfile);
        if (fat == 0) {
            flag = true;
            break;
        }
        cp += 2;
    }

    if (flag == true) {
        if (set) {
            fat = 0xffff;
            writen(&fat, cp, 2, curr_disk->diskfile);
        }
        return (int)(cp - start);
    }

    return -1;
}

int addNewCluster(size_t file) {
    int fst = 0, *chain, max = 0, len = 0, newClst = 0;
    size_t cp = curr_disk->fatPos[0] * curr_disk->bytesPerSector + 4;

    readn(&fst, file+0x1a, 2, curr_disk->diskfile);
    readn(&max, file+0x1c, 4, curr_disk->diskfile);

    chain = getChain(fst, ceil((float)(max) / BYTES_PER_CLUSTER));
    while (chain[len] < 0xffff) len++;
    newClst = getUnusedCluster(true);
    cp += chain[len - 1] * 2;
    writen(&newClst, cp, 2, curr_disk->diskfile);

    // 返回新簇的地址
    return newClst;
}

int freeCluster(int ord) {
    size_t start = curr_disk->fatPos[0]*curr_disk->bytesPerSector + 4;
    size_t end = (curr_disk->fatPos[0]+curr_disk->fatSize)*curr_disk->bytesPerSector - 16;
    int ret = 0, fat = 0, wb = 0;

    readn(&fat, ord + start, 2, curr_disk->diskfile);
    while (fat != 0xffff) {
        writen(&wb, ord + start, 2, curr_disk->diskfile);
        ret++;
        ord += 2;
        readn(&fat, ord + start, 2, curr_disk->diskfile);
    }
    writen(&wb, ord + start, 2, curr_disk->diskfile);
    ret++;

    return ret;
}

int *getChain(int fst, int max_cluster) {
    size_t start = curr_disk->fatPos[0] * curr_disk->bytesPerSector + 4;
    int now = fst, next = 0, i = 0;
    int *chain = (int *)malloc(sizeof(int) * max_cluster + 1);

    do {
        chain[i] = now;
        i++;
        readn(&next, now * 2 + start, 2, curr_disk->diskfile);
    } while(next < 0xffff && i < max_cluster);

    chain[i] = 0xffff;

    return chain;
}

// -1 未找到; 0 根目录; >0 文件起始位置
size_t find(char *path, size_t base) {
    char *p = path;
    char name[12] = {0}, cmpName[12] = {0};
    int *chain, fst = 0, prop = 0;
    size_t cp = base, /*循环变量*/i = 0, end, len = 0;
    bool found = false;
    byte buff[32];

    // 获取待匹配的文件名
    if (*p == '/') {
        p++;
    }
    if (*p == '\0' || p == NULL) {
        if (base == 0) {
            return 0;
        }
        return base;
    }
    while (*p != '\0' && *p != '/') {
        // 不处理长文件名
        if (i > 12) return 0;

        name[i] = *p;
        p++;
        i++;
        if (*p == '.') {
            len = i;
        }
    }
    len = len == 0 ? i : len;

    if (*path == '/') {
        // 根目录开始
        cp = curr_disk->rootPos * curr_disk->bytesPerSector;
        while (cp < ROOT_END) {
            readn(cmpName, cp, len, curr_disk->diskfile);
            cmpName[len] = '.';
            readn(cmpName + len + 1, cp + 8, 3, curr_disk->diskfile);
            // 找到
            if (strncmp(cmpName, name, len) == 0) {
                found = true;
                break;
            }
            cp += 32;
        }
    } else {
        // 子目录
        // 参数错误
        if (base == 0) return 0;

        // 读子目录首簇、属性和长度
        readn(&fst, base + 0x1a, 2, curr_disk->diskfile);
        readn(&i, base + 0x1c, 4, curr_disk->diskfile);
        chain = getChain(fst, ceil((float)(i) / BYTES_PER_CLUSTER));

        // 比较子目录中的文件与目标文件
        i = 0;
        while (chain[i] < 0xffff) {
            cp = curr_disk->dataPos * curr_disk->bytesPerSector + chain[i] * BYTES_PER_CLUSTER;
            end = cp + BYTES_PER_CLUSTER;
            while (cp < end) {
                readn(cmpName, cp, len, curr_disk->diskfile);
                cmpName[len] = '.';
                readn(cmpName + len + 1, cp + 8, 3, curr_disk->diskfile);

                // 找到
                if (strncmp(cmpName, name, len) == 0) {
                    found = true;
                    break;
                }

                cp += 32;
            }

            if (found) {
                break;
            }

            i++;
        }
    }

    if (found) {
        readn(&prop, cp + 0xb, 1, curr_disk->diskfile);

        if (*p == '\0') return cp;
        if (*p == '/' && is_subdir(prop)) return find(++p, cp);
    }

    return -1;
}

void initFileBuff(void *buff, byte prop, char *name) {
    byte *file = (byte *)buff;
    time_t ts = getTS();
    int fst = 0, len = strlen(name), i = 0;

    memset(file, 0, 32);
    // 处理文件名和扩展名
    while (name[i] != '.' && i < len) i++;
    if (i == len) {
        memcpy(file, name, 8);
    } else {
        memcpy(file, name, i > 8 ? 8 : i);
        memcpy(file+8, name+i+1, len-i-1 > 3 ? 3 : len - i - 1);
    }
    // 文件类型
    memcpy(file+0xb, &prop, 1);
    // 添加时间
    i = getDOSTime(ts);
    memcpy(file+0x16, &i, 2);
    i = getDOSDate(ts);
    memcpy(file+0x18, &i, 2);
    // 首簇号
    fst = getUnusedCluster(true);
    memcpy(file+0x1a, &fst, 2);
}

size_t appendToDir(size_t father, byte *son, bool inRoot) {
    int fst = 0, *chain, filelen = 0, i = 0;
    size_t cp, end;
    byte fchar;

    readn(&fst, father+0x1a, 2, curr_disk->diskfile);
    readn(&filelen, father+0x1c, 4, curr_disk->diskfile);

    // 根目录
    if (inRoot == true) {
        cp = curr_disk->rootPos * curr_disk->bytesPerSector;
        end = ROOT_END;
        while (cp < end) {
            readn(&fchar, cp, 1, curr_disk->diskfile);
            if (fchar == 0xe5 || fchar == 0) {
                writen(son, cp, 32, curr_disk->diskfile);
                return cp;
            }

            cp += 32;
        }

        return 0;
    }

    // 子目录
    chain = getChain(fst, ceil((float)(filelen) / BYTES_PER_CLUSTER));
    i = 0;
    while (chain[i] < 0xffff) {
        cp = curr_disk->dataPos * curr_disk->bytesPerSector + chain[i] * BYTES_PER_CLUSTER;
        end = cp + BYTES_PER_CLUSTER;
        while (cp < end) {
            readn(&fchar, cp, 1, curr_disk->diskfile);
            if (fchar == 0xe5 || fchar == 0) {
                writen(son, cp, 32, curr_disk->diskfile);
                filelen += 32;
                writen(&filelen, father+0x1c, 4, curr_disk->diskfile);
                return cp;
            }

            cp += 32;
        }
        i++;
    }
    cp = GET_CLUSTER_POS(addNewCluster(father));
    writen(son, cp, 32, curr_disk->diskfile);
    filelen += 32;
    writen(&filelen, father+0x1c, 4, curr_disk->diskfile);

    return 0;
}

int _create(char *path, int mode) {
    char name[13] = {0}, *p = path, parent[13] = {0};
    byte file[32];
    int len = strlen(path), i = 0;
    size_t father = 0;

    // 获取待创建的文件名
    p = path + len -1;
    if (*p == '/') return -1;

    i = 0;
    while (*p != '/') {
        i++;
        p--;
    }
    strncpy(name, p+1, i);

    // 初始化缓冲区
    initFileBuff(file, mode, name);

    // 获取父目录偏移量
    memcpy(parent, path, len - i);
    father = find(parent, 0);
    // 未找到
    if (father == -1) return -1;

    // 添加文件至父目录
    if (father == 0) {
        appendToDir(father, file, true);
    } else if (father != -1) {
        appendToDir(father, file, false);
    } else {
        return -1;
    }

    return 0;
}

int deleteFile(char *path) {
    size_t pos = find(path, 0);
    int fst_cluster = 0;
    byte fst_char = 0xe5;

    if (pos != -1 && strcmp(path, "/") == 0) {
        readn(&fst_cluster, pos + 0x1a, 2, curr_disk->diskfile);
        writen(&fst_char, pos, 1, curr_disk->diskfile);
        freeCluster(fst_cluster);

        return 1;
    }

    return -1;
}

int _read(size_t pos, void *buff, size_t n) {
    int fst = 0, *chain, i = 0, avail = 0;
    size_t last = n, filelen = 0;

    if (pos <= 0) return -1;
    readn(&fst, pos+0x1a, 2, curr_disk->diskfile);
    readn(&filelen, pos+0x1c, 4, curr_disk->diskfile);
    chain = getChain(fst, ceil((float)(filelen) / BYTES_PER_CLUSTER));

    while (chain[i] != 0xffff && filelen > 0 && last > 0) {
        if (filelen > BYTES_PER_CLUSTER) {
            avail = BYTES_PER_CLUSTER;
        } else {
            avail = filelen;
        }

        if (last > avail) {
            readn(buff+n-last, GET_CLUSTER_POS(chain[i]), avail, curr_disk->diskfile);
            last -= avail;
            filelen -= avail;
        } else {
            readn(buff+n-last, GET_CLUSTER_POS(chain[i]), last, curr_disk->diskfile);
            filelen -= last;
            last = 0;
        }
    }

    return n - last;
}

int _write(size_t pos, const void *buff, size_t n) {
    int cluster = 0, i = 0;
    size_t last = n, filelen = 0;

    if (pos <= 0) return -1;
    readn(&cluster, pos+0x1a, 2, curr_disk->diskfile);

    while (last > 0) {
        if (last > BYTES_PER_CLUSTER) {
            writen(buff+n-last, GET_CLUSTER_POS(cluster), BYTES_PER_CLUSTER, curr_disk->diskfile);
            last -= BYTES_PER_CLUSTER;
            filelen += BYTES_PER_CLUSTER;
        } else {
            writen(buff+n-last, GET_CLUSTER_POS(cluster), last, curr_disk->diskfile);
            filelen += last;
            last = 0;
        }
        if (last > 0) {
            cluster = addNewCluster(pos);
        }
    }

    writen(&filelen, pos+0x1c, 4, curr_disk->diskfile);

    return last;
}

int readFile(char *path, void *buff, size_t n) {
    size_t pos = find(path, 0);
    int prop = 0;

    readn(&prop, pos+0xb, 1, curr_disk->diskfile);
    if (is_subdir(prop) || is_vol(prop)) return -1;

    return _read(pos, buff, n);
}

int writeFile(char *path, const void *buff, size_t n) {
    size_t pos = find(path, 0);
    int prop = 0;

    readn(&prop, pos+0xb, 1, curr_disk->diskfile);
    if (is_subdir(prop) || is_vol(prop)) return -1;

    return _write(pos, buff, n);
}

byte *readdir(char *path) {
    size_t pos = find(path, 0), filelen = 0, prop = 0, num = 0, i = 0;
    size_t cp = curr_disk->rootPos * curr_disk->bytesPerSector;
    byte *buff = NULL;
    char fchar = '\0';

    if (pos == 0) {
        // 根目录
        num = curr_disk->rootSize * curr_disk->bytesPerSector / 32;
        buff = (byte *)malloc(32*num+1);
        while (cp < ROOT_END) {
            readn(&fchar, cp, 1, curr_disk->diskfile);

            if (fchar != 0 && fchar != 0xe5) {
                readn(buff+i*32, cp, 32, curr_disk->diskfile);
                i++;
            }
            cp += 32;
        }

        buff[i*32] = 0xff;
    } else if (pos == -1) {
        return NULL;
    } else {
        // 子目录
        readn(&prop, pos+0xb, 1, curr_disk->diskfile);
        if (!is_subdir(prop)) return NULL;

        readn(&filelen, pos+0x1c, 4, curr_disk->diskfile);
        num = filelen / 32;
        buff = (byte *)malloc(sizeof(byte)*32*num+1);

        _read(pos, buff, filelen);
        buff[32*num] = 0xff;
    }

    return buff;
}
