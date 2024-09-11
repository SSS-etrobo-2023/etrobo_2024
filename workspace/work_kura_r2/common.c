#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>
#include "app.h"
#include "common.h"

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

void getDateTime(char **data) {
    struct tm *ltime;
    time_t ntime;

    time(&ntime);
    ltime = localtime(&ntime);

    snprintf(*data, 20, "%04d/%02d/%02d %02d:%02d:%02d",
             ltime->tm_year + 1900, ltime->tm_mon + 1, ltime->tm_mday,
             ltime->tm_hour, ltime->tm_min, ltime->tm_sec);

    return;
}

char log_lv_str[3][6] = {
    "ERROR", "DEBUG", "TEST "
};

void out_log_file(uint8_t level, const char *file, const int32_t line, const char *format, ...) {
    int8_t string[512] = {'\0'};
    char ctime[20] = {'\0'};
    va_list list;
    FILE *fp = NULL;

    va_start(list, format);
    vsnprintf((char *)string, 512, format, list);
    va_end(list);

    getDateTime((char **)&ctime);

    fp = fopen(LOG_FILE, "a");
    if (fp != NULL) {
        //fprintf(fp, "[%s] [%s:%-10s:L%-5d]: %s", log_lv_str[level - 1], ctime, getFileName(file), line, string);
        fprintf(fp, "[%s] [%-10s:L%-5d]: %s", log_lv_str[level - 1], getFileName(file), line, string);
        fclose(fp);
    }

    return;
}
