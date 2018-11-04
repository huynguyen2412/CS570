	void main()
{
    int status;
    int i;
    pid_t first,second,third;

    // make 2 pipes (cat to grep and grep to cut); each has 2 fds

    int pipes[4];
    pipe(pipes); // sets up 1st pipe
    pipe(pipes + 2); // sets up 2nd pipe

    // we now have 4 fds:
    // pipes[0] = read end of cat->grep pipe (read by grep)
    // pipes[1] = write end of cat->grep pipe (written by cat)
    // pipes[2] = read end of grep->cut pipe (read by cut)
    // pipes[3] = write end of grep->cut pipe (written by grep)

    // Note that the code in each if is basically identical, so you
    // could set up a loop to handle it.  The differences are in the
    // indicies into pipes used for the dup2 system call
    // and that the 1st and last only deal with the end of one pipe.

    // fork the first child (to execute cat)

    //"cat scores | grep Villanova | cut -b 1-10".
  
    first = fork();
    if(first < 0){
        fprintf(stderr, "1st fork failed\n");
        exit(9);
    }
    else(first == 0){
        //create 2 pipes
        pipe(pipes);
        pipes(pipes + 2);

        //fork 2nd child
        second = fork();
        if(second < 0){
            fprintf(stderr, "2nd fork failed\n");
            exit(9);
        }
        else if(second == 0){
        	//create 3rd child
        	third = fork();
	        if(third < 0){
	            fprintf(stderr, "3rd fork failed\n");
	            exit(9);
        	}
        	else if(third > 0){//third execute
        		//write end to 2nd child's pipe
        		dup2(pipes[1],STDOUT_FILENO); 
        		close(pipes[0]);
        		close(pipes[1]);
        		close(pipes[2]);
        		close(pipes[3]);

        		//exec first command
				if(execvp(*newargv,newargv) < 0){
					fprintf(stderr, "ERROR secondChild execute failed\n");
					exit(9);
				}
        	}
        }
        //2nd child exec
        else if(second > 0){
        	//read end from 3rd child
        	dup2(pipes[0],STDIN_FILENO);
        	//write end to 1st child
        	dup2(pipe[3],STDOUT_FILENO);
    		close(pipes[0]);
    		close(pipes[1]);
    		close(pipes[2]);
    		close(pipes[3]);
    		//exec 2nd command
			if(execvp(newargv[(pipeArr[0])], newargv+(*(pipeArr+0))) < 0){
				fprintf(stderr, "ERROR secondChild execute failed\n");
				exit(9);
			}
        }
    }//1st execute
    else if(child > 0){
    	dup2(pipes[2],)
    }



}