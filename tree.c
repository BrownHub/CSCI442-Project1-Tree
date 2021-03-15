#include <errno.h>
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

//Prints directory/file with correct indentation
int lvlPrint(char *dirName, int lvl);

//Recursive function to traverse directory tree
int dirWalk(char *drctry, int lvl);

//Returns filepath of symlink's target
char *readLnkPth(char *lnk, struct stat sb);

int main(int argc, char *argv[])
{
        int err;
        if (argc > 2){
                perror("usage: tree [optional: directory_name]");
                err = -1;
        } else if (argc == 2){
                err = dirWalk(argv[1], 0);
        } else {
                err = dirWalk(".", 0);
        }
        
        if(err != 0)
                return -1;
        
        return 0;
}

//Prints directory/file with correct indentation
int lvlPrint(char *dirName, int lvl)
{
        for (int i = 1; i < lvl; i++)
                printf("%s", "\t");
        
        printf("%s\n", dirName);

        return 0;
}

//Recursive function to traverse directory tree
int dirWalk(char *drctry, int lvl)
{
        struct stat filStat;
        DIR *dp;
        struct dirent *dirp;
        lvl++;
        
        if (lstat(drctry, &filStat) == -1) {
                perror("lstat");
                
                return -1;
        }
        
        if((filStat.st_mode & S_IFMT) == S_IFLNK){
                char *symPth = readLnkPth(drctry, filStat);
         
                if(dirWalk(symPth, lvl - 1) != 0){
                        free(symPth);
                        return -1;
                }
                
                free(symPth);
                return 0;
        }

        if ((dp = opendir(drctry)) == NULL){
                size_t msgSiz = strlen("Can't open  ") + strlen(drctry) + 1;
                char errMsg[msgSiz];
                snprintf(errMsg, msgSiz, "Can't open %s ", drctry);
                printf("%s\n", errMsg);

                fprintf(stderr, "Failed ");
                return -1;
        }
        
        while ((dirp = readdir(dp)) != NULL){
                if (strcmp(dirp->d_name, "..") == 0 || strcmp(dirp->d_name, ".") == 0)
                        continue;

                size_t filPthSiz = strlen(drctry) + strlen("/") + strlen(dirp->d_name) + 1;
                char filPth[filPthSiz];
                snprintf(filPth, filPthSiz, "%s/%s", drctry, dirp->d_name);

                if (dirp->d_type == DT_LNK){
                        struct stat sb;
                        
                        if (lstat(filPth, &sb) == -1) {
                                perror("lstat");
                                return -1;
                        }
                        
                        char *symPth = readLnkPth(filPth, sb);

                        size_t lnkPthSiz = strlen("SYM ");
                        lnkPthSiz = lnkPthSiz + strlen(dirp->d_name) + strlen(" -> ")
                                + strlen(symPth) + 1;
                        char lnkMsg[lnkPthSiz];
                        snprintf(lnkMsg, lnkPthSiz, "SYM %s -> %s", dirp->d_name, symPth);
                        
                        lvlPrint(lnkMsg, lvl);

                        free(symPth);
                } else if (dirp->d_type == DT_DIR){
                        lvlPrint(dirp->d_name, lvl);
                        dirWalk(filPth, lvl);
                } else {
                        lvlPrint(dirp->d_name, lvl);
                }
        }
        
        closedir(dp);        
        return 0;
}

//Returns filepath of symlink's target
char *readLnkPth(char *lnk, struct stat sb)
{
        ssize_t nbytes;
        char *buf = malloc(PATH_MAX);

        nbytes = readlink(lnk, buf, PATH_MAX);
        if (nbytes == -1)
                perror("readlink");
        
        if (nbytes == PATH_MAX)
                printf("(Returned buffer may have been truncated)");
                        
        buf[nbytes] = '\0';
                        
        return buf;
}

