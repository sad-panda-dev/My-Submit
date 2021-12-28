/* CSCI296 Project 03 : MySubmit
 * @author Rick Beaudet
 * This programs function is to copy a students file or files into a special directory to
 * hold the submission.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <pwd.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>

#define maxSize 1000
#define buffSize 4096

int checkIfDir(char *);
void showDir(char *);
int hideDirs(const struct dirent *);
const char *getUsername();
char *getCurDir();
void copyFile(char *, char *, char *);
void copyPermission(const char *, const char *);

int main()
{

    char home[maxSize]; // start the path of the submission directory
    strcpy(home, "/home/");
    char usrNam[maxSize];
    strcpy(usrNam, getUsername());       // I think that this is the appropriate way to do this because you will be using not me
    char *homDir = strcat(home, usrNam); // creates beginning of path ie "/home/username"

    printf("Please enter the name of the course: \n"); // prompt the user for the course name

    char usrInpt[maxSize];
    scanf("%s", usrInpt); // take the user input

    char sub[maxSize];
    strcpy(sub, "/submissions/");     // submit folder for path
    char *path = strcat(homDir, sub); // adds submissions folder to the path ie "/home/username/submissions/"

    char slash[maxSize];
    strcpy(slash, "/");

    // validate that the submissions directory exists
    if (checkIfDir(path) == -1)
    {
        printf("%s does not exist", path);
        exit(1);
    }

    char *newPath = strcat(path, usrInpt); // build path including submit folder and user input
                                           // ie "/home/username/submissions/coursename"
    char *fixdPath = strcat(newPath, slash);

    // validate that the directory exists
    if (checkIfDir(newPath) == -1)
    {
        printf("%s does not exist", newPath);
        exit(1);
    }

    char *usrSubDir = strcat(fixdPath, getUsername()); // grab the user's username to add to path

    //  printf(usrSubDir);
    if (checkIfDir(usrSubDir) == -1)
    {
        mkdir(usrSubDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH); // create user directory
                                                                 // within courses submission folder if none exists
    }

    printf("What is the name of the assignment you wish to submit: \n");
    // prompt the user to enter the name of the assignment
    // they wish to submit

    char *usrSubDirPath = strcat(usrSubDir, slash);
    // printf(usrSubDirPath);
    // printf("\n");

    char assDir[maxSize];
    scanf("%s", assDir); // take the users input for the name of the assignment

    char *fullSubPath = strcat(usrSubDirPath, assDir);
    //  printf(fullSubPath);
    printf("\n");

    if (checkIfDir(fullSubPath) == -1)
    { // create directory for assignment if none exists
        mkdir(fullSubPath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }

    char cwd[PATH_MAX]; // grab the current working directory of the user
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        printf("Current working dir: %s\n", cwd);
    }
    else
    {
        perror("getcwd() error"); // unable to get current working directory
        return 1;
    }
    printf("Here are the contents of your current working directory:\n\n");

    showDir(cwd); // print the contenst of the users current working directory

    // Prompt the user for the name/names of the assignment(s) they wish to submit
    printf("Enter the file(s) you want to submit seperated by a comma with no spaces \n");
    printf("Enter a '*' if you wish to submit all files:\n");

    char submitFiles[maxSize];

    scanf("%s", submitFiles); // take in the users input, either file names seperated by

    // printf(submitFiles);
    char filesToSubmit[maxSize];
    char source[maxSize];      // this will be the path for the files to be submitted
    char destination[maxSize]; // this will be the path of the location for the copies

    struct dirent **files;

    int numOfFiles = scandir(cwd, &files, hideDirs, alphasort);

    if (submitFiles[0] == '*') // copy everything
    {
        for (int i = 0; i < numOfFiles; i++)
        {
            // build path for file source
            strcpy(filesToSubmit, files[i]->d_name);
            strcpy(source, cwd);
            strcat(source, slash);
            strcat(source, filesToSubmit);

            // printf("Source = %s\n", source);  // for debug

            // build path for file destination
            strcpy(destination, fullSubPath);
            strcat(destination, slash);
            strcat(destination, filesToSubmit);

            // printf("destination = %s\n", destination);  // for debug

            copyFile(filesToSubmit, destination, source);
            copyPermission(source, destination);
        }
    }
    else
    {
        char submission[maxSize];
        strcpy(submission, submitFiles); // copy files entered in by user into the temporary array
        char *fileTokn;

        // couldn't figure out why, I know it is possible, but just using a space as a delimiter
        fileTokn = strtok(submission, ","); // didn't work properly, so I used a comma instead

        while (fileTokn != NULL)
        {
            // printf("Token = %s\n", fileTokn);  // for debug

            // build path for source

            strcpy(source, cwd);
            strcat(source, slash);
            strcat(source, fileTokn);

            // printf("Source = %s\n", source);  // for debug

            // build path for destination

            strcpy(destination, fullSubPath);
            strcat(destination, slash);
            strcat(destination, fileTokn);

            // printf("Destination = %s\n", destination); // for debug
            copyFile(fileTokn, destination, source);
            copyPermission(source, destination);

            fileTokn = strtok(NULL, ",");
        }
    }

    showDir(fullSubPath);

    return 0;
}

int checkIfDir(char *path)
/*
 * Function to check if given directory exists
 */
{
    struct stat info;
    return stat(path, &info);
}

