/*
CS570 Operating Systems
Professor John Carroll
Huy Nguyen
p2 header file
*/

#include <stdio.h>
#include "getword.h"
#include <unistd.h>	//fork(),dup2(),execvp()
#include <stdlib.h>
#include <sys/stat.h>	//stat()
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>	//wait()
#include <signal.h>	//signal(),killpg()
#include <dirent.h>
#include <fcntl.h>
#include <string.h> //strcmp()
#include "OTHER.h"
#define MAXITEM 100 /* max number of words per line */
#define MAXPIPES 10

int parse();

void myhandler();
