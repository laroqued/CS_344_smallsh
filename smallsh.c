#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>

#define MAX_LENGTH_INPUT 2048
#define MAX_ARGS 512
#define MAX_PATH 256

// Global variables
int isBackGround = 1;

// Prototypes
void getLine(char *charArray[], int *backgroundJob, char sourceFD[], char targetFD[], int pid);
void execFunction(char *charArray[], int *childExitStatus, int *backgroundJob, char sourceFD[], char targetFD[], struct sigaction SIGINT_action);
void SIGTSTP_catch(int signo);
void displayExitStatus(int childExitMethod);
void backgroundProcess(int *backgroundJob, int isBackGround, int *childExitStatus, int spawnPid);
void clearInput(char *userInput[]);

int main()
{
     // ============================================================================================
     /*
          // Citation for the following function:
          // Date: 11/01/2021
          // Copied from /OR/ Adapted from /OR/ Based on:
          // Source URL: https://oregonstate.instructure.com/courses/1830250/pages/exploration-signal-handling-api?module_item_id=21468881
     */
     // ============================================================================================
     struct sigaction SIGINT_action = {0};  // Initialize SIGINT_action struct to be empty
     struct sigaction SIGTSTP_action = {0}; // Initialize SIGTSTP_action struct to be empty
     int pid = getpid();                    // Process Id
     int isShellRunning = 1;                // True
     int i;
     int childExitMethod = 0;      // Spawn the child process
     int backgroundJob = 0;        /* At any given time, there can only one job running in the foreground
                                      while multiple jobs may be running the background.*/
     char sourceFD[MAX_PATH] = ""; // Microsoft Windows has a MAX_PATH limit of ~256 characters.
     char targetFD[MAX_PATH] = "";
     char *userInput[MAX_ARGS]; // Project requires a maximum of 512 arguments.
     char compareString[2] = "";

     // 8. Signals SIGINT & SIGTSTP
     // Fill out the SIGTSTP_action struct
     // A CTRL-C command from the keyboard sends a SIGINT signal to the parent process and all children at the same time
     SIGINT_action.sa_handler = SIG_IGN;      // We register the SIG_IGN constant as the handler for SIGTERM, SIGHUP and SIGQUIT.
     sigfillset(&SIGINT_action.sa_mask);      // Block all catchable signals while handle_SIGINT is running
     SIGINT_action.sa_flags = 0;              // No flags set
     sigaction(SIGINT, &SIGINT_action, NULL); // Install our signal handler

     // A CTRL-Z command from the keyboard sends a SIGTSTP signal to your parent shell process and all children at the same time .
     SIGTSTP_action.sa_handler = SIGTSTP_catch;
     sigfillset(&SIGTSTP_action.sa_mask);
     SIGTSTP_action.sa_flags = 0;               // No flags set
     sigaction(SIGTSTP, &SIGTSTP_action, NULL); // SIGTSTP is issued at a terminal to stop the process group currently running in the foreground.

     // ============================================================================================
     /*
          // Citation for the following function:
          // Date: 11/01/2021
          // Copied from /OR/ Adapted from /OR/ Based on:
          // Source URL: https://oregonstate.instructure.com/courses/1830250/pages/exploration-process-api-executing-a-new-program?module_item_id=21468874
          // Source URL: https://www.geeksforgeeks.org/difference-between-null-pointer-null-character-0-and-0-in-c-with-examples/
     */
     // ============================================================================================
     // Store using input in the array userInput
     i = 0;
     while (MAX_ARGS > i)
     {
          userInput[i] = NULL;
          fflush(stdout);
          i++;
     }

     do
     {
          // Get the input for the user
          getLine(userInput, &backgroundJob, sourceFD, targetFD, pid);

          // Any line that begins with the # character is a comment line and will be ignored

          // Allow for comments
          if (userInput[0][0] == '#')
          {
               fflush(stdout);
               continue;
          }

          // 4. Built-in Commands

          // Built-in command exit is used to exit the program/shell
          else if (strcmp(userInput[0], "exit") == 0)
          {
               fflush(stdout);
               isShellRunning = 0; // False
          }

          else if (userInput[0][0] == '\0') // ‘\0’ is defined to be a null character. It is a character with all bits set to zero.
          {
               fflush(stdout);
               continue;
          }

          // =======================================================================

          // Built-in command cd is used to change the directory
          else if (strcmp(userInput[0], "cd") == 0)
          {
               if (userInput[1]) // User will enter directory name and navigate to that directory
               {
                    if (chdir(userInput[1]) == -1)
                    {
                         printf("Directory not found.\n");
                         fflush(stdout);
                    }
               }
               else
               {
                    chdir(getenv("HOME")); // Then environment variable "Home" will navigate to the root directory
               }
          }

          // =======================================================================

          else if (strcmp(userInput[0], "status") == 0)
          {
               fflush(stdout);
               displayExitStatus(childExitMethod);
          }

          else
          {
               execFunction(userInput, &childExitMethod, &backgroundJob, sourceFD, targetFD, SIGINT_action);
          }
          clearInput(userInput);

          backgroundJob = 0;
          sourceFD[0] = '\0';
          targetFD[0] = '\0';

     } while (isShellRunning);

     return 0;
}
// ============================================================================================
/*
     // Citation for the following function:  getLine(char *charArray[], int *backgroundJob, char sourceFD[], char targetFD[], int pid)
     // Date: 11/01/2021
     // Copied from /OR/ Adapted from /OR/ Based on:
     // Source URL: https://www.tutorialspoint.com/c_standard_library/c_function_strtok.htm
     // Source URL: https://en.cppreference.com/w/c/experimental/dynamic/strdup
     // Source URL: https://www.educative.io/edpresso/splitting-a-string-using-strtok-in-c
     // Source URL:
     // Source URL:
*/
// ============================================================================================
void getLine(char *charArray[], int *backgroundJob, char sourceFD[], char targetFD[], int pid)
{

     char userInput[MAX_LENGTH_INPUT];
     int i;
     int j;
     char compareString[2] = "";
     // Get userInput from the user.
     printf(": ");
     fflush(stdout);
     fgets(userInput, MAX_LENGTH_INPUT, stdin);

     i = 0;
     int isLine = 0;
     while (!isLine && i < MAX_LENGTH_INPUT)
     {
          if (userInput[i] == '\n')
          {
               userInput[i] = '\0';
               fflush(stdout);
               isLine = 1;
          }
          i++;
     }

     if (!strcmp(userInput, compareString)) // The strcmp() compares two strings character by character. If the strings are equal, the function returns 0.
     {
          charArray[0] = strdup(compareString); // Returns a pointer to a null-terminated byte string, which is a duplicate of the string pointed to by user
          return;
     }
     // ============================================================================================
     /*
          // Citation for the following function: getLine()
          // Date: 11/01/2021
          // Copied from /OR/ Adapted from /OR/ Based on:
          // Source URL: https://www.educative.io/edpresso/splitting-a-string-using-strtok-in-c
          // Source URL: https://www.includehelp.com/c-programs/snprintf-function-in-c-language-with-example.aspx
          // Source URL: https://www.codegrepper.com/code-examples/c/how+to+concatenate+two+characters+in+c+without+using+function
     */
     // ============================================================================================
     const char delimeter[2] = " ";
     char *symbol = strtok(userInput, delimeter); // The strtok() function is used in tokenizing a string based on a delimiter.
                                                  // loop through the string to extract token
     i = 0;
     while (symbol != NULL)

     {

          if (!strcmp(symbol, "&"))
          {
               *backgroundJob = 1;
          }
          else if (!strcmp(symbol, "<"))
          {
               symbol = strtok(NULL, delimeter);
               strcpy(sourceFD, symbol);
               fflush(stdout);
          }
          else if (!strcmp(symbol, ">"))
          {
               symbol = strtok(NULL, delimeter);
               strcpy(targetFD, symbol);
               fflush(stdout);
          }
          // ============================================================================================
          /*
               // Citation for the following function: getLine() --appending 2 characters ($$)
               // Date: 11/01/2021
               // Copied from /OR/ Adapted from /OR/ Based on:
               // Source URL: https://www.codegrepper.com/code-examples/c/how+to+concatenate+two+characters+in+c+without+using+function
          */
          // ============================================================================================

          else
          {
               charArray[i] = strdup(symbol);

               j = 0;
               while (charArray[i][j])
               {
                    if (charArray[i][j] == '$' && charArray[i][j + 1] == '$')
                    {
                         charArray[i][j] = '\0';
                         snprintf(charArray[i], MAX_PATH, "%s%d", charArray[i], pid);
                         fflush(stdout);
                     
                    }
                    j++;
               }
          }
          
          symbol = strtok(NULL, delimeter);

          i++;
     }
}

