#include "ls.h"

/* List of all available options for the ls command */
#define OPTSTRING "AacdFfhiklnqRrSstuw"
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
bool f_FLAG = false;
bool h_FLAG = false;
bool k_FLAG = false;
bool n_FLAG = false;
bool q_FLAG = false;
bool R_FLAG = false;
bool s_FLAG = false;
bool w_FLAG = false;
bool d_FLAG = false;

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
getSpace(FTSENT *file){
    char* buf = (char *)malloc(sizeof(char)*MAX_BUF);
    for(int i=0;i<file->fts_level;i++){
        strcat(buf,"\t");
    }
    return buf;
}

char*
convertHumanRedable(double size){
    char* buf = (char *)malloc(sizeof(char)*MAX_BUF);
    int index = 0;
    char *suffix[] = {"B", "KB", "MB", "GB", "TB", "PB"};
    if(h_FLAG) {
        while(size > 1024){
            size /= 1024;
            index++;
        }
        sprintf(buf, "%.*f", index, size);
        strcat(buf,suffix[index]);
    } else if(k_FLAG) {
        sprintf(buf, "%.*f", 2, size/1024);
        strcat(buf,suffix[1]);
    } else {
        sprintf(buf, "%f", size);
        strcat(buf,"B");
    }
    return buf;
}

void 
printFileDescription(FTSENT *file, bool dir){

    char* buf = (char *)malloc(sizeof(char)*MAX_BUF);
    strmode(file->fts_statp->st_mode, buf);
    char* spaces = getSpace(file);

    printf("%s%-10s%-10hu\n",spaces,"File mode:",file->fts_statp->st_mode);
    printf("%s%-10s%-10s\n",spaces,"File permission:",buf);

    printf("%s%-10s%-10hu\n",spaces,"Number of links:",file->fts_nlink);

    struct passwd *owner_name = getpwuid(file->fts_statp->st_uid);
    struct group *grp_name = getgrgid(file->fts_statp->st_gid);

    if(l_FLAG){
        printf("%s%-10s%-10s\n",spaces,"Owner name:", owner_name->pw_name);
        printf("%s%-10s%-10s\n",spaces,"Group name:", grp_name->gr_name);
    } else {
        printf("%s%-10s%-10d\n",spaces,"Owner ID:", file->fts_statp->st_uid);
        printf("%s%-10s%-10d\n",spaces,"Group ID:", file->fts_statp->st_gid);
    }

    if(S_ISBLK(file->fts_statp->st_mode) || S_ISCHR(file->fts_statp->st_mode)) {
        printf("%s%-10s%-10s%-10d\n",spaces,"Number of bytes in the file [With device number]:",convertHumanRedable(file->fts_statp->st_size), file->fts_statp->st_dev);
    } else {
        printf("%s%-10s%-10s\n",spaces,"Number of bytes in the file:",convertHumanRedable(file->fts_statp->st_size));
    }

    if(S_ISLNK(file->fts_statp->st_mode)) {
        printf("%s%-10s%-10s%-2s%-5s\n",spaces,"Path name:",file->fts_path,"->",(file->fts_link ? file->fts_link->fts_name : ""));
    } else {
        printf("%s%-10s%-10s\n",spaces,"Path name:",file->fts_path);
    }
    
    struct tm *time = localtime( u_FLAG ? &file->fts_statp->st_atimespec.tv_sec : &file->fts_statp->st_mtimespec.tv_sec);
    printf("%s%-10s%-10d\n",spaces, u_FLAG ? "File was last accessed (Month):" : "File was last modified (Month):",time->tm_mon);
    printf("%s%-10s%-10d\n",spaces, u_FLAG ? "File was last accessed (Day):" : "File was last modified (Day):",time->tm_mday);
    printf("%s%-10s%-10d%-10d\n",spaces,u_FLAG ? "File was last accessed (Hours:Minutes):" : "File was last modified (Hours:Minutes):",time->tm_hour,time->tm_min);

    if(dir) printf("%s%-10s%-10lld\n",spaces,"Block allocated to directory:",file->fts_statp->st_blocks);
}

void 
printFile(FTSENT *file){
    if(file->fts_level > 1 && !R_FLAG) return;
    if(S_ISDIR(file->fts_statp->st_mode)){
        if(file->fts_info == FTS_DP) return;
        if(strlen(file->fts_name) > 0 && file->fts_name[0]=='.' && !a_FLAG) return;
        if(file->fts_info == FTS_DOT && A_FLAG) return;
        printf("%s%-10s%c\n",getSpace(file), file->fts_name, suffix(file));
        if(l_FLAG || n_FLAG) printFileDescription(file, true);
    }
    else {
        if(i_FLAG){
            printf("%s%-10s%c\t\t inode = %-12llu\n",getSpace(file), file->fts_name,suffix(file), file->fts_statp->st_ino);
        } else {
            printf("%s%-10s%c\n",getSpace(file), file->fts_name,suffix(file));
        }
        if(l_FLAG || n_FLAG) printFileDescription(file, false);
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
                stpcpy(buf,argv[j]);
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
                if(c_FLAG){
                    c_FLAG = false;
                }
                u_FLAG = true;
                break;
            case 'r':
                r_FLAG = true;
                break;
            case 'c':
                if(u_FLAG){
                    u_FLAG = false;
                }
                c_FLAG = true;
                break;
            case 'l':
                if(n_FLAG){
                    n_FLAG = false;
                }
                l_FLAG = true;
                break;
            case 'f':
                f_FLAG = true;
                break;
            case 'h':
                h_FLAG = true;
                break;
            case 'k':
                k_FLAG = true;
                break;
            case 'n':
                if(f_FLAG){
                    f_FLAG = false;
                }
                n_FLAG = true;
                break;
            case 'q':
                if(w_FLAG){
                    w_FLAG = false;
                }
                q_FLAG = true;
                break;
            case 'R':
                R_FLAG = true;
                break;
            case 's':
                s_FLAG = true;
                break;
            case 'w':
                if(q_FLAG){
                    q_FLAG = false;
                }
                w_FLAG = true;
                break;
            case 'd':
                d_FLAG = true;
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