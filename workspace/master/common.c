#include <stdio.h>
#include "app.h"

const char* getFileName(const char *path){
    const char *name = NULL;
    const char *tmp = NULL;

    if (path && *path) {
        name = strrchr(path, '/');
        tmp = strrchr(path, '\\');

        if (tmp) {
            return name && name > tmp ? name + 1 : tmp + 1;
        }
    }

    return name ? name + 1 : path;
}
