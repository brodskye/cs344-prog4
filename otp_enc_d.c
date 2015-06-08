/*
Becky Solomon
encrypt server
otp_enc_d.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg){
    perror(msg);
    exit(1);
}

int charToInt (char c){
	if (c == ' '){
		return 26;
	}
	else {
		return (c - 'A');
	}
	return 0;
}

char intToChar(int i){
	if (i == 26){
		return ' ';
	}
	else {
		return (i + 'A');
	}
}

void encrypt(char message[], char key[]){
	  int i;
	  char n;
	  for (i=0; message[i] != '\0' ; i++){
	  		n = (charToInt(message[i]) + charToInt(key[i])) % 27;
	  		message[i] = intToChar(n);
	  }
	  return;
}

int main(int argc, char *argv[])
{

     int sockfd, newsockfd, portno, optval, n, status;
     socklen_t clilen;
     char buffer[512];
     char key[512];
     struct sockaddr_in serv_addr, cli_addr; //socket addresses
     pid_t pid, sid; //process id 
     
     
     if (argc != 2) {
         fprintf(stderr,"usage %s <port>\n", argv[0]);
         exit(EXIT_FAILURE);
     }  
     
     sockfd = socket(AF_INET, SOCK_STREAM, 0); //create the socket
     if (sockfd < 0) 
        error("ERROR opening socket");
     optval = 1;
     setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
        
     bzero((char *) &serv_addr, sizeof(serv_addr)); //clear out garbage from address
     portno = atoi(argv[1]); //get port number
     serv_addr.sin_family = AF_INET;	//fill in host address
     serv_addr.sin_addr.s_addr = INADDR_ANY; //any client can connect
     serv_addr.sin_port = htons(portno); //fill in port number
     
     if (bind(sockfd, (struct sockaddr *) &serv_addr, 
              sizeof(serv_addr)) < 0) //bind address to socket
              error("ERROR on binding");
              
     listen(sockfd,5); //wait for client to call

	while(1){   
		  clilen = sizeof(cli_addr); 
		  newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen); //answer call
		  if (newsockfd < 0){ 
				 error("ERROR on accept");
		  }
		  
		  //fork
		  pid = fork();
		  
		  if (pid < 0){
				error("ERROR forking process");
				exit(EXIT_FAILURE);
		  } 
		  
		  if (pid == 0){ //child process. child will handle connection
		  
		  	 bzero(buffer, sizeof(buffer));
		  	 int bytes_remaining = sizeof(buffer);
		  	 int bytes_read = 0;
		  	 char *p = buffer; //keep track of where in buffer we are
		  	 char *keyStart;
		  	 int numNewLines = 0;
		  	 int i;
		  	 
		  	 //receive authentication message and reply
          read(newsockfd, buffer, sizeof(buffer)-1);
          if (strcmp(buffer, "enc_bs") != 0) {
                perror("invalid client");
                //write error back to client
                char response[]  = "invalid";
                write(newsockfd, response, sizeof(response));
                _Exit(2);
          } 
          else {
                //write confirmation back to client
                char response[] = "enc_d_bs";
                write(newsockfd, response, sizeof(response));
          }
		  	 
		  	 bzero(buffer, sizeof(buffer));
		  	 
			 while (1){ 
				 bytes_read = read(newsockfd, p, bytes_remaining);
				 if (bytes_read == 0){ // We're done reading 
					 break;
				 }
				 if (bytes_read < 0) {
					 error("Server: ERROR reading from socket");
				 }
				 for (i=0; i < bytes_read ; i++){ //search for newlines in buffer 
					 if(p[i] == '\n'){
						  numNewLines++;
						  if (numNewLines == 1){  //first newline signals start of key
					 	 	 keyStart = p+i;
					 	  }
					 }
				 }
				 
				 if (numNewLines == 2){	//second newline signals end of transmission
					  break;			 
				 }

				 p += bytes_read;
				 bytes_remaining -= bytes_read;
	 		 }
	 		 char message[512];
	 		 bzero(message, sizeof(message));
	 		 
	 		 strncpy(message, buffer, keyStart-buffer); //separate message and key
	 		 printf("EServer: message before encryption: %s\n", message);
	 		 encrypt(message, keyStart); //encrypt message
	 		 printf("eServer: message after encryption: %s\n", message);
	 		 write(newsockfd, message, sizeof(message)); //send encrypted message
		  }
		  close(newsockfd);
		  
		  while (pid > 0){ 	//parent process. wait for children to finish
				pid = waitpid(-1, &status, WNOHANG);
		  }
	  }
	//end loop
     close(sockfd);
     
     
     return 0; 
}
