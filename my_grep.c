// CISC 3350: HW 5 Assignment Program
//
// Instructions: Below each one of the Task boxes 1-6,
//               write the code that the task describes.
//               After the correct lines of code are
//               added, the program will compile and
//               run correctly.
//               The code that each task requests ranges
//               from 1 line to 7 lines of code.
//               Do not change or delete any existing code.

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

// Constants:
#define MAX_NUM 32768 // Maximum number of files allowed in my_grep
#define BUF_SIZE 4001 // Maximum length allowed for a line read from
                      //     a file (including the null character)

// Global Variables:
char * PATTERN; // Will point at the string that we search for.
long long total_lines_read = 0; // By the end of the program
                                //    will contain the total
                                //    number of lines that were
                                //    searched by all threads.

// Mutexes:
/************************************************************\
|*                          Task 1                          *|
|* Right below this comment, define and intialize a mutex   *|
|*    variable. Name this mutex as you wish.                *|
|* This mutex variable will be used to synchronize the      *|
|*    activities of the threads and prevent them from       *|
|*    corrupting the data in the global (shared) variable   *|
|*    'total_lines_read' that we defined above.             *|
|* You will write 1 line of code for this Task 1.           *|
\************************************************************/
pthread_mutex_t my_mutex = PTHREAD_MUTEX_INITIALIZER;


// Function: 'searchfile'
// Input: filename (void *)
// Output: pointer to number of lines searched in file
// Purpose: each thread will run this function and will
//          search for the pattern inside the file
//          whose name is passed in 'filename', write
//          the lines of text where PATTERN was found to
//          stdout, and return the number of lines
//          where PATTERN was found.
void * searchfile (void * filename)
{
    // Declare a buffer:
    char *buffer;
         //*tempPtr; // general purpose char pointer
    // and variable to hold buffer length:
    int buffer_len;
    // A file pointer:
    FILE * infile;
    // Also, variable holding number of matched lines:
    long long matching = 0;

    // Allocate memory for the char buffer:
    if ((buffer = (char*) malloc (BUF_SIZE * sizeof (char))) == NULL)
    {
        perror ("my_grep: malloc");
        exit (EXIT_FAILURE);
    }

    // Cast filename to (char*) and open the file
    if ((infile = fopen ((char*) filename, "r")) == NULL)
    {
        perror ("my_grep: fopen");
        free (buffer);
        exit (EXIT_FAILURE);
    }

    // Using 'fgets' loop to read from file:
    while (fgets (buffer, BUF_SIZE, infile) != NULL)
    {
       /************************************************************\
       |*                          Task 2                          *|
       |* Right below this comment, acquire the lock of the mutex  *|
       |*    variable you defined in Task 1.                       *|
       |* You will write 1 line of code for this Task 2.           *|
       \************************************************************/
       pthread_mutex_lock (&my_mutex); // Locking


       // Increment the total number of read lines.
       //    Race conditions happen with this code line,
       //    which is why we lock this statement using a mutex.
       total_lines_read++;

       /************************************************************\
       |*                          Task 3                          *|
       |* Right below this comment, release the lock of the mutex  *|
       |*    variable you defined in Task 1.                       *|
       |* You will write 1 line of code for this Task 3.           *|
       \************************************************************/
       pthread_mutex_unlock (&my_mutex); // Unlocking

      
       // Finding the length of the line:
       buffer_len = strlen(buffer);
      
       // Case when the line is longer than 4000 chars (disallowed):
       if (buffer_len == BUF_SIZE - 1 && buffer[BUF_SIZE - 2] != '\n')
       {
           fprintf (stderr, "my_grep: An excessively long line was detected. Returning from thread...\n");
           free (buffer);
           exit (EXIT_FAILURE);
       }
       // Case when empty line is read in (allowed; continue reading new lines)
       else if (buffer_len == 1 && buffer[0] == '\n')
           continue;
      
       // If a match was found, print the line.
       if (strstr(buffer, PATTERN) != NULL)
       {
          printf ("%s: %s", (char *) filename, buffer);
    
          // Increment the number of matches: 
          matching++;
       }
    } // End of while loop

    // Dealocating memory:
    free (buffer);
    // Closing files:
    fclose (infile);

    // Print out how many matchings were found in this file:
    printf ("--%s has %lld matched lines\n", (char *) filename, matching);

    // Finally, exit the thread and return a pointer to the # of matching lines:
    /************************************************************\
    |*                          Task 4                          *|
    |* Right below this comment, call pthread_exit(). It takes  *|
    |*    a single variable. You should pass the variable       *|
    |*    'matching' after casting it to (void*).               *|
    |* You will write 1 line of code for this Task 4.           *|
    \************************************************************/
    pthread_exit ((void *)matching);


} // End of 'searchfile'

