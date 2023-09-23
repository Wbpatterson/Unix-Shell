#include <iostream>
#include <cstring>
#include <string>
#include <iomanip>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <algorithm>

/*
    TO DO:
    - need to finish implementing built in commands (path-> store in seperate file and use thread)
    - better format and reduce redundant cstring and cstring operations
    - make print and tokenCount lambda functions
    - make sure that instructions that involve paths are not specific to your machine
*/

// prints char*[] elements
void print(char** arr, int size){
    // cannot use for loop for risk of accessing non existant memory 
    int i = 0;
    while(arr[i] != nullptr){
        std::cout << arr[i] << " ";
        i++;
    }
    std::cout << "\n";
}

// retruns the total amount of elements in char*[]
int tokenCount(char** arr){   
    int i = 0;
    while(arr[i] != nullptr){
        i++;
    }
    return i;
}

// status flags
enum status{
    UNACTIVE=0,
    ACTIVE
    };

int main(){
    constexpr int MAX = 50;
    char path[MAX] = "~";
    while(ACTIVE){
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

        const char* delim = " "; // tokens i.e commands and arguments seperated by spaces
        char* token = strtok(input, delim);

        char* argv[MAX];
        std::size_t i = 0; // index for argv

        // gets remaining argument from input to run command
        while(token){
            if(strcmp(token, " ") == 0)
                continue;
            argv[i] = token;
            token = strtok(nullptr,delim);
            i++;
        }

        argv[i] = nullptr; // ensures char*[] ends with nullptr
    
        pid_t cmd_fork = -1; // default value so it wont step in fork branches for built ins
        
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

                    // ensures if error occurs with cd while in home ~ is used for path and not fullhome name
                    if(strcmp(getcwd(path, MAX),"/home") == 0)
                        strcpy(path, "~");
            }
        }
        else{ // if no built in command is detected seperate process will be ran to run executable
            cmd_fork = fork();
        }
        
        // runs executable commands
        if(cmd_fork == 0){
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