#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

void usage(int argc, char *argv[]){
	if (argc != 2){ 
		printf("Usage: %s length\n", argv[0]); 
		exit (0);
	}
}

int main (int argc, char *argv[]){

	int i, length;
	char key[length + 1];
	char randLetter;
	
	usage(argc, argv);
	
	srand(time(NULL));
	
	length = atoi(argv[1]); //cast given length from char to int
	
	for (i=0; i<length; i++){
		randLetter = " ABCDEFGHIJKLMNOPQRSTUVWXYZ"[rand() % 27];
		key[i] = randLetter;
		
	}
	key[length] = '\0'; //null terminating char of key string
	
	printf("%s\n", key);
	
	return 0;
}