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
#include <sys/wait.h>

int main(int argc, char** argv){
  int i, listeningPort, socketfd, clientSocket,status;
  struct sockaddr_in server;
  
  if(argc < 2){ 
    fprintf(stderr, "Must include port number\n"); //if it isn't the right amount of arguments
    exit(1);
  }
  
  else{
    listeningPort = atoi(argv[1]); 
  }    //listening port is converted

  if((socketfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){ 
    fprintf(stderr,"create the socket\n");   //socket can't be created
    exit(1);
  }
  
  server.sin_family = AF_INET;
  server.sin_port = htons(listeningPort);
  server.sin_addr.s_addr = INADDR_ANY;
  
  if(bind(socketfd,(struct sockaddr *) &server, sizeof(server)) == -1){ 
    fprintf(stderr,"failed to bind\n"); //not able to bind together
    exit(1);
  }
  
  if(listen(socketfd, 5) == -1) {
  fprintf(stderr, "listen to call failed\n"); //call failed
  exit(1); 
  } 
  
  while(1){
    clientSocket = accept(socketfd, NULL, NULL);
    if(clientSocket == -1){
      fprintf(stderr,"accept call failed\n"); //failed
      exit(1);
    }
    
    int pid = fork();
    if(pid == -1){ 
      fprintf(stderr,"fork\n"); //process id has an error
    }
    else if(pid == 0){
      int toSend = htonl(0);
      
      if(send(clientSocket, &toSend, sizeof(toSend),0) == -1){
        fprintf(stderr, "client send failed\n"); //send is failed
      }
      int cNum;
      if(recv(clientSocket, &cNum, sizeof(cNum), 0) == -1){
        fprintf(stderr,"receiving cipher text end_d -1\n"); //recieve failed
      }
      else if(cNum == 0){
        fprintf(stderr, "receiving cipther text size 0\n");
      }
      int cLen = ntohl(cNum); 
      int kNum; //converted
      
      if(recv(clientSocket, &kNum, sizeof(kNum), 0) == -1){ 
        fprintf(stderr, "recieving key text size end_d -1\n"); //fails to receive size
      } 
      else if(kNum == 0){
        fprintf(stderr, "key text size of 0\n");
      }
      
      int kLen = ntohl(kNum); //converts
      char *cipherText = malloc(sizeof(char)* cLen); //allocated
      char buffer[1024];
      memset(cipherText, '\0', cLen);
      int len = 0;
      int rec;
      while(len < cLen){ //while but not recieved
        memset((char *)buffer, '\0', sizeof(buffer));
        rec = recv(clientSocket, &buffer, 1024, 0); 
        
        if(rec == -1){
          fprintf(stderr, "recieving cipther text file -1\n");
          break;
        }
        else if(rec == 0){
          if(len < cLen){ 
            break; //not enough
          }
        }
        else{
          strncat(cipherText, buffer, (rec-1));
        }
        len += (rec - 1); 
      }      //add to below len
      
      cipherText[cLen - 1] = '\0'; 
      
      char *keyText = malloc(sizeof(char) * kLen); 
      memset((char *)&buffer, '\0', sizeof(buffer)); //allocate
      memset(keyText, '\0', kLen);
      len = 0;
      
      while(len <= kLen){
        memset((char *) buffer, '\0', sizeof(buffer));
        rec = recv(clientSocket, &buffer, 1024, 0);
          if(rec == -1){ //failed
            fprintf(stderr, "receiivng key text file dec_d\n");
            break;
          }
          else if(rec == 0){ // end of data send
            break;
          }
          else{
            strncat(keyText,buffer, (rec-1)); //concat
          }
          len += (rec - 1);
      }
      
      keyText[kLen - 1] = '0';
      
      int cipherNum, keyNum, decNum;
      for(i = 0; i < cLen - 1; i++){
        if(cipherText[i] == ' '){ //space place
          cipherNum = 26;
        }
        else{
          cipherNum = cipherText[i] - 65; //letter 
        }
        
        if(keyText[i] == ' '){//place space
          keyNum = 26l;
        }
        else{
          keyNum= keyText[i] - 65;
        }
        
        decNum = cipherNum - keyNum;
        if( decNum < 0){ //if < 0 then + 27
          decNum += 27;
        }
        if(decNum == 26){
          cipherText[i] = ' ';
        }
        else{
          cipherText[i] = 'A' + (char)decNum;
        }
      }
        
      len = 0;
      while(len <= cLen){
        char plainSend[1024];
        strncpy(plainSend, &cipherText[len], 1023); //copy
        plainSend[1024] = '\0'; //term
          
        if(send(clientSocket, &plainSend, 1024, 0) == -1){
          fprintf(stderr, "sending decryption text\n");
        }
        len += 1023;
      }
        
      free(cipherText);//cipher text is free
    }
    else{
      close(clientSocket);
        
      do{
        waitpid(pid, &status, 0);
      } while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }
  }
  close(socketfd);
  return 0;
}