void showDir(char *path)
/*
 * Function to display contents of directory in alphabetical order
 */
{
    struct dirent **contents;
    struct stat info;

    int names = scandir(path, &contents, hideDirs, alphasort);
    int i = 0;

    if (names == -1)
    { // error with scandir
        perror("scandir");
        exit(1);
    }

    // print the header
    // originally tried to set up the way that the instructions show
    // but I was unable to get the name to print on the same line as everything else
    // I think it had something to do with the format of ctime()
    printf("Contents of Directory:      Last Modified \n");
    printf("Size  Name                 Date       Time\n");

    while (i < names)
    { // print contents
        stat(contents[i]->d_name, &info);
        if (S_ISDIR(info.st_mode))
        { // we only want files so if it finds a directory we skip it
            i++;
        }
        printf("%5ld %-20s %-20s\n", info.st_size, contents[i]->d_name, ctime(&info.st_mtime));

        i++;
    }
}

int hideDirs(const struct dirent *entry)
/*
 * This will keep the hidden directories hidden when listing them
 */
{
    char begin = entry->d_name[0];
    return (begin != '.');
}

const char *getUsername()
/*
 * Function to grab the users username
 */
{
    struct passwd *pw_ptr;
    static char usrNum[20];

    if ((pw_ptr = getpwuid(getuid())) == NULL)
    {
        sprintf(usrNum, "%d", getuid());
        return usrNum;
    }
    return pw_ptr->pw_name;
}

void copyFile(char *file, char *destination, char *source)
/*
 * Function to copy file from source to destination
 *
 */
{
    int in_fd, out_fd, rd;
    char *buffer[buffSize];

    in_fd = open(file, O_RDONLY);

    if (in_fd == -1)
    {
        printf("Error: %s", strerror(errno));
        unlink(destination);
        exit(1);
    }

    out_fd = open(destination, O_RDWR | O_CREAT | O_TRUNC, 0666);

    if (out_fd == -1)
    {
        printf("Error: %s", strerror(errno));
        exit(1);
    }

    rd = read(in_fd, buffer, buffSize);

    while (rd > 0)
    {

        rd = write(out_fd, buffer, rd);

        if (rd == -1)
        {
            perror("write failure");
            exit(1);
        }

        rd = read(in_fd, buffer, buffSize);
    }
    close(in_fd);
    close(out_fd);
}

void copyPermission(const char *fromFile, const char *toFile)
/*
 * Function to give the copy the same permissions of the original file
 */
{
    struct stat tmp;
    stat(fromFile, &tmp);
    chmod(toFile, tmp.st_mode);
}
