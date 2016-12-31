/*************************
Aviral Sinha
CS 344
Program 2: Adventure
******************/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <dirent.h>
#include <assert.h>
#include <pthread.h>

#define ROOM_NAMES 10
#define MIN_CONNECT 3
#define MAX_CONNECT 6
#define USED_ROOMS 7


struct RM_Types //create room types
{
   char * start;
   char * middle;
   char * end;
};

void intro();
char * makeroomdir(int);
struct RM_Types makerooms(char *);
void adventure(struct RM_Types);
void randomize(char **, size_t);
void * getTime();

int main(void)
{
   
   srand(time(NULL)); //Random number generator 

   
   intro();

  int pid = getpid(); //Process ID to be used with creation of directory
 
  char * room_dir = makeroomdir(pid); //create directory for rooms
  struct RM_Types user_rooms = makerooms(room_dir);

  adventure(user_rooms); //Loop game until user wins

  free(room_dir);

   return 0;
}

void intro()
{
   printf("Welcome to the World of Adventure through Premier League Teams\n\n");
   printf("Find the team in the fewest number of steps\n\n");
}

char * makeroomdir(int pid)
{

   int buffer = 20;  //Set aside new directroy to verify
   char * new_dir = malloc(buffer);
   assert(new_dir != 0);

   char * username = "sinhaav.rooms.";  // name directory

   snprintf(new_dir, buffer, "%s%d", username, pid); //stringify named pieces 

   
   struct stat dir = {0};

   if (stat(new_dir, &dir) == -1)
   {
      mkdir(new_dir, 0755); // make directory 
   }

   return new_dir;  

}
struct RM_Types makerooms(char * newroom)
{
	char * roomNames[ROOM_NAMES]; //all the room names
	roomNames[0] = "Manchester";
	roomNames[1] = "Arsenal";
	roomNames[2] = "Tottenham";
	roomNames[3] = "Liverpool";
	roomNames[4] = "Chelsea";
	roomNames[5] = "Everton";
	roomNames[6] = "Leeds";
	roomNames[7] = "Southampton";
	roomNames[8] = "Leicester";
	roomNames[9] = "Watford";



   struct RM_Types rms;   //structure for rooms
   rms.middle = newroom;

   int buffer = 128;  //initialize and verify
   char * file = malloc(buffer);
   assert(file != 0);
  
   
   randomize(roomNames, ROOM_NAMES);  //game randomness
   
//Make Files
   int i;
   for (i = 0; i < USED_ROOMS; i++)
   {
      snprintf(file, buffer, "%s/%s", newroom, roomNames[i]);
      FILE * input = fopen(file, "w"); // write to file
      if (input == NULL)
      {
         perror("File couldn't be opened\n");
         exit(1);
      }
      else 
      {
         fprintf(input, "Team Name: %s\n", roomNames[i]); 
      }
      fclose(input);
   }
   
   char * saved[USED_ROOMS]; //Fill an array with rooms

   for (i = 0; i < USED_ROOMS; i++)
   {
      saved[i] = roomNames[i];
   } 

   int begin_room = rand() % USED_ROOMS; //Randomize the starting and ending point
   int end_room = rand() % USED_ROOMS;

   while (begin_room == end_room)  //If random is same as before then start from beginning
   {
      end_room = rand() % USED_ROOMS;
   } 


   char * connect; // track  connections
   int counter; // count connections
   int j, k; // loops

   for (i = 0; i < USED_ROOMS; i++)
   {
      randomize(saved, USED_ROOMS); // Change up the possible connections to go to
      snprintf(file, buffer, "%s/%s", newroom, roomNames[i]);

      FILE * input = fopen(file, "a"); // append to the room file
      if (input == NULL)
      {
         perror("File couldn't be opened\n");
         exit(1);
      }
      else
      {
         // Connections randomized based on room specifications
         counter = rand() % 4 + 3;

         k = 0;    //Looping for room connections
         for (j = 0; j < counter; j++)
         {
            connect = saved[k];
            if (connect == roomNames[i]) // don't allow room to go back to itself
            {
               k++;
               connect = saved[k];
            }
            fprintf(input, "Connection %d: %s\n", j+1, connect);
            k++;
         }
        
         if (i == begin_room)   //check to see if room is start or end, then store as a file
         {
            fprintf(input, "Room Type: START_ROOM\n");
            rms.start = roomNames[i];
         }
         else if (i == end_room)
         {
            fprintf(input, "Room Type: END_ROOM\n");
            rms.end = roomNames[i];
         }
         else  
         {
            fprintf(input, "Room Type: MID_ROOM\n"); //everything is considered as midroom 
         }
      }
      fclose(input);

   }
   free(file);

