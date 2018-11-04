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


#include "p2.h"



int amperSand = 0, directIN = 0, directOUT = 0, pipeCounter = 0, oWrite = 0;
char argvStorage[MAXITEM*STORAGE]; //this array get a string from getword function
char *newargv[MAXITEM]; //this pointer array will keep the first character of argvStorage array
char *fileIn, *fileOut; //pointers point to file in argvStorage
int sizeC = 0; //check if it is EOF or not
char *pipeArgv[1000];//array of pipe index
int inDesc,outDesc;//check the status of the descriptor if in or out when detect the redirection.
int rpipe = 0;//index of pipeArgv
int pipeArr[10];//array of index of the pipes


int parse();
void pipeExecution();
void multiPipe();
void testPipe();
void pipeHandling();

int main()
{
	int argc;
	pid_t pid, childPid;
	int status; //return current procces status
	DIR *dirp;
	struct dirent *dp;
	struct stat sb;

	/*Handle signal for termination*/
	pid = getpid();
	if (setpgid(pid,0) != 0){
		perror("pgid fail");
		exit(1);
	}
	signal(SIGTERM,myhandler); //send signal out when process killed


	for (;;)
	{
		printf("p2: ");
		// fflush(stdout);
		//call parse function
		argc = parse();

		//EOF detect
		if (sizeC == -1)
			break;
		if (argc == 0) //reissue prompt if line is empty
			continue;

/*******This code block handle if no argument when redirection character
			is detected. It mimic tcsh.
			ez: < somefile 										*/
		if (newargv[0] == NULL){
			fprintf(stderr, "The command is invalid null\n");
			continue;
		}
/************ This block code handle if first argument is "echo"
			If the number argument is more than 2. The error appear apporiately. **********/
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


/****************This block code handle if first argument is "ls-F"****************************/
		if(strcmp(newargv[0],"ls-F") == 0){
			//only handle 0 or 1 argument
			// if(argc > 2){
			// 	fprintf(stderr, "ERROR! Too many argument\n");
			// }
			/* Treat "ls-F" as "ls ." if there are no argument pass */
			if(argc == 1){//no argument after "ls-F"
				dirp = opendir(".");
				//check if there dir open successful or not
				//error message will appear if it fail
				if(stat(".", &sb) == -1){
					fprintf(stderr, "The ls-F has failed\n");
					exit(EXIT_FAILURE);
				}
				while(dirp){//loop through the directory
					//Check if the pointer is not NULL
					//return pointer with four fields entry
					if((dp = readdir(dirp)) != NULL){
						
						if(lstat(dp->d_name, &sb) == -1)
							fprintf(stderr,"ls-F struct stat failed\n");
						/*Use AND wise bit mask to check a 
							file type code from a mode value*/
						//check if it is soft link or 'broken'
						if ((sb.st_mode & S_IFMT) == S_IFLNK){
							//check if the link is broken or not
							if (stat(dp->d_name, &sb) == -1)
								//append '&' if is 'broken' link
								printf("%s&\n",dp->d_name);
							else
								//otherwise, the syslink is exist
								printf("%s@\n",dp->d_name);
						}
						//print out the directory name and information
						//append '/' to directory
						else if((sb.st_mode & S_IFMT) == S_IFDIR){
							printf("%s/\n",dp->d_name);
						}


						//check if it is an executable file
						//IS_REG call stat() system call to check if it regular
						//0111 indicate permssion following d-x-x-x 
						else if (S_ISREG(sb.st_mode) && sb.st_mode & 0111)
							//append '*' if it is executable file
							printf("%s*\n",dp->d_name);
						else
							printf("%s\n",dp->d_name);

					}
					//close the entry when return NULL
					else{
						closedir(dirp);
						break;
					}
				}
			}
			/*	When ls-F has one argument
				If there is more than two
				Read the first argument after ls-F and ignore the left over*/
	else{

		int i =1;
		while(i < argc){
			//check if the argument is a field in the stat structure
			if(lstat(newargv[i],&sb) == -1){//check -l
				fprintf(stderr, "%s: No such file or directory\n",newargv[i]);
				i++;
				continue;
			}
			
			//if it is not a directory
			//just treat as a ls command does
			else if((sb.st_mode & S_IFMT) != S_IFDIR){
				printf("%s\n",newargv[i]);
				i++;
				continue;
			}

			//check if can access to a directory
			else if(chdir(newargv[i]) == -1){
				fprintf(stderr, "The %s unreadable directory\n",newargv[i]);
				i++;
				continue;
			}


			//open the directory before print the files or another folder in its
			//return NULL if open failure
			if( (dirp = opendir(newargv[i])) == NULL){
				printf("%s:failed to open directory\n",newargv[i]);
				i++;
				continue;
			}

			//print out the name of the file in current directory
			while(dirp){
				if((dp = readdir(dirp)) != NULL){						
					if(lstat(dp->d_name, &sb) == -1)
						fprintf(stderr,"ls-F struct stat failed\n");
					/*Use AND wise bit mask to check a 
						file type code from a mode value*/
					//check if it is soft link or 'broken'
					if ((sb.st_mode & S_IFMT) == S_IFLNK){
						//check if the link is broken or not
						if (stat(dp->d_name, &sb) == -1)
							//append '&' if is 'broken' link
							printf("%s&\n",dp->d_name);
						else
							//otherwise, the syslink is exist
							printf("%s@\n",dp->d_name);
					}
					//print out the directory name and information
					//append '/' to directory
					else if((sb.st_mode & S_IFMT) == S_IFDIR)
						printf("%s/\n",dp->d_name);


					//check if it is an executable file
					//IS_REG call stat() system call to check if it regular
					//0111 indicate permssion following d-x-x-x 
					else if (S_ISREG(sb.st_mode) && sb.st_mode & 0111)
						//append '*' if it is executable file
						printf("%s*\n",dp->d_name);
					else
						printf("%s\n",dp->d_name);

				}
				//close the entry when return NULL
				else{
					closedir(dirp);
					break;
				}
			}

			i++; //increase count till hit last argc
			//printf("Increase count in ls-F\n");
		}

		
	}
	continue;//break ls-F
}




	// int j = 0;
	// for(j;j<10;j++){
	// 	printf("The value of pipe array: %d\n",pipeArr[j]);
	// }


/**************SET UP FOR ANY REDIRECTION********************/
		/*INPUT REDIRECTION*/
		/*	Return error if there are more than 2 input character.
			Otherwise, open the file,if is not succesful. Print out an error.
			Then dup a new descriptor to STD_IN*/
		if(directIN != 0 && pipeCounter == 0){
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
			} 
		}
		/*OUTPUT REDIRECTION*/
		/* Like INPUT REDIRECTION return an error if there are more than 2 output chars
		Check if the file is existed or not. If not create a new file; otherwise,
		print out an error*/
		if(directOUT != 0 && pipeCounter == 0){

			if (directOUT > 2){
				fprintf(stderr, "The command line is ambiguous\n");
				continue;
			}

			if(directOUT == 1){
				fprintf(stderr, "The output redirection is missing\n");
				continue;
			}
			/*	Check an output file nam e from commnand line. If it is not created,
				make a new file. Otherwise, appear an error if the file existed already*/
			outDesc = open(fileOut, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);//only read and write permission
			if(outDesc < 0){
				if(errno == EEXIST){
					fprintf(stderr, "The file  \" %s \"  already existed \n",fileOut);
					directOUT = 0;
					continue;
				}
			}

		}

		if(oWrite != 0 && pipeCounter == 0){
			if (oWrite > 2){
				fprintf(stderr, "The command line is ambiguous\n");
				continue;
			}

			if(oWrite == 1){
				fprintf(stderr, "The overWrite redirection is missing\n");
				continue;
			}
			/*	Check an output file name from commnand line. If it is not created,
				make a new file. Otherwise, over write if the file existed already*/
			outDesc = open(fileOut, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);//only read and write permission
			if(outDesc < 0){
				fprintf(stderr, "ERROR: Unsucessful overwrite on %s \n",fileOut);
				continue;
			}
		}

		//fflush(stdout);
/**************PIPE FLAG****************/
		if(pipeCounter != 0){
			if(pipeCounter > MAXPIPES){
				fprintf(stderr, "Too many pipes\n");
				continue;
			}else if(pipeCounter == 1 ){
				pipeExecution();
				continue;
			}else{
				pipeHandling();
				continue;
			}
		}

		//fflush(stdin);
/**************FORK********************/
		fflush(stdout);
		fflush(stderr);
		//fflush(stdin);

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

			//set a descriptor to STD_OUT
			if(oWrite != 0){
				dup2(outDesc,1);
				close(outDesc);
			}

			/*	Redirected stdin to /dev/null 
				if the '<' redirection doesn't appear
				redirect stdin to /dev/null */
			if(directIN == 0 && amperSand != 0){
				int check;
				check = open("/dev/null",O_RDONLY);//open the null device
				if(check < 0){
					fprintf(stderr, "Can not open /dev/null device.\n");
					exit(9);
				}
				dup2(check,STDIN_FILENO);
				close(check);
			}

			//execute without pipe
			if(execvp(*newargv,newargv) < 0){
				printf("ERROR exec failed\n");
				exit(9);
			}

			
		}


		//After check dev null and redirection
		if(amperSand != 0){
			printf("%s [%d]\n",*newargv, childPid); //print current Process ID
			amperSand = 0; //reset amperSand
			continue;
		}
		else {			
			/* Wait for the child completion.
				Return the exit status of child*/
			for (;;){
				pid_t pid;
				pid = wait(NULL);
				if(pid == childPid)
					break;
			}

         	// wait(&status);
          //  	if(status > 0) 
          //  		fprintf(stderr, "Exit %d\n", WEXITSTATUS(status));
		}


	}//end for

	//Terminate any children that are
	//still running
	killpg(getpgrp(), SIGTERM);
	printf("p2 terminated.\n");// Make sure this printf come AFTER killpg
	exit(0);

}// main

