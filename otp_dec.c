/*
Becky Solomon
decrypt client
otp_dec.c
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

long get_file_length(char *filename) {
	 FILE *file = fopen(filename, "r");
    fpos_t position; 
    long length;

    fgetpos( file, &position ); // save previous position in file

    if ( fseek( file, 0, SEEK_END ) // seek to end
        || ( length = ftell( file ) ) == -1 ) { // determine offset of end
        perror( "Finding file length" ); // handle overflow
    }

    fsetpos( file, &position ); // restore position

    return length;
}


void sendFile(char *filename, int sockfd, int filelength){
	//fprintf(stderr, "\n\nsending %s\n", filename);
	 //fprintf(stderr,"filelength: %d\n", filelength);
	 
	 int fd = open(filename, 'r'); //open for readonly file specified in command line
    char buffer[100000];
    bzero(buffer, sizeof(buffer));
    int bytes_read, bytes_written;
	
    while (filelength > 0){ //read in the file in chunks until the whole file is read
		 bytes_read = read(fd, buffer, sizeof(buffer));
		 if (bytes_read == 0){ // We're done reading from the file
			  break;
		 }
		 if (bytes_read < 0) {
			  // handle errors
			  perror("Client: ERROR reading file");
			  exit(1);
		 }
		 filelength -= bytes_read;
		 //fprintf(stderr,"filelength: %d\n", filelength);
	 }	 
		 char *p; 
		 p = buffer; //keep track of where in buffer we are
		 while (bytes_read > 0) {
			  bytes_written = write(sockfd, p, bytes_read);
			  //fprintf(stderr,"bytes_written: %d\n", bytes_written);
			  //fprintf(stderr,"bytes_read: %d\n", bytes_read);
			  if (bytes_written < 0) {
					// handle errors
					perror("Client: ERROR writing to socket");
					exit(1);
			  }
			  bytes_read -= bytes_written;
			  p += bytes_written;
		 }
	  return;
}

void receiveFile(int sockfd){
	 int n;
    char buffer[100000];
    bzero(buffer,100000);

    while (1){ //read in the file in chunks until the whole file is read
		 int bytes_read;
		 bytes_read = read(sockfd, buffer, sizeof(buffer));
		 if (bytes_read == 0){ // We're done reading from the file
			  break;
		 }
		 if (bytes_read < 0) {
			  error("ERROR reading file");
		 }
		 //write to stdout
		 n = write(1, buffer, bytes_read);
		 if (n < 0) {
			  error("ERROR writing to socket");
		 } 
	 }
}

int main(int argc, char *argv[])
{
    int clientsockfd, portno, n, optval, plaintextfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;
	 FILE *fp;
	 const char hostname[] = "localhost";
	 
    char buffer[100000];
    bzero(buffer,100000);
    if (argc != 4) {
       fprintf(stderr,"usage %s <inputfile> <key> <port>\n", argv[0]);
       exit(1);
    }
    portno = atoi(argv[3]);
    clientsockfd = socket(AF_INET, SOCK_STREAM, 0); //create socket
    if (clientsockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }
    server = gethostbyname(hostname); //look up server
    if (server == NULL) {
        perror("ERROR, no such host");
        exit(1);
    }
    optval = 1;
    setsockopt(clientsockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)); //allow reuse of port
    
    bzero((char *) &serv_addr, sizeof(serv_addr)); //clear address
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(clientsockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){ //connect to server socket
        perror("ERROR connecting");
        exit(1);
    }
    
    char auth[]="dec_bs";
    write(clientsockfd, auth, sizeof(auth));
    read(clientsockfd, buffer, sizeof(buffer));
    if (strcmp(buffer, "dec_d_bs") != 0) {
        fprintf(stderr, "unable to contact otp_enc_d on given port\n");
        exit(2);
    }
    
    //check that key is at least as long as message
    long infilelength = get_file_length(argv[1]);
    long keylength = get_file_length(argv[2]);    
    if (infilelength > keylength){
    	 fprintf(stderr, "key is too short\n");
    	 exit(1); 
    }
    
	 //send plaintextfile
    sendFile(argv[1], clientsockfd, infilelength);
    //send key
	 sendFile(argv[2], clientsockfd, keylength);

    n = read(clientsockfd,buffer,sizeof(buffer));
    if (n < 0){
         perror("ERROR reading from socket");
         exit(1);
    }
    printf("%s\n",buffer);
    close(clientsockfd);
    return 0;
}