// Main function:
int main (int argc, char *argv[])
{
    // Check if we have the right number of arguments (at least 3)
    if (argc < 3)
    {
        fprintf (stderr, "my_grep: at least 3 arguments must be passed!\n"); 
        exit (EXIT_FAILURE);
    }
    // At this point, we know that argc >= 3.
    // However, we must make sure that the number of files is not too big:
    if (argc > MAX_NUM)
    {
        fprintf (stderr, "my_grep: too many filenames were passed!\n");
        exit (EXIT_FAILURE);
    }

    // Variable declaration:
    pthread_t *threads;   // Pointer to threads array that will be created
    long long result,     // Will hold returned values from 'pthread' functions
              sum = 0,    // Sum of total matches returned from the threads
              i;          // Loop index
    void *lines;          // Number of matching lines returned by a thread

    // Allocating memory for the PATTERN string:
    if ((PATTERN = (char*) malloc (strlen(argv[1]) * sizeof(char))) == NULL)
    {
        perror ("my_grep: malloc:");
        exit (EXIT_FAILURE);
    }

    // Copying the string in argv[1] into PATTERN:
    if (strcpy (PATTERN, argv[1]) == NULL)
    {
        fprintf (stderr, "my_grep: strcpy failed!\n");
        free (PATTERN);
        exit (EXIT_FAILURE);
    }

    // Allocate the threads array on the heap:
    if ((threads = (pthread_t*) malloc ((argc - 2) * sizeof (pthread_t))) == NULL)
    {
       perror ("my_grep: malloc:");
       free (PATTERN);
       exit (EXIT_FAILURE);
    }

    // In a loop, create threads, each for each filename
    //    (for a total of argc - 2 threads) to search the files.
    for (i = 0; i < argc - 2; i++)
    {
       /************************************************************\
       |*                          Task 5                          *|
       |* Right below this comment, call pthread_create() to       *|
       |*    create a thread for each file. This function accepts  *|
       |*    exactly 4 arguments:                                  *|
       |*    1. A pointer to threads[i] (Use the symbol: &).       *|
       |*    2. NULL, since no more configurations are needed.     *|
       |*    3. The function name: searchfile.                     *|
       |*    4. A pointer to the filename: (void *)(argv[i + 2])   *|
       |* Assign the returned value from pthread_create() to the   *|
       |*    'result' variable.                                    *|
       |* Below this line of code:                                 *|
       |* If result is not 0:                                      *|
       |*    1. Set the variable 'errno' to 'result'.              *|
       |*       [Don't define 'errno' in your program: this        *|
       |*        variable is already defined in <errno.h>.]        *|
       |*    2. Call perror() to print an error message.           *|
       |*    3. free() each of 'PATTERN' and 'threads'.            *| 
       |*    4. Exit the program with EXIT_FAILURE.                *|
       |* You will write up to 7 lines of code for this Task 5.    *|
       \************************************************************/
       result = pthread_create (&(threads[i]), NULL, searchfile, (void *)(argv[i + 2]));
       if (result != 0)
       {
            errno = result;
            perror("pthread_create");
            free(PATTERN);
            free(threads); 
            return EXIT_FAILURE;
       }

    }

    // If we reached this line, all threads were successfully created.
    // Then, after the threads run, we have to join them:
    for (i = 0; i < argc - 2; i++)
    {
       /************************************************************\
       |*                          Task 6                          *|
       |* Right below this comment, call pthread_join() to         *|
       |*    join (wait for) the created threads. This function    *|
       |*    accepts exactly 2 arguments:                          *|
       |*    1. The thread variable: threads[i].                   *|
       |*    2. A pointer to the 'lines' variable (Use: &).        *|
       |* Assign the returned value from pthread_join() to the     *|
       |*    'result' variable.                                    *|
       |* Below this line of code:                                 *|
       |* If result is not 0:                                      *|
       |*    1. Set the variable 'errno' to 'result'.              *|
       |*       [Don't define 'errno' in your program: this        *|
       |*        variable is already defined in <errno.h>.]        *|
       |*    2. Call perror() to print an error message.           *|
       |*    3. free() each of 'PATTERN' and 'threads'.            *|
       |*    4. Exit the program with EXIT_FAILURE.                *|
       |* You will write up to 7 lines of code for this Task 6.    *|
       \************************************************************/
       result = pthread_join (threads[i], (void *)&lines);
        if (result != 0)
        {
            errno = result;
            perror("pthread_join");
            free(PATTERN);
            free(threads);
            return EXIT_FAILURE;
        }







       printf ("[main] Thread #%lld returned with value: %lld\n", i+1, (long long)lines); 
       sum += (long long)lines; // add up the returned values 
    }

    // Print total # of matching lines + total # of lines scanned:
    printf ("[main] Total of %lld matched lines among total of %lld lines scanned\n", sum, total_lines_read);

    // Free the memory allocated for PATTERN:
    free (PATTERN);
    // Free the memory allocated for the threads:
    free (threads);

    return EXIT_SUCCESS;

} // End of main

// End of Program