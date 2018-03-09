
#include "snd_recv_help.h"

int main(int argc, char *argv[])
{
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
    char buffRecv[256];
	char buffer[256];
    char bufSize[256];
    
    char* password = "&&&&&"; // Allows us to know we are working with a friendly server
    
	if (argc < 4) { fprintf(stderr,"USAGE: %s plaintext key port\n", argv[0]); exit(0); } // Check usage & args

	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");
	
	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting");

    // Clear the arrays
	memset(buffer, '\0', sizeof(buffer));
    memset(bufSize, '\0', sizeof(bufSize));
    memset(buffRecv, '\0', sizeof(buffRecv));
    
    CheckPassword(buffer, buffRecv, socketFD, password);
    
    // load the buffer with our arguments
    memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
    sprintf(buffer, "%s %s", argv[2], argv[3]); // Load the buffer with our arguments
    sprintf(bufSize, "%d", (int) strlen(buffer));
    
    // Clear the receiver buffer and Send the message size to the server
    memset(buffRecv, '\0', sizeof(buffRecv));
    SendMessageSize(bufSize, buffRecv, socketFD);
    
    // Send our data to the Server
    
	// Get return message from server, reuse our buffer
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
	charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end
	if (charsRead < 0) error("CLIENT: ERROR reading from socket");
	printf("CLIENT: I received this from the server: \"%s\"\n", buffer);

	close(socketFD); // Close the socket
	return 0;
}
