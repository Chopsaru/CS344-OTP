#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>

// Error function used for reporting issues
void error(const char *msg) {
  perror(msg);
  exit(1);
} 

// interate thru charLib to return position of c int, -1 if c is not in charLib
int convertChar(char c){
  char * charLib = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
  int i;
  for (i = 0; i < 27; i++){
    if (charLib[i] == c){
      return i;
    }
  }
  return -1;
}

// return char at position c of charLib
char convertInt(int c){
  char * charLib = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
  return charLib[c];
}

//decripts text using key and assigns to decipher
void decrypt(char decipher[], char text[], char key[]){
  int textLen = strlen(text)-1;
  memset(decipher, '\0', 71680);
  int textint = 0;
  int keyint = 0;
  int decipherint = 0;

  int i;
  for(i = 0; i < textLen; i++){
    textint = convertChar(text[i]);
    keyint = convertChar(key[i]);
    //reverse encrypt process
    decipherint = (textint - keyint) % 27;
    //if the resulting int is lower then zero, compensate
    if(decipherint < 0){
			decipherint += 27;
		}
    decipher[i] = convertInt(decipherint);
  }
}

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address, int portNumber){
 
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address)); 

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);
  // Allow a client at any address to connect to this server
  address->sin_addr.s_addr = INADDR_ANY;
}

int main(int argc, char *argv[]){
  int connectionSocket, charsRead;
  char buffer[71680];
  char key[71680];
  char text[71680];
  char decipher[71680];
  char auth[2];
  int authRead, authSend;
  pid_t pid;
  struct sockaddr_in serverAddress, clientAddress;
  socklen_t sizeOfClientInfo = sizeof(clientAddress);


  // Check usage & args
  if (argc < 2) { 
    fprintf(stderr,"USAGE: %s port\n", argv[0]); 
    exit(1);
  } 
  
  // Create the socket that will listen for connections
  int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (listenSocket < 0) {
    error("ERROR opening socket");
  }

  // Set up the address struct for the server socket
  setupAddressStruct(&serverAddress, atoi(argv[1]));

  // Associate the socket to the port
  if (bind(listenSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0){
    error("ERROR on binding");
  }

  // Start listening for connetions. Allow up to 5 connections to queue up
  listen(listenSocket, 5); 
  
  // Accept a connection, blocking if one is not available until one connects
  while(1){
    // Accept the connection request which creates a connection socket
    connectionSocket = accept(listenSocket, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); 
    if (connectionSocket < 0){
      error("ERROR on accept");
    }

    //create child to process 
    pid = fork();
			switch (pid){
        case -1:
					perror("fork() failed\n");
					exit(1);
					break;
        case 0:
          //server suthentication 
          memset(auth, '\0', sizeof(auth));
          //recieve auth char from client and assign to auth
          authRead = recv(connectionSocket, auth, sizeof(auth), 0); 
          if (authRead < 0){
            error("ERROR reading from socket");
          }
          // if auth != d then the client is NOT dec_client and therefore invalid
          // return 'n' via connectionSocket to indicate this
          if (strcmp(auth, "d") != 0){
              strcpy(auth, "n");
              authSend = send(connectionSocket, auth, sizeof(auth), 0); 
              if (authSend < 0){
                error("ERROR writing to socket");
              }
          }
          // if auth == d then the client is dec_client and therefore valid
          //return 'y' via connectionSocket to indicate this
          else{
              strcpy(auth, "y");
              authSend = send(connectionSocket, auth, sizeof(auth), 0); 
              if (authSend < 0){
                error("ERROR writing to socket");
              }
          }

          // Get the message from the client and display it
          memset(text, '\0', 71680);
          memset(key, '\0', 71680);
          // Read the client's text from the socket
          charsRead = recv(connectionSocket, text, 71680, 0); 
          if (charsRead < 0){
            error("ERROR reading from socket");
          }

          // Read the client's key from the socket
          charsRead = recv(connectionSocket, key, 71680, 0); 
          if (charsRead < 0){
            error("ERROR reading from socket");
          }
        
          decrypt(decipher, text, key);

          // Send a Success message back to the client
          charsRead = send(connectionSocket, decipher, sizeof(decipher), 0); 
          if (charsRead < 0){
            error("ERROR writing to socket");
          }
          // Close the connection socket for this client
          close(connectionSocket); 
          exit(0);

        //parent waits for child  to finish then kills zombay ay ay
        default:
          wait(NULL);
          break;
    }
  }
  // Close the listening socket, we're done here.
  close(listenSocket); 
  return 0;
}
