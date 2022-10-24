#include "ls.h"

/* List of all available options for the ls command */
#define OPTSTRING "AacdFfhiklnqRrSstuw"
#define MAX_BUF 512
/*Macro for finding maximum of 2*/
#define MAX(X, Y) (((X) >= (Y)) ? (X) : (Y))

/*
Options flags to store all the flags passed as arguments
*/
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
bool q_FLAG = true;
bool R_FLAG = false;
bool s_FLAG = false;
bool w_FLAG = false;
bool d_FLAG = false;

bool super_user = false;
float total_size;
long block_size = 512;
/*
Making argument structure to pre process the arguments, remove all the flags and store the remanining args passed
*/
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

/*
This method is used to print the suffix after each file if F_FLAG is passed, It will check the file_mode and make a decision
/ : Directory
@ : Link
| : Pipeline
% : Whiteout
* : Executable
= : Socket
*/
char
suffix(FTSENT *file)
{
    if(!F_FLAG) return '\0';
    if(S_ISDIR(file->fts_statp->st_mode)) return '/';
    if(S_ISLNK(file->fts_statp->st_mode)) return '@';
    if(S_ISFIFO(file->fts_statp->st_mode)) return '|';
    if((file->fts_statp->st_mode & S_IXUSR) != 0) {
        if((file->fts_statp->st_mode & S_ISUID) != 0) return '%';
        return '*';
    } else {
        if((file->fts_statp->st_mode & S_ISUID) != 0) return '=';
    }
    return '\0';
}

/*
This method is used to add spaces based on the level of files, before printing it
*/
char*
getSpace(FTSENT *file)
{
    char* buf = (char *)malloc(sizeof(char)*MAX_BUF);

    /*
    Checking is malloc returns a NULL buffer then throw an error
    */
    if(buf == NULL){
        (void)printf("Error while allocating buffer : %s\n", strerror(errno));
		exit(1);
    }

    /*
    Adding tabs '\t' based on the file level
    */
    for(int i=0;i<file->fts_level;i++){
        strcat(buf,"\t");
    }
    return buf;
}

/*
This method is used to convert the size to human readable format if -h or -k flag is passed, basically it keeps deviding size until
we reach to the required unit of file
*/
char*
convertHumanRedable(double size)
{

    /*
    Initilizing buffer to store the final size with unit
    */
    char* buf = (char *)malloc(sizeof(char)*MAX_BUF);

    /*
    Checking is malloc returns a NULL buffer then throw an error
    */
    if(buf == NULL){
        (void)printf("Error while allocating buffer : %s\n", strerror(errno));
		exit(1);
    }

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
        /*2 signifies we need 2 digits after the decimal point and it will show size in Kilobytes (K)*/
        sprintf(buf, "%.*f", 2, size/1024);
        strcat(buf,suffix[1]);
    } else {
        sprintf(buf, "%f", size);
        strcat(buf,"B");
    }
    return buf;
}

/*
This method updates the file name if it contains the non printable character and add '?' instead of that
*/
char*
getFileName(char *name)
{
    for(int i=0;i< (int)strlen(name);i++){
        name[i] = isprint((unsigned int)name[i]) ? name[i] : '?';
    }
    return name;
}

