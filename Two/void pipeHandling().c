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
