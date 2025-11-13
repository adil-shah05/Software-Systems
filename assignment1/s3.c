#include "s3.h"

// Global Redirect flags
static bool g_input_redirect = false;
static bool g_output_redirect = false;
static bool g_append_redirect = false;
static char *g_redirect_filename = NULL;

///Simple for now, but will be expanded in a following section
void construct_shell_prompt(char shell_prompt[])
{
    strcpy(shell_prompt, "[s3]$ ");
}

///Prints a shell prompt and reads input from the user
void read_command_line(char line[], char lwd[])
{
    char shell_prompt[MAX_PROMPT_LEN];
    construct_shell_prompt(shell_prompt);
    printf("%s", shell_prompt);

    ///See man page of fgets(...)
    if (fgets(line, MAX_LINE, stdin) == NULL)
    {
        exit(1);
    }
    ///Remove newline (enter)
    line[strlen(line) - 1] = '\0';
}

void parse_command(char line[], char *args[], int *argsc)
{
    // Reset global redirect flags
    g_input_redirect = false;
    g_output_redirect = false;
    g_append_redirect = false;
    g_redirect_filename = NULL;
    
    char *token = strtok(line, " ");
    *argsc = 0;

    while (token != NULL && *argsc < MAX_ARGS - 1)
    {
        if(strcmp(token, "<") == 0){
            g_input_redirect = true;
            token = strtok(NULL, " ");  
            g_redirect_filename = token;
            token = strtok(NULL, " ");  
            continue;
        }

        if(strcmp(token,">>") == 0) {
            g_append_redirect = true;
            token = strtok(NULL, " ");  
            g_redirect_filename = token;
            token = strtok(NULL, " ");  
            continue;
        } 

        if(strcmp(token,">") == 0) {  
            g_output_redirect = true;
            token = strtok(NULL, " ");  
            g_redirect_filename = token;
            token = strtok(NULL, " ");  
            continue;
        }

        args[(*argsc)++] = token;
        token = strtok(NULL, " ");
    }
    
    args[*argsc] = NULL; ///args must be null terminated
}

void reap_all(int num_processes){
    for(int i = 0; i < num_processes; i++) {
        wait(NULL);
    }
}

///Launch related functions
void child(char *args[], int argsc)
{
    execvp(args[ARG_PROGNAME], args);
    exit(1);
}

void child_with_output_redirected(char *args[], int argsc, bool append_redirect, char *output_file){
    setup_output_redirect(append_redirect, output_file);
    execvp(args[ARG_PROGNAME], args);
    exit(1);
}

void child_with_input_redirected(char *args[], int argsc, char *input_file){
    setup_input_redirect(input_file);
    execvp(args[ARG_PROGNAME], args);
    exit(1);
}

void launch_program(char *args[], int argsc){
    int rc = fork();

    if(rc == 0){
        child(args, argsc);
    }else{
        wait(NULL);
    }
}

void launch_program_with_redirection(char *args[], int argsc){
    int rc = fork(); 

    if(rc == 0) {  // Child process
        if(g_output_redirect || g_append_redirect) {
            child_with_output_redirected(args, argsc, g_append_redirect, g_redirect_filename);
        } else if(g_input_redirect) {
            child_with_input_redirected(args, argsc, g_redirect_filename);
        }
    } else {  // Parent process
        wait(NULL);
    }
}

bool command_with_redirection(char *line){
    if (strchr(line, '<') != NULL || strchr(line, '>') != NULL) {
        return true;
    }else{
        return false;
    }
}

bool is_cd(char *line){
    // Skip leading spaces
    while(*line == ' ') line++;
    
    // Check if starts with "cd" followed by space or end of string
    return (strncmp(line, "cd ", 3) == 0 || strcmp(line, "cd") == 0);
}

void init_lwd(char lwd[]){
    // Get current working directory and store in lwd
    if(getcwd(lwd, MAX_PROMPT_LEN - 6) == NULL){
        exit(1);
    }

}

