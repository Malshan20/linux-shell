#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_LINE 1024
#define MAX_ARGS 64

// Asynchronously reap background zombie processes when they exit
void handle_sigchld(int sig) {
    (void)sig; // Suppress unused parameter warning
    // waitpid with WNOHANG reaps all terminated children without blocking
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

// Handle Ctrl+C so it doesn't kill the main shell container
void handle_sigint(int sig) {
    (void)sig;
    write(STDOUT_FILENO, "\nosh> ", 6);
}

// Built-in command handler
int handle_builtins(char **args) {
    if (args[0] == NULL) return 1; // Empty command

    if (strcmp(args[0], "exit") == 0) {
        exit(0);
    }
    if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) {
            fprintf(stderr, "osh: expected argument to \"cd\"\n");
        } else {
            if (chdir(args[1]) != 0) {
                perror("osh");
            }
        }
        return 1;
    }
    return 0; // Not a builtin command
}

// Execute a single command context with potential I/O redirection
void execute_single_command(char **args, int background) {
    char *input_file = NULL;
    char *output_file = NULL;
    int args_count = 0;

    // Scan for redirection operators
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "<") == 0) {
            input_file = args[i + 1];
            args[i] = NULL; // Truncate args array at operator
            break;
        } else if (strcmp(args[i], ">") == 0) {
            output_file = args[i + 1];
            args[i] = NULL; 
            break;
        }
        args_count++;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        return;
    }

    if (pid == 0) {
        // --- CHILD PROCESS ---
        // Restore default behavior for Ctrl+C inside foreground children
        if (!background) {
            signal(SIGINT, SIG_DFL);
        }

        // Input Redirection
        if (input_file) {
            int fd_in = open(input_file, O_RDONLY);
            if (fd_in < 0) {
                perror("Error opening input file");
                exit(EXIT_FAILURE);
            }
            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        }

        // Output Redirection
        if (output_file) {
            int fd_out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd_out < 0) {
                perror("Error opening output file");
                exit(EXIT_FAILURE);
            }
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }

        // Transform into target binary image
        if (execvp(args[0], args) < 0) {
            perror("Execution error");
            exit(EXIT_FAILURE);
        }
    } else {
        // --- PARENT PROCESS ---
        if (!background) {
            // Block until foreground child finishes execution
            waitpid(pid, NULL, 0);
        } else {
            printf("[Process spawned with PID: %d]\n", pid);
        }
    }
}

// Execute two commands joined by a pipeline pipe operator: cmd1 | cmd2
void execute_pipeline(char **args_left, char **args_right, int background) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("Pipe generation failed");
        return;
    }

    pid_t pid1 = fork();
    if (pid1 == 0) {
        // --- CHILD 1 (Writer) ---
        if (!background) signal(SIGINT, SIG_DFL);
        
        // Tie stdout to write end of the pipe channel
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]); // Close unused read end
        close(pipefd[1]);

        if (execvp(args_left[0], args_left) < 0) {
            perror("Pipeline command 1 failed");
            exit(EXIT_FAILURE);
        }
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        // --- CHILD 2 (Reader) ---
        if (!background) signal(SIGINT, SIG_DFL);

        // Tie stdin to read end of the pipe channel
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[1]); // Close unused write end
        close(pipefd[0]);

        if (execvp(args_right[0], args_right) < 0) {
            perror("Pipeline command 2 failed");
            exit(EXIT_FAILURE);
        }
    }

    // Parent closes its references to the pipe channels
    close(pipefd[0]);
    close(pipefd[1]);

    if (!background) {
        waitpid(pid1, NULL, 0);
        waitpid(pid2, NULL, 0);
    } else {
        printf("[Pipeline spawned: PID %d and PID %d]\n", pid1, pid2);
    }
}

int main(void) {
    char input[MAX_LINE];
    char *args[MAX_ARGS];
    
    // Wire system signals
    struct sigaction sa;
    sa.sa_handler = handle_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction registration failed");
        exit(1);
    }

    signal(SIGINT, handle_sigint);

    while (1) {
        printf("osh> ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\n");
            break; // Handle EOF (Ctrl+D) gracefully
        }

        // Clean newline character
        input[strcspn(input, "\n")] = '\0';

        // Tokenize raw string into arguments array
        int i = 0;
        char *token = strtok(input, " ");
        while (token != NULL && i < MAX_ARGS - 1) {
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL;

        if (args[0] == NULL) continue; // Skip blank entry line

        // Check if command should run in background asynchronously
        int background = 0;
        if (i > 0 && strcmp(args[i - 1], "&") == 0) {
            background = 1;
            args[i - 1] = NULL; // Strip operator token
        }

        // Catch builtins like exit/cd
        if (handle_builtins(args)) {
            continue;
        }

        // Check for Pipeline pattern validation
        int pipe_index = -1;
        for (int j = 0; args[j] != NULL; j++) {
            if (strcmp(args[j], "|") == 0) {
                pipe_index = j;
                break;
            }
        }

        if (pipe_index != -1) {
            args[pipe_index] = NULL; // Cut the array in half
            char **args_left = args;
            char **args_right = &args[pipe_index + 1];
            execute_pipeline(args_left, args_right, background);
        } else {
            execute_single_command(args, background);
        }
    }
    return 0;
}