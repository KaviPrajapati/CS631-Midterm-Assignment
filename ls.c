#include "ls.h"

/* List of all available options for the ls command */
#define OPTSTRING "AacdFfhiklnqRrSstuw"
#define PATH_MAX 1024
#define MAX_BUF 512

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

bool
isFile(char *path){
    struct stat buffer;
    stat(path, &buffer);
    return S_ISREG(buffer.st_mode);
}

bool
isDirectory(char *path){
    struct stat buffer;
    stat(path, &buffer);
    return S_ISDIR(buffer.st_mode);
}

void
ls(char* path)
{
    if(isFile(path)) {
        printf("\t%s\n",path);
        return;
    }
    DIR *dp;
    struct dirent *dirp;
    char buf[PATH_MAX];
    char *dire_path = realpath(path, buf);

	if ((dp = opendir(dire_path)) == NULL){
		fprintf(stderr, "can't open '%s'\n", dire_path);
		exit(1);
	}

	while ((dirp = readdir(dp)) != NULL){
        printf("\t%s\n", dirp->d_name);
    }

	closedir(dp);
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
sortHelp(const void *a, const void *b)
{
    char *val_a = *(char **)a;
    char *val_b = *(char **)b;

    if((isFile(val_a) && isFile(val_b)) || (isDirectory(val_a) && isDirectory(val_b))){
        return 0;
    } else if(isFile(val_a)){
        return -1;
    }
    return 1;
}

int
main(int argc, char *argv[])
{
    int ch;
    while ((ch = getopt(argc, argv, OPTSTRING)) != -1){
        switch (ch) {
            case 'A':
                printf("%s\n","A");
                break;
            default:
                usage();
        }
    }

    Struct nargv = parseArgs(argc, argv);
    qsort(nargv.argv, nargv.length, sizeof(nargv.argv[0]), sortHelp);
    if(nargv.length > 1) {
        for(int i=1;i<nargv.length;i++){
            printf("\n%s\n",nargv.argv[i]);
            ls(nargv.argv[i]);
        }
    } else {
        printf("\n%s\n", "Current Directory (.)");
        ls(".");
    }
    return(0);
}