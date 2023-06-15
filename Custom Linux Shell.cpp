#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <ctype.h>
#include <stdlib.h>
#include <iostream>
using namespace std;


bool exit_flag = false; // Flag to terminate shell
int read_backup = dup(0); // Standard Input Backup
int write_backup = dup(1); // Standard Output Backup
int count; // Pipe Number for  switching pipes


void Exec1(char *arr_arguments[]){
    pid_t pid;
    if (strcmp(arr_arguments[0], "exit") != 0){
        pid = fork();
        if (pid < 0) { // error
            cout << "Fork Failed" << endl;
        }
        else if ( pid == 0) { // child process
            execvp(arr_arguments[0], arr_arguments);
        }
        else { // parent process
          	wait(NULL);
        }
    }
    else {
        exit_flag = true;
    }
}

void Exec2(char *arr_arguments[]){ // For the only case where last operator was pipe
	dup2(write_backup, 1); // directing output to terminal

    pid_t pid;
    if (strcmp(arr_arguments[0], "exit") != 0){
        pid = fork();
        if (pid < 0) { // error
            cout << "Fork Failed" << endl;
        }
        else if ( pid == 0) { // child process
            execvp(arr_arguments[0], arr_arguments);
        }
        else { // parent process
            wait(NULL);
        }
    }
    else {
        exit_flag = true;
    }
}

void Input_Redirect(char *fileName){ // arr_input
    int in = open(fileName, O_RDONLY);
    dup2(in, 0);
    close(in);
}

void Output_Redirect(char *fileName){ // output
    int out = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, 0777);
    dup2(out, 1);
    close(out);
}

void Pipe_Redirect(char *arr_arguments[]){ // pipe
	int fd1[2]; // PipeNo1
	int fd2[2]; // PipeNo2

	if(count % 2 == 0){
		pipe(fd1);
		dup2(fd1[1], 1);
		close(fd1[1]);
	}
	else if(count % 2 != 0){
		pipe(fd2);
		dup2(fd2[1], 1);
		close(fd2[1]);
	}

    Exec1(arr_arguments);

	if(count % 2 == 0){
		dup2(fd1[0], 0);
		close(fd1[0]);
	}
	else if(count % 2 != 0){
		dup2(fd2[0], 0);
		close(fd2[0]);
	}

	count++;
}

char *tokenization(char *arr_input){
    char *arr_tokenized = new char[512];

    int j = 0;
    // Add spaces around operators
    for (int i = 0; i < strlen(arr_input); i++) {
        if (arr_input[i] != '>' && arr_input[i] != '<' && arr_input[i] != '|') {
            arr_tokenized[j++] = arr_input[i];
        } 
        else {
            arr_tokenized[j++] = ' ';
            arr_tokenized[j++] = arr_input[i];
            arr_tokenized[j++] = ' ';
        }
    }
    arr_tokenized[j++] = '\0';

    return arr_tokenized;
}


int main(){
    char *arr_arguments[256]; // Array to store command line arr_arguments
    
    while (exit_flag == false) {
        cout << "MoosaShell$ ";

        count = 0; // Pipe count for switching pipes
        char last_op = '\0'; // Checking last operater

        char arr_input[256]; // Array for taking arr_input
        cin.getline(arr_input, 256);

        char *tokens = tokenization(arr_input);

        char *command = strtok(tokens, " ");
        int i = 0;
        while (command) {
            if (*command == '<') {
				last_op = '<';
                Input_Redirect(strtok(NULL, " "));
            } 
			else if (*command == '>') {
				last_op = '>';
                Output_Redirect(strtok(NULL, " "));
			}
            else if (*command == '|') {
				last_op = '|';
                arr_arguments[i] = NULL;
                Pipe_Redirect(arr_arguments);
                i = 0;
            } 
			else {
                arr_arguments[i] = command;
                i++;
            }
            command = strtok(NULL, " ");
        }
        arr_arguments[i] = NULL;

		if(last_op == '|'){
        	Exec2(arr_arguments);
		}
		else if(last_op == '>'){
        	Exec1(arr_arguments);
		}
		else if(last_op == '<'){
        	Exec1(arr_arguments);
		}
		else if(last_op == '\0'){
        	Exec1(arr_arguments);
		}
        
        // Restoring standard input and output
		dup2(read_backup, 0);
		dup2(write_backup, 1);
    }

    return 0;
}