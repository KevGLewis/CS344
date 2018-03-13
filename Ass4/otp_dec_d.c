#include <netdb.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include "snd_recv_help.h"

int listenSocketFD;

// Set up the InputFileNames struct and Return Message
void SetupSignalHandlers()
{
    
    struct sigaction SIGCHLD_action = {{0}}, SIGTERM_action = {{0}};
    //struct sigaction ignore_action = {{0}};
    
    SIGCHLD_action.sa_handler = catchSIGCHLD;
    sigfillset(&SIGCHLD_action.sa_mask);
    SIGCHLD_action.sa_flags = 0;
    
    SIGTERM_action.sa_handler = catchSIGTERM;
    sigfillset(&SIGTERM_action.sa_mask);
    SIGTERM_action.sa_flags = 0;
    
    //ignore_action.sa_handler = SIG_IGN;
    //sigaction(SIGINT, &SIGINT_action, NULL);
    //sigaction(SIGINT, &ignore_action, NULL); // Ignore we don't want the shell to end
    sigaction(SIGCHLD, &SIGCHLD_action, NULL);
    sigaction(SIGTERM, &SIGTERM_action, NULL);
}

// Catche the signal and then wait for the child until all children
// have been waited for that have closed.
void catchSIGCHLD(int signo)
{
    int childExitMethod = -5;
    pid_t childPID;
    childPID = waitpid(-1, &childExitMethod, WNOHANG);
    while(childPID != 0)
    {
        
        childPID = waitpid(-1, &childExitMethod, WNOHANG);
    }
}

// Catch the term signal
void catchSIGTERM(int signo)
{
    char* message = "terminated by signal 2\n";
    write(STDOUT_FILENO, message, 24);
    close(listenSocketFD); // Close the listening socket
    exit(0);
}

int main(int argc, char *argv[])
{
    int establishedConnectionFD, portNumber;
    int p;
    pid_t pid = 0;
	socklen_t sizeOfClientInfo;
    char* buffer = NULL;
	struct sockaddr_in serverAddress, clientAddress;
    struct InputFileNames clientInput;
    
    char* password = "#####"; // Let's us acknowledge we are talking with a friendly client


	if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) error("ERROR opening socket");

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

    
    // Finally set up our signal handlers
    for (p = 0; p < 5; p++)
    {
        pid = fork();
        if (pid == -1)
        {
            perror("Hull Breach!");
            exit(1);
        }
        else if (pid == 0)
        {
            break;
        }
    }
    
    if(pid != 0)
    {
        SetupSignalHandlers();
    }
    
    while (1)
    {
        // initialize the structs to null
        InitializeStructs(&clientInput);
        
        // Accept a connection, blocking if one is not available until one connects
        sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
        establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
        if (establishedConnectionFD < 0) error("ERROR on accept");

        // Handshake with the client
        if(PasswordReceive(establishedConnectionFD, password))
        {
            // Receive the data from the Client
            if(ReceiveFileData(&buffer, establishedConnectionFD))
            {
                perror("Error Receiving Data");
                continue;
            }
            ParseInput(buffer, &clientInput);
            free(buffer);
            buffer = NULL;
            CryptInput(&buffer, &clientInput, 0);  // 0 Toggle indicates it will Decrypt
            SendFileData(&buffer, establishedConnectionFD);
            free(buffer);
        }
        
        else
        {
            continue; // Go back to the beginning of the while loop
        }
        
        close(establishedConnectionFD); // Close the existing socket which is connected to the client
    }
    
    
    
	return 0;
}
                   
                   

                   

