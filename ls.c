#include "ls.h"

/* List of all available options for the ls command */
#define OPTSTRING "AacdFfhiklnqRrSstuw"
#define PATH_MAX 2048
#define MAX_BUF 512
#define MAX_NUMBER_OF_FILES 1024

bool A_FLAG = false;
bool a_FLAG = false;
bool i_FLAG = false;
bool F_FLAG = false;
bool S_FLAG = false;
bool t_FLAG = false;
bool u_FLAG = false;
bool r_FLAG = false;
bool c_FLAG = false;
bool l_FLAG = false;

struct arguments {
    int length;
    char **argv;
};

typedef struct arguments Struct;

void
usage()
{
    printf("%s\n","Usage");
}

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

char
suffix(FTSENT *file){
    if(!F_FLAG) return '\0';
    if(S_ISDIR(file->fts_statp->st_mode)) return '/';
    if((file->fts_statp->st_mode & S_IFMT)==S_IXUSR) return '*';
    if(S_ISLNK(file->fts_statp->st_mode)) return '@';
    if(S_ISWHT(file->fts_statp->st_mode)) return '%';
    if(S_ISSOCK(file->fts_statp->st_mode)) return '=';
    if(S_ISFIFO(file->fts_statp->st_mode)) return '|';
    return '\0';
}

char*
getPermissionString(mode_t mode, int read, int write, int execute, char *type){
    char* buf = (char *)malloc(sizeof(char)*MAX_BUF);
    if((mode & read) != 0){
        strcat(buf,"r");
    } else {
        strcat(buf,"-");
    }
    if((mode & write) != 0){
        strcat(buf,"w");
    } else {
        strcat(buf,"-");
    }
    if((mode & execute) != 0){
        if(strcmp(type,"owner")==0 && (mode & S_ISUID) != 0){
            strcat(buf,"s");
        } else if(strcmp(type,"group")==0 && (mode & S_ISGID) != 0){
            strcat(buf,"s");
        } else if(strcmp(type,"other")==0 && (mode & S_ISVTX) != 0) {
            strcat(buf,"t");
        } else{
            strcat(buf,"x");
        }
    } else {
        if(strcmp(type,"owner")==0 && (mode & S_ISUID) != 0){
            strcat(buf,"S");
        } else if(strcmp(type,"group")==0 && (mode & S_ISGID) != 0){
            strcat(buf,"S");
        } else if(strcmp(type,"other")==0 && (mode & S_ISVTX) != 0) {
            strcat(buf,"T");
        } else{
            strcat(buf,"-");
        }
    }
    return buf;
}

void 
printFileDescription(FTSENT *file, bool dir){
    printf("\t\t%-10s%-10hu\n","File mode:",file->fts_statp->st_mode);
    printf("\t\t\t%-10s%-10s\n","Owner permission:",getPermissionString(file->fts_statp->st_mode, S_IRUSR, S_IWUSR, S_IXUSR,"owner"));
    printf("\t\t\t%-10s%-10s\n","Group permission:",getPermissionString(file->fts_statp->st_mode, S_IRGRP, S_IWGRP, S_IXGRP,"group"));
    printf("\t\t\t%-10s%-10s\n","Other permission:",getPermissionString(file->fts_statp->st_mode, S_IROTH, S_IWOTH, S_IXOTH,"other"));
    
    printf("\t\t%-10s%-10hu\n","Number of links:",file->fts_nlink);

    struct passwd *owner_name = getpwuid(file->fts_statp->st_uid);
    struct group *grp_name = getgrgid(file->fts_statp->st_gid);

    printf("\t\t%-10s%-10s\n","Owner name:",owner_name->pw_name);
    printf("\t\t%-10s%-10s\n","Group name:",grp_name->gr_name);

    if(S_ISBLK(file->fts_statp->st_mode) || S_ISCHR(file->fts_statp->st_mode)) {
        printf("\t\t%-10s%-10lld%-10d\n","Number of bytes in the file [With device number]:",file->fts_statp->st_size, file->fts_statp->st_dev);
    } else {
         printf("\t\t%-10s%-10lld\n","Number of bytes in the file:",file->fts_statp->st_size);
    }

    if(S_ISLNK(file->fts_statp->st_mode)) {
        printf("\t\t%-10s%-10s%-2s%-5s\n","Path name:",file->fts_path,"->",(file->fts_link ? file->fts_link->fts_name : ""));
    } else {
        printf("\t\t%-10s%-10s\n","Path name:",file->fts_path);
    }
    
    struct tm *time = localtime(&file->fts_statp->st_mtimespec.tv_sec);
    printf("\t\t%-10s%-10d\n","File was last modified (Month):",time->tm_mon);
    printf("\t\t%-10s%-10d\n","File was last modified (Day):",time->tm_mday);
    printf("\t\t%-10s%-10d%-10d\n","File was last modified (Hours:Minutes):",time->tm_hour,time->tm_min);

    if(dir) printf("\t\t%-10s%-10lld\n","Block allocated to directory:",file->fts_statp->st_blocks);
}

