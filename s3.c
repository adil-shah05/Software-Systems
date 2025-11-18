#include "s3.h"

// Global redirect flags and filenames
static bool g_input_redirect = false;
static bool g_output_redirect = false;
static bool g_append_redirect = false;
static char *g_input_filename = NULL;
static char *g_output_filename = NULL;

// Constructs the shell prompt string
void construct_shell_prompt(char shell_prompt[])
{
    strcpy(shell_prompt, "[s3]$ ");
}

// Reads a command line from user input
void read_command_line(char line[], char lwd[])
{
    char shell_prompt[MAX_PROMPT_LEN];
    construct_shell_prompt(shell_prompt);
    printf("%s", shell_prompt);

    if (fgets(line, MAX_LINE, stdin) == NULL)
    {
        exit(1);
    }
    line[strlen(line) - 1] = '\0';
}

// Parses command line into args array and detects redirects
void parse_command(char line[], char *args[], int *argsc)
{
    g_input_redirect = false;
    g_output_redirect = false;
    g_append_redirect = false;
    g_input_filename = NULL;
    g_output_filename = NULL;
    
    char *token = strtok(line, " ");
    *argsc = 0;

    while (token != NULL && *argsc < MAX_ARGS - 1)
    {
        if(strcmp(token, "<") == 0){
            g_input_redirect = true;
            token = strtok(NULL, " ");  
            g_input_filename = token;
            token = strtok(NULL, " ");  
            continue;
        }

        if(strcmp(token,">>") == 0) {
            g_append_redirect = true;
            token = strtok(NULL, " ");  
            g_output_filename = token;
            token = strtok(NULL, " ");  
            continue;
        } 

        if(strcmp(token,">") == 0) {  
            g_output_redirect = true;
            token = strtok(NULL, " ");  
            g_output_filename = token;
            token = strtok(NULL, " ");  
            continue;
        }

        args[(*argsc)++] = token;
        token = strtok(NULL, " ");
    }
    
    args[*argsc] = NULL;
}

// Checks if command is cd
bool is_cd(char *line){
    while(*line == ' ') line++;
    return (strncmp(line, "cd ", 3) == 0 || strcmp(line, "cd") == 0);
}

// Initializes last working directory with current directory
void init_lwd(char lwd[]){
    if(getcwd(lwd, MAX_PROMPT_LEN - 6) == NULL){
        exit(1);
    }
}

// Executes cd command with support for home and previous directory
void run_cd(char *args[], int argsc, char lwd[]) {
    char current_dir[MAX_PROMPT_LEN - 6];
    if(getcwd(current_dir, MAX_PROMPT_LEN - 6) == NULL) {
        return;
    }
    
    char *home_dir = getenv("HOME");
    
    if(argsc == 1 || args[1] == NULL) {
        if(home_dir == NULL) {
            return;
        }
        if(chdir(home_dir) != 0) {
            return;
        }
    }else if(strcmp(args[1], "-") == 0) {
        if(chdir(lwd) != 0) {
            return;
        }
        printf("%s\n", lwd);
    } else {
        if(chdir(args[1]) != 0) {
            return;
        }
    }
    
    strcpy(lwd, current_dir);
}

// Checks if command contains pipes
bool is_pipelined(char *line){
    bool in_quotes = false;
    char quote_char = '\0';
    
    for(char *p = line; *p; p++) {
        if((*p == '"' || *p == '\'') && !in_quotes) {
            in_quotes = true;
            quote_char = *p;
        } else if(in_quotes && *p == quote_char) {
            in_quotes = false;
        }
        
        if(!in_quotes && *p == '|') {
            return true;
        }
    }
    return false;
}

// Checks if command contains batch separator
bool is_batched(char *line){
    bool in_quotes = false;
    char quote_char = '\0';
    
    for(char *p = line; *p; p++) {
        if((*p == '"' || *p == '\'') && !in_quotes) {
            in_quotes = true;
            quote_char = *p;
        } else if(in_quotes && *p == quote_char) {
            in_quotes = false;
        }
        
        if(!in_quotes && *p == ';') {
            return true;
        }
    }
    return false;
}

// Checks if command contains subshell
bool has_subshell(char *line) {
    bool in_quotes = false;
    char quote_char = '\0';
    bool has_open = false;
    bool has_close = false;
    
    for(char *p = line; *p; p++) {
        if((*p == '"' || *p == '\'') && !in_quotes) {
            in_quotes = true;
            quote_char = *p;
        } else if(in_quotes && *p == quote_char) {
            in_quotes = false;
        }
        
        if(!in_quotes) {
            if(*p == '(') has_open = true;
            if(*p == ')') has_close = true;
        }
    }
    return has_open && has_close;
}

// Checks if command has I/O redirection
bool command_with_redirection(char *line){
    bool in_quotes = false;
    char quote_char = '\0';
    
    for(char *p = line; *p; p++) {
        if((*p == '"' || *p == '\'') && !in_quotes) {
            in_quotes = true;
            quote_char = *p;
        } else if(in_quotes && *p == quote_char) {
            in_quotes = false;
        }
        
        if(!in_quotes && (*p == '<' || *p == '>')) {
            return true;
        }
    }
    return false;
}

// Splits command line by pipe delimiters
int tokenise_pipeline(char *line, char *commands[]){
    char *token = strtok(line, "|");
    int num_commands = 0;

    while (token != NULL && num_commands < MAX_ARGS - 1){
        commands[(num_commands)++] = token;
        token = strtok(NULL, "|");
    }
    
    return num_commands; 
}