// ============================================================================================
/*
     // Citation for the following function: execFunction()
     // Date: 11/01/2021
     // Copied from /OR/ Adapted from /OR/ Based on:
     // Source URL: https://oregonstate.instructure.com/courses/1830250/pages/exploration-signal-handling-api?module_item_id=21468881
     // Source URL: https://oregonstate.instructure.com/courses/1830250/pages/exploration-processes-and-i-slash-o?module_item_id=21468882
     // Source URL: https://oregonstate.instructure.com/courses/1830250/pages/exploration-process-api-monitoring-child-processes?module_item_id=21468873
     // Source URL: https://support.sas.com/documentation/onlinedoc/sasc/doc/lr2/execvp.htm
*/
// ============================================================================================
void execFunction(char *charArray[], int *childExitStatus, int *backgroundJob, char sourceFD[], char targetFD[], struct sigaction SIGINT_action)
{
     char compareString[2] = "";
     int sourceFile;
     int targetFile;
     int result;
     pid_t spawnPid = -5;

     // Fork a new process
     spawnPid = fork();
     switch (spawnPid)
     {

     case -1:;
          perror("fork() failed!");
          exit(1);
          break;

     case 0:;
          // In the child process
          SIGINT_action.sa_handler = SIG_DFL;
          sigaction(SIGINT, &SIGINT_action, NULL);

          // Create input file
          if (strcmp(sourceFD, compareString))
          {
               sourceFile = open(sourceFD, O_RDONLY);
               if (sourceFile == -1)
               {
                    perror("source open()");
                    exit(1);
               }
               result = dup2(sourceFile, 0);
               if (result == -1)
               {
                    perror("source dup2()");
                    exit(2);
               }
               // this process or one of its child processes calls exec, the file descriptor fd will be closed in that process
               fcntl(sourceFile, F_SETFD, FD_CLOEXEC);
          }
          // ==================================================================================

          // Create output file
          if (strcmp(targetFD, compareString))
          {
               targetFile = open(targetFD, O_WRONLY | O_CREAT | O_TRUNC, 0644);
               if (targetFile == -1)
               {
                    perror("target open()");
                    exit(1);
               }
               result = dup2(targetFile, 1);
               if (result == -1)
               {
                    perror("target dup2()");
                    exit(2);
               }
               // this process or one of its child processes calls exec, the file descriptor fd will be closed in that process
               fcntl(targetFile, F_SETFD, FD_CLOEXEC);
          }

          // execvp replaces the calling process image with a new process image.
          if (execvp(charArray[0], (char *const *)charArray))
          {
               printf("%s: no such file or directory\n", charArray[0]);
               fflush(stdout);
               exit(2);
          }
          break;
          // ==================================================================================
          // In the parent process
     default:;
          backgroundProcess(backgroundJob, isBackGround, childExitStatus, spawnPid);
     }
}
// ============================================================================================

