#include <iostream>
#include <cstring>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <algorithm>
#include <stdlib.h>
#include <regex>
#include <sys/stat.h>
#include <fcntl.h>

/*
    Student Name: Willie Patterson
    Date: 10/01/2023
    Class/Section: COMP 350 
*/

/*
    At this checkpoint (2), the shell can now take and execute built-in commands, 
    traditonal unix commands, aswell as the change the displayed current directory

    NOTE: 
    a copy of the users enviroment varibles has been made for the purposes of 
    this project, so any changes made with the path command will NOT be permanent 
    after the program has terminated
*/


// returns the size of an array of char* (char*[size])
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

int findCmd(char* cmd, char** arr){
    //check if < or > exist in args
    int i = 0;
    while(arr[i] != nullptr){
        if (strcmp(arr[i], cmd) == 0)
            return i; 
        i++;
    }

    return -1;
}

// finds envrioment varible in environ
int  environFind(char* var){
    int var_size = size(environ);
    for(int i = 0; i < var_size; i++){
        if(strncmp(environ[i], var, strlen(var)) == 0)
            return i;
    }
    return -1;
}

// check if path enviroment variable contains a specified item
int pathContains(char* regex){
    int index = environFind((char*)"PATH=");
    std::regex pattern(regex);
    std::cmatch matches;
    std::regex_search(environ[index], matches, pattern);
    return matches.empty();
}

char exec_path[100];

int access(const char* command, char* path){
    char temp[strlen(path) + 1];
    strcpy(temp, path);

    char* token = strtok(temp, "=:");
    char input[100];
    struct stat buf;
    while(token != nullptr){
        strcpy(input, token);
        strcat(input, command);
        //std::cout << input << "\n";
        if(stat(input, &buf) == 0){
            strcpy(exec_path, input);
            return 1;
        }
        //sleep(1);
        token = strtok(nullptr, "=:");
    }

    return 0;
}

int redirection = 0;
char redirectFile[100];
int file_desc = 0;


int main(int argc, char* argv[]){
    constexpr int MAX = 1024;
    char path[MAX];
    getcwd(path, MAX); // starts program by showing location of open current dir

     // creates a copy of user enviroment variables array
    int n = size(environ);
    char* enviroment[n+1];
    environCopy(enviroment);
    environ = enviroment;  // ensures that enviroment variables on graders computer are not reset 

    while(1){
        char input[MAX];
        std::cout << "ash " << path << "> ";
        std::cin.getline(input, MAX); 

        // built in exit command
        if(strcmp(input, "exit") == 0){
            std::cout << "Willie Patterson: Aggie Shell" << "\n";
            std::exit(EXIT_SUCCESS);
        }

        // if no input is entered go to next interation
        if(strlen(input) == 0){
            continue;
        }

        char* token = strtok(input, " "); // tokens i.e commands and arguments seperated by spaces
        char* args[MAX]{};
        std::size_t i = 0; // index for argv

        // retrieves remaining arguments from input to run command
        while(token){
            if(strcmp(token, " ") == 0)
                continue;
            args[i] = token;
            token = strtok(nullptr, " ");
            i++;
        }

        //args[i] = nullptr; // ensures char*[] ends with nullptr
        int redirect = 0;
    
        pid_t cmd_fork = -1; // default value so fork will not step in fork branches for built ins
        
        // built in commands will specified 
        if(strcmp(args[0], "cd") == 0){
            int length = i;
            
            if(length == 1){
                strcpy(path, "");

                if(chdir("/home") == -1) // sets directory back to home 
                    perror("An error has occurred"); 
            }
            else{

                if(chdir(args[1]) == -1) // changes to desired directory
                    perror("An error has occurred"); 

                // changes shell promt to match current directory
                if(strcmp(getcwd(path, MAX),"/home") == 0)
                    strcpy(path, "");
            }
        }
        else if(strcmp(args[0], "path") == 0){
            // required dir to run linux commands:
            // - /bin
            // - /usr/bin

            int i = 1;
            char path[250];
            strcpy(path, "PATH=\"");
            while(args[i] != nullptr){
                strcat(path, args[i]);
                strcat(path, ":");
                i++;
            }
            strcat(path, "\"");
            int path_index = environFind((char*)"PATH=");
            strcpy(environ[path_index], path);
            // environ[path_index] = path;
            continue;
        }
        else{ 
            // if no built in command is detected seperate process will be ran to run executable

            // checks if redirection is detected and acts accordingly 
            if(redirect = findCmd((char*)">", args) > -1){
                redirection = 1;
                int m = size(args);
                if(!args[redirect + 1] || m > redirect+2){ // [ls, -la,  >, output.txt, nullptr]
                    std::cout << "An error has occurred: No such file or directory\n";
                    continue;
                }
                else{
                    strcpy(redirectFile, args[redirect + 1]);
                    args[redirect] = nullptr;
                    file_desc = open(redirectFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                }
            }
            
            cmd_fork = fork();
        }
        
        // runs executable commands
        if(cmd_fork == 0){

            if(redirection == 1){
               dup2(file_desc, STDOUT_FILENO); // standard output redirected to file
               dup2(file_desc, STDERR_FILENO); // standard errors redirected to file
            }

            // char input[100];
            char* input = new char[100];
            strcpy(input, "/");
            strcat(input, args[0]);
            int i = environFind((char*)"PATH="); // finds our PATH Field
            access(input, environ[i]); // finds and checks if variable in our PATH leads to a command
            delete [] input;
            execv(exec_path, args);
            perror("An error has occurred"); // if commnand not found error process returns
            exit(EXIT_FAILURE);
        }

        if(cmd_fork > 0){
            wait(nullptr); // lets child process finish before retrurning to parent
        }
    }

    return 0;
}