// Splits command line by semicolon delimiters
int tokenise_batch(char *line, char *commands[]){
    char *token = strtok(line, ";");
    int num_commands = 0;

    while (token != NULL && num_commands < MAX_ARGS - 1){
        commands[(num_commands)++] = token;
        token = strtok(NULL, ";");
    }
    
    return num_commands; 
}

// Extracts content between matching parentheses
char* extract_subshell(char *line, char *extracted) {
    char *start = strchr(line, '(');
    if(!start) return NULL;
    
    int depth = 0;
    char *p = start;
    
    while(*p) {
        if(*p == '(') depth++;
        if(*p == ')') {
            depth--;
            if(depth == 0) {
                int len = p - start - 1;
                strncpy(extracted, start + 1, len);
                extracted[len] = '\0';
                return extracted;
            }
        }
        p++;
    }
    
    return NULL; 
}

// Child process that executes a program
void child(char *args[], int argsc)
{
    execvp(args[ARG_PROGNAME], args);
    exit(1);
}

// Child process with output redirection
void child_with_output_redirected(char *args[], int argsc, bool append_redirect, char *output_file){
    setup_output_redirect(append_redirect, output_file);
    execvp(args[ARG_PROGNAME], args);
    exit(1);
}

// Child process with input redirection
void child_with_input_redirected(char *args[], int argsc, char *input_file){
    setup_input_redirect(input_file);
    execvp(args[ARG_PROGNAME], args);
    exit(1);
}

// Launches a basic program without redirection
void launch_program(char *args[], int argsc){
    int rc = fork();

    if(rc == 0){
        child(args, argsc);
    }else{
        wait(NULL);
    }
}

// Launches program with I/O redirection
void launch_program_with_redirection(char *args[], int argsc){
    int rc = fork(); 

    if(rc == 0) {
        if(g_input_redirect) {
            setup_input_redirect(g_input_filename);
        }
        if(g_output_redirect || g_append_redirect) {
            setup_output_redirect(g_append_redirect, g_output_filename);
        }
        execvp(args[ARG_PROGNAME], args);
        exit(1);
    } else {
        wait(NULL);
    }
}

// Sets up input redirection from file
void setup_input_redirect(char *input_file) {
    int fd = open(input_file, O_RDONLY);
    if(fd >= 0) {
        dup2(fd, STDIN_FILENO);
        close(fd);
    }
}

// Sets up output redirection to file
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

// Launches a pipeline of commands with pipes
void launch_pipeline(char *commands[], int num_commands){
    int pipefd[num_commands - 1][2];

    for(int j = 0; j < num_commands - 1; j++) {
        pipe(pipefd[j]);
    }

    for(int i = 0; i < num_commands; i++){
        int rc = fork();

        if(rc == 0){
            char *args[MAX_ARGS];
            int argsc;
            
            if(has_subshell(commands[i])) {
                char extracted[MAX_LINE];
                if(extract_subshell(commands[i], extracted)) {
                    if(i > 0) {
                        dup2(pipefd[i-1][0], STDIN_FILENO);
                    }
                    if(i < num_commands - 1) {
                        dup2(pipefd[i][1], STDOUT_FILENO);
                    }
                    
                    for(int k = 0; k < num_commands - 1; k++) {
                        close(pipefd[k][0]);
                        close(pipefd[k][1]);
                    }
                    
                    execvp("./s3", (char*[]){"s3", "-sub", extracted, NULL});
                    exit(1);
                }
            }
            
            parse_command(commands[i], args, &argsc);
            
            if(g_input_redirect) {
                setup_input_redirect(g_input_filename);
            } else if(i > 0) {
                dup2(pipefd[i-1][0], STDIN_FILENO);
            }
            
            if(g_output_redirect || g_append_redirect) {
                setup_output_redirect(g_append_redirect, g_output_filename);
            } else if(i < num_commands - 1) {
                dup2(pipefd[i][1], STDOUT_FILENO);
            }
            
            for(int k = 0; k < num_commands - 1; k++) {
                close(pipefd[k][0]);
                close(pipefd[k][1]);
            }
            
            execvp(args[0], args);
            exit(1);
        }
    }

    for(int k = 0; k < num_commands - 1; k++) {
        close(pipefd[k][0]);
        close(pipefd[k][1]);
    }

    reap_all(num_commands);
}

// Launches a subshell by spawning new shell instance
void launch_subshell(char *subshell_cmd) {
    int rc = fork();
    
    if(rc == 0) {
        execvp("./s3", (char*[]){"s3", "-sub", subshell_cmd, NULL});
        exit(1);
    } else {
        wait(NULL);
    }
}

// Executes a single command with appropriate handler
void run_cmd(char *command, char lwd[]) {
    char *args[MAX_ARGS];
    int argsc;
    
    char *p = command;
    while(*p == ' ' || *p == '\t') p++;
    
    if(*p == '(' && has_subshell(command)) {
        char extracted[MAX_LINE];
        if(extract_subshell(command, extracted)) {
            launch_subshell(extracted);
            return;
        }
    }
    
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

// Waits for multiple child processes
void reap_all(int num_processes){
    for(int i = 0; i < num_processes; i++) {
        wait(NULL);
    }
}