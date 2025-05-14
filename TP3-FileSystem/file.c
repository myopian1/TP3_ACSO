#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "file.h"
#include "inode.h"
#include "diskimg.h"

#define INDIRECT_BLOCKS (DISKIMG_SECTOR_SIZE / sizeof(int))  // Número de punteros en un bloque indirecto

/**
 * Obtiene un bloque de un archivo, considerando tanto los bloques directos como los indirectos.
 */
int file_getblock(struct unixfilesystem *fs, int inumber, int blockNum, void *buf) {
    if (!fs || !buf || inumber < 1 || blockNum < 0) {
        return -1;
    }

    struct inode in;
    if (inode_iget(fs, inumber, &in) < 0) {
        return -1;
    }

    int fileSize = inode_getsize(&in);
    int totalBlocks = (fileSize + DISKIMG_SECTOR_SIZE - 1) / DISKIMG_SECTOR_SIZE;

    if (blockNum >= totalBlocks) {
        return 0; // El bloque está más allá del tamaño del archivo
    }

    // Obtener el sector físico del bloque
    int sector = inode_indexlookup(fs, &in, blockNum);
    if (sector <= 0) {
        return 0; // Bloque no asignado
    }

    // Leer el sector físico
    int result = diskimg_readsector(fs->dfd, sector, buf);
    if (result < 0) {
        return -1;
    }

    // Último bloque: devolver solo los bytes válidos
    if (blockNum == totalBlocks - 1) {
        int remaining = fileSize % DISKIMG_SECTOR_SIZE;
        return remaining == 0 ? DISKIMG_SECTOR_SIZE : remaining;
    }

    return DISKIMG_SECTOR_SIZE;
}
