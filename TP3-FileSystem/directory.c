#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include "file.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

int directory_findname(struct unixfilesystem *fs, const char *name,
                       int dirinumber, struct direntv6 *dirEnt) {
    struct inode dirinode;
    if (inode_iget(fs, dirinumber, &dirinode) < 0) {
        return -1;
    }
    int size = inode_getsize(&dirinode);
    int numBlocks = (size + DISKIMG_SECTOR_SIZE - 1) / DISKIMG_SECTOR_SIZE;
    for (int b = 0; b < numBlocks; b++) {
        char block[DISKIMG_SECTOR_SIZE];
        int bytes = file_getblock(fs, dirinumber, b, block);
        if (bytes < 0) {
            return -1;
        }
        int numEntries = bytes / sizeof(struct direntv6);
        struct direntv6 *entries = (struct direntv6 *)block;
        for (int i = 0; i < numEntries; i++) {
            if (strncmp(entries[i].d_name, name, sizeof(entries[i].d_name)) == 0) {
                *dirEnt = entries[i];
                return 0;
            }
        }
    }
    return -1;
}