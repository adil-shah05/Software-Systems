#include "s3.h"

///Simple for now, but will be expanded in a following section
void construct_shell_prompt(char shell_prompt[])
{
    strcpy(shell_prompt, "[s3]$ ");
}

///Prints a shell prompt and reads input from the user
void read_command_line(char line[])
{
    char shell_prompt[MAX_PROMPT_LEN];
    construct_shell_prompt(shell_prompt);
    printf("%s", shell_prompt);

    ///See man page of fgets(...)
    if (fgets(line, MAX_LINE, stdin) == NULL)
    {
        perror("fgets failed");
        exit(1);
    }
    ///Remove newline (enter)
    line[strlen(line) - 1] = '\0';
}

void parse_command(char line[], char *args[], int *argsc, bool *input_redirect, bool *output_redirect, bool *redirect_filename)
{
    
    char *token = strtok(line, " ");
    *argsc = 0;

    while (token != NULL && *argsc < MAX_ARGS - 1)
    {
    
        if(strcmp(token, "<") == 0){
            *input_redirect = true;

            token = strtok(NULL, " ");
            *redirect_filename = token;

            continue;
        }

        if(strcmp(token,">") == 0){
            *output_redirect = true;

            token = strtok(NULL, " ");
            *redirect_filename = token;

            continue;
        }

        args[(*argsc)++] = token;
        token = strtok(NULL, " ");
    }
    
    args[*argsc] = NULL; ///args must be null terminated
}

///Launch related functions
void child(char *args[], int argsc)
{
    execvp(args[ARG_PROGNAME], args);
}

void child_with_output_redirected(char *args[], int argsc, char *output_file){
    int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    dup2(fd, STDOUT_FILENO);
    close(fd);

    execvp(args[ARG_PROGNAME], args);
}

void child_with_input_redirected(char *args[], int argsc, char *input_file){
    int fd = open(input_file, O_RDONLY | O_CREAT | O_TRUNC, 0644);
    dup2(fd, STDIN_FILENO);
    close(fd);

    execvp(args[ARG_PROGNAME], args);
}

void launch_program(char *args[], int argsc)
{

    int rc = fork();

    if(rc == 0){
        child(args, argsc);
    }else{
        int rc = wait(NULL);
    }


}

void launch_program_with_redirection(char *args[], int *argsc, bool *input_redirect, bool *output_redirect, char *redirect_filename){
    
    int rc = fork(); 

    if(rc == 0) {  // Child process
        
        if(*output_redirect) {
            child_with_output_redirected(args, *argsc, redirect_filename);
        } else if(*input_redirect) {
            child_with_input_redirected(args, *argsc, redirect_filename);
        }

    } else {  // Parent process
       wait(NULL);
    }
}

void command_with_redirection(char line[]){
    if (strchr(line, '<') != NULL || strchr(line, '>') != NULL) {
        return true;
    }else{
        return false;
    }
}