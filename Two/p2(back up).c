/*
CS570 Operating Systems
Professor John Carroll
Huy Nguyen
Description: 
    The program include main functions are main() and parse(). 
    Parse() function read the user input and return the total number of words from command line.
    The main() interpret the user input regarding to p2 convention. It will slightly be similar
    to UNIX shell.
*/


#include <stdio.h>
#include "p2.h"
#define MAXITEM 100 /* max number of words per line */


int BGFLAG = 0, directIN = 0, directOUT = 0, PIPEFLAG = 0;
char argvStorage[255]; //this array get a string from getword function
char *newargv[MAXITEM]; //this pointer array will keep the first character of argvStorage array
char *fileIn, *fileOut; //pointers point to file in argvStorage
int sizeC = 0; //check if it is EOF or not
char *pIndex[1000];//array of pipe index
char buf[1000]; //get data from pipe
int inDesc,outDesc;//check the status of the descriptor if in or out when detect the redirection.

int parse();

int main()
{
	signal(SIGTERM,myhandler);
	int argc;
	pid_t pid, childPid;
	int status; //return current procces status
	DIR *dirp;
	struct dirent *dp;
	struct stat sb;

	for (;;)
	{
		printf("p2: ");
		// fflush(stdout);
		//call parse function
		argc = parse();

		if (sizeC == -1)
			break;
		if (argc == 0) //reissue prompt if line is empty
			continue;
		if (newargv[0] == NULL){
			fprintf(stderr, "Invalid command");
			continue;
		}

		/*This block code handle if first argument is "echo"
			If the number argument is more than 2. The error appear apporiately.*/
		if(strcmp(newargv[0],"cd") == 0){
			//Error message will print out
			//because too many argument
			if(argc > 2){
				fprintf(stderr, "ERROR! No more than one argument\n");
			}
			//check HOME directory
			//if it is valid, change to HOME directory
			else if(argc == 1){//with no parameter
				//print error if HOME directory not found
				if(chdir(getenv("HOME")) != 0)
					fprintf(stderr, "ERROR! Couldn't find HOME directory\n");
			//print error if directory is unvalid
			}else{
				if(chdir(newargv[1]) != 0)
					fprintf(stderr, "ERROR! %s: No such file or directory\n", newargv[1]);
			}
			continue;
		}

		/*This block code handle if first argument is "ls-F"*/
		if(strcmp(newargv[0],"ls-F") == 0){
			//only handle 0 or 1 argument
			if(argc > 2){
				fprintf(stderr, "ERROR! Too many argument\n");
			}
			/* Treat "ls-F" as "ls ." if there are no argument pass */
			if(argc == 1){//no argument after "ls-F"
				dirp = opendir(".");
				//check if there dir open successful or not
				//error message will appear if it fail
				if(stat(".", &sb) == -1){
					perror("The ls-F has failed");
					exit(EXIT_FAILURE);
				}
				while(dirp){//loop through the directory
					//Check if the pointer is not NULL
					//return pointer with four fields entry
					if((dp = readdir(dirp)) != NULL){
						//Use AND wise bit mask to check a directory file
						//print out the directory name and information
						if((sb.st_mode & S_IFMT) == S_IFDIR)
							printf("%s\n",dp->d_name);
					}
					//close the entry when return NULL
					else{
						closedir(dirp);
						break;
					}
				}
			}
			/*When ls-F has one argument
				If there is more than two
				Read the first argument after ls-F and ignore the left*/
			else{
				//check if the argument is a field in the stat structure
				if(stat(newargv[1],&sb) == -1){//check -l
					fprintf(stderr, "%s: No such file or directory\n",newargv[1]);
					continue;
				}
				
				//if it is not a directory
				//just treat as a ls command does
				else if((sb.st_mode & S_IFMT) != S_IFDIR){
					printf("%s \n",newargv[1]);
					continue;
				}

				//check if can access to a directory
				else if(chdir(newargv[1]) == -1){
					fprintf(stderr, "Can not access %s. Permission denied \n",newargv[1]);
					continue;
				}
				//print out the name of the file in current directory
				while(dirp){
					if((dp = readdir(dirp)) != NULL){
					//Use AND wise bit mask to check a directory file
					//print out the directory name and information
						if((sb.st_mode & S_IFMT) == S_IFDIR)
							printf("%s\n",dp->d_name);
					}
					//close the entry when return NULL
					else{
						closedir(dirp);
						break;
					}
				}
			}


			continue;//break ls-F
		}

/**************INPUT REDIRECTION****************/
		/*	Return error if there are more than 2 input character.
			Otherwise, open the file,if is not succesful. Print out an error.
			Then dup a new descriptor to STD_IN*/
		if(directIN != 0){
			if (directIN > 2){
				fprintf(stderr, "The command line is ambiguous\n");
				continue;
			}
			
			if(directIN == 1){//missing the input argument
				fprintf(stderr, "The input redirection is missing\n");
				continue;
			}
			//create a new open file descriptor
			//return -1 and appear error if the file open is unsucessful
			inDesc = open(fileIn,O_RDONLY);
			if(inDesc == -1){
				fprintf(stderr, "ERROR: Unsucessful open on %s \n",fileIn);
				continue;
			}else{//set a descriptor to STD_IN
				//close(inDesc);
				dup2(inDesc,0);
				close(inDesc);
			} 
		}
/**************OUTPUT REDIRECTION****************/
		/* */
		if(directOUT != 0){
			if (directOUT > 2){
				fprintf(stderr, "The command line is ambiguous\n");
				continue;
			}

			if(directOUT == 1){
				fprintf(stderr, "The output redirection is missing\n");
			}
			/*	Check an output file name from commnand line. If it is not created,
				make a new file. Otherwise, appear an error if the file existed already*/
			outDesc = open(fileOut, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IXUSR);
			if(outDesc < 0){
				if(errno == EEXIST)
					fprintf(stderr, "The file  \" %s \"  already existed \n",fileOut);
				directOUT = 0;
				continue;
			}else{//set a descriptor to STD_OUT
				dup2(outDesc,1);
				close(outDesc);
			}

		}


/**************PIPE FLAG****************/
		if(PIPEFLAG > 0){

			int fd_in;			
			if(PIPEFLAG > 1){
				fprintf(stderr, "Too many argument\n");
				continue;
			}

			int fd[2];
			pid_t child1_pid, child2_pid;
			//create a pipe
			if(pipe(fd) == -1){
				fprintf(stderr, "Pipe failed\n");
				return 1;
			}

			//create the second child first
			child2_pid = fork();
			if(child2_pid < 0){
				fprintf(stderr, "Child 2 fork failed\n");
			}
			//parent process
			else if (child2_pid > 0){
				//create the first child
				if((child1_pid = fork()) == -1){
					fprintf(stderr, "Child 1 fork failed\n");
				}
				//execute and put data into the pipe line
				else if (child1_pid == 0){
					dup2(fd[1],STDOUT_FILENO);
					close(fd[0]); //close the read end
					close(fd[1]);
					execvp(*newargv,newargv);
				}
			}
			//child 2 process
			else if (child2_pid == 0){
				dup2(fd[0],STDIN_FILENO);
				close(fd[0]);
				close(fd[1]); //close the write end
				execvp(*pIndex,pIndex);
			}

			/* Parent process */
			close(fd[0]);
			close(fd[1]);

			/* Break if the second child is found*/
			for(;;){
				pid_t pid;
				pid = wait(NULL);
				if(pid == child2_pid){
					break;
				}
			}

			/*Inform if the parent sucecced*/
			printf("Parent is Done\n");
			exit(EXIT_SUCCESS);

		}//end PIPE FLAG

/**************FORK********************/
		// fflush(stdout);
		// fflush(stderr);
		childPid = fork(); //create a child process
		//Return a negative number if fork is unsuccessful.
		if (childPid < 0){
			printf("ERROR! can't fork\n");
			exit(1);
		}
		else if(childPid == 0){ //return a new child process
						/*******Redirect I/O as requested**********/
			//set a descriptor to STD_IN
			if(directIN != 0){
				//close(inDesc);
				dup2(inDesc,0);
				close(inDesc);
			} 
			//set a descriptor to STD_OUT
			if(directOUT != 0){
				dup2(outDesc,1);
				close(outDesc);
			}
			execvp(*newargv,newargv);
			printf("ERROR exec failed\n");
			exit(9);
		}
		else {
			// pid = wait(NULL);
			// if (pid == childPid)
			// 	continue;
			/* Wait for the child completion.
				Return the exit status of child*/
         	wait(&status);
           	if(status > 0) 
           		fprintf(stderr, "Exit %d\n", WEXITSTATUS(status));
		}


	}//end for
		killpg(getpgrp(),SIGTERM);
		printf("p2 terminated. \n");
		exit(0);

}// main

