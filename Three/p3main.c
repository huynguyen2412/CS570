/*  p3main.c
      
      This is the driver program for Program p3.
      It constitutes an important part of the specification for Program 3.
      You code, however, will all be in p3helper.c
      (The p3helper.c stub provided in the assignment directory
      should compile and link with this -- and run (though the results
      are clearly wrong).)

      If a run gets jammed, Control-C will normally terminated it. 
      */
  
#include "p3.h"
  
/* Your p3helper() code should NOT access the following three semaphores: */
static sem_t alldone; /* for coordinating the windup printout */
static sem_t statsprotect; /* to guard access to the stats_file */
static sem_t randomprotect; /* rand() is not thread safe; see saferand below */

static pthread_t reader_thread[MAXCUSTOMERS];
static pthread_t writer_thread[MAXCUSTOMERS];

static int stats[2] = { 0, 0 }; /* Shared record-keeping area.  Your
                                   added code should not access this array. */
static int failure = 0;         /* set if there are ever simultaneous readers
                                   and writers.  Your added code should not
                                   access this variable. */
static int maxconcur = 0;       /* track the maximum concurrency of readers */
static int maxconcuw = 0;       /* track the maximum concurrency of writers */
static int protocol = 1;        /* defaults to reader/writer fairness */

long saferand(void);  /* the standard random wrapped in a semaphore */
static void *reader(void *pn);  /* code run by each reader thread */
static void *writer(void *pn); /* code run by each writer thread */

/* BLECH! without the following arrays, it looks like threads
   can get the wrong reader_id (and similarly for the writers):*/
static int rdr[MAXCUSTOMERS];
static int wrtr[MAXCUSTOMERS];

int main(int argc, char *argv[]) {
  int reader_id;
  int writer_id;
  int nr_today; /* Total number of customers on this run */
  int i; /* loop counter */
  int pres; /* result code I'll use for pthread error reporting */

  printf("p3 is now running (pid = %d).\n",(int)getpid());
  fflush(stdout);

/* Check the first argument (argv[1]) to ensure a legal protocol was chosen. */
  switch ( atoi(argv[1]) ) {
    case FAIR: break; 
    case WRIT: protocol = 0; break; 
    default: printf( "Illegal input.  The first argument must be either 0 (for writer\n");
    printf("priority) or 1 (for FCFS fairness to both readers and writers. ABORTING!\n");
    exit(7);
}

  /* Initialize random number generator (with the second command
     line argument if any is provided). I'm doing this so we
     can simulate a variety of timings on different runs. */
  srand( (argc<2) ? SEED : atoi(argv[2]) );
  
  initstudentstuff(protocol);  /* provided by you, in p3helper.c */

  CHK(sem_init(&alldone,LOCAL,0));
  CHK(sem_init(&statsprotect,LOCAL,1));
  CHK(sem_init(&randomprotect,LOCAL,1));
  
  /* Starting up "today's" mix of customers: */
  nr_today = saferand()%MAXCUSTOMERS + 1;
  printf("nr_today = %d\n",nr_today);
  fflush(stdout);

  reader_id=0;
  writer_id=0;
  for ( i=0; i < nr_today; i++ ) {
    if ( saferand()%2 ) {
      reader_id++;
      rdr[reader_id] = reader_id;
      if ((pres = pthread_create(&reader_thread[reader_id],
                                 NULL,
                                 reader,
                                 (void *)&(rdr[reader_id]))) != 0) {
        fprintf(stderr, "Exiting process: could not create reader %d, %s\n",
                reader_id,strerror(pres));
        exit(1);
      }
    } else {
      writer_id++;
      wrtr[writer_id] = writer_id;
      if ((pres = pthread_create(&writer_thread[writer_id],
                                 NULL,
                                 writer,
                                 (void *)&(wrtr[writer_id]))) != 0) {
        fprintf(stderr, "Exiting process: could not create writer %d, %s\n",
                writer_id,strerror(pres));
        exit(1);
      }
    }
  }

  printf("Number of readers:%d; Number of writers:%d\n",reader_id,writer_id);
  /* Now the parent blocks till all the customers are done: */
  for (i=0; i< nr_today ; i++) {
    CHK(sem_wait(&alldone));
  }
  printf("alldone -- THE DATABASE IS NOW CLOSED\n");
  printf("DAILY REPORT:\nTotal number of different customers:\n");
  printf("Number of readers:%d; Number of writers:%d\n",reader_id,writer_id);
  printf("People still in the database (better be 0 for each kind!):\n");
  printf("Number of readers left: %d; Number of writers left: %d\n",
         stats[READER],stats[WRITER]);
  printf("There were =%d= instances of protocol failure for %s%c\n",
          failure, getenv("USER"), ( failure == 0 ) ? '.' : '!' );
  printf("Maximum simultaneous reader occupancy was @%d@ for %s\n", maxconcur, getenv("USER"));
  printf("Maximum simultaneous writer occupancy was @%d@ for %s\n", maxconcuw, getenv("USER"));
  fflush(stdout);
  exit(0);
}

