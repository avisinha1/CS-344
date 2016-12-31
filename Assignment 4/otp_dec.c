#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>

int main(int argc, char **argv){
  
  if (argc < 4){ //if there are more than 4 arguments
    fprintf(stderr, "incorrect # of arguments,  specify");
    exit(1);
  }
  
  int portNum = atoi(argv[3]); //portnum = conversation
  int fdCipher = open(argv[1], O_RDONLY); //ciphertext is open
  int fdKey = open(argv[2], O_RDONLY); //key open
  
  if (fdCipher == -1 || fdKey == -1){ //double check if opened
  fprintf(stderr, "couldn't open the files\n");
  exit(1);
  }
  
  
  int cLen = lseek(fdCipher, 0, SEEK_END); //length of cipher
  int kLen = lseek(fdKey, 0, SEEK_END);//length of key
  
  if (kLen < cLen) { //if key > cipher
  fprintf(stderr, "Key is shorter than Cipher\n");
  exit(1);
    }
  
  char *cText = malloc(sizeof(char)* cLen); //cipher is held
  lseek(fdCipher, 0, SEEK_SET); //filelength start
  
  if(read(fdCipher, cText, cLen) == -1){
    fprintf(stderr, "Reading cipher text dec\n");
    exit(1);
  }
  
  cText[cLen] = '\0';
  
  int i;
  for(i = 0; i < cLen; i++){
    if(isalpha(cText[i]) || isspace(cText[i]) || ispunct(cText[i])){ //if letter and spacing does nothing
    
    }
    else{
      fprintf(stderr, "Cipher text has a invalid character\n");
      exit(1);
    }
  }
  
  char *kText = malloc(sizeof(char)* kLen);
  lseek(fdKey, 0, SEEK_SET);
  
  if(read(fdKey, kText, kLen) == -1){
    fprintf(stderr, "Reading key text\n");
    exit(1);
  }
  
  int socketfd;
  if((socketfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
    fprintf(stderr, "socket error\n");
    exit(1);
  }
  
   struct hostent * server_ip_address; //creating struct for the address 
   server_ip_address = gethostbyname("localhost"); 
   
   if(server_ip_address == NULL) { //empty address
  fprintf(stderr, "host name\n");
  exit(1);
   }
   
   struct sockaddr_in server;
   memset((char *)&server, 0, sizeof(server)); 
   server.sin_family = AF_INET;
   server.sin_port = htons(portNum);
   memcpy(&server.sin_addr, server_ip_address->h_addr, server_ip_address->h_length);
   
   if(connect(socketfd, (struct sockaddr*) &server, sizeof(server)) == -1){
  fprintf(stderr, "cant connecting\n");
  exit(2);
   }
   
   int rec;
   int confirmationNum;
   if((rec = recv(socketfd, &confirmationNum, sizeof(confirmationNum), 0)) == -1){
  fprintf(stderr, "not able to receiving enc\n");
  exit(1);
   } 
   else if(rec == 0){
  fprintf(stderr, "not able to receive enc is equal to 0\n");
  exit(1);
   }
   
   int conf = ntohl(confirmationNum); // = confirmation number

   if (conf!= 0){ //if its wrong # 
  fprintf(stderr, " Error: wrong confrim number, could not get otp_dec_d on port %d\n", portNum);
  exit(2);
   }
   
                                
   int cLenSend = htonl(cLen); //converts to unsigned integer from host byte to network byte

   if(send(socketfd, &cLenSend, sizeof(cLenSend), 0) == -1) { 
  fprintf(stderr, "Error: sending cipher text file\n");
  exit(1);
   }

   int kLenSend = htonl(kLen); 

   if(send(socketfd, &kLenSend, sizeof(kLenSend), 0) == -1) { //key not sent properly
  fprintf(stderr, "Error: sending key text file\n");
  exit(1);
   }
   
   
   int lengt = 0;
   while (lengt <= cLen) {
  char cipherSend[1024];
  strncpy(cipherSend, &cText[lengt], 1023); //copy of cipher to send
  cipherSend[1024] = '\0'; //term

    if(send(socketfd, cipherSend, 1024, 0) == -1){ //not sent properly
     printf("Error: sending cipher text!!\n");
     exit(1);
  }
  lengt += 1023; //Add sent
   }
   
   lengt = 0; //go back to 0
   
    
   while (lengt <= kLen) { 
  char keySend[1024];
  strncpy(keySend, &kText[lengt], 1023);
  keySend[1024] = '\0';

    if(send(socketfd, &keySend, 1024, 0) == -1){
     fprintf(stderr, "Error: sent key text\n");
     exit(1);
  }
  lengt += 1023; //add len sent to len
   }
   
   char *plainText = malloc(sizeof(char) * cLen); //allocate memory 
   char buffer[1024]; //buffer created
   memset(plainText, '\0', cLen); 
   lengt = 0;
   rec = 0;
   while(lengt < cLen){ 
    memset((char *)buffer, '\0', sizeof(buffer));
    rec = recv(socketfd, &buffer, 1024, 0); 
    
      if( rec == -1){
        fprintf(stderr, "Error receiving plain text file data\n");
        exit(1);
      }
      else if(rec == 0){
        if( lengt < cLen){ 
          fprintf(stderr, "Error receiving plain text file \n");
          exit(1);
        }
      }
      else{
        strncat(plainText, buffer, (rec - 1));
      }
    lengt += (rec-1);
   }
   
   plainText[cLen - 1] = '\0';
   printf("%s\n", plainText);
   free(plainText); //free plain text 
   free(kText); //free Key text 
   free(cText); // free cipherText 

   return 0;
}