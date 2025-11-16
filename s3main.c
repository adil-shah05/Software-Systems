#include "s3.h"

int main(int argc, char *argv[]){

    char line[MAX_LINE];
    char lwd[MAX_PROMPT_LEN-6]; 

    init_lwd(lwd);

    char *args[MAX_ARGS];

    int argsc;

    if(argc >= 3 && strcmp(argv[1], "-sub") == 0) {
        strcpy(line, argv[2]);
        
        if(is_batched(line)) {
            char *batch_commands[MAX_ARGS];
            int num_batch = tokenise_batch(line, batch_commands);

            for(int i = 0; i < num_batch; i++) {
                run_cmd(batch_commands[i], lwd);
            }
            
        } else {
            run_cmd(line, lwd);
        }
        
        exit(0);
    }

    while (1) {
        read_command_line(line, lwd);
        
        char *p = line;
        while(*p == ' ' || *p == '\t') p++;
        
        if(*p == '(' && has_subshell(line)) {
            char extracted[MAX_LINE];
            if(extract_subshell(line, extracted)) {
                launch_subshell(extracted);
            }
            
        } else if(is_batched(line)) {
            char *batch_commands[MAX_ARGS];
            int num_batch = tokenise_batch(line, batch_commands);
            for(int i = 0; i < num_batch; i++) {
                run_cmd(batch_commands[i], lwd);
            }
        } else {
            run_cmd(line, lwd);
        }
    }

    return 0;
}