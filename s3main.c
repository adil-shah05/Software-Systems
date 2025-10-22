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
    char *redirect_filename = NULL;

    while (1) {

        read_command_line(line);
        
        if(command_with_redirection(line))
        {///Command with redirection
           parse_command(line, args, &argsc, &input_redirect, &output_redirect, &redirect_filename);
           launch_program_with_redirection(args, &argsc, &input_redirect, &output_redirect, &redirect_filename);
           reap();
       }
       else ///Basic command
       {
           parse_command(line, args, &argsc, 0, 0, NULL);
           launch_program(args, argsc);
           reap();
       }
    }

    return 0;
    
}
