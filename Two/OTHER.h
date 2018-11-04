/*
CS570 Operating Systems
Professor John Carroll
Huy Nguyen
System header file
*/
#include <stdio.h>
#include <unistd.h>	//fork(),dup2(),execvp()
#include <stdlib.h>
#include <sys/stat.h>	//stat()
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>	//wait()
#include <signal.h>	//signal(),killpg()
#include <dirent.h>
#include <fcntl.h>