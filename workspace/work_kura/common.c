#include <stdio.h>
#include <time.h>
#include <string.h>
#include "app.h"

FILE *fp = NULL;

const char *getFileName(const char *path){
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

const char *getDateTime(void) {
    struct tm *ltime;
    time_t ntime;
    char buf[20] = {'\0'};

    time(&ntime);
    ltime = localtime(&ntime);

    snprintf(buf, "%04d/%02d/%02d %02d:%02d:%2d",
             ltime->tm_year + 1900, ltime->tm_mon + 1, ltime->tm_mday,ltime->tm_hour, ltime->tm_min, ltime->tm_sec);

    return buf;
}
