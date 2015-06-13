/*
Becky Solomon
otp_enc.c
encrypt client
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 


long get_file_length(char *filename) {
	 FILE *file = fopen(filename, "r");
    fpos_t position; 
    long length;

    fgetpos( file, &position ); // save previous position in file

    if ( fseek( file, 0, SEEK_END ) // seek to end
        || ( length = ftell( file ) ) == -1 ) { // determine offset of end
        fprintf(stderr, "ERROR Finding file length" ); 
        exit(1);
    }

    fsetpos( file, &position ); // restore position

    return length;
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


int main(int argc, char *argv[])
{
    int clientsockfd, portno, n, optval, plaintextfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;
	 FILE *fp;
	 const char hostname[] = "localhost";

    char buffer[100000];
    bzero(buffer,sizeof(buffer));
    if (argc != 4) {
       fprintf(stderr,"usage %s <inputfile> <key> <port>\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[3]);
    clientsockfd = socket(AF_INET, SOCK_STREAM, 0); //create socket
    if (clientsockfd < 0) 
        error("Client: ERROR opening socket");
    
    server = gethostbyname(hostname); //look up server
    if (server == NULL) {
        fprintf(stderr,"Client: ERROR, no such host\n");
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

    if (connect(clientsockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){ //connect to server socket
        perror("Client: ERROR connecting");
        exit(1);
	 }
    char auth[]="enc_bs";
    write(clientsockfd, auth, sizeof(auth));
    read(clientsockfd, buffer, sizeof(buffer));
    if (strcmp(buffer, "enc_d_bs") != 0) {
        fprintf(stderr,"unable to contact otp_enc_d on given port\n");
        exit(2);
    }
    
    //check that key is at least as long as message
    long infilelength = get_file_length(argv[1]);
    long keylength = get_file_length(argv[2]);   
    if (infilelength > keylength){
    	 fprintf(stderr, "key is too short"); 
    	 exit(1);
    }

    //check that plaintext contains only valid characters
	 int plainfd = open(argv[1], 'r'); 
    while (read(plainfd, buffer, 1) != 0) {
        if (buffer[0] != ' ' && (buffer[0] < 'A' || buffer[0] > 'Z')) {

            if (buffer[0] != '\n') {
                fprintf(stderr,"%s contains invalid characters\n", argv[1]);
                exit(EXIT_FAILURE);
            }
        }
    }
    
    bzero(buffer, sizeof(buffer));
    
	 //send plaintextfile
    sendFile(argv[1], clientsockfd, infilelength);

    //send key
	 sendFile(argv[2], clientsockfd, keylength);

	 //read server response
    n = read(clientsockfd,buffer,sizeof(buffer)-1);

    if (n < 0){
         perror("Client: ERROR reading from socket");
         exit(1);
    }
    printf("%s\n",buffer);
    close(clientsockfd);
    return 0;
}
