
/*********************
Aviral Sinha
CS 344
Program 3: smallsh.c
Assignment Details: Write a shell in C that will run command 
line instructions and return results similar to previously used shells. 
Shell will support exit, cd and status and comments on lines using # in the beginning.
**********************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_CHARS 2048       //max of 2048 characters in a line
#define MAX_ARGS 512         //max of 512 arguments
#define TOK_DELIM " \t\r\n\a" //strtok delimiters


struct CmdFlags {   //holds various status flags for the input
   char *inputFile;     //name of input file
   char *outputFile;    //name of output file
   int redirInput;      //flag if input is redirected   
   int redirOutput;     //flag if output is redirected
   int argc;            //keep count of arguments limited
   int background;      //see if it can be run in the background
};

char *LineRead();
char **LineSplit(char *line, struct CmdFlags *cmd);
void launchcontrol(char **args, struct CmdFlags *cmd);
void Processfg(pid_t pid, char *exitStatus);
void death(); //remove and kill zombies


int main(int argc, char **argv)
{
   int i;            //controls loop
   int exitShell = 0;   //controls shell loop
   pid_t processes[100];   //list of all open processes
   int processCount = 0;
   char exitStatus[MAX_CHARS] = "Last fg process not detected"; //exit status of last fg process
     
   struct sigaction defaultAction, ignoreAction;
      

  
   defaultAction.sa_handler = SIG_DFL;
   ignoreAction.sa_handler = SIG_IGN;
   
   sigaction(SIGINT, &ignoreAction, NULL);

   do
   {
      char *line = NULL;     //raw command
      char **args;           //parsed command line
      struct CmdFlags *cmd = malloc(sizeof(struct CmdFlags)); //manage command flags 
        
      cmd->redirInput = 0; 
      cmd->redirOutput = 0;   
      cmd->argc = 0;       // reset argument count
      cmd->background = 0; // reset background flag

      
      death(); 

      printf(": ");        //prompt the shell

     
      line = LineRead();            //read the input
      args = LineSplit(line, cmd);  //parse it into arguments and get the information

      if (cmd->argc == 0 || args[0] == NULL) //check if it is a blank line
      {
         exitShell = 0;
      }

      else if (strncmp(args[0], "#", 1) == 0) //check if it is a comment line 
      {
         exitShell = 0;  //reprompt
      }

      else if (strcmp(args[0], "exit") == 0) //built in command to exit
      {
         if (cmd->argc > 2)  //look for any outlying arguments
         {
            printf("exit: unexpected arguments\n");
            fflush(stdout);
            
         }
         else if (args[1])
         {
            printf("exit: unexpected argument '%s'\n", args[1]);
            fflush(stdout);
            
         }
         else
         {
            exitShell = 1;
            

            printf("killing the background processes\n");
            fflush(stdout);
            for (i = 0; i < processCount; i++)
            {
               kill(processes[processCount], SIGTERM);
            }
         }
      }

      else if (strcmp(args[0], "cd") == 0) //built in command for cd
      {
         if (cmd->argc > 2)
         {
            fprintf(stderr, "cd: unexpected arguments\n");
           
         }
         
         if (args[1])
         {
            if (chdir(args[1]) != 0) //unsuccessful chdir
            {
               fprintf(stderr, "%s: no file or directory found\n",
                  args[1]);
               
            }
         }
         else
         {
            chdir(getenv("HOME"));
           
         }
      }

      else if (strcmp(args[0], "status") == 0) //built in command for status
      {
         fprintf(stdout, "%s\n", exitStatus);
         fflush(stdout);
         
      }

      else //command is excuted if all else
      {
         pid_t cpid;
         cpid = fork();

         if (cpid == 0)         //child process 
         {
            if (!cmd->background)   
            {
               sigaction(SIGINT, &defaultAction, NULL); //foreground processes can be interrupted with signals
            }

            launchcontrol(args, cmd);
         }
         else if (cpid == -1)    //not able to fork
         {
            perror("fork");
            exit(EXIT_FAILURE);
         }
         else                 //parent process
         {
            if (cmd->background) 
            {                                    //background
               processes[processCount] = cpid;
               processCount++;
               printf("background pid is %d\n", cpid);
               fflush(stdout);
            }
            else                                //foreground
            {
               Processfg(cpid, exitStatus);
            }
         }
      }

      free(line);   //memory deallocated
      free(args);
      free(cmd);
   }
   while (!exitShell);

   return 0;
}


char *LineRead() //reads a line set to the max size specified above in MAX_CHARS and then returns the input line
{
   char *line = NULL;
   ssize_t buffer = MAX_CHARS;
   getline(&line, &buffer, stdin);

   return line;
}


char **LineSplit(char *line, struct CmdFlags *cmd) //takes the input line and command flags struct and parses into the input line. Decides if the input specifies either input or output redirection. Returns a parsed command in an array of character arrays
{
   int bufferSize = MAX_ARGS;
   int readStatus = 1;  //1 = continue, 0 = stop, 2 = input, 3 = output
   char **tokens = malloc(bufferSize * sizeof(char *));
   char *token;

   cmd->redirInput = 0;
   cmd->redirOutput = 0;
   cmd->argc = 0;
   cmd->background = 0;

   if (!tokens)
   {
      fprintf(stderr, "smallsh: allocation error\n");
      exit(EXIT_FAILURE);
   }

   token = strtok(line, TOK_DELIM); //tokenize string
   while (token != NULL && readStatus != 0)
   {
      if (strcmp(token, "&") == 0)     //background process 
      {
         cmd->background = 1;
         readStatus = 0;      //end of command
      }
      else if (strcmp(token, "<") == 0)  //input redirect
      {
         cmd->redirInput = 1;
         readStatus = 2;     
      }
      else if (strcmp(token, ">") == 0)   //output redirect
      {
         cmd->redirOutput = 1;
         readStatus = 3;      //next argument read in as a output file
      }
      else  
      {
         if (readStatus == 2)       //last argument was a <
         {
            cmd->inputFile = token;
         }
         else if (readStatus == 3)    //last argument was a >
         {
            cmd->outputFile = token;
         }
         else
         {
            tokens[cmd->argc] = token;
            cmd->argc++;
            tokens[cmd->argc] = NULL;  //terminates list of arguments
         }
      }

      token = strtok(NULL, TOK_DELIM); 
   }

  

   return tokens;
}


void launchcontrol(char **args, struct CmdFlags *cmd) //executes the command line with additional info from command flags. Redirects any input or output if needed
{
   int inputfd, outputfd;  //descriptors for the file
   char fileName[MAX_CHARS] = "";

  
   if (cmd->redirInput) //input redirect
   {
      
      inputfd = open(cmd->inputFile, O_RDONLY); // new input file to redirect is opened
      if (inputfd == -1)
      {
         fprintf(stderr, "smallsh: cant open %s for input\n",
            cmd->inputFile);
         exit(EXIT_FAILURE);
      }
      
      if (dup2(inputfd, 0) == -1) //redirect stdin so fd is 0 points 
      {
         fprintf(stderr, "smallsh: cant open %s for input\n",
            cmd->inputFile);
         exit(EXIT_FAILURE);
      }
   }
   else if (cmd->background)  //redirect, if no input file is specified
   {
      inputfd = open("/dev/null", O_RDONLY); 
      if (inputfd == -1)
      {
         fprintf(stderr, "smallsh: cant open /dev/null for input\n");
         exit(EXIT_FAILURE);
      }
      if (dup2(inputfd, 0) == -1)
      {
         fprintf(stderr, "smallsh: cant open /dev/null for input\n");
         exit(EXIT_FAILURE);
      }
   }

   if (cmd->redirOutput) //output redirect
   {
      outputfd = open(cmd->outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
      if (outputfd == -1) //new output file to redirect
      {
         fprintf(stderr, "smallsh: cant open %s for the output\n",
            cmd->outputFile);
         exit(EXIT_FAILURE);
      }
      if (dup2(outputfd, 1) == -1)
      {
         fprintf(stderr, "smallsh: cant open %s for the output\n", 
            cmd->outputFile);
         exit(EXIT_FAILURE);
      }
   }

   execvp(args[0], args);

   fprintf(stderr, "%s: no file or directory found\n", args[0]); //error if command is not valid
   exit(EXIT_FAILURE);
}


void Processfg(pid_t pid, char *exitStatus) //handles the status of all foreground processes, then sets the exit status for the foreground process.
{
   int status;

   waitpid(pid, &status, 0);

   if (WIFEXITED(status))
   {
      sprintf(exitStatus, "exit value %d", WEXITSTATUS(status));
   }
   if (WIFSIGNALED(status))
   {
      sprintf(exitStatus, "terminated by signal %d", WTERMSIG(status));
      printf("%s\n", exitStatus);
      fflush(stdout);
   }
}


void death() //handles the status of any background process, sets the exit status for all the background process.
{
   pid_t cpid;
   int status;

   cpid = waitpid(-1, &status, WNOHANG);
   if (cpid > 0)
   {
      if (WIFEXITED(status))
      {
         printf("background pid %d is done: exit value %d\n", cpid,
            WEXITSTATUS(status));
         fflush(stdout);
      }
      else if (WIFSIGNALED(status))
      {
         printf("background pid %d is done: terminated by signal %d\n",
            cpid, WTERMSIG(status));
         fflush(stdout);
      }
   }
}

