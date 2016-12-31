#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <netinet/in.h>

int main(int argc, char ** argv){
  
  int i, listeningPort, socketfd, clientSocket, status;

   if(argc < 2){  //arguments
  fprintf(stderr, "Error: You must include a port number\n");
  exit(1);
   }
   else{
    listeningPort = atoi(argv[1]); //try to recieve listening port
   }

   if((socketfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
  fprintf(stderr, "Error:with socket creation\n");
  exit(1);
   }

   struct sockaddr_in server;
   server.sin_family = AF_INET;
   server.sin_port = htons(listeningPort);
   server.sin_addr.s_addr = INADDR_ANY;

   if(bind(socketfd, (struct sockaddr *) &server, sizeof(server)) == -1){
  fprintf(stderr, "Error: bind call failed\n");
  exit(1);
   }
   
   if(listen(socketfd, 5) == -1){   //call listen error
  fprintf(stderr, "Error: listen call\n");
  exit(1);
   }
   
   while(1){ 
  clientSocket = accept(socketfd, NULL, NULL);
  if (clientSocket == -1){//if the accept fails
      fprintf(stderr, "Error: The accept call failed\n");
      exit(1);
  }
  
  int pid = fork(); //process gets forked
  if (pid == -1){//the fork process has an error
     fprintf(stderr, "fork error\n");
  }
  else if(pid == 0) {
     int toSend = htonl(1);
     if(send(clientSocket, &toSend, sizeof(toSend),0) == -1){ 
    fprintf(stderr, "Error: failed to send to client send\n"); //cannot send to client
    exit(1);
     }

     int pNum; //size of plain text
     if(recv(clientSocket, &pNum, sizeof(pNum), 0) == -1){//recieve error
    fprintf(stderr, "Error: receiving plain text size end_d is -1\n");
     }
     else if(pNum == 0){ //plain text is 0 
    fprintf(stderr, "Error: plain text size of 0\n");
     }

     int pLen = ntohl(pNum);
     int kNum;    //convert
     if(recv(clientSocket, &kNum, sizeof(kNum), 0) == -1){ //receive size has an error
    fprintf(stderr, "Error: receiving key text size end_d == -1\n");
     }
     else if(kNum == 0){ // = 0
    fprintf(stderr, "Error: key text size of 0\n");
     }
     int kLen = ntohl(kNum);//convert
       char *plainText = malloc(sizeof(char) * pLen); //allocation
       char buffer[1024];
       memset(plainText, '\0', pLen); //cleared 

     int len = 0;
     int rec;
     while(len <= pLen){//file not  sent
        memset((char *)buffer, '\0', sizeof(buffer));//clear buffer 
      rec = recv(clientSocket, &buffer, 1024, 0);//receive

        if(rec == -1){ //recieving data is causing an error
      fprintf(stderr, "Error:receiving plain text file == -1\n");
      break;
        }
        else if(rec == 0){
      if (len < pLen){
      break;
      }
        }
        else{
        strncat(plainText,buffer,(rec - 1)); 
        } 
        len += (rec-1);//total len is added
     }
     plainText[pLen - 1] = '\0'; 

       char *keyText = malloc(sizeof(char) * kLen); //keytext allocation
      
       memset((char *)buffer, '\0', sizeof(buffer)); //buffer
     memset(keyText, '\0', kLen);
     len = 0;

     while(len <= kLen){//not yet recieved
          memset((char *)buffer, '\0', sizeof(buffer)); 
      rec = recv(clientSocket, &buffer, 1024, 0); 

      if(rec == -1){ //data recieve is not working correctly
        fprintf(stderr, "Error: receiving key text file = -1\n");
        break;
      }
      else if(rec == 0){
           break; //end of data
      }
      else{
        strncat(keyText,buffer,(rec - 1)); 
       }
         
        len += (rec - 1);
     }
     keyText[kLen - 1] = '\0'; //add total len

     int plainNum;
     int keyNum;
     int encNum;
  
  
     for (i = 0; i < pLen - 1; i++){ //encrypt
    if(plainText[i] == ' ') {
      plainNum = 26; 
    }
    else{//letter
       plainNum = plainText[i] - 65; //spacing 
    }
    if(keyText[i] == ' ') {
       keyNum = 26;
    }
    else {
       keyNum = keyText[i] - 65;
    }
  
    encNum = plainNum + keyNum;
    if (encNum >= 27){
       encNum -= 27;
    }
    if(encNum == 26){ 
       plainText[i] = ' ';
    }
    else{
       plainText[i] = 'A' + (char)encNum;
    }
     }

    
     len = 0;
     while (len <= pLen) { //not complete sent
    char cipherSend[1024];
    strncpy(cipherSend, &plainText[len], 1023); 
    cipherSend[1024] = '\0'; 

      if(send(clientSocket, &cipherSend, 1024, 0) == -1){ //sending an error
        fprintf(stderr, "Error: sending encryption text\n");
    }
    len += 1023; //total len
     }                //keytext and plaintext free
     free(plainText);
     free(keyText);
  }      
  else{
     close(clientSocket); 
     do{
    waitpid(pid, &status, 0);
     }while(!WIFEXITED(status) && !WIFSIGNALED(status));   //close all connections
  }
   }
   close(socketfd); 
   return 0;
}