/*
This method will be called when -l or -n flags will be passed, it will basically prints all the information related to file like,
- file mode
- number of links
- owner name
- group name
- number of bytes in the file
- abbreviated month file was last modified
- day-of-month file was last modified
- hour and minute file was last modified
- pathname
*/
void 
printFileDescription(FTSENT *file, bool dir)
{

    char* buf = (char *)malloc(sizeof(char)*MAX_BUF);

    /*
    Checking is malloc returns a NULL buffer then throw an error
    */
    if(buf == NULL){
        (void)printf("Error while allocating buffer : %s\n", strerror(errno));
		exit(1);
    }

    /*
    Changing file mode to permission string
    */
    strmode(file->fts_statp->st_mode, buf);
    /*
    Getting spaces to add before printing description or file name
    */
    char* spaces = getSpace(file);

    printf("\t%s%-10s%-10hu\n",spaces,"File mode : ",file->fts_statp->st_mode);
    printf("\t%s%-10s%-10s\n",spaces,"File permission : ",buf);
    printf("\t%s%-10s%-10hu\n",spaces,"Number of links : ",file->fts_nlink);

    /*
    Using getpwid and getgrgid to get owner_name and group_name
    */
    struct passwd *owner_name = getpwuid(file->fts_statp->st_uid);
    struct group *grp_name = getgrgid(file->fts_statp->st_gid);
    struct tm *time = localtime( u_FLAG ? &file->fts_statp->st_atimespec.tv_sec : &file->fts_statp->st_mtimespec.tv_sec);

    if(l_FLAG){
        printf("\t%s%-10s%-10s\n",spaces,"Owner name : ", owner_name->pw_name);
        printf("\t%s%-10s%-10s\n",spaces,"Group name : ", grp_name->gr_name);
    } else {
        printf("\t%s%-10s%-10d\n",spaces,"Owner ID : ", file->fts_statp->st_uid);
        printf("\t%s%-10s%-10d\n",spaces,"Group ID : ", file->fts_statp->st_gid);
    }

    if(S_ISBLK(file->fts_statp->st_mode) || S_ISCHR(file->fts_statp->st_mode)) {
        printf("\t%s%-10s%-10s%-10ld\n",spaces,"Number of bytes in the file [With device number] : ",convertHumanRedable(file->fts_statp->st_size), file->fts_statp->st_dev);
    } else {
        printf("\t%s%-10s%-10s\n",spaces,"Number of bytes in the file : ",convertHumanRedable(file->fts_statp->st_size));
    }

    if(S_ISLNK(file->fts_statp->st_mode)) {
        printf("\t%s%-10s%-10s%-2s%-5s\n",spaces,"Path name : ",getFileName(file->fts_path),"->",(file->fts_link ? file->fts_link->fts_name : ""));
    } else {
        printf("\t%s%-10s%-10s\n",spaces,"Path name : ",getFileName(file->fts_path));
    }
    
    printf("\t%s%-10s%-10d\n",spaces, u_FLAG ? "File was last accessed (Month) : " : "File was last modified (Month) : ",time->tm_mon);
    printf("\t%s%-10s%-10d\n",spaces, u_FLAG ? "File was last accessed (Day) : " : "File was last modified (Day) : ",time->tm_mday);
    printf("\t%s%-10s%d%s%d\n",spaces,u_FLAG ? "File was last accessed (Hours:Minutes) : " : "File was last modified (Hours:Minutes) : ",time->tm_hour,":",time->tm_min);

    /*
    If it is directory we need to print allocated block as well
    */
    if(dir) printf("\t%s%-10s%-10ld\n",spaces,"Block allocated to directory : ",file->fts_statp->st_blocks);
}

/*
This method prints file names according to flags passed
*/
void 
printFile(FTSENT *file)
{
    
    /*
    if d_FLAG is passed, need to restrcits all the files/directories with higher level
    */
    if(d_FLAG && file->fts_level > 0) return;
    /*
    if R_FLAG is not there then we must not show the files/directories which are at higher depth 
    */
    if(file->fts_level > 1 && !R_FLAG) return;
    /*
    if A_FLAG is passed then we must restrict '.' and '..' 
    */
    if((file->fts_info == FTS_DOT || (strlen(file->fts_name) > 0 && file->fts_name[0]=='.')) && A_FLAG) return;

    /*
    if file is not found then need to throw an error
    */
    if(file->fts_errno){
        printf("ls %s: No such file or directory\n",file->fts_name);
        return;
    }

    /*
    File_name with only printable characters
    */
    char* file_name = getFileName(file->fts_name);

    if(S_ISDIR(file->fts_statp->st_mode)){
        /*
        Restrict to print directory name if it's postorder
        (As we know fts_read will travers directory twice once in preorder and second in postorder)
        */
        if(file->fts_info == FTS_DP) return;
        /*
        file name starts with '.' and a_FLAG is not passed then need to return
        */
        if(strlen(file->fts_name) > 0 && file->fts_name[0]=='.' && !a_FLAG) return;

        if(s_FLAG){
            /* Finding total size using file block and block size given as environment variable */
            total_size += (file->fts_statp->st_blocks / block_size);
            printf("%s%-10ld%-10s%c\n",getSpace(file),(long)(ceil)(file->fts_statp->st_blocks / block_size), file_name, suffix(file));
        } else {
            printf("%s%-10s%c\n",getSpace(file), file_name, suffix(file));
        }
        if(l_FLAG || n_FLAG){
            if(!s_FLAG) total_size += (file->fts_statp->st_blocks / block_size);
            printFileDescription(file, true);
        }
    }else {
        if(a_FLAG && strlen(file->fts_name) > 0 && file->fts_name[0]=='.') return;
        if(i_FLAG){
            /* Printing inode if i_FLAG is passed */
            printf("%s%-10s%c\t\t inode = %-12lu\n",getSpace(file), file_name,suffix(file), file->fts_statp->st_ino);
        } else if(s_FLAG){
            total_size += (file->fts_statp->st_blocks / block_size);
            printf("%s%-10ld%-10s%c\n",getSpace(file), (long)(ceil)(file->fts_statp->st_blocks / block_size) ,file_name,suffix(file));
        } else {
            printf("%s%-10s%c\n",getSpace(file), file_name,suffix(file));
        }
        if(l_FLAG || n_FLAG) {
            if(!s_FLAG) total_size += (file->fts_statp->st_blocks / block_size);
            printFileDescription(file, false);
        }
    }
}

