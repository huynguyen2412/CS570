/* p3.h

   This is the header file for cs570, assignment p3.
   See the assignment file, program3, for other details.

 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include <semaphore.h>
#include <pthread.h>
#include <errno.h>
#include "CHK.h"

#define FALSE 0
#define TRUE 1
#define WRIT 0
#define FAIR 1
#define LOCAL 0  /* in sem_init to indicate only for use of threads
                    in this process */

long random(void);
long saferand(void);

#define ARRIVAL_DELAY() sleep((random()%MAXLATENESS)+1)
#define READING() sleep((random()%2)+1)
#define WRITING() sleep((random()%2)+1)
#define PROCESSING() sleep((random()%2)+1)

#define READER 0
#define WRITER 1

#define SEED 200    /* Default random number seed */
#define MAXCUSTOMERS   12     /* maximum customers in a day */
#define MAXSESSIONS 10 /* maximum times a customer comes back in a day */
#define MAXLATENESS 30 /* some customers don't arrive till later */

#define SMAX 1024   /* size of formatting buffer */

/* initstudentstuff is where you can do things like initialize your semaphores.
   (Such a function is not usually good style, but in this case it
   makes it easy for us to keep student and driver things separate. */
void initstudentstuff(int);

/*-----------------------------------------------------------------------
 * Name: prolog
 * Purpose: Get into the database, subject to The Rules.
 See the program3 assignment file for The Rules.
 (The routine does not return until there is success.)
 * Input parameters: The kind of customer, READER or WRITER
 *                   and the protocol to implement (FAIR or WRITer-Priority)
 * Output parameters: None.
 * Other side effects: Concurrency effects should satisfy the requirements
 of the program3 assignment file
 * Routines called: ...
 * References: The program3 assignment file.
 
IMPORTANT NOTE: You supply all the code here! (But remember that you
are not allowed to access stats[]. If you need shared data, declare and
use your own auxiliary variables/arrays.
 */
void prolog(int kind, int protocol);

/*-----------------------------------------------------------------------
 * Name: epilog
 * Purpose: give up access permission to the database, subject to The Rules.
 See the program3 assignment file for The Rules.
 * Input parameters: The kind of customer, READER or WRITER
 *                   and the protocol to implement (FAIR or WRITer-Priority)
 * Output parameters: None.
 * Other side effects: Concurrency effects should satisfy the requirements
 of the program3 assignment file.
 * Routines called: ...
 * References: The program3 assignment file.
 
IMPORTANT NOTE: You supply all the code here! (But remember that you
are not allowed to access stats[]. If you need shared data, declare and
use your own auxiliary variables/arrays.
 */
void epilog(int kind, int protocol);
