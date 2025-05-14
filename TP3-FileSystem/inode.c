#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "inode.h"
#include "diskimg.h"

int inode_iget(struct unixfilesystem *fs, int inumber, struct inode *inp) {
    if (fs == NULL || inp == NULL || inumber < 1) {
        return -1;
    }
    const int INODES_PER_SECTOR = 16;
    int sector = INODE_START_SECTOR + ((inumber-1) / INODES_PER_SECTOR);
    int offset = (inumber-1) % INODES_PER_SECTOR;
    struct inode inodes[INODES_PER_SECTOR];
    int result = diskimg_readsector(fs->dfd, sector, inodes);
    if (result == -1) {
        return -1;
    }
    memcpy(inp,(struct inode*) inodes+offset, sizeof(struct inode));
    return 0;
}

int inode_indexlookup(struct unixfilesystem *fs, struct inode *inp, int blockNum) {
    if (!fs || !inp || blockNum < 0) {
        return -1;
    }
    if (!(inp->i_mode & IALLOC)) {
        return -1;
    }
    if (!(inp->i_mode & ILARG)) {
        if (blockNum < 0 || blockNum >= 8) return -1;
        return inp->i_addr[blockNum];
    }
    if (blockNum < 7 * 256) {  //indireccion simple
        int indirectBlockIndex = blockNum / 256;
        int offset = blockNum % 256;
        uint16_t indirectBlockSector = inp->i_addr[indirectBlockIndex];
        if (indirectBlockSector == 0) return 0;
        uint8_t buffer[DISKIMG_SECTOR_SIZE];
        if (diskimg_readsector(fs->dfd, indirectBlockSector, buffer) < 0) {
            return -1;
        }
        uint16_t *entries = (uint16_t *)buffer;
        return entries[offset];
    } else {                                    //indireccion doble
        int doubleOffset = blockNum - (7 * 256);
        int firstLevelIndex = doubleOffset / 256;
        int secondLevelIndex = doubleOffset % 256;

        uint16_t doubleIndirectSector = inp->i_addr[7];
        if (doubleIndirectSector == 0) return 0;

        uint8_t buffer1[DISKIMG_SECTOR_SIZE];
        if (diskimg_readsector(fs->dfd, doubleIndirectSector, buffer1) < 0) {
            return -1;
        }
        uint16_t *indirectBlockPtrs = (uint16_t *)buffer1;
        uint16_t secondLevelSector = indirectBlockPtrs[firstLevelIndex];
        if (secondLevelSector == 0) return 0;
        uint8_t buffer2[DISKIMG_SECTOR_SIZE];
        if (diskimg_readsector(fs->dfd, secondLevelSector, buffer2) < 0) {
            return -1;
        }
        uint16_t *dataBlocks = (uint16_t *)buffer2;
        return dataBlocks[secondLevelIndex];
    }
}

int inode_getsize(struct inode *inp) {
  return ((inp->i_size0 << 16) | inp->i_size1); 
}             