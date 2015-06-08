/*
A simple server in the internet domain using TCP
The port number is passed as an argument */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno;
     socklen_t clilen;
     char buffer[256];
     struct sockaddr_in serv_addr, cli_addr; //socket addresses
     int n;
     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     sockfd = socket(AF_INET, SOCK_STREAM, 0); //create the socket
     if (sockfd < 0) 
        error("ERROR opening socket");
        
     bzero((char *) &serv_addr, sizeof(serv_addr)); //clear out garbage from address
     portno = atoi(argv[1]); //get port number
     serv_addr.sin_family = AF_INET;	//fill in host address
     serv_addr.sin_addr.s_addr = INADDR_ANY; //any client can connect
     serv_addr.sin_port = htons(portno); //fill in port number
     
     if (bind(sockfd, (struct sockaddr *) &serv_addr, 
              sizeof(serv_addr)) < 0) //bind address to socket
              error("ERROR on binding");
              
//loop        
     listen(sockfd,5); //wait for client to call
     clilen = sizeof(cli_addr); 
     newsockfd = accept(sockfd, 
                 (struct sockaddr *) &cli_addr, 
                 &clilen); //answer call
     if (newsockfd < 0){ 
          error("ERROR on accept");
     }
   //fork
   //child process  
     bzero(buffer,256); //clear buffer
     n = read(newsockfd,buffer,255); //n is number of bytes transferred, 255 is max acceptable # of chars
     if (n < 0) error("ERROR reading from socket");
     printf("Here is the message: %s\n",buffer);
     
     n = write(newsockfd,"I got your message",18);
     if (n < 0) error("ERROR writing to socket");
     close(newsockfd);
     
//end loop
     close(sockfd);
     return 0; 
}
