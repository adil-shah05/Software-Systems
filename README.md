# Unix Shell Implementation

## Overview

This project implements a simplified Unix shell in C that demonstrates core operating system concepts including process management, inter-process communication, and I/O redirection. The shell provides an interactive command-line interface that executes system programs and built-in commands.

## Implemented Features

### Command Execution

The shell parses user input and executes external programs by forking child processes and using the exec family of system calls. The parent shell process waits for child processes to complete before returning control to the user.

**Example:**
```
mysh> ls -l
mysh> cat file.txt
```

### I/O Redirection

The shell supports redirecting standard input and output streams using the `<` and `>` operators. This is implemented by manipulating file descriptors in child processes before executing commands.

**Input Redirection:**
```
mysh> wc < input.txt
```

**Output Redirection:**
```
mysh> echo "Hello World" > output.txt
```

### Command Pipelines

Multiple commands can be chained together using the pipe operator `|`, allowing the output of one command to serve as the input to another. The shell creates anonymous pipes and coordinates multiple child processes to implement this feature.

**Example:**
```
mysh> cat data.txt | grep "search term" | wc -l
```

### Built-in Commands

The shell includes several built-in commands that are executed directly without forking:

**cd** - Change the current working directory
```
mysh> cd /path/to/directory
```

**pwd** - Print the current working directory
```
mysh> pwd
```

**exit** - Terminate the shell
```
mysh> exit
```

### Batch Mode

The shell can execute commands from a file in batch mode, processing each line as a separate command. This mode does not display prompts and terminates after executing all commands.

**Usage:**
```
./mysh batchfile.txt
```

### Error Handling

The implementation includes comprehensive error handling for invalid commands, file access errors, system call failures, and syntax errors in command parsing.

## Technical Implementation

The shell is built using fundamental Unix system calls and follows the POSIX standard:

**Process Management:** fork(), execvp(), wait(), waitpid()

**File Operations:** open(), close(), dup2()

**Inter-Process Communication:** pipe()

**File Descriptors:** Proper management and cleanup of file descriptors to prevent resource leaks

## Architecture

The implementation follows a modular design with separate functions for parsing commands, handling redirection, creating pipelines, and executing built-ins. This separation of concerns improves code maintainability and readability.

## Compilation

```
gcc -o mysh shell.c -Wall -Werror
```

## Running the Shell

**Interactive Mode:**
```
./mysh
```

**Batch Mode:**
```
./mysh commands.txt
```