/*-----------------------------------------------------------------------
 * Name: reader
 * Purpose: Simulates a reader in our database.
 This routine is intended to be run as a separate thread.
 Prints an entrance announcement (to stdout).
 Prints (to stdout) a second line when it is leaving the database.
 Upon completion, sem_post(&alldone) and terminates.
 * Input parameters: The reader's id number.
 *                   and the protocol to implement (FAIR or WRITER-Priority)
 * Output parameters: None.
 * Other side effects: See above. Any error reporting is done from within
 the routine.
 * Routines called: prolog,epilog,sem_post,sem_wait
 * References: See the program3 assignment file
 */
void *reader(void *pn) {
  int  k; /* loop counter for turns */
  int turn; /* should be one more than the loop counter :-( */
  int goal;
  int n;

  goal = (saferand()%MAXSESSIONS)+1;
  n = *(int *)pn;
  
  ARRIVAL_DELAY();
  for (k=0; k<goal ; k++) {
    turn = k+1;
    prolog(READER, protocol); /* get permission to enter the database */

    /* The following lines can be thought of as the customer's signing in.
       In fact, they provide us with the major testing output of the program
     */    
    CHK(sem_wait(&statsprotect));
    (stats[READER])++;
    printf("R%d [%d/%d]. Just after entering: "
            "%d READERS and %d writers\n",
            n,turn,goal,stats[READER],stats[WRITER]);
    if ( stats[READER] > maxconcur ) {
	    maxconcur = stats[READER];
    }
    if ( stats[WRITER] > 1 ) {
            failure++;
            printf("ERROR! More than one writer is present in the database!\n");
    }
    if ( stats[READER]&&stats[WRITER] ) {
            failure++;
            printf("ERROR! Both types of customers are present in the database!\n");
    }
    fflush(stdout);
    CHK(sem_post(&statsprotect));
    
    READING(); /* this macro is defined in p3.h */
    
    /* The following lines can be thought of as the customer's signing out.
       In fact, they provide us with the major testing output of the program
     */    
    CHK(sem_wait(&statsprotect));
    (stats[READER])--;
    printf("R%d [%d/%d]. Just after leaving: "
            "%d READERS and %d writers\n",
            n,turn,goal,stats[READER],stats[WRITER]);
    fflush(stdout);
    CHK(sem_post(&statsprotect));
    
    epilog(READER, protocol); /* customer gives up his/her access permission to database */
    PROCESSING();PROCESSING();
  }
  printf("R%d all done\n",n);
  fflush(stdout);
  CHK(sem_post(&alldone));
  return(NULL);
}

/*-----------------------------------------------------------------------
 * Name: writer
 * Purpose: Simulates a writer in our database.
 This routine is intended to be run as a separate thread.
 Prints an entrance announcement (to stdout).
 Prints (to stdout) a second line when it is leaving the database.
 Upon completion, sem_post(&alldone) and terminates.
 * Input parameters: The writer's id number.
 *                   and the protocol to implement (FAIR or WRITER-Priority)
 * Output parameters: None.
 * Other side effects: See above. Any error reporting is done from within
 the routine.
 * Routines called: prolog,epilog,sem_post,sem_wait
 * References:  See the program3 assignment file
 */
void *writer(void *pn) {
  int  k; /* loop counter for turns */
  int turn; /* should be one more than the loop counter :-( */
  int goal;
  int n;

  goal = (saferand()%MAXSESSIONS)+1;
  n = *(int *)pn;
  
  ARRIVAL_DELAY();
  for (k=0; k<goal ; k++) {
    turn = k+1;
    prolog(WRITER, protocol); /* get permission to enter the database */

    /* The following lines can be thought of as the customer's signing in.
       In fact, they provide us with the major testing output of the program
     */    
    CHK(sem_wait(&statsprotect));
    (stats[WRITER])++;
    printf("W%d [%d/%d]. Just after entering: "
            "%d readers and %d WRITERS\n",
            n,turn,goal,stats[READER],stats[WRITER]);
    if ( stats[WRITER] > maxconcuw ) {
	    maxconcuw = stats[WRITER];
    }
    if ( stats[WRITER] > 1 ) {
            failure++;
            printf("ERROR! More than one writer is present in the database!\n");
    }
    if ( stats[READER]&&stats[WRITER] ) {
            failure++;
            printf("ERROR! Both types of customers are present in the database!\n");
    }
    fflush(stdout);
    CHK(sem_post(&statsprotect));
    
    WRITING(); /* this macro is defined in p3.h */
    
    /* The following lines can be thought of as the customer's signing out.
       In fact, they provide us with the major testing output of the program
     */    
    CHK(sem_wait(&statsprotect));
    (stats[WRITER])--;
    printf("W%d [%d/%d]. Just after leaving: "
            "%d readers and %d WRITERS\n",
            n,turn,goal,stats[READER],stats[WRITER]);
    fflush(stdout);
    CHK(sem_post(&statsprotect));

    epilog(WRITER, protocol); /* customer gives up his/her access permission to database*/
    PROCESSING();PROCESSING();
  }
  printf("W%d all done\n",n);
  fflush(stdout);
  CHK(sem_post(&alldone));
  return(NULL);
}

long saferand(void) {
  long result;
  
  CHK(sem_wait(&randomprotect));
  result = rand();
  CHK(sem_post(&randomprotect));
  return result;
}