int parse()
{
	int p = 0; 

	//this pointer will keep track the argvStorage array for each of loops
	//the getword function will not overwrite the argvStorage array
	int ptrArgv = 0; 
	int count = 0;
	int wCount = 0; //return the number of argc
	/* Set initial for the direction file*/
	fileIn = '\0';
	fileOut = '\0';
	directIN = directOUT = 0;


	/* Read from stdin to argvStorage array. The ptrArgv will keep track the argvStorage array
		If it is meta character, set a flag appropriately. Otherwise,
		set the address of first char of argvStorage to the newargv*/
	while ((sizeC = getword(argvStorage + ptrArgv)) > 0)
	{
		/*	Redirect input detect if there is '<' character
			only count 1 redirect input at the time.
			If the size of the word after '<' character not equal 0
			directIN flag again, and a array pointer is assigned to the input*/
		if( (sizeC == 1 && *(argvStorage + ptrArgv) == '<') || (sizeC > 0 && directIN == 1) ){
			directIN++;
			// sizeC = getword(argvStorage+ptrArgv);
			// if (sizeC > 0)
			// 	fileIn = argvStorage + ptrArgv;
			//check if next word is not NULL
			//Point to the first character of input direct
			if(directIN == 2)//after the first '< ' is detected
				fileIn = (argvStorage + ptrArgv);
		}
		/*	Redirect output detect if there is '>' character
			only count 1 redirect output at the time.
			If there is double output ('>>') the array pointer will be assigned after 2nd '>' char is read.
			If the size of the word after '>' character not equal 0
			directIN flag again, and a output pointer is assigned to the output*/
		else if( (sizeC == 1 && *(argvStorage + ptrArgv) == '>') || (sizeC > 0 && directOUT == 1) ){
			directOUT++;
			if(directOUT == 2)//after the first '>' is dectected
				fileOut = (argvStorage + ptrArgv);
		}
		else{
			//Put the address of first char of each argvStorage to the pointer array
			newargv[p] = argvStorage + ptrArgv;
			p++;
		}

		argvStorage[ptrArgv + sizeC] = '\0';
		//point to the next address of next word in argvStorage, the getword will not overwrite the argvStorage array
		ptrArgv = ptrArgv + sizeC + 1;
		wCount++;


	}//end while

	newargv[p] = NULL;
	pIndex[PIPEFLAG] = NULL;
	return wCount;

}//end parse

void myhandler(){

}//end myhandler



		