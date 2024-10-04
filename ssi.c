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
        free(host);

    }
    exit(1); //malloc error or hostname error

}

void tokenize_args(char usr_input[], char* args[]){
    char *token;
    int num_tokens = 0;
    
    //tokenize and store tokens in args[]
    token = strtok(usr_input, " ");
    
    while (token != NULL && num_tokens < max_args){
        args[num_tokens] = token;
        num_tokens++;
        token = strtok(NULL, " \n");
    }

    //mark end of args?
    if (num_tokens < max_args){
        args[num_tokens] = '\0';
    }
}

/*leftshift, array_to_str, create_bgpro, and append are part3 helpers*/
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
    new_bg->command = strdup(command_str);

    //check if strdup failed
    if (!new_bg->command) {
        free(new_bg);
        exit(1);
    }
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
        exit(1);
    } else {
        //parent process - wait for child to finish so that child does not enter 'zombie state'
        // wait for this specific child process to finish
        pid_t w_pid = waitpid(pid, NULL, 0);
        
        if (w_pid == -1) {
            perror("waitpid");
            exit(1);
        }

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

void part3(char *args[], int* num_bgs, bg_pro** head){
    leftshift(args);

    pid_t pid;
    pid = fork();
    
    if (pid < 0) {
        //fork failed
        perror("fork failed");
        exit(1);
    
    } else if (pid==0){ 
        //child process

        //redirect output so it doesn't display on terminal
        if (freopen("/dev/null", "w", stdout) == NULL) {
            perror("freopen stdout failed");
            exit(1);
        }

        if (freopen("/dev/null", "w", stderr) == NULL) {
            perror("freopen stderr failed");
            exit(1);
        }
       
        execvp(args[0], args);
        
        //following line is executed if execvp fails
        perror("execvp failed");
        exit(1);
        
    } else {
        //create bg in list
        bg_pro *new_bg = create_bg_pro(pid, args);
        append(head, new_bg, num_bgs);
    }      
}
void bglist(bg_pro* head, int num_bgs){
    bg_pro* cur = head;
    int i=1;
    while(cur != NULL && i<=num_bgs){
       printf("%d: %s\n", cur->pid, cur->command); 
       i++;
    }
    printf("Total Background jobs: %d\n", num_bgs);
}

void free_bg_list(bg_pro* head) {
    bg_pro *cur = head;
    while (cur != NULL) {
        bg_pro *next = cur->next;
        free(cur->command);  // free command since it was dynamically allocated with strdup
        free(cur);           // free the struct itself
        cur = next;
    }
}

void sigintHandler(int sig_num) { 
    //reset
    signal(SIGINT, sigintHandler);
    return;
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

        while(strcmp(usr_input, "\n")==0){
            printf("%s@%s: %s > ", usr, host, cwd);
            fgets(usr_input, sizeof(usr_input), stdin);
        }

        //remove newline from user input
        usr_input[strcspn(usr_input, "\n")] = '\0';

        //check for "exit" command
        if(strcmp(ex, usr_input) == 0){
            break;
        }
        //tokenize the input
        tokenize_args(usr_input, args);
        
        //cd
        if (strcmp(args[0], "cd") == 0){
            part2(args);
        
        //bg
        } else if (strcmp(args[0], "bg") == 0){
            part3(args, &num_bgs, &head);
        
        //bglist
        } else if (strcmp(args[0], "bglist") == 0){
            bglist(head, num_bgs);

        //regular command
        }else {
            part1(args);
            
        }
        
        //check if bg process has terminated
        if (num_bgs > 0){
            pid_t ter = waitpid(0, NULL, WNOHANG);
            while (ter > 0){
                if(ter>0){ //some child has terminated
                    if(head->pid == ter){
                        printf("%d: %s has terminated.\n", head->pid, head->command);
                        bg_pro* temp = head; //remove terminated process from list
                        head = head->next;
                        free(temp); //to free after a process has been removed
                    } else {
                        bg_pro* cur = head;

                        //find terminated process
                        while(cur->next->pid != ter){
                            cur = cur->next;
                        }
                        printf("%d: %s has terminated.\n", cur->next->pid, cur->next->command);
                        
                        //remove terminated process from list
                        bg_pro* temp = cur->next;
                        cur->next = cur->next->next;
                        free(temp);

                        
                    }
                    num_bgs--;
                }
                //update ter
                ter = waitpid(0, NULL, WNOHANG);
            }
        }
        free(cwd);
    }
    //free(cwd);
    free(host);
    free_bg_list(head);
    
    return;
    
    
}

//remember to free!
int main(){
    //set handler for ctrl C
    signal(SIGINT, sigintHandler);
    run_shell();
    
    return 0;
}