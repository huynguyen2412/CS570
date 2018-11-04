/* p3helper.c
 * Huy Nguyen
 * Program 3
 * CS570
 * Professor Carroll
 * SDSU
 * 11/1/2017
 *
 * This is the only file you may change. (In fact, the other files should
 * be symbolic links to:
 *   ~cs570/Three/p3main.c
 *   ~cs570/Three/p3.h
 *   ~cs570/Three/Makefile
 *   ~cs570/Three/CHK.h    )
 *
 */
#include "p3.h"

/* You may put any semaphore (or other global) declarations/definitions here: */
static sem_t execute_lock; /*  The lock to enter the database*/
static sem_t reader_lock; /*  PRIORITY protocol: lock the reader when the writer is in database	*/
//static sem_t flag_lock; /*  */
static sem_t getline_lock; /* Fairness protocol: lock the reader when the writer enters the database */

static int writer_count = 0;
static int reader_count = 0;
static int customer_count = 0;

/* General documentation for the following functions is in p3.h
 * Here you supply the code:
 */
void initstudentstuff(int protocol) {
  
  printf("initialization started ");
  fflush(stdout);

  /* Initialize the locks */
  CHK(sem_init(&execute_lock,LOCAL,1));
  CHK(sem_init(&reader_lock,LOCAL,1));
  //CHK(sem_init(&flag_lock,LOCAL,1));
  CHK(sem_init(&getline_lock,LOCAL,1));

  printf("initialization complete \n");
  fflush(stdout);
  

}

void prolog(int kind, int protocol) {
	customer_count++;//keep track the number of readers and writers
	if (customer_count > MAXCUSTOMERS){
		printf("Total customer: %d\n",customer_count);
		assert(0); // terminate the program
	}


	/* In this case, the reader will wait for the getline_lock to enter to the database.
		When the readers get the getline_lock, the multiple readers after its can access the database 
		concurrency.
		*/
	if(protocol == FAIR){
		if(kind == READER){
			//the reader can't access database
			//have to wait since writer is in line
			if(writer_count >=1){
				printf("Reader get the GET_LINE LOCK \n" );
				fflush(stdout);
				sem_wait(&getline_lock);//get to another line wait until writer finishes
				sem_wait(&execute_lock);//wait in line when the writer is in database
				sem_post(&getline_lock);//return the lock for another readers are waiting on line
			}
			else if(reader_count == 0){
				sem_wait(&execute_lock);
				printf("Reader get execution LOCK \n" );
				fflush(stdout);
			}

			reader_count++;	
			printf("Reader count = %d \n",reader_count);
			fflush(stdout);
		}
		else //WRITER 
		{

			//waiting if the reader is in database
			//get the lock to get in line 
			//if the readers arrive after they have to wait
			printf("Writer get the GET_LINE LOCK \n" );
			fflush(stdout);
			sem_wait(&getline_lock);

			/*When the writer is in the database. The readers after its have to wait for getline_lock and execute_lock*/
			writer_count++;

			printf("Writer get the EXECUTE LOCK, count = %d \n",writer_count);
			fflush(stdout);
			sem_wait(&execute_lock);
		}
	}


	printf("prolog KIND %d, protocol %d \n",kind, protocol );
    fflush(stdout);


    /* In this case a writer hold a reader_lock. The readers can only access
    	when the last writer release the reader_lock.
    	*/
	if(protocol == WRIT) {//priority
		if(kind == READER){
			//check if any writer is in-line waiting then 
			//enforce READER to wait outside database by taking reader_lock()
			//sem_wait(flag_lock);
			if (writer_count > 0) {
				//sem_wait(flag_lock);
				printf("reader get READER LOCK \n" );
				fflush(stdout);
				sem_wait(&reader_lock);
				printf("reader release READER LOCK \n" );	
				fflush(stdout);
				sem_post(&reader_lock);
			}

			printf("Reader gets EXECUTE LOCK \n" );	
			fflush(stdout);
			sem_wait(&execute_lock);
			return;
 
		//WRITER HANDLING				
		}else {
			writer_count++;
			if (writer_count == 1)	{
				//acquire READER lock
                printf("WRITER  #%d Get READER LOCK \n",writer_count );
                fflush(stdout);
				sem_wait(&reader_lock);
			}
     		writer_count--;
            printf("WRITER  %d holding reader lock \n",writer_count );
            fflush(stdout);     		
			//and if this is the last WRITER - then return reader_lock
			//so next reader can acquire the lock and execute.
			if (writer_count==0)
			{
				printf("WRITER Release READER LOCK \n" );
				fflush(stdout);
				sem_post(&reader_lock);
			}

			printf("Writer gets EXECUTE LOCK \n" );	
			fflush(stdout);			

			sem_wait(&execute_lock);
		}
	}//WRITER
}

void epilog(int kind, int protocol) {
	customer_count--;
	if(protocol == FAIR){
		if(kind == READER){
			reader_count--;
			//the reader is last one
			if (reader_count == 0){
				printf("Reader Release execute LOCK \n" );
				fflush(stdout);
				sem_post(&execute_lock);
			}
			printf("Reader count = %d \n",reader_count);
			fflush(stdout);
		}
		else //WRITER
		{
			//return the lock 
			printf("WRITER Release getline LOCK \n" );
			fflush(stdout);
			sem_post(&getline_lock);
			printf("WRITER Release execution LOCK \n" );
			fflush(stdout);
			sem_post(&execute_lock);

			writer_count--;
			printf("Writer count = %d \n",writer_count);
			fflush(stdout);
		}

	}

	if(protocol == WRIT){//PRIORITY
		//if READER completed execution - just return the lock
		if(kind == READER){
			printf("READER Release EXE LOCK \n" );
			fflush(stdout);
			sem_post(&execute_lock);
		
		//if WRITER completed execution - return the lock
		} else if (kind == WRITER){
			printf("WRITER Release EXE LOCK \n" );
			fflush(stdout);
			sem_post(&execute_lock);
			
		};		
	}
}  