int parse()
{
	int i = 0; 

	//this pointer will keep track the argvStorage array for each of loops
	//the getword function will not overwrite the argvStorage array
	int ptrArgv = 0; 
	int count = 0;
	int wCount = 0; //return the number of argc
	/* Set initial for the direction file*/
	fileIn = '\0';
	fileOut = '\0';
	int p = 0;//index of pipeArgv
	directIN = directOUT = oWrite = 0;
	pipeCounter = 0;
	amperSand = 0;
	rpipe = 0;

	int iPipe = 0; //index of pipe array

	/* Read from stdin to argvStorage array. The ptrArgv will keep track the argvStorage array
		If it is meta character, set a flag appropriately. Otherwise,
		set the address of first char of argvStorage to the newargv*/
	while ((sizeC = getword(argvStorage + ptrArgv)) > 0)
	{
		if( (sizeC == 1 && *(argvStorage + ptrArgv) == '|') ){
			pipeCounter++;
			newargv[i++] = NULL;
			pipeArr[iPipe++] = i;//pipe array starts at 0 ...
			rpipe = i; 
			continue;
		}
		/* Break the parse when hit the ampersand character
			*/
		else if(*(argvStorage + ptrArgv) == '&'){
			// if (directIN == 1 || directOUT == 1){
			// 	if (fileIn == NULL && fileOut == NULL){
			// 		break;
			// 	}
			// }
			amperSand++;
			break;
			//}
		}
		/*	Redirect input detect if there is '<' character
			only count 1 redirect input at the time.
			If the size of the word after '<' character not equal 0
			directIN flag again, and a array pointer is assigned to the input*/
		else if( (sizeC == 1 && *(argvStorage + ptrArgv) == '<') || (sizeC > 0 && directIN == 1) )
		{
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

		//handling ">!" 
		else if( (sizeC == 2 && strcmp((argvStorage + ptrArgv),">!") == 0 ) || (sizeC > 0 && oWrite == 1) ){
			oWrite++;
			if(oWrite == 2)//after the first '>' is dectected
				fileOut = (argvStorage + ptrArgv);
		}
		else{
			//Put the address of first char of each argvStorage to the pointer array
			newargv[i] = argvStorage + ptrArgv;
			i++;
		}

		//argvStorage[ptrArgv + sizeC] = '\0';
		//point to the next address of next word in argvStorage, the getword will not overwrite the argvStorage array
		ptrArgv = ptrArgv + sizeC + 1;
		wCount++;


	}//end while

	newargv[i] = NULL;
	//pipeArgv[p] = NULL;
	return wCount;

}//end parse

void myhandler(){

}//end myhandler

	

void pipeExecution(){
	int fd[2];
	pid_t child, secondChild;

	//fflush(stdin);
	/* create second child first execute and wait for first child */
	child = fork();
	if(child < 0){
		fprintf(stderr, "Child 2 fork failed\n");
		exit(9);
	}
	else if(child == 0){
		//create a pipe
		int checkPipe = pipe(fd);
		if(checkPipe == -1){
			fprintf(stderr, "Pipe failed\n");
			exit(9);
		}

		//fflush(stdin);
		//second child FORK
		secondChild = fork();
		if(secondChild < 0){
			fprintf(stderr, "Child 2 fork failed\n");
			exit(9);
		}
		else if (secondChild ==0){

			if(directIN != 0){
				//create a new open file descriptor
				//return -1 and appear error if the file open is unsucessful
				inDesc = open(fileIn,O_RDONLY);
				if(inDesc == -1){
					fprintf(stderr, "ERROR: Unsucessful open on %s \n",fileIn);
				}
				//read from STDIN 
				dup2(inDesc,STDIN_FILENO);
				close(inDesc);
			}

			//write to STDOUT
			dup2(fd[1],STDOUT_FILENO);
			close(fd[0]);
			close(fd[1]);
	
			if(execvp(*newargv,newargv) < 0){
				fprintf(stderr, "ERROR secondChild execute failed\n");
				exit(9);
			}
		}
		else if (secondChild > 0){ //first child read from STDIN

			if(directOUT != 0){
				outDesc = open(fileOut, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);//only read and write permission
				if(outDesc < 0){
					if(errno == EEXIST){
						fprintf(stderr, "The file  \" %s \"  already exists \n",fileOut);
						directOUT = 0;
					}
				}
				dup2(outDesc,STDOUT_FILENO);
				close(outDesc);
			}

			if(oWrite != 0){
				outDesc = open(fileOut, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);;//only read and write permission
				if(outDesc < 0){
					fprintf(stderr, "ERROR: Unsucessful overwrite on %s \n",fileOut);
					oWrite = 0;
				}
				dup2(outDesc,STDOUT_FILENO);
				close(outDesc);
			}


			dup2(fd[0],STDIN_FILENO);
			close(fd[1]);
			close(fd[0]);

			if (execvp(newargv[rpipe],newargv+rpipe) < 0){
				fprintf(stderr, "ERROR child execute failed\n");
				exit(9);
			}
		}

	}


	/*The parent wait for first child finishes its execution*/


	/*	Redirected stdin to /dev/null 
		if the '<' redirection doesn't appear
		redirect stdin to /dev/null */
	if(directIN == 0 && amperSand != 0){
		int check;
		check = open("/dev/null",O_RDONLY);//open the null device
		if(check < 0){
			fprintf(stderr, "Can not open /dev/null device.\n");
			exit(9);
		}
	}


	//After check dev null and redirection
	if(amperSand != 0){
		printf("%s [%d]\n",*newargv, child); //print current Process ID
		amperSand = 0; //reset amperSand
		//continue;
	}
	else {			
		/* Wait for the child completion.
			Return the exit status of child*/
		for (;;){
			pid_t pid;
			pid = wait(NULL);
			if(pid == child)
				break;
		}

		//printf("Parent kill children\n");
     	// wait(&status);
      //  	if(status > 0) 
      //  		fprintf(stderr, "Exit %d\n", WEXITSTATUS(status));
	}
			
		

}







void multiPipe()
{
  int status;
  int i;

  int fd[4];
  
  pipe(fd); // sets up 1st pipe
  pipe(fd + 2); // sets up 2nd pipe


  
  if (fork() == 0)
    {

      dup2(fd[1], 1);

      close(fd[0]);
      close(fd[1]);
      close(fd[2]);
      close(fd[3]);

      execvp(*newargv, newargv);
    }
  else
    {
      // fork second child (to execute grep)

 	 	if (fork() == 0)
		{
		  
		  dup2(fd[0], 0);

		  dup2(fd[3], 1);

		  // close all ends of pipes

		  close(fd[0]);
		  close(fd[1]);
		  close(fd[2]);
		  close(fd[3]);

		  execvp(newargv[(pipeArr[0])], newargv+(*(pipeArr+0)));
		}
      	else
		{
	  	// fork third child (to execute cut)

	  		if (fork() == 0)
		    {

		      dup2(fd[2], 0);

		      // close all ends of pipes

		      close(fd[0]);
		      close(fd[1]);
		      close(fd[2]);
		      close(fd[3]);

		      execvp(newargv[(pipeArr[1])], newargv+(*(pipeArr+1)));
	    	}
		}
    }
      
  // only the parent gets here and waits for 3 children to finish
  
  close(fd[0]);
  close(fd[1]);
  close(fd[2]);
  close(fd[3]);

  for (i = 0; i < 3; i++)
    wait(&status);
}



void pipeHandling()
{
	int fd[2];
	int fd2[2];
	pid_t first, second, third;


	first = fork();
	if(first < 0){
	    fprintf(stderr, "2nd fork failed\n");
	    exit(9);
	}
	else if(first == 0)
	{
		//create 1st pipe
		int check1 = pipe(fd);
		if(check1 == -1){
			fprintf(stderr, "Pipe 1 failed\n");
			exit(9);
		}

		//second child is created
		//to read from 3rd child
		second = fork();
		if(second < 0){
		    fprintf(stderr, "2nd fork failed\n");
		    exit(9);
		}
		else if(second == 0)
		{
			//second pipe is created
			int check = pipe(fd2);
			if(check == -1){
				fprintf(stderr, "Pipe 2 failed\n");
				exit(9);
			}

			//third child execute
			third = fork();
			if(third == 0){
				if(third == 0){
					//write end to 2nd pipe
					dup2(fd2[1],STDOUT_FILENO);
					close(fd2[1]);
					close(fd2[0]);
				}
				if(execvp(*newargv,newargv) < 0){
					fprintf(stderr, "ERROR thirdChild execute failed\n");
					exit(9);
				}
			}
			//second child execute
			else
			{
				//read end from first argument(3rd child)
				dup2(fd2[0],STDIN_FILENO);
				close(fd2[0]);
				close(fd2[1]);


				//write end to 1st pipe
				dup2(fd[1],STDOUT_FILENO);
				close(fd[1]);
				close(fd[0]);

				//second argument execute
				if(execvp(newargv[(pipeArr[0])], newargv+(*(pipeArr+0))) < 0){
					fprintf(stderr, "ERROR secondChild execute failed\n");
					exit(9);
				}
			}
		}//second child
		else if (second > 0)//first child read from second child
		{
			dup2(fd[0],STDIN_FILENO);
			close(fd[0]);
			close(fd[1]);

			//second argument execute
			if(execvp(newargv[(pipeArr[1])], newargv+(*(pipeArr+1))) < 0){
				fprintf(stderr, "ERROR firstChild execute failed\n");
				exit(9);
			}
		}
	}//end first


	//parent wait for first child
	for (;;){
		pid_t pid;
		pid = wait(NULL);
		if(pid == first)
			break;
	}
}

// void pipeHandling()
// {
// 	int fd[4];
// 	//int fd2[2];
// 	pid_t first, second, third;



// 	first = fork();
// 	if(first < 0){
// 	    fprintf(stderr, "2nd fork failed\n");
// 	    exit(9);
// 	}
// 	else if(first == 0)
// 	{
// 		//create 1st pipe
// 		int check1 = pipe(fd);

// 		if(check1 == -1){
// 			fprintf(stderr, "Pipe 1 failed\n");
// 			exit(9);
// 		}

// 		//second pipe is created
// 		int check2 = pipe(fd+2);
// 		if(check2 == -1){
// 			fprintf(stderr, "Pipe 2 failed\n");
// 			exit(9);
// 		}

// 		//second child is created
// 		//to read from 3rd child
// 		second = fork();
// 		if(second < 0){
// 		    fprintf(stderr, "2nd fork failed\n");
// 		    exit(9);
// 		}
// 		else if(second == 0)
// 		{
			

// 			//third child execute
// 			third = fork();
// 			if(third == 0){
// 				if(third == 0){
// 					//write end to 2nd pipe
// 					dup2(fd[1],STDOUT_FILENO);
// 					close(fd[1]);
// 					close(fd[0]);
// 					close(fd[2]);
// 					close(fd[3]);
					
// 				}
// 				if(execvp(*newargv,newargv) < 0){
// 					fprintf(stderr, "ERROR thirdChild execute failed\n");
// 					exit(9);
// 				}
// 			}
// 			//second child execute
// 			else
// 			{
// 				//read end from first argument(3rd child)
// 				dup2(fd[0],STDIN_FILENO);
// 				close(fd[0]);
// 				close(fd[1]);
// 				close(fd[2]);
// 				close(fd[3]);
				

// 				//write end to 1st pipe
// 				dup2(fd[3],STDOUT_FILENO);
// 				close(fd[3]);
// 				close(fd[2]);
// 				close(fd[0]);
// 				close(fd[1]);
				
// 				printf("2nd wrote to 1st\n");	

// 				//second argument execute
// 				if(execvp(newargv[(pipeArr[0])], newargv+(*(pipeArr+0))) < 0){
// 					fprintf(stderr, "ERROR secondChild execute failed\n");
// 					exit(9);
// 				}
// 			}
// 		}//second child
// 		else if (second > 0)//first child read from second child
// 		{
// 			dup2(fd[2],STDIN_FILENO);
// 			close(fd[2]);
// 			close(fd[3]);
// 			close(fd[0]);
// 			close(fd[1]);
					
// 			printf("1st read from 2nd\n");	

// 			//second argument execute
// 			if(execvp(newargv[(pipeArr[1])], newargv+(*(pipeArr+1))) < 0){
// 				fprintf(stderr, "ERROR firstChild execute failed\n");
// 				exit(9);
// 			}
// 		}
// 	}//end first


// 	//parent wait for first child
// 	for (;;){
// 		pid_t pid;
// 		pid = wait(NULL);
// 		if(pid == first)
// 			break;
// 	}
// }
