#include <iostream>
#include <cstring>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <algorithm>
#include <stdlib.h>

/*
    TO DO:
    - better format and reduce redundant cstring and cstring operations
    - make print and tokenCount lambda functions
    - make sure that instructions that involve paths are not specific to your machine
    - add a strcut to store state of enviroment variables
*/

// prints char*[] elements
void print(char **arr) {
    // cannot use for loop for risk of accessing non existant memory 
    int i = 0;
    while(arr[i] != nullptr){
        std::cout << arr[i] << " ";
        i++;
    }
    std::cout << "\n";
}

// returns the size of an array of char* 
int size(char** arr){
    int i = 0;
    while(arr[i] != nullptr){
        i++;
    }

    return i;
}

// used to create a copy of the environment variable array 
char** environCopy(char** arr){
    int arr_size = size(environ);
    for(int i = 0; i < arr_size; i++){
        arr[i] = environ[i];
    }
    arr[arr_size  + 1] = nullptr;

    return arr;
}

// finds envrioment varible in environ
int  environFind(char* var){
    int var_size = size(environ);
    for(int i = 0; i < var_size; i++){

        if(strncmp(environ[i], var, strlen(var)) == 0)
            return i;
    }

    return 1;
}

int main(int argc, char* argv[]){
    constexpr int MAX = 1024;
    char path[MAX];
    getcwd(path, MAX); // starts program by showing location of open cur dir

     // creates a copy of our enviroment variables array
    int n = size(environ);
    char* enviroment[n+1];
    environCopy(enviroment);
    //environ = enviroment; // ensures that enviroment variables on graders computer are not reset 

    while(1){
        char input[MAX];
        std::cout << "[ash " << path << "]$ ";
        std::cin.getline(input, MAX); 

        // built in exit command
        if(strcmp(input, "exit") == 0){
            //std::cout << "Willie Patterson: Aggie Shell" << "\n";
            std::exit(EXIT_SUCCESS);
        }

        // if no input is entered go to next interation
        if(strlen(input) == 0){
            continue;
        }

        char* token = strtok(input, " "); // tokens i.e commands and arguments seperated by spaces
        // environ = enviroment;
        char* argv[MAX];
        std::size_t i = 0; // index for argv

        // retrieves remaining arguments from input to run command
        while(token){
            if(strcmp(token, " ") == 0)
                continue;
            argv[i] = token;
            token = strtok(nullptr, " ");
            i++;
        }

        argv[i] = nullptr; // ensures char*[] ends with nullptr
    
        pid_t cmd_fork = -1; // default value so fork will not step in fork branches for built ins
        
        // built in commands will specified 
        if(strcmp(argv[0], "cd") == 0){
            int length = i;
            
            if(length == 1){
                strcpy(path, "~");

                if(chdir("/home") == -1) // sets directory back to home 
                    perror("no dir"); 
            }
            else{

                if(chdir(argv[1]) == -1) // changes to desired directory
                    perror("no dir"); 

                // ensures if error occurs with cd while in home symbol ~ is used for path and not full home path name
                if(strcmp(getcwd(path, MAX),"/home") == 0)
                    strcpy(path, "~");
            }
        }
        else if(strcmp(argv[0], "path") == 0){
            // required dir to run linux commands:
            // - /bin
            // - /usr/bin
            // - /usr/local/bin

            int i = 1;
            char path[250];
            strcpy(path, "PATH=\"");
            while(argv[i] != nullptr){
                strcat(path, argv[i]);
                strcat(path, ":");
                i++;
            }
            strcat(path, "\"");
            int path_index = environFind((char*)"PATH=");
            environ[path_index] = path;
            continue;
        }
        else{ // if no built in command is detected seperate process will be ran to run executable
            cmd_fork = fork();
        }
        
        // runs executable commands
        if(cmd_fork == 0){
            //execvpe(argv[0], argv, enviroment); // creates and execution with defined evirmoment variables
            execvp(argv[0], argv);
            perror("error"); // if commnand not found error process returns
            exit(EXIT_FAILURE);
        }

        if(cmd_fork > 0){
            wait(nullptr); // lets child process finish before retrurning to parent
        }
    }

    return 0;
}