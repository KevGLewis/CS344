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

// Let's set up the global room name list
#define NUMNAMES 10
#define NUMROOMS 7
#define STR_SIZE 10000

char* ROOMNAME[10] = {
    "Foyer",
    "Kitchen",
    "Billiard",
    "Bedroom",
    "Garage",
    "Workshop",
    "Music",
    "Dance",
    "Caviar",
    "Bakery"
};

char* ROOMTYPE[3] = {
    "START_ROOM",
    "MID_ROOM",
    "END_ROOM"
};

struct Room
{
    int id;
    char** name;
    char** type;
    int numOutboundConnections;
    struct Room* outboundConnections[6];
};

void InitializeRooms(struct Room *MapRooms, int size){
    int i, r, j, isUnique;
  // First Clear the array
  memset(MapRooms,0,size * sizeof(struct Room));

    //set and initialie to 0
    int *nameOrder = malloc(size * sizeof(int));
    for(i = 0; i < size; i++){
        nameOrder[i] = -1;
    }
    
  // Randomly generate the name order, corresponding with the
  // global room name arrays
  int count = 0; // keeps track of how many names have been assigned
    
  for(i = 0; i < size; i++){
     // need to check if that room name has already been assigned
    do {
        r = rand() % 10;
        isUnique = 1;
        for(j = 0; j < count; j++){
            if(nameOrder[j] == r){
                isUnique = 0;
        }
      }
    } while(isUnique == 0);
    nameOrder[i] = r;
    count++;
  }
    
    r = rand() % size;
    MapRooms[r].name = &ROOMTYPE[0]; // set the Start Room
    
    i = rand() % size; // Determine the index for the end room
    while(i == r){
        i = rand() % size; // Ensure the index is Unique
    }
    MapRooms[i].name = &ROOMTYPE[2]; // Set the EndRoom

    // Initialize each room
  for(i = 0; i < size; i++){
    MapRooms[i].id = i;
    MapRooms[i].numOutboundConnections = 0;
    MapRooms[i].name = &ROOMNAME[nameOrder[i]];
      if(MapRooms[i].type != &ROOMTYPE[0] || MapRooms[i].type != &ROOMTYPE[2]){
          MapRooms[i].type = &ROOMTYPE[1];
      }
  }
    
    free(nameOrder);

}


// Returns true if all rooms have 3 to 6 outbound connections, false otherwise
bool IsGraphFull(struct Room *x)
{
  bool isFull = true;
  int i;
    for (i = 0; i < NUMROOMS; i++){
    if(x[i].numOutboundConnections > 3 &&
    x[i].numOutboundConnections < 6 ){
      isFull = false;
    }
  }

  return isFull;
}

// Returns true if a connection can be added from Room x (< 6 outbound connections), false otherwise
bool CanAddConnectionFrom(struct Room *x)
{
    if(x->numOutboundConnections <6){
        assert(x->numOutboundConnections < 6);
        return true;
    }
    else{
        return false;
    }
}

// Returns a random Room, does NOT validate if connection can be added
struct Room* GetRandomRoom(struct Room *roomsIn)
{
    int r = rand() % NUMROOMS;
    return &roomsIn[r];
}

// Returns true if Rooms x and y are the same Room, false otherwise
bool IsSameRoom(struct Room *x, struct Room *y)
{
    if(x == y){
        return true;
    }
    return false;
}

// Returns true if a connection from Room x to Room y already exists, false otherwise
bool ConnectionAlreadyExists(struct Room *x, struct Room *y)
{
    int i;
    for(i = 0; i < x->numOutboundConnections; i++){
        if((*x).outboundConnections[i] == y){
            return true;
        }
    }
    
    return false;
}

// Connects Rooms x and y together, does not check if this connection is valid
void ConnectRoom(struct Room *x, struct Room *y)
{
    x->outboundConnections[x->numOutboundConnections] = y;
    x->numOutboundConnections++;
}

// Adds a random, valid outbound connection from a Room to another Room
void AddRandomConnection(struct Room *roomsIn)
{
  struct Room *A;  // Maybe a struct, maybe global arrays of ints
  struct Room *B;

  while(true)
  {
      A = GetRandomRoom(roomsIn);

    if (CanAddConnectionFrom(A) == true)
      break;
  }

  do
  {
      B = GetRandomRoom(roomsIn);
  } while(CanAddConnectionFrom(B) == false || IsSameRoom(A, B) == true || ConnectionAlreadyExists(A, B) == true);
  

    assert(A != B);
    assert(B->numOutboundConnections < 6);
    assert(A->numOutboundConnections < 6);
    
  ConnectRoom(A, B);  // TODO: Add this connection to the real variables,
  ConnectRoom(B, A);  //  because this A and B will be destroyed when this function terminates
}

void PrintRoomsToFile(struct Room *roomsIn){
    int i;
    char fileName[STR_SIZE];
    char *pre = "room";
    
    snprintf(fileName, sizeof(fileName), "%s%d%s", pre, roomsIn->id, ".txt");
    
    FILE *fp;
    fp = fopen(fileName, "w");
    
    // Print the Name
    fprintf(fp, "ROOM NAME: %s\n", *roomsIn->name);
    
    // Now Print the Connections
    for(i = 1; i <= roomsIn->numOutboundConnections; i++){
        fprintf(fp, "CONNECTION %d: %s", i, *(roomsIn->outboundConnections[i-1]->name));
    }
    
    // Finally print the room type.
    fprintf(fp, "ROOM TYPE: %s", *(roomsIn->type));
    fclose(fp);
}

// once all of the room structs have been created, saves the rooms to a file
void SaveRoomsToFile(struct Room *roomsIn, int size){
    int i;
    
    // Get the process id
    int pid = getpid();
    
    char dirName[STR_SIZE];
    char *pre = "lewikevi.rooms.";
    
    snprintf(dirName, sizeof(dirName), "%s%d", pre, pid);
    mkdir(dirName, 0755);

    //Now print the rooms to file
    for(i = 0; i < size; i++){
        PrintRoomsToFile(&roomsIn[i]);
    }
    
}

int main(int argc, char* argv[])
{
  srand((int)time(NULL));   // should only be called once

  // Declare an array of 6 rooms

    struct Room MapRooms[NUMROOMS];
    InitializeRooms(MapRooms, NUMROOMS);
  //

  while (IsGraphFull(MapRooms) == false)
  {
      AddRandomConnection(MapRooms);
  }

  SaveRoomsToFile(MapRooms, NUMROOMS);
}
