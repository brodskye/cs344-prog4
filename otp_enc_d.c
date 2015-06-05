/*
Becky Solomon
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

	  for (i=0; message[i] != '\n' ; i++){
	  		n = (charToInt(message[i]) + charToInt(key[i])) % 27;
	  		message[i] = intToChar(n);
	  }
	  return;
}

void decode(char message[], char key[]){
	  int i;
	  char n;

	  for (i=0; message[i] != '\n' ; i++){
	  		n = charToInt(message[i]) - charToInt(key[i]);
	  		if (n<0){
	  			n += 27;
	  		}
	  		message[i] = intToChar(n);
	  }
	  return;
}

int main(int argc, char *argv[])
{

     int sockfd, newsockfd, portno, optval, n, status;
     socklen_t clilen;
     char buffer[256];
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
		//loop for handling up to 5 connections 
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
		  
		  if (pid == 0){ 
		  //child process. child will handle connection
		  		//do i close sockfd here?
		  		bzero(buffer,256); //clear buffer
		  		n = read(newsockfd,buffer,sizeof(buffer)-1); //n is number of bytes transferred
		  		if (n < 0){
		  			error("ERROR reading from socket");
		  		}
		  		printf("Server: Here is the message: %s\n",buffer);
	  
		  }
		  
		  while (pid > 0){ 
		  //parent process. wait for children to finish
				close(newsockfd);
				pid = waitpid(-1, &status, WNOHANG);
		  }
	  }
	//end loop
     close(sockfd);
     
     
     return 0; 
}
