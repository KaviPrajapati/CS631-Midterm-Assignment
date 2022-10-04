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

void
ls(char* path)
{
    DIR *dp;
    struct dirent *dirp;
    char buf[PATH_MAX];
    char *dire_path = realpath(path, buf);

	if ((dp = opendir(dire_path)) == NULL){
		fprintf(stderr, "can't open '%s'\n", dire_path);
		exit(1);
	}

	while ((dirp = readdir(dp)) != NULL){
        printf("%s\n", dirp->d_name);
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
    for(int i=0;i<nargv.length;i++){
        printf("%s\n",nargv.argv[i]);
    }
    // ls(".");
    return(0);
}