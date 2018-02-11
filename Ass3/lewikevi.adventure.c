// ***********************************************
// Build Rooms
// Purpose: Create rooms for the adventure project
// Each room consistst of a name, connection, and room type
// *************************************************

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <dirent.h>

# define NROOMS 7
# define BUFFER 256

struct Room
{
    int id;
    char* name;
    char* type;
    int numConnx;
    char *connections[6];
};

struct RoomContainer
{
    int steps;
    int path[1000];
    int count; //Stores where we need to add the next Room
    struct Room roomArray[NROOMS];
};

void InitializeRoomContainer(struct RoomContainer *contIn)
{
    int i;
    contIn->count = 0;
    contIn->steps = 0;
    
    // Need to clear out all of the information within the rooms
    for(i = 0; i < NROOMS; i++)
    {
        contIn->roomArray[i].id = 0;
        contIn->roomArray[i].numConnx = 0;
    }
}

void ItrNextRoom(struct RoomContainer *roomCont)
{
    roomCont->count++;
}

void AddNameToRoom(struct RoomContainer *roomCont, char *nameIn)
{
    roomCont->roomArray[roomCont->count].name = malloc(BUFFER * sizeof(char));
    memset(roomCont->roomArray[roomCont->count].name, '\0', sizeof(*(roomCont->roomArray[roomCont->count].name)));
    
    strcpy(roomCont->roomArray[roomCont->count].name, nameIn);
    roomCont->roomArray[roomCont->count].id = roomCont->count;
}

void AddTypeToRoom(struct RoomContainer *roomCont, char *type)
{
    roomCont->roomArray[roomCont->count].type = malloc(BUFFER * sizeof(char));
    memset(roomCont->roomArray[roomCont->count].type, '\0', sizeof(*(roomCont->roomArray[roomCont->count].type)));
    
    // Set the value
    strcpy(roomCont->roomArray[roomCont->count].type, type);
}

// Add the name of the connecting rooms to the struct array
void AddConnection (struct RoomContainer *roomCont, char* connxName)
{
    // Point to the room we are currently dealing with
    struct Room *roomIn = &(roomCont->roomArray[roomCont->count]);
    
    roomIn->connections[roomIn->numConnx] = malloc(BUFFER * sizeof(char));
    memset(roomIn->connections[roomIn->numConnx], '\0', sizeof(*(roomIn->connections[roomIn->numConnx])));
    
    // Set the value
    strcpy(roomIn->connections[roomIn->numConnx], connxName);
    
    roomIn->numConnx++;
}

// Process each line, parsing it into different veriables.
void processLine(struct RoomContainer *roomCont, char* lineIn)
{
    char begin[20], middle[20], end[20];
    sscanf(lineIn, "%s %s %s", begin, middle, end);
    
    if(strcmp(begin, "ROOM") == 0)
    {
        if(strcmp(middle, "NAME:") == 0)
        {
            AddNameToRoom(roomCont, end);
        }
        
        else
        {
            AddTypeToRoom(roomCont, end);
        }
        
    }
    else
    {
        AddConnection(roomCont, end);
    }
}

// Sequentially process the file
void processFile(struct RoomContainer *roomCont, FILE *fpIn){
   
    size_t bufferSize = 0; // Holds how large the allocated buffer is
    char* lineEntered = NULL; // Points to a buffer allocated by getline() that holds our entered string
    
    while(getline(&lineEntered, &bufferSize, fpIn) != -1) //read in the line.
    //while(fgets(lineEntered, BUFFER, fpIn)) //read in the line.
    {
        processLine(roomCont, lineEntered);
    }

    free(lineEntered);
    ItrNextRoom(roomCont);
}

// Get the name of the temporary directory
char* getRoomDirectory(){
    
    int newestDirTime = -1; // Modified timestamp of newest subdir examined
    char targetDirPrefix[32] = "lewikevi.rooms."; // Prefix we're looking for
    char *newestDirName = malloc(BUFFER * sizeof(char)); // Holds the name of the newest dir that contains prefix
    memset(newestDirName, '\0', sizeof(*newestDirName));
    
    DIR* dirToCheck; // Holds the directory we're starting in
    struct dirent *fileInDir; // Holds the current subdir of the starting dir
    struct stat dirAttributes; // Holds information we've gained about subdir
    
    dirToCheck = opendir("."); // Open up the directory this program was run in
    
    if (dirToCheck > 0) // Make sure the current directory could be opened
    {
        while ((fileInDir = readdir(dirToCheck)) != NULL) // Check each entry in dir
        {
            if (strstr(fileInDir->d_name, targetDirPrefix) != NULL) // If entry has prefix
            {
                stat(fileInDir->d_name, &dirAttributes); // Get attributes of the entry
                
                if ((int)dirAttributes.st_mtime > newestDirTime) // If this time is bigger
                {
                    newestDirTime = (int)dirAttributes.st_mtime;
                    memset(newestDirName, '\0', sizeof(*newestDirName));
                    strcpy(newestDirName, fileInDir->d_name);
                }
            }
        }
    }
    
    closedir(dirToCheck); // Close the directory we opened
    
    return newestDirName;
}

