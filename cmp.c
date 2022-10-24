#include "cmp.h"

/*
This sort method used for sorthing the arguments passed to command based on files and directory. Means it will first sort all the
files and then sort all the directories before sorting lexicographical order
*/ 
int
sortHelpArgs(const void *a, const void *b)
{
    char *val_a = *(char **)a;
    char *val_b = *(char **)b;

    /*
    Getting stat for both the files to check whether it's file or directory
    */
    struct stat buffer_a, buffer_b;
    int res_a = stat(val_a, &buffer_a);
    int res_b = stat(val_b, &buffer_b);

    /*
    If any of the stat returns non-zero, then throwing appropriate error
    */
    if(res_a != 0 || res_b != 0) {
        (void)printf("Error while accessing stat : %s\n", strerror(errno));
		exit(1);
    }

    if((S_ISDIR(buffer_a.st_mode) && S_ISDIR(buffer_b.st_mode)) || (!S_ISDIR(buffer_a.st_mode) && !S_ISDIR(buffer_b.st_mode))){
        return strcmp(val_a, val_b);
    } else if(S_ISDIR(buffer_a.st_mode)){
        return 1;
    }
    return -1;
}

/*
This sort method used for sorthing the children of the tree based on file should be display first and then directories
*/ 
int
sortHelpFTS(const FTSENT **a, const FTSENT **b)
{
    int val_a = (*a)->fts_statp->st_mode;
    int val_b = (*b)->fts_statp->st_mode;

    if((S_ISDIR(val_a) && S_ISDIR(val_b)) || (!S_ISDIR(val_a) && !S_ISDIR(val_b))){
        return r_FLAG ? (strcmp((*b)->fts_name, (*a)->fts_name)) : (strcmp((*a)->fts_name, (*b)->fts_name));
    } else if(S_ISDIR(val_a)){
        return 1;
    }
    return -1;
}

/*
This sort method used for sorting the entire file-system tree based on the size of the file
*/
int
sortBySize(const FTSENT **a, const FTSENT **b)
{
    long val_a = (*a)->fts_statp->st_size;
    long val_b = (*b)->fts_statp->st_size;

    /*
    r_FLAG given for reverse sort
    */
    if(r_FLAG){
        return val_a - val_b;
    } else {
        return val_b - val_a;
    }
}

/*
This sort method used for sorting tree based on the access time or modification time or last changed time of file
*/
int
sortByModifiedTime(const FTSENT **a, const FTSENT **b)
{
    /*
    getting timespec based on the appropriate flags
    u_Flag : get access time
    c_Flag : get last changed time
    t_Flag : get modification time
    */
    struct timespec val_a = c_FLAG ? (*a)->fts_statp->st_ctimespec : u_FLAG ? (*a)->fts_statp->st_atimespec : (*a)->fts_statp->st_mtimespec;
    struct timespec val_b = c_FLAG ? (*b)->fts_statp->st_ctimespec : u_FLAG ? (*b)->fts_statp->st_atimespec : (*b)->fts_statp->st_mtimespec;

    /*
    r_FLAG given for reverse sort
    */
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