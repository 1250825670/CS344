/*********************************************************************
**
** Author: Daniel Beyer
** Date: 02/07/17
** Description: CS344 Program 2 - beyerda.buildrooms.c  This program creates a series of files
** that hold descriptions of the in-game rooms and how the rooms are connected for an adventure game.
** In the game, the player will begin in the "starting room" and will win the game upon entering the
** "ending room", which causes the game to exit and displaying the path taken by the player.  During
** the game, the player can also enter a command that returns the current time utilizing mutexes and multithreading.
**
*************************************************************************/
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/* Constants */
const char* roomType[3] = {"START_ROOM", "END_ROOM", "MID_ROOM"};

const char* roomNames[10] = {"Bridge", "Ready_Room", "Crew_Quarters",
			    "Ten_Forward", "Engineering", "Holodeck",
			    "Transporter_Room", "Sick_Bay", "Cargo_Bay",
			    "Security"};



/*********************************************************
 * Struct for rooms
 ********************************************************/
struct Room {
	const char* name;
	const char* type;
	int connections[6];
	int numConnections;
};

/**********************************************
* Function prototypes
*********************************************/
char *createDir();
void createRooms(struct Room rooms[7]);
void addConn(int room, struct Room rooms[7]);
void shuffle(int array[]);
void writeRoomFiles(struct Room rooms[7], char *directory);


/*******************************************************
 * Function: createDir
 * Description: Creates directory where room files are stored
 * Parameters: None
 * Pre-Conditions: None
 * Post-Conditions: Directory created
********************************************************/
char *createDir() {

	int pid = getpid();
	char* dirAddress = "beyerda.rooms.";
	char *directory = malloc(30);

	sprintf(directory, "%s%d", dirAddress, pid);

	//Need to add 2nd arg 0700 for linux dev
	mkdir(directory, 0700);
    return directory;
}

/************************************************************
 * Function: createRooms
 * Description: Creates rooms and assigns names, connections, types
 * Parameters: array of rooms, directory, int
 * Pre-Conditions: Array of rooms created
 * Post-Conditions: Room structs populated with data
****************************************************************/
void createRooms(struct Room rooms[7]) {
    int i;

    //Add number of connections, type, and name data
    for(i = 0; i<7; i++) {
        rooms[i].numConnections = 0;
    }

    //Assign names, types, connections
    int a;
    for(a = 0; a<7; a++) {
        int taken, j, k;
        taken=1;
        while(taken) {
            j = rand()%10;
            taken = 0;
            for (k = 0; k < 7; k++) {
                if(rooms[k].name == roomNames[j]) {
                    taken = 1;
                }
            }
        }
        rooms[a].name = roomNames[j];

        //Here I am assigning the START_ROOM type to the first room
        if(a == 0) {
            rooms[a].type = roomType[0];
        }
        else if(a == 1) {
            rooms[a].type = roomType[1];
        }
        else {
            rooms[a].type = roomType[2];
        }
        //Randomizing the connections for each room
        int numCons = rand() % 4+3;
        while(rooms[a].numConnections < numCons) {
            addConn(a, rooms);
        }
    }


}

/********************************************************
 * Function: addConn()
 * Description: Add connections to room structs
 * Parameters: int c, rooms struct
 * Pre-Conditions: valid room struct
 * Post-Conditions: Each room struct has connections added
**************************************************************/
void addConn(int room, struct Room rooms[7]) {
    int connectingRoom, inValid, i;
    inValid = 1;

    //Testing to see if connection already exists or room connecting to itself
    while(inValid) {
            inValid = 0;
        connectingRoom = rand() % 7;
        if(connectingRoom == room) {
            inValid = 1;
        }

        for(i = 0; i<rooms[room].numConnections; i++) {
            if(rooms[room].connections[i] == connectingRoom) {
                inValid = 1;
            }
        }
    }
    //Connecting room is valid, so create connections
        rooms[room].connections[rooms[room].numConnections] = connectingRoom;
        rooms[room].numConnections++;
        rooms[connectingRoom].connections[rooms[connectingRoom].numConnections] = room;
        rooms[connectingRoom].numConnections++;

    }


/*********************************************************
 * Function: shuffle
 * Description: Shuffle elements of array to randomize room order
 * Source: http://stackoverflow.com/questions/6127503/shuffle-array-in-c
***********************************************************/
void shuffle(int array[]) {
    int i, n;
    n=10;
    for (i = 0; i<n; i++) {
        int j = i + (rand() % 9);
        int t = array[j];
        array[j] = array[i];
        array[i] = t;
    }

}

/****************************************************************************
 * Function: writeRoomFiles()
 * Description: Writes room files to "<STUDENT ONID USERNAME>.rooms.<PROCESS ID>" directory
 * Parameters: array of room structs, char directory
 * Pre-Conditions: array of structs and directory already exists
 * Post-Conditions: Files written to directory
***************************************************************************/
void writeRoomFiles(struct Room rooms[7], char *directory)
{
    //change directory
    chdir(directory);

    int i, j;

    //Open file for writing, write room name
    for(i = 0; i<7; i++) {

        FILE *fp = fopen(rooms[i].name, "w");
        fprintf(fp, "ROOM NAME: %s\n", rooms[i].name);

        //write connections
        for(j=0; j<rooms[i].numConnections; j++) {
            fprintf(fp, "CONNECTION %d: %s\n", j+1, rooms[rooms[i].connections[j]].name);
        }

        //Write room type
        fprintf(fp, "ROOM TYPE: %s\n", rooms[i].type);

        //Close file
        fclose(fp);
    }

}
int main()
{
    //Seed random number generator
    srand(time(NULL));

    char *roomsDir = createDir();
    struct Room rooms[7];
    createRooms(rooms);
    int i, j;
    for(i=0;i<7;i++) {
        //printf("%s\n", rooms[i].name);
       // printf("%d\n", rooms[i].numConnections);
        for(j=0; j<rooms[i].numConnections; j++) {
           // printf("%s", rooms[rooms[i].connections[j]].name);

        }
       // printf("\n");
       // printf("%s\n", rooms[i].type);
    }
    writeRoomFiles(rooms, roomsDir);

return 0;
}