/*
     // Citation for the following function:
     // Date: 11/01/2021
     // Copied from /OR/ Adapted from /OR/ Based on:
     // Source URL: https://oregonstate.instructure.com/courses/1830250/pages/exploration-signal-handling-api?module_item_id=21468881
     // Source URL:
*/
// ============================================================================================

void SIGTSTP_catch(int signo)
{
     /*
           A signal interrupts the sequence of instructions that the process is executing.
           The instructions in the signal handler will be executed. Then, if the signal handler
           does not terminate the process, the sequence of instructions for the process will be resumed.
           */

     if (isBackGround == 0)
     {

          char *catch_message = "Exiting foreground-only mode\n";
          write(1, catch_message, 29);
          fflush(stdout);
          isBackGround = 1; // Set to true and display message
     }

     else
     {
          char *catch_message = "Entering foreground-only mode (& is now ignored)\n";
          write(1, catch_message, 49);
          fflush(stdout);
          isBackGround = 0; // Set to false and display message
     }
}
// ============================================================================================
/*
     // Citation for the following function: displayExitStatus(int)
     // Date: 11/01/2021
     // Copied from /OR/ Adapted from /OR/ Based on:
     // Source URL: https://oregonstate.instructure.com/courses/1830250/pages/exploration-processes-and-i-slash-o?module_item_id=21468882
     // Source URL:
*/
// ============================================================================================
void displayExitStatus(int childExitMethod)
{

     if (WIFEXITED(childExitMethod))
     {
          printf("exit value %d\n", WEXITSTATUS(childExitMethod));
          fflush(stdout);
     }
     else
     {
          printf("terminated by signal %d\n", WTERMSIG(childExitMethod));
          fflush(stdout);
     }
}
// ============================================================================================
/*
     // Citation for the following function: backgroundProcess()
     // Date: 11/01/2021
     // Copied from /OR/ Adapted from /OR/ Based on:
     // Source URL: https://oregonstate.instructure.com/courses/1830250/pages/exploration-signal-handling-api?module_item_id=21468881
     // Source URL: https://oregonstate.instructure.com/courses/1830250/pages/exploration-processes-and-i-slash-o?module_item_id=21468882
     // Source URL: https://oregonstate.instructure.com/courses/1830250/pages/exploration-process-api-monitoring-child-processes?module_item_id=21468873
*/
// ============================================================================================
void backgroundProcess(int *backgroundJob, int isBackGround, int *childExitStatus, int spawnPid)
{
     if (*backgroundJob && isBackGround)
     {

          // // WNOHANG specified. If the child hasn't terminated, waitpid will immediately return with value 0
          pid_t childPid = waitpid(spawnPid, childExitStatus, WNOHANG);
          printf("background pid is %d\n", spawnPid);
          fflush(stdout);
     }
     else
     {
          pid_t childPid = waitpid(spawnPid, childExitStatus, 0);
     }

     while ((spawnPid = waitpid(-1, childExitStatus, WNOHANG)) > 0)
     {
          printf("child %d terminated\n", spawnPid);
          displayExitStatus(*childExitStatus);
          fflush(stdout);
     }
     return;
}
void clearInput(char *userInput[])
{

     int i = 0;
     while (userInput[i])
     {
          userInput[i] = NULL;
          fflush(stdout);
          i++;
     }
     return;
}
