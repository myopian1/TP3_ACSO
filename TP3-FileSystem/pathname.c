
#include "pathname.h"
#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

int pathname_lookup(struct unixfilesystem *fs, const char *pathname) {
    if (pathname == NULL || pathname[0] != '/') {
        return -1;
    }
    char *pathcopy = strdup(pathname);
    if (pathcopy == NULL) {
        return -1;
    }
    int current_inumber = 1;
    char *token = strtok(pathcopy, "/");
    struct direntv6 dirEnt;
    while (token != NULL) {
        if (directory_findname(fs, token, current_inumber, &dirEnt) < 0) {
            free(pathcopy);
            return -1;
        }
        current_inumber = dirEnt.d_inumber;
        token = strtok(NULL, "/");
    }
    free(pathcopy);
    return current_inumber;
}