// Process all of the files within the temporary directory we created
void ProcessRoomsInDir(struct RoomContainer *roomCont)
{
    FILE *fp;
    struct dirent *fileInDir; // Holds the current subdir of the starting dir
    char* newDirectName = getRoomDirectory();
    char* filePath = malloc(BUFFER * sizeof(char)); // Holds the filepath of the file;
    DIR *dirToProcess = opendir(newDirectName);
    
    if (dirToProcess > 0) // Make sure the current directory could be opened
    {
        while ((fileInDir = readdir(dirToProcess)) != NULL) // Check each entry in dir
        {
            if(strlen(fileInDir->d_name) > 2)
            {
                //Create the file name
                sprintf(filePath, "%s/%s", newDirectName, fileInDir->d_name);
                fp = fopen(filePath, "r");

                processFile(roomCont, fp);
            }
        }
    }
    
    closedir(dirToProcess); // Close the directory we opened
    free(newDirectName);
    free(filePath);
}

// Free up the memory we previously allocated
void cleanup(struct RoomContainer *roomCont)
{
    int i, j;
    
    // Free up the memory we allocated.
    for(i = 0; i < NROOMS; i++){
        free(roomCont->roomArray[i].name);
        free(roomCont->roomArray[i].type);
        for(j = 0; j < roomCont->roomArray[i].numConnx; j++){
            free(roomCont->roomArray[i].connections[j]);
        }
    }
}

// Find beginning room, returns the index of the beginning room
int FindBegin(struct RoomContainer *contIn)
{
    int i;
    
    for (i = 0; i < contIn->count; i++)
    {
        if(strcmp(contIn->roomArray[i].type, "START_ROOM") == 0)
        {
            return i;
        }
    }
    return -1;
}

int AtTheEnd(struct RoomContainer *contIn, int indexIn)
{
    if(strcmp(contIn->roomArray[indexIn].type, "END_ROOM") == 0)
    {
        return 1; // We've found the end of the game
    }
    
    return 0; // Haven't found the end yet;
}


void printRoom(struct Room *roomIn)
{
    int i;
    printf("CURRENT LOCATION: %s\n", roomIn->name);
    
    // Print the possible connections by iterating over those within the room
    printf("POSSIBLE CONNECTIONS: ");
    
    for(i = 0; i < roomIn->numConnx; i++)
    {
        printf("%s", roomIn->connections[i]);
        
        if(i == roomIn->numConnx - 1)
        {
            printf(".\n");
        }
        else
        {
            printf(", ");
        }
    }
    
    printf("WHERE TO? >");
}

// Give a string, will return the the index of that room in the room array.
int getRoomIndex(struct RoomContainer *contIn, char* nameIn)
{
    int i;
    
    // Assume that we don't know the size of the array
    for (i = 0; i < (sizeof(contIn->roomArray)/sizeof(struct Room)); i++)
    {
        if (strcmp(contIn->roomArray[i].name, nameIn) == 0)
        {
           return i;
        }
    }
    return -1;
}

int getInteraction(struct RoomContainer *contIn, int index)
{
    int i, indexOut;
    size_t bufferSize = 0; // Holds how large the allocated buffer is
    char* lineEntered = NULL; // Points to a buffer allocated by getline() that holds our entered string
    
    getline(&lineEntered, &bufferSize, stdin); //read in the line.
    
    printf("\n"); // Add in a line to comply with designated format.

    lineEntered[strcspn(lineEntered, "\n")] = 0; // Remove the \n at the end of line entered
    
    // Add in code here to deal with the time.
    
    // Iterate over the room names and see if any match
    for(i = 0; i < contIn->roomArray[index].numConnx; i++)
    {
        if (strcmp(contIn->roomArray[index].connections[i], lineEntered) == 0)
        {
            indexOut = getRoomIndex(contIn, lineEntered);
            free(lineEntered);
            
            // Record the path we are taking to get to the end.
            contIn->path[contIn->steps] = i;
            contIn->steps++;
            return indexOut;
        }
    }
    
    // If we make it to this point, then it is not a match
    printf("HUH? I DON’T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
    
    free(lineEntered);
    return index;
}

void printPath(struct RoomContainer *contIn)
{
    int i;
    printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", contIn->steps);
    
    for(i = 0; i < contIn->steps; i++)
    {
        printf("%s\n", contIn->roomArray[contIn->path[i]].name);
    }
}

void playGame(struct RoomContainer *contIn)
{
    int index = FindBegin(contIn); // Find the beginning index
    
    while(AtTheEnd(contIn, index) == 0)
    {
        printRoom(&(contIn->roomArray[index]));
        index = getInteraction(contIn, index);
    }
    
    // When finished, Print the path and the steps
    printPath(contIn);
    
}

int main(int argc, char* argv[])
{
    srand((int)time(NULL));   // should only be called once
    
    // Declare a roomContainer struct and initialize it
    
    struct RoomContainer rCont;
    InitializeRoomContainer(&rCont);
    
    ProcessRoomsInDir(&rCont);
 
    playGame(&rCont);

    cleanup(&rCont);
    return 1;
}