void 
printFile(FTSENT *file){
    if(strlen(file->fts_name) > 0 && file->fts_name[0]=='.' && !a_FLAG) return;
    if(S_ISDIR(file->fts_statp->st_mode)){
        printf("\t%-10s%c\n",file->fts_name, suffix(file));
        if(l_FLAG) printFileDescription(file, true);
    }
    else {
        if(i_FLAG){
            printf("\t%-10s%c\t\t inode = %-12llu\n",file->fts_name,suffix(file), file->fts_statp->st_ino);
        } else {
            printf("\t%-10s%c\n",file->fts_name,suffix(file));
        }
        if(l_FLAG) printFileDescription(file, false);
    }
}

Struct
parseArgs(int argc, char *argv[])
{
    int i = 0;
    while(i < argc)
    {
        if(strlen(argv[i]) > 0 && argv[i][0]=='-'){
            int j = i+1;
            while(j < argc){
                char* buf = (char *)malloc(sizeof(char)*MAX_BUF);
                strcpy(buf,argv[j]);
                argv[j-1] = buf;
                j++;
            }
            argc--;
        } else {
            i++;
        }
    }

    Struct s;
    s.length = argc;
    s.argv = argv;
    return s;
}

int
main(int argc, char *argv[])
{
    int ch;
    while ((ch = getopt(argc, argv, OPTSTRING)) != -1){
        switch (ch) {
            case 'A':
                A_FLAG = true;
                break;
            case 'a':
                a_FLAG = true;
                break;
            case 'i':
                i_FLAG = true;
                break;
            case 'F':
                F_FLAG = true;
                break;
            case 'S':
                S_FLAG = true;
                break;
            case 't':
                t_FLAG = true;
                break;
            case 'u':
                u_FLAG = true;
                break;
            case 'r':
                r_FLAG = true;
                break;
            case 'c':
                c_FLAG = true;
                break;
            case 'l':
                l_FLAG = true;
                break;
            default:
                usage();
        }
    }

    Struct nargv = parseArgs(argc, argv);
    qsort(nargv.argv, nargv.length, sizeof(nargv.argv[0]), sortHelpArgs);

    for(int i=1;i<nargv.length;i++){
        
        char *currentPath[2] = { nargv.argv[i] };
        FTS *fts;
        if(S_FLAG){
            fts = fts_open(currentPath, FTS_PHYSICAL, &sortBySize);
        } else if(t_FLAG){
            fts = fts_open(currentPath, FTS_PHYSICAL, &sortByModifiedTime);
        } else {
            fts = fts_open(currentPath, FTS_PHYSICAL, &sortHelpFTS);
        }
        FTSENT *file_system;

        while((file_system = fts_read(fts)) != NULL){
            printFile(file_system);
        }
        fts_close(fts);

    }
    return(0);
}