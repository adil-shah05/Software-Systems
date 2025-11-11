#include "s3.h"

int main(int argc, char *argv[]){

    ///Stores the command line input
    char line[MAX_LINE];

    ///Stores pointers to command arguments.
    ///The first element of the array is the command name.
    char *args[MAX_ARGS];

    ///Stores the number of arguments
    int argsc;

    //redirect detection and data storage
    bool input_redirect = false;
    bool output_redirect = false;
    bool append_redirect = false;
    char *redirect_filename = NULL;

    while (1) {

        input_redirect = false;
        output_redirect = false;
        append_redirect = false;
        redirect_filename = NULL;

        read_command_line(line);

        parse_command(line, args, &argsc, &input_redirect, &output_redirect, &append_redirect, &redirect_filename);

        if(strcmp(args[0], "cd") == 0){
            chdir(args[1]);
        }
        else if(command_with_redirection(line)) ///Command with redirection
        {
           launch_program_with_redirection(args, &argsc, &input_redirect, &output_redirect, &append_redirect, redirect_filename);
           reap();
       }
       else ///Basic command
       {
           launch_program(args, argsc);
           reap();
       }
    }

    return 0;
    
}