void run_cd(char *args[], int argsc, char lwd[]) {
    // Save current directory before changing
    char current_dir[MAX_PROMPT_LEN - 6];
    if(getcwd(current_dir, MAX_PROMPT_LEN - 6) == NULL) {
        return;
    }
    
    char *home_dir = getenv("HOME");
    
    if(argsc == 1 || args[1] == NULL) {
        // cd with no args → go home
        if(home_dir == NULL) {
            return;
        }
        if(chdir(home_dir) != 0) {
            return;
        }
    }else if(strcmp(args[1], "-") == 0) {
        // cd - → go to previous directory
        if(chdir(lwd) != 0) {
            return;
        }

        printf("%s\n", lwd);  // Print the directory we switched to
    } else {
        // cd <directory>
        if(chdir(args[1]) != 0) {
            return;
        }
    }
    
    // Update lwd to where we were before the change
    strcpy(lwd, current_dir);
}

bool is_pipelined(char *line){
    if(strchr(line, '|') != NULL){
        return true;
    }

    return false;
}

bool is_batched(char *line){
    if(strchr(line, ';') != NULL){
        return true;
    }

    return false;
}

int tokenise_pipeline(char *line, char *commands[]){
    
    char *token = strtok(line, "|");
    int num_commands = 0;

    while (token != NULL && num_commands < MAX_ARGS - 1){
        commands[(num_commands)++] = token;
        token = strtok(NULL, "|");
    }
    
    return num_commands; 
}

int tokenise_batch(char *line, char *commands[]){
    
    char *token = strtok(line, ";");
    int num_commands = 0;

    while (token != NULL && num_commands < MAX_ARGS - 1){
        commands[(num_commands)++] = token;
        token = strtok(NULL, ";");
    }
    
    return num_commands; 
}

void launch_pipeline(char *commands[], int num_commands){
    int pipefd[num_commands - 1][2];

    //Create all pipes
    for(int j = 0; j < num_commands - 1; j++) {
        pipe(pipefd[j]);
    }

    //Fork for each command
    for(int i = 0; i < num_commands; i++){
        int rc = fork();

        if(rc == 0){  // Child
            char *args[MAX_ARGS];
            int argsc;
            parse_command(commands[i], args, &argsc);

            
            if(g_input_redirect) {
                setup_input_redirect(g_redirect_filename);
            } else if(i > 0) {
                dup2(pipefd[i-1][0], STDIN_FILENO);
            }
            
            if(g_output_redirect || g_append_redirect) {
                setup_output_redirect(g_append_redirect, g_redirect_filename);
            } else if(i < num_commands - 1) {
                dup2(pipefd[i][1], STDOUT_FILENO);
            }
            
            // Close all pipes
            for(int k = 0; k < num_commands - 1; k++) {
                close(pipefd[k][0]);
                close(pipefd[k][1]);
            }
            
            execvp(args[0], args);
            exit(1);
        }
    }

    // Parent: Close all pipes
    for(int k = 0; k < num_commands - 1; k++) {
        close(pipefd[k][0]);
        close(pipefd[k][1]);
    }

    // Parent: wait for all children
    reap_all(num_commands);
}

void setup_input_redirect(char *input_file) {
    int fd = open(input_file, O_RDONLY);
    if(fd >= 0) {
        dup2(fd, STDIN_FILENO);
        close(fd);
    }
}

void setup_output_redirect(bool append, char *output_file) {
    int fd;
    if(append) {
        fd = open(output_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
    } else {
        fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    }
    if(fd >= 0) {
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
}

void run_cmd(char *command, char lwd[]) {
    char *args[MAX_ARGS];
    int argsc;
    
    if(is_cd(command)) {
        parse_command(command, args, &argsc);
        run_cd(args, argsc, lwd);
    } else if(is_pipelined(command)) {
        char *commands[MAX_ARGS];
        int num_commands = tokenise_pipeline(command, commands);
        launch_pipeline(commands, num_commands);
    } else if(command_with_redirection(command)) {
        parse_command(command, args, &argsc);
        launch_program_with_redirection(args, argsc);
        reap();
    } else {
        parse_command(command, args, &argsc);
        launch_program(args, argsc);
        reap();
    }
}

