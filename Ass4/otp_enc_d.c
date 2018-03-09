#include "snd_recv_help.h"

int listenSocketFD;

struct InputFileNames
{
    char* keyFileName;
    char* textToEncryptFileName;
};

struct ReturnMessage
{
    int messageSize;
    char* message;
};

// Set up the InputFileNames struct and Return Message

void InitializeStructs(struct InputFileNames* ifn, struct ReturnMessage* rn)
{
    ifn->keyFileName = NULL;
    ifn->textToEncryptFileName = NULL;
    
    rn->message = NULL;
    rn->messageSize = 0;
}
// The input will be in the form of keyFileName " " parseTextFileName
void ParseInput(char* input, struct InputFileNames* fileNames)
{
    char *token;
    
    // The first input is the filename
    token = strtok(input, " ");
    fileNames->keyFileName = calloc(strlen(token) + 1, sizeof(char));
    strcpy(fileNames->keyFileName, token);
    
    // The second input is the text to encrpt file name
    token = strtok(NULL, " ");
    fileNames->textToEncryptFileName = calloc(strlen(token) + 1, sizeof(char));
    strcpy(fileNames->textToEncryptFileName, token);
}

void EncryptText(struct InputFileNames* ifn, struct ReturnMessage* rn)
{
    
}

// Free the dynamically allocated portions of the structs
void CleanupStructs(struct InputFileNames* ifn, struct ReturnMessage* rn)
{
    free(ifn->keyFileName);
    free(ifn->textToEncryptFileName);
    
    free(rn->message);
}

void catchSIGCHLD(int signo);

// Set up the signal handler for the parent
void SetupSignalHandlers()
{
    
    struct sigaction SIGCHLD_action = {{0}}, SIGTERM_action = {{0}}, ignore_action = {{0}};
    
    SIGCHLD_action.sa_handler = catchSIGCHLD;
    sigfillset(&SIGCHLD_action.sa_mask);
    SIGCHLD_action.sa_flags = 0;
    
    SIGTERM_action.sa_handler = catchSIGCHLD;
    sigfillset(&SIGTERM_action.sa_mask);
    SIGTERM_action.sa_flags = 0;
    
    ignore_action.sa_handler = SIG_IGN;
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
    int establishedConnectionFD, portNumber, charsRead, p, dataSize, sizeDataRecv;
    int count;
    pid_t pid;
	socklen_t sizeOfClientInfo;
	char buffer[256];
    char buffSpurt[256];
	struct sockaddr_in serverAddress, clientAddress;
    struct InputFileNames clientInput;
    struct ReturnMessage clientReturn;
    
    char* password = "&&&&&"; // Let's us acknowledge we are talking with a friendly client


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
    SetupSignalHandlers();
    
    /*
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
    }*/
    
    while (1)
    {
        count = 0;
        
        // initialize the structs to null
        InitializeStructs(&clientInput, &clientReturn);
        
        // Accept a connection, blocking if one is not available until one connects
        sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
        establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
        if (establishedConnectionFD < 0) error("ERROR on accept");

        // Get the message from the client and display it
        memset(buffer, '\0', 256);
        memset(buffSpurt, '\0', 256);
    
        // The first data received is the size of the message so we know if we have
        // gotten all of the data within a message
        charsRead = (int) recv(establishedConnectionFD, buffSpurt, 255, 0); // Read the client's message from the socket
        if (charsRead < 0) error("ERROR reading from socket");
        
        dataSize = atoi(buffSpurt);
        
        // Send a Success message back to the client
        charsRead = (int) send(establishedConnectionFD, "Connection Successfull", 22, 0); // Send success back
        if (charsRead < 0) error("ERROR writing to socket");
        
        
        ReceiveFileData(buffer, establishedConnectionFD, dataSize);
        
        ParseInput(buffer, &clientInput);
        
        printf("Filename = %s\nTextToEncrypt = %s\n", clientInput.keyFileName, clientInput.textToEncryptFileName);

        // Send a Success message back to the client
        charsRead = (int) send(establishedConnectionFD, "I am the server, and I got your message", 39, 0); // Send success back
        if (charsRead < 0) error("ERROR writing to socket");
        close(establishedConnectionFD); // Close the existing socket which is connected to the client
        exit(0);
    }
    
    
    
	return 0;
}
                   
                   

                   

