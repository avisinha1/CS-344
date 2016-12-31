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

int main(int argc, char **argv) {
   int i;

   if (argc < 4){ //dont allow too many arguments
  fprintf(stderr, "Error: Please specifiy plaintext, key, and port number\n");
  exit(1);
   }

   int portNum = atoi(argv[3]); //retrieve and convert the port
   int fdPlain = open(argv[1], O_RDONLY); //read
   int fdKey = open(argv[2], O_RDONLY);

   if(fdPlain == -1 || fdKey == -1){ //error when opening
  fprintf(stderr, "Error: opening files\n");
  exit(1);
   }
//key text length and plain text length
   int pLen = lseek(fdPlain, 0, SEEK_END); 
   int kLen = lseek(fdKey, 0, SEEK_END); 

   if(kLen < pLen){ //see if key is smaller
  fprintf(stderr, "Error: Key too small\n");
  exit(1);
   }

   char *plainText = malloc(sizeof(char) * pLen); //allocate
   lseek(fdPlain, 0, SEEK_SET); //set to the start of the file

   if(read(fdPlain, plainText, pLen) == -1){   //text read
  fprintf(stderr, "Error: reading plain text enc\n");
  exit(1);
   }
   
   plainText[pLen - 1] = '\0';

  
   for(i = 0; i < pLen - 1; i++){ //see if correct
  if(isalpha(plainText[i]) || isspace(plainText[i])){ //space or letter 
    
  }
  else{ //error
     fprintf(stderr, "Error: A Plain text has an invalid char\n");
     exit(1);
  }
   }

   char *keyText = malloc(sizeof(char) * kLen); //allocate to hold the text
   lseek(fdKey, 0, SEEK_SET); 

   if(read(fdKey, keyText, kLen) == -1){
  fprintf(stderr, "Error:reading key text enc\n");
  exit(1);
   }
   keyText[kLen - 1] = '\0'; 
 
   for(i = 0; i < kLen - 1; i++){
  if(isalpha(keyText[i]) || isspace(keyText[i])) {
   
  }
  else { //error
     fprintf(stderr, "Error:Key text invalid char\n");
     exit(1);
  }
   }
   
   int socketfd;

   if((socketfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){//creation error
  fprintf(stderr, "error: creating socket error\n");
  exit(1);
   }

   struct hostent * server_ip_address;
   server_ip_address = gethostbyname("localhost");

   if(server_ip_address == NULL){
  fprintf(stderr, "error: with host name\n");
  exit(1);
   }
   
   struct sockaddr_in server;
   memset((char *)&server, 0, sizeof(server));
   server.sin_family = AF_INET;
   server.sin_port = htons(portNum);
   memcpy(&server.sin_addr, server_ip_address->h_addr, server_ip_address->h_length);
  
   if(connect(socketfd, (struct sockaddr*) &server,sizeof(server)) == -1){ //socket connection
  fprintf(stderr, "Error: connecting \n");
  exit(2);
   }

   int rec;
   int confNum;
  
   if((rec = recv(socketfd, &confNum, sizeof(confNum), 0)) == -1){ //error when recieving 
  fprintf(stderr, "Error: receiving enc\n");
  exit(1);
   } 
   else if(rec == 0){
  fprintf(stderr, "Error: receiving enc is 0\n");
  exit(1);
   }

   int confirm = ntohl(confNum);
   if(confirm != 1){ //wrong confirmation
  fprintf(stderr, "Error: could not contact otp_enc_d on port %d\n", portNum);
  exit(2);
   }
   
   int pLenSend = htonl(pLen); //convert

   if(send(socketfd, &pLenSend, sizeof(pLenSend), 0) == -1){
  fprintf(stderr, "Error: sending plain text file\n");
  exit(1);
   }

   int kLenSend = htonl(kLen); //convert

   if(send(socketfd, &kLenSend, sizeof(kLenSend), 0) == -1){
  fprintf(stderr, "Error: sending key text file\n");
  exit(1);
   }

   int len = 0;
   while(len <= pLen){ //file still hasn't been sent
  char plainSend[1024];
  strncpy(plainSend, &plainText[len], 1023);//copy to text send
  plainSend[1024] = '\0';//term
   
  if(send(socketfd, &plainSend, 1024, 0) == -1){ //fail to send
     fprintf(stderr, "Error: sending plain text\n");
     exit(1);
  }
  len += 1023;//total length addition
   }

   
   len = 0;
   while (len <= kLen) {//whole key not sent
  char keySend[1024];
  strncpy(keySend, &keyText[len], 1023); //copied
  keySend[1024] = '\0'; 

    if(send(socketfd, &keySend, 1024, 0) == -1){
    fprintf(stderr, "Error: sending key text\n");
    exit(1);
    }
  len += 1023; //total len
   }

   char *cipherText = malloc(sizeof(char) * pLen); //memory allocated for cipher
   char buffer[1042]; 
   memset(cipherText, '\0', pLen); //cipher text
   len = 0;
   rec = 0;
   
   while(len < pLen) { //whole file isn't recieved 
    memset((char *)buffer, '\0', sizeof(buffer));
    rec = recv(socketfd, buffer, 1024, 0);
        if(rec == -1){
      fprintf(stderr, "Error: receiving cipher text file == -1\n");
      exit(1);
    }       
    else if(rec == 0){
      if(len < pLen){
        fprintf(stderr, "Error: receiving cipher text file <\n");
        exit(1);
      }
    }
    else {
      strncat(cipherText,buffer,(rec-1)); //string concat
    }    
    len += (rec-1); //Add total
   }

   cipherText[pLen - 1] = '\0';

   printf("%s\n", cipherText); //print

   
   free(plainText);
   free(keyText);
   free(cipherText);

   return 0;
}