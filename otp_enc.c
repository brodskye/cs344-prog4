/*
Becky Solomon
otp_enc.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int clientsockfd, portno, n, optval;
    struct sockaddr_in serv_addr;
    struct hostent *server;
	 FILE *fp;
	 
    char buffer[256];
    if (argc < 3) {
       fprintf(stderr,"usage %s inputfile port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    clientsockfd = socket(AF_INET, SOCK_STREAM, 0); //create socket
    if (clientsockfd < 0) 
        error("ERROR opening socket");
    const char hostname[] = "localhost";
    server = gethostbyname(hostname); //look up server
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    optval = 1;
    setsockopt(clientsockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)); //allow reuse of port
    
    bzero((char *) &serv_addr, sizeof(serv_addr)); //clear address
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    
    if (connect(clientsockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) //connect to server socket
        error("ERROR connecting");
    //printf("Please enter the message: ");
    
    bzero(buffer,256);
     /*
    printf("Please enter the message: ");
    bzero(buffer,256);
    fgets(buffer,255,stdin);
    n = write(sockfd,buffer,strlen(buffer));
    */
    int fd;
    fd = open(argv[1], 'r'); //open file specified in command line
    while (1){
		 int bytes_read;
		 bytes_read = read(fd, buffer, sizeof(buffer));
		 if (bytes_read == 0){ // We're done reading from the file
			  break;
		 }
		 if (bytes_read < 0) {
			  // handle errors
			  error("ERROR reading file");
		 }

		 void *p;
		 p = buffer; //keep track of where in buffer we are
		 int bytes_written;
		 while (bytes_read > 0) {
			  bytes_written = write(clientsockfd, p, bytes_read);
			  if (bytes_written <= 0) {
					// handle errors
					error("ERROR writing to socket");
			  }
			  bytes_read -= bytes_written;
			  p += bytes_written;
		 }
	}
	   
    bzero(buffer,256);
    n = read(clientsockfd,buffer,255);
    if (n < 0) 
         error("ERROR reading from socket");
    printf("%s\n",buffer);
    close(clientsockfd);
    return 0;
}
