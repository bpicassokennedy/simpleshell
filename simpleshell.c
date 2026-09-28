/*
// name: bella picasso-kennedy
// purpose: simple linux shell
// date: september 17, 2026
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

int parseInput(char * input, char splitWords[][500], int maxWords);
int executeCommand(char * const* command, const char* infile, const char* outfile, char * const* command_2);
void changeDirectories(const char * path);

int main(void){
    char input[1024];
    char cwd[1024];
    char words[200][500];
    const char * netid = "bpicassokennedy";
    int count;

    while(1){ 
        getcwd(cwd, sizeof(cwd)); // what directory is this process in right now
        printf("%s:%s$ ", netid, cwd); // builds prompt format 

        if(fgets(input, sizeof(input), stdin) == NULL){
            break;
        }

        count = parseInput(input, words, 200); // returns num of words actually found

        if(count == 0){
            continue; // no real input, will go back to top of loop
        }

        if(strcmp(words[0], "exit") == 0){ //  
            return 0;
        } else if(strcmp(words[0], "cd") == 0){
            if(count != 2){ // did the user type cd and one path? 
                printf("Path Not Formatted Correctly!\n");
            }
            else{
                changeDirectories(words[1]); // pass second word of user input
            }
        } else{
            char * infile = NULL;
            char * outfile = NULL;
            int pipeIndex = -1;
            int redirIndex = -1;
            char redirChar = 0;

            for(int i = 0; i < count; i++){
                if(strcmp(words[i], "|") == 0){ // pipe was found
                    pipeIndex = i;
                    break;
                } else if(strcmp(words[i], "<") == 0 || strcmp(words[i], ">") == 0){ // redirection found
                    redirIndex = i;
                    redirChar = words[i][0];
                    break;
                }
            }

            if(pipeIndex != -1){ // pipe was found 
                int w1 = pipeIndex; // however many words came before |
                char **command1 = malloc((w1 + 1) * sizeof(char *));
                for(int i = 0; i < w1; i++){
                    command1[i] = words[i];
                }
                command1[w1] = NULL;

                int w2 = count - pipeIndex - 1; // however many words came after |
                char **command2 = malloc((w2 + 1) * sizeof(char *));
                for(int i = 0; i < w2; i++){
                    command2[i] = words[pipeIndex + 1 + i];
                }
                command2[w2] = NULL;

                executeCommand(command1, NULL, NULL, command2);

                free(command1);
                free(command2);
            } else if(redirIndex != -1){
                if(redirIndex + 1 < count){ // is there a word after the redirect symbol?
                    if(redirChar == '<'){
                        infile = words[redirIndex + 1];
                    } else{
                        outfile = words[redirIndex + 1];
                    }
                }

                int w = redirIndex; // everything before the redirect symbol is now the command array
                char **command = malloc((w + 1) * sizeof(char *));
                for(int i = 0; i < w; i++){
                    command[i] = words[i];
                }
                command[w] = NULL;

                executeCommand(command, infile, outfile, NULL);

                free(command);
            } else{ // nothing was found no redirection or pipe
                char **command = malloc((count + 1) * sizeof(char *));
                for(int i = 0; i < count; i++){
                    command[i] = words[i];
                }
                command[count] = NULL;

                executeCommand(command, NULL, NULL, NULL);

                free(command);
            }
        }
    }
}

// take cli entered by the user as the input, and split it 
int parseInput(char * input, char splitWords[][500], int maxWords){
    int count = 0; 
    char *token = strtok(input, " \t\n"); // input is modified, and a space is the delimiter

    while(token != NULL && count < maxWords){
        strcpy(splitWords[count], token); // destination, source 
        count++;
        token = strtok(NULL, " \t\n"); // don't start over, continue where you left off
    }

    return count; // count of how many words actually copied into splitWords
}

int executeCommand(char * const* command, const char* infile, const char* outfile, char * const* command_2){
    int pipe_fds[2]; // for read-in [0] and write-out [1]
    int hasPipe = (command_2 != NULL); // does it actually need to deal w/ a pipe at all? 

    if(hasPipe){
        if(pipe(pipe_fds) == -1){ // failure case 
            printf("fork Failed: %s\n", strerror(errno));
            return 1;
        }
    }

    fflush(stdout); 

    // fork first or only child
    pid_t pid1 = fork();
    if(pid1 == -1){
        printf("fork Failed: %s\n", strerror(errno));
        return 1;
    }
    
    if(pid1 == 0){
        // child process one 
        if(infile != NULL && infile[0] != '\0'){
            int fd_in = open(infile, O_RDONLY, 0666);
            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        }
        if(outfile != NULL && outfile[0] != '\0'){
            int fd_out = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }
        if(hasPipe){
            dup2(pipe_fds[1], STDOUT_FILENO);
            close(pipe_fds[0]);
            close(pipe_fds[1]);
        }

        execvp(command[0], command); // child process becomes new program if this runs
        printf("exec Failed: %s\n", strerror(errno));
        fflush(stdout);
        _exit(1); // terminates child process 
    }

    // parent process 
    pid_t pid2 = -1;
    if(hasPipe){
        fflush(stdout);
        pid2 = fork();
        if(pid2 == -1){
            printf("fork Failed: %s\n", strerror(errno));
            return 1;
        }

        if(pid2 == 0){
            // child process 2
            dup2(pipe_fds[0], STDIN_FILENO); // receive whatever the first child wrote into the pipe
            close(pipe_fds[0]);
            close(pipe_fds[1]);

            execvp(command_2[0], command_2); // replace child w/ second command 
            printf("exec Failed: %s\n", strerror(errno));
            fflush(stdout);
            _exit(1);
        }

        close(pipe_fds[0]);
        close(pipe_fds[1]);
    }

    int status1, status2; // exit status information 
    waitpid(pid1, &status1, 0);
    if(WEXITSTATUS(status1) != 0){ // extracts error code as long as code is not 0 
        printf("Child finished with error status: %d\n", WEXITSTATUS(status1));
    }

    if(hasPipe){
        waitpid(pid2, &status2, 0);
        if(WEXITSTATUS(status2) != 0){
            printf("Child finished with error status: %d\n", WEXITSTATUS(status2));
        }
    }

    return 0; 
}

void changeDirectories(const char * path){
    int result = chdir(path); // change calling process's current working directory to wherever path points to 

    if(result == -1){ // returns -1 on failure
        printf("chdir Failed: %s\n", strerror(errno)); // error code translated to readable string
    }
}