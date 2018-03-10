
#include "snd_recv_help.h"

int main(int argc, char *argv[])
{
	int socketFD, portNumber;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[1056];
    
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
	if (socketFD < 0)
    {
        error("CLIENT: ERROR opening socket");
        exit(2);
    }
	
	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting");
    
    // Handshake with the server
    if(!PasswordSend(buffer, socketFD, password))
    {
        // Close it if it was not successfull
        close(socketFD);
        exit(2);
    }
    
    VerifyInput(argv[1], argv[2]);
    
    // load the buffer with our arguments and send the data to the server
    memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
    sprintf(buffer, "%s %s", argv[1], argv[2]); // Load the buffer with our arguments
    SendFileData(buffer, socketFD);
    ReceiveFileData(buffer, socketFD);

    printf("%s", buffer);
    
	close(socketFD); // Close the socket
    
    exit(0);
}