/*
This method parse the arguments, extract the files/directory without options and store the updated arguments in new array
*/
Struct
parseArgs(int argc, char *argv[])
{
    int i = 0;
    while(i < argc)
    {
        /* Checking if current arg is Flag or not */
        if(strlen(argv[i]) > 0 && argv[i][0]=='-'){
            int j = i+1;
            while(j < argc){
                char* buf = (char *)malloc(sizeof(char)*MAX_BUF);
                /*
                Checking is malloc returns a NULL buffer then throw an error
                */
                if(buf == NULL){
                    (void)printf("Error while allocating buffer : %s\n", strerror(errno));
                    exit(1);
                }

                stpcpy(buf,argv[j]);
                argv[j-1] = buf;
                j++;
            }
            argc--;
        } else {
            i++;
        }
    }
    /* Updated args store to Struct */
    Struct s;
    s.length = argc;
    s.argv = argv;
    return s;
}

/*
Traverse all the arguments passed and try to open file_system using fts_open
*/
void
traversArgs(char **argv, int argc, int start)
{
    for(int i=start;i<argc;i++){
        char *currentPath[2] = { argv[i] };
        FTS *fts;

        fts = fts_open(currentPath, FTS_PHYSICAL, &sortHelpFTS);
        if(S_FLAG){
            fts = fts_open(currentPath, FTS_PHYSICAL, &sortBySize);
        }
        if(t_FLAG){
            fts = fts_open(currentPath, FTS_PHYSICAL, &sortByModifiedTime);
        }
        
        FTSENT *file_system;
        total_size = 0.0;
        while((file_system = fts_read(fts)) != NULL){
            printFile(file_system);
        }
        /* If -s -l or -n flag pass then need to print total file size / block size */
        if(s_FLAG || l_FLAG || n_FLAG){
            printf("total %ld\n\n", (long)ceil(total_size));
        }
        if(fts_close(fts)!=0){
            (void)printf("Error while closing fts : %s\n", strerror(errno));
		    exit(1);
        }
    }
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
                if(k_FLAG){
                    k_FLAG = false;
                }
                h_FLAG = true;
                break;
            case 'k':
                if(h_FLAG){
                    h_FLAG = false;
                }
                k_FLAG = true;
                break;
            case 'n':
                if(l_FLAG){
                    l_FLAG = false;
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
    /* If BLOCKSIZE is given in environment variable then extract that */
	if(getenv("BLOCKSIZE")!=NULL) {
        char *ptr;
        block_size = MAX(block_size, strtol(getenv("BLOCKSIZE"), &ptr, 10));
    }
    if(block_size >= 512) block_size = block_size / 512;

	if(getenv("TZ")!=NULL) {
        char* buf = (char *)malloc(sizeof(char)*MAX_BUF);
        if(buf == NULL){
            (void)printf("Error while allocating buffer : %s\n", strerror(errno));
            exit(1);
        }
		strcpy(buf, getenv("TZ"));
		setenv("TZ", buf, 1);
   		tzset();
	}

     /* Checking if user is super user / root user or not */
    super_user = (getuid()==0);

    /* If super user then need to set A_Flag by default */
    if(super_user){
        A_FLAG = true;
    }
    if(d_FLAG){
        a_FLAG = true;
    }

    Struct nargv = parseArgs(argc, argv);
    /* Sort arguments based on file comes first and then directories */
    qsort(nargv.argv, nargv.length, sizeof(nargv.argv[0]), sortHelpArgs);
    if(nargv.length > 1){
        traversArgs(nargv.argv, nargv.length, 1);
    } else {
        /* If nothing passed in arguments then it should be current directory */
        char *currentDir = ".";
        char **list = { &currentDir };
        traversArgs(list,1,0);
    }
    return(0);
}