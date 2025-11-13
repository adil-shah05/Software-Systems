#include "s3.h"

int main(int argc, char *argv[]){

    ///Stores the command line input
    char line[MAX_LINE];

    ///The last (previous) working directory 
    char lwd[MAX_PROMPT_LEN-6]; 

    init_lwd(lwd);

    //Stores pointers to command arguments.
    ///The first element of the array is the command name.
    char *args[MAX_ARGS];

    ///Stores the number of arguments
    int argsc;

    while (1) {
        read_command_line(line, lwd);
        
        if(is_batched(line)) {
            char *batch_commands[MAX_ARGS];
            int num_batch = tokenize_batch(line, batch_commands);
            
            for(int i = 0; i < num_batch; i++) {
                run_cmd(batch_commands[i], lwd);
            }
        } else {
            run_cmd(line, lwd);
    }

    return 0;
}