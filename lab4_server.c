/* A simple TCP server that allows file downloads */
#include <stdio.h>
#include <sys/types.h>
#include <sys/unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <strings.h>


#define SERVER_TCP_PORT     3000    /* default port binding if user doesn't explicitly state one on server runtime */
#define BUFLEN              100     /* buffer length */

int fileTransfer(int);
void reaper(int);

int main(int argc, char **argv) {
    int     sd, new_sd, client_len, port;
    struct  sockaddr_in server, client;

    switch(argc){
    case 1: // Sets port binding to default port since user didn't provide one
        port = SERVER_TCP_PORT;
        break;
    case 2: // Sets port binding to user specified port
        port = atoi(argv[1]);
        break;
    default:
        fprintf(stderr, "Usage: %s [port]\n", argv[0]);
        exit(1);
    }
    
    // Create a stream socket
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "Can't create a socket\n");
        exit(1);
    }

    // Bind an address to the socket
    bzero((char *)&server, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sd, (struct sockaddr *)&server, sizeof(server)) == -1){
        fprintf(stderr, "Can't bind name to socket\n");
        exit(1);
    }

    // queue up to 5 connect requests  
    listen(sd, 5);

    (void) signal(SIGCHLD, reaper);

    while(1) {
      client_len = sizeof(client);
      new_sd = accept(sd, (struct sockaddr *)&client, &client_len);
      if(new_sd < 0){
        fprintf(stderr, "Can't accept client \n");
        exit(1);
      }
      switch (fork()){
      case 0:       /* child */
        (void) close(sd);
        exit(fileTransfer(new_sd));
      default:      /* parent */
        (void) close(new_sd);
        break;
      case -1:
        fprintf(stderr, "fork: error\n");
      }
    }
}

/*
This method contains the main file transfer program.
The server will read in a single string from the client which will contain a file name.
The server will attempt to open the file to check to see if the file exists.
    
    If the file exists, write an initial 0 byte to the client, to signal that the following
    data will be the contents of the file, for the client to write into its own file
    
    If the file does not exist, write an initial 1 byte to the client, to signal that the 
    following data will be the error message to display to the terminal.
*/
int fileTransfer(int sd) {
    char    buffer[BUFLEN];     // to store string sent in by the client
    int     n;
    FILE    *filePtr;           // File pointer to access requested file

    n = read(sd, buffer, BUFLEN);   // Read in request from client, n represents the number of bytes read
    buffer[strlen(buffer)-1] = '\0';
    printf("Read in %i bytes from client:\n", n);
    printf("%s\n", buffer);
    filePtr = fopen(buffer, "rb");   // Attempt to open the file
    
    if(filePtr == NULL){ // Check if file exists
        // File does not exist
        printf("File does not exist\n");
        
        char buf[]= "1";
        write(sd, buf, 2);  // The 1 char signifies that the content is an error message
        
        char errorMessage[BUFLEN] = "ERROR File not found: ";   // Error message to send after the 0 flag  
        strcat(errorMessage, buffer);
        write(sd, errorMessage, sizeof(errorMessage));
    }else{
        // File exists, send contents over in packets of max 100 bytes
        printf("File exists\n");
        
        char buf[]= "0";
        write(sd, buf, 2);  // The 0 char signifies that the content is the file
        
        int read = 0;
        char fileOutput[BUFLEN];
        while((read = fread(fileOutput,1,BUFLEN,filePtr)) > 0){
            write(sd,fileOutput,BUFLEN);
        }
    }
    
    close(sd);
    return(0);
}

// reaper
void reaper(int sig) {
    int status;
    while(wait3(&status, WNOHANG, (struct rusage *)0) >= 0);
}
