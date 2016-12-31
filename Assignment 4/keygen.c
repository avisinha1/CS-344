#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

void GetRandomizedString(char *stringHolder, int stringLength){
  static const char possibleChars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ "; //characters that can be used

  int i;
  for (i = 0; i < stringLength; i++){ 
      stringHolder[i] = possibleChars[rand() % (sizeof(possibleChars) - 1)]; //random character goes into each holder
  }
  stringHolder[stringLength] = 0;
}

int main(int argc, char *argv[]){
  
  srandom(time(NULL));

  char *randomizedString;

  
  if (argc != 2){ 
    printf("keygen keylength\n");   //incorrect number of each argument
    exit(1);
  }

  if (argc == 2){ 
    int randomStringLength;
    randomStringLength = atoi(argv[1]);
    randomizedString = (char*)malloc(sizeof(char)*(randomStringLength+1)); //the right amount of arguments

    GetRandomizedString(randomizedString, randomStringLength);
    printf("%s\n", randomizedString);
  }


  free(randomizedString);  //string is free

  return 0;  
}
