#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

//#define STD_SZ 1024
//#define TOKENS_SZ 100
int std_sz = 1024;
int max_args = 100;

char* get_directory(){
    char *dir;
    size_t size = std_sz;

    dir = (char *)malloc(size * sizeof(char));

    if (dir!=NULL){ //check memory was allocated correctly
       
        if (getcwd(dir, size) != NULL){ 
            return dir; //since getcwd() will store the result into buffer
        
        } else {
            free(dir);
            exit(1); //getcwd failed
        }
    }
    return NULL;
}
char* get_hn(){
    char *host;
    size_t size = std_sz;

    host = (char *)malloc(size * sizeof(char));

    if (host != NULL){

        int h = gethostname(host, size);
        if (h == 0){
            return host;
        }
    } else {
        free(host);
        exit(1); //malloc error or hostname error
    }
    return NULL;
}

void tokenize_args(char usr_input[], char* args[]){
    char *token;
    int num_tokens = 0;

    //remove newline
    usr_input[strcspn(usr_input, "\n")] = '\0';
    
    //tokenize and store tokens in args[]
    token = strtok(usr_input, " ");
    
    while (token != NULL && num_tokens < max_args){
        args[num_tokens] = token;
        num_tokens++;
        token = strtok(NULL, " ");
    }

    //mark end of args?
    if (num_tokens < max_args){
        args[num_tokens] = NULL;
    }
}

void part_1(){
    char *cwd = get_directory();
    char *usr = getlogin();
    char *host = get_hn();
    
    char usr_input[std_sz]; //declare buffer for storing user input
    char *args[max_args]; //to store args from input
    pid_t pid;

    //print usr@host: cwd > and then get commands from user
    printf("%s@%s: %s > ", usr, host, cwd);
    fgets(usr_input, sizeof(usr_input), stdin);

    //tokenize the input
    tokenize_args(usr_input, args);

    //fork and execute
    pid = fork();
    if (pid < 0) {
        //fork failed
        perror("part 1 fork failed");
        exit(1);
    } else if (pid == 0) {
        //child process - execute user commands
        execvp(args[0], args);

        //following line is executed if execvp fails
        perror("execvp failed");
    } else {
        //parent process - wait for child to finish
        //is this needed though?
        wait(NULL);

    }
    //free dynamically allocated memory - could maybe do this at the end of main?
    free(cwd);
    free(host);
}

//remember to free cwd and host since malloc was used!
int main(){
    part_1();
    return 0;
}