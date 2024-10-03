#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

//#define STD_SZ 1024
//#define TOKENS_SZ 100
int std_sz = 1024;
int max_args = 100;

//define bg process structure
typedef struct bg_pro{
    pid_t pid; 
    char *command;
    struct bg_pro* next;
} bg_pro;

/*get_directory, get_hn, and tokenize_args are run_shell helpers*/
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

/*leftshift, array_to_str, create_bgpro, and append are
part3 helpers*/
void leftshift(char *arr[]){
    int i = 0;
    
    while(arr[i+1]!=NULL){
        //printf("iteration %d:\n%s\n", i, arr[i]);
        arr[i] = arr[i+1];
        //printf("%s\n", arr[i]);
        i++;
    }
    arr[i] = '\0';
}

void array_to_str(char* arr[], char* buffer){
    //takes an array and transforms it into a string
    int i = 0;
    int len;
    buffer[0] = '\0';
    
    while(arr[i]!=NULL){
        strcat(buffer, arr[i]);
        strcat(buffer, " ");
        i++;
        
    }
    //now remove extra space and add null terminator
    len = strlen(buffer);
    buffer[len-1] = '\0';
    //printf("args: %s", buffer);
}

bg_pro* create_bg_pro(pid_t pid, char* args[]){ //*args or args[]?
    bg_pro *new_bg = (bg_pro *)malloc(sizeof(bg_pro));
    
    if(!new_bg){
        exit(1);
    }
    
    char command_str[1024];
    array_to_str(args, command_str);
    new_bg->pid = pid;
    new_bg->command = command_str;
    new_bg->next = NULL;

    return new_bg;
}

void append(bg_pro** head, bg_pro* new_bg, int* num_bgs){

    //add bg to linked list
    if (*num_bgs == 0 && *head == NULL){
        *head = new_bg;

    } else {
        //find end
        bg_pro* cur = *head;

        while (cur->next != NULL){
            cur = cur->next;
        }
        //add at end
        cur->next = new_bg;
    }
    (*num_bgs)++;
    return;
}

/* modularized parts 1, 2, 3 to be called in run_shell */
void part1(char *args[]){
    //fork and execute
    pid_t pid;
    pid = fork();
    
    if (pid < 0) {
        //fork failed
        perror("fork failed");
        exit(1);
    
    } else if (pid == 0) {
        //child process - execute user commands
        
        execvp(args[0], args);

        //following line is executed if execvp fails
        perror("execvp failed");
    } else {
        //parent process - wait for child to finish so that child does not enter 'zombie state'
        wait(NULL);

    }

}

void part2(char *args[]){

    char *dest = args[1]; 
    char *home = getenv("HOME");
    
    if (dest==NULL || strcmp(dest, "~") == 0){
        //go to home
        chdir(home);

    } else {
        //go to dest
        if (chdir(dest)==0){
            // chdir successful
            return;
        
        } else {
            perror("chdir failed");
            exit(1);
        }
    }
    return;
}

void part3(char *args[], int num_bgs, bg_pro* head){
    leftshift(args);

    pid_t pid;
    pid = fork();
    
    if (pid < 0) {
        //fork failed
        perror("fork failed");
        exit(1);
    
    } else if (pid==0){ 
        execvp(args[0], args);

        //following line is executed if execvp fails
        perror("execvp failed");
    } else {
        //create bg in list
        bg_pro *new_bg = create_bg_pro(pid, args);
        append(&head, new_bg, &num_bgs);
    }  
    
}


void run_shell(){
    char usr_input[std_sz]; //declare buffer for storing user input
    char *args[max_args]; //to store args from input
    const char *ex = "exit";
    int num_bgs = 0;
    
    char *usr = getlogin();
    char *host = get_hn();
    char *cwd;

    bg_pro* head = NULL; //initialize head ptr

    //print usr@host: cwd > and then get commands from user
    while(1){
        //open command line for user input
        cwd = get_directory();
        printf("%s@%s: %s > ", usr, host, cwd);
        fgets(usr_input, sizeof(usr_input), stdin);

        //remove newline from user input
        usr_input[strcspn(usr_input, "\n")] = '\0';

        //check for "exit" command
        if(strcmp(ex, usr_input)==0){
            break;
        }
        //tokenize the input
        tokenize_args(usr_input, args);
        
        if (strcmp(args[0], "cd")==0){
            part2(args);
        } else if (strcmp(args[0], "bg")==0){
            part3(args, num_bgs, head);
        }
        else {
            part1(args);
            
        }
    }
    free(cwd);
    free(host);
    return;
    
    
}

//remember to free cwd and host since malloc was used!
int main(){
    run_shell();
    
    return 0;
}