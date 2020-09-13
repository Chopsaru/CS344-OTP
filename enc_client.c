#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <ctype.h>

/**
* Client code
* 1. Create a socket and connect to the server specified in the command arugments.
* 2. Prompt the user for input and send that input as a message to the server.
* 3. Print the message received from the server and exit the program.
*/

// Error function used for reporting issues
void error(const char *msg) {
  perror(msg);
  exit(1);
} 

// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address, int portNumber){
 
    // Clear out the address struct
    memset((char*) address, '\0', sizeof(*address));

    // The address should be network capable
    address->sin_family = AF_INET;
    // Store the port number
    address->sin_port = htons(portNumber);

    // Get the DNS entry for this host name
    struct hostent* hostInfo = gethostbyname("localhost");
    if (hostInfo == NULL) {
        fprintf(stderr, "CLIENT: ERROR, no such host\n");
        exit(0);
    }
    // Copy the first IP address from the DNS entry to sin_addr.s_addr
    memcpy((char*) &address->sin_addr.s_addr, 
        hostInfo->h_addr_list[0],
        hostInfo->h_length);
}


int main(int argc, char *argv[]) {
    int socketFD, portNumber, charsWritten, charsRead;
    struct sockaddr_in serverAddress;
    char buffer[71680];
    int nBytes;
    int authSend, authRead;
    char auth[2] = "e";
    FILE* fd;
    memset(buffer, '\0', sizeof(buffer));

    // Check usage & args
    if (argc != 4) { 
        error("Incorrect number of args"); 
    } 

    // Create a socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0); 
    if (socketFD < 0){
        error("CLIENT: ERROR opening socket");
    }

    // Set up the server address struct
    setupAddressStruct(&serverAddress, atoi(argv[3]));

    // Connect to server
    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
        error("CLIENT: ERROR connecting");
    }

    //open files from args and get lengths
    int key = open(argv[2], O_RDONLY);
    int kSize = lseek(key, 0, SEEK_END);
    int srctext = open(argv[1], O_RDONLY);
    int tSize = lseek(srctext, 0 , SEEK_END);

    //check if key file is big enough
    if(tSize > kSize){
        error("Error: key is too short.");
    }

    //check if source file contains invalid characters
    while(read(srctext, buffer, 1) != 0){
        if(!(isspace(buffer[0]) || isalpha(buffer[0]))){
            fprintf(stderr, "Error: found invalid character in %s\n", argv[1]);
            exit(1);
        }
    }

    

    //send server enc authentication char
    authSend = send(socketFD, auth, sizeof(auth), 0); 
    if (authSend < 0){
        error("CLIENT: ERROR writing to socket:1");
    }
    //recieve a 'y' if valid, 'n' if invalid
    memset(auth, '\0', sizeof(auth));
    authRead = recv(socketFD, auth, sizeof(auth), 0); 
    if (authRead < 0){
        error("CLIENT: ERROR reading from socket");
    }
    if (strcmp(auth, "y") != 0){
        error("ERROR connected client NOT enc_client");
    }

    //send files to server
    //source text file
    fd = fopen(argv[1], "r");
    memset(buffer, '\0', sizeof(buffer));
    while((tSize = fread(buffer, sizeof(char), 71680, fd)) > 0){
        if((nBytes = send(socketFD, buffer, tSize, 0)) < 0){
            error("CLIENT: ERROR writing to socket:2");
            break;
        }
        memset(buffer, '\0', sizeof(buffer)); 
    }
    fclose(fd);
        
    //key file
    fd = fopen(argv[2], "r");
    memset(buffer, '\0', sizeof(buffer));
    while((kSize = fread(buffer, sizeof(char), 71680, fd)) > 0){
        if((nBytes = send(socketFD, buffer, kSize, 0)) < 0){
            error("CLIENT: ERROR writing to socket:3");
            break;
        }
        memset(buffer, '\0', sizeof(buffer)); 
    }
    fclose(fd);

    // Get encrypred message returned from server
    // Clear out the buffer again for reuse
    memset(buffer, '\0', sizeof(buffer));
    // Read data from the socket, leaving \0 at end
    charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); 
    if (charsRead < 0){
        error("CLIENT: ERROR reading from socket");
    }
    fprintf(stdout, "%s\n", buffer);


    // Close the socket
    close(socketFD); 
    return 0;
}