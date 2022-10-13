#include "cmp.h"

bool u_FLAG;
bool c_FLAG;
bool r_FLAG;

int
sortHelpArgs(const void *a, const void *b)
{
    char *val_a = *(char **)a;
    char *val_b = *(char **)b;

    struct stat buffer_a, buffer_b;
    stat(val_a, &buffer_a);
    stat(val_b, &buffer_b);

    if((S_ISDIR(buffer_a.st_mode) && S_ISDIR(buffer_b.st_mode)) || (!S_ISDIR(buffer_a.st_mode) && !S_ISDIR(buffer_b.st_mode))){
        return 0;
    } else if(S_ISDIR(buffer_a.st_mode)){
        return 1;
    }
    return -1;
}

int
sortHelpFTS(const FTSENT **a, const FTSENT **b)
{
    int val_a = (*a)->fts_statp->st_mode;
    int val_b = (*b)->fts_statp->st_mode;

    if((S_ISDIR(val_a) && S_ISDIR(val_b)) || (!S_ISDIR(val_a) && !S_ISDIR(val_b))){
        return 0;
    } else if(S_ISDIR(val_a)){
        return 1;
    }
    return -1;
}

int
sortBySize(const FTSENT **a, const FTSENT **b)
{
    long val_a = (*a)->fts_statp->st_size;
    long val_b = (*b)->fts_statp->st_size;

    if(r_FLAG){
        return val_a - val_b;
    } else {
        return val_b - val_a;
    }
}

int
sortByModifiedTime(const FTSENT **a, const FTSENT **b)
{
    struct timespec val_a = c_FLAG ? (*a)->fts_statp->st_ctimespec : u_FLAG ? (*a)->fts_statp->st_atimespec : (*a)->fts_statp->st_mtimespec;
    struct timespec val_b = c_FLAG ? (*b)->fts_statp->st_ctimespec : u_FLAG ? (*b)->fts_statp->st_atimespec : (*b)->fts_statp->st_mtimespec;

    if(r_FLAG){
        if (val_a.tv_nsec > val_b.tv_nsec) return 1;
        if (val_a.tv_nsec < val_b.tv_nsec) return -1;
        return (strcmp((*b)->fts_name, (*a)->fts_name));
    } else {
        if (val_a.tv_nsec < val_b.tv_nsec) return 1;
        if (val_a.tv_nsec > val_b.tv_nsec) return -1;
        return (strcmp((*a)->fts_name, (*b)->fts_name));
    }
}