   return rms;
}    


/*********
Adventure
*********/
void adventure(struct RM_Types room_in)
{
//initiate the beginning 
   char * firstroom = room_in.start;
   char * middleroom = room_in.middle;
   char * lastroom = room_in.end;

   int counter = 0;
   int flag; 
   int buffer = 128;
   int i;
   int tim;

   char (*steps)[15] = malloc(sizeof *steps * 8); //Memory alocation through room access, file storage and the path it takes
   assert(steps != 0);

   char (*contents)[15] = malloc(sizeof *contents * 8);
   assert(contents != 0);

   char user_choice[15]; // Where should it go

   char * file = malloc(buffer);
   assert(file != 0);

   int lines; //loop til the current room equals the end room
   while (!(strcmp(firstroom, lastroom)) == 0)
   {
      snprintf(file, buffer, "%s/%s", middleroom, firstroom); 
      FILE * input = fopen(file, "r"); // read

      int room_connects = 0;  //track the connections between rooms

      if (input)
      {
         while ((lines = getc(input)) != EOF)
         {
            if (lines == '\n')
            {
               room_connects++;
            }
         }
      }  
      room_connects-= 2; 

      char str[20];  //File Navigation
      fseek(input, 11, SEEK_SET);
      fgets(str, 20, input);
   
      int length = strlen(str); //remove newlines
      if (str[length - 1] == '\n')
      {
         str[length - 1] = 0;
      }
      strcpy(contents[0], str);
      
      for (i = 1; i <= room_connects; i++) //check if there are any connections and store it
      {
         fseek(input, 14, SEEK_CUR);
         fgets(str, 20, input);
         
         // Remove newlines
         length = strlen(str);
         if (str[length - 1] == '\n')
         {
            str[length - 1] = 0;
         }
         strcpy(contents[i], str);
      }   

      flag = 0;   //validate and compare the contents then print it for the user
      while (flag != 1)
      {
         printf("Current Location: %s\n", contents[0]); //status update
         printf("Possible Connections: ");
   
         for (i = 1; i <= room_connects; i++)
         {
            if (i == room_connects)
            {
               printf("%s.\n", contents[i]);
            }
            else
            {
               printf("%s, ", contents[i]);
            }
         }

         
         printf("Where to?>"); //user choice
         scanf("%s", user_choice);
         tim = 0;
            //time keeping
         if(strcmp(user_choice, "time") == 0){
            //time.txt
            printf("\n");
            tim = 1;
            //pthread_t time_thread;
            //pthread_create(&time_thread, NULL, (void *)getTime, NULL);

            pthread_t time_thread; //for second thread
            int result;
            
            pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;
            pthread_mutex_lock(&myMutex);//lock
            result = pthread_create(&time_thread, NULL, &getTime, NULL);
            pthread_join(time_thread, NULL);
            pthread_mutex_unlock(&myMutex);//unlock
         }
         for (i = 1; i <= room_connects; i++)
         {
            if (strcmp(user_choice, contents[i]) == 0)
            {
               flag = 1;
               firstroom = user_choice;
            }
         }
         if (flag != 1 && tim !=1) //print error message and have user re-enter 
         {
            printf("\nThat isn't a destination, please try again\n\n");
         }
      }

      printf("\n");
      strcpy(steps[counter], firstroom); // store the visited rooms
      counter++; 
      fclose(input);
   }

   printf("You've reached the final destination! \n"); //end on the selected room
   printf("You had to go through %d destinations. The path was:\n", counter);
   for (i = 0; i < counter; i++)
   {
      printf("%s\n", steps[i]);
   }

   free(steps);
   free(contents);
   free(file);

}
/***********
Randomize
************/
void randomize(char **list_in, size_t num)
{
   struct timeval tv;  

   gettimeofday(&tv, NULL);

   int usec = tv.tv_usec;
   srand48(usec);

   if (num > 1) 
   {
      size_t i;
      for (i = num - 1; i > 0; i--)
      {
         size_t j = (unsigned int) (drand48()*(i+1));
         char * t = list_in[j];
         list_in[j] = list_in[i];
         list_in[i] = t;
      }
   }
}

void * getTime(){      //show time on screen
   FILE *fp = fopen("time.txt", "w+");  //write into text file
   struct timeval usec_time; 
   time_t now = time(0);
   fprintf(fp, ctime(&now)); //time written to text file
   fclose(fp);
   
   
   int c;
   FILE *file;
   file = fopen("time.txt", "r"); //open and read file only
   printf("\n");
   if (file) {
      while ((c = getc(file)) != EOF) //while loop for end of file
         putchar(c); 
      fclose(file);
   }
}
   
