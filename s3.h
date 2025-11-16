#ifndef _S3_H_
#define _S3_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_LINE 1024
#define MAX_ARGS 128
#define MAX_PROMPT_LEN 256

enum ArgIndex
{
    ARG_PROGNAME,
    ARG_1,
    ARG_2,
    ARG_3,
};

static inline void reap()
{
    wait(NULL);
}

///Shell I/O and related functions
void read_command_line(char line[], char lwd[]);
void construct_shell_prompt(char shell_prompt[]);
void parse_command(char line[], char *args[], int *argsc);

///Child functions
void child(char *args[], int argsc);
void child_with_output_redirected(char *args[], int argsc, bool append_redirect, char *output_file);
void child_with_input_redirected(char *args[], int argsc, char *input_file);

///Program launching functions
void launch_program(char *args[], int argsc);
void launch_program_with_redirection(char *args[], int argsc);

///Built-in command functions
bool is_cd(char *line);
void init_lwd(char lwd[]);
void run_cd(char *args[], int argsc, char lwd[]);

///Pipeline functions
bool is_pipelined(char *line);
int tokenise_pipeline(char *line, char *commands[]);
void launch_pipeline(char *commands[], int num_commands);

///Batch functions
bool is_batched(char *line);
int tokenise_batch(char *line, char *commands[]);
void run_cmd(char *command, char lwd[]);

///Helper functions for redirects
void setup_input_redirect(char *input_file);
void setup_output_redirect(bool append, char *output_file);

///Utility functions
bool command_with_redirection(char *line);
void reap_all(int num_processes);

///Subshell functions
bool has_subshell(char *line);
char* extract_subshell(char *line, char *extracted);
void launch_subshell(char *subshell_cmd);

#endif