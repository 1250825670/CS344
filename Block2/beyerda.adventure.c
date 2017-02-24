/*********************************************************************
**
** Author: Daniel Beyer
** Date: 02/07/17
** Description: CS344 Program 2 - beyerda.adventure.c  This program is the interface for the adventure
** game presented to the player after the rooms have been generated.
** In the game, the player will begin in the "starting room" and will win the game upon entering the
** "ending room", which causes the game to exit and displaying the path taken by the player.  During
** the game, the player can also enter a command that returns the current time utilizing mutexes and multithreading.
**
*************************************************************************/
#include <fcntl.h>
#include <time.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>



//Arrays to hold RoomTypes and RoomNames
const char* roomType[3] = {"START_ROOM", "END_ROOM", "MID_ROOM"};

const char* roomNames[10] = {"Bridge", "Ready_Room", "Crew_Quarters",
			    "Ten_Forward", "Engineering", "Holodeck",
			    "Transporter_Room", "Sick_Bay", "Cargo_Bay",
			    "Security"};

//index to store threads and mutex lock
pthread_mutex_t mutex;

/*********************************************************
 * Struct for rooms
 ********************************************************/
struct Room {
	const char* name;
	const char* type;
	int connections[6];
	int numConnections;
};

/****************************************************
 * Function prototypes
****************************************************/
char *getDir();
const char* getStartRoom();
void play_game(char startRoom[50]);
//int getRoomData(struct Room *room, char roomName[50]);


/************************************************************
 * Function: writeTime()
 * Description: Write current time to file currentTime.txt
 * Parameters: None
 * Pre-Conditions: None
 * Post-Conditions: Time.txt written
 * Source: http://stackoverflow.com/questions/7411301/how-to-introduce-date-and-time-in-log-file
 ************************************************************/
 void* writeTime() {


    FILE *fp;
    fp = fopen("currentTime.txt", "w+");
    char buff[50];
    struct tm *sTm;

    //These time structs are from the source above, it seems to work well
    time_t now = time (0);
    sTm = gmtime (&now);

    strftime (buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", sTm);
    //printf("%s", buff);
    fputs(buff, fp);
    fclose(fp);

    //return 0;
}


 /************************************************************
  * Function: printTime()
  * Description: Prints contents of currentTime.txt file
  * Parameters: None
  * Pre-Conditions: Valid time.txt file
  * Post-Conditions: time.txt contents printed to screen
*************************************************************/
void printTime() {

    char buffer[50];
    FILE *fp;
    fp = fopen("currentTime.txt", "r");
    if(fp == NULL) {
        perror("Error\n");
    }
    else {
            //print buffer line
        fgets(buffer, 50, fp);
        printf("\n%s\n", buffer);
        fclose(fp);
    }

}

/***************************************************************
 * Function: p_thread2()
 * Description: Initializes second thread and call writeTime and handles locking and unlocking
 * Parameters: None
 * Pre-Conditions: None
 * Post-Conditions: None
 * Source: http://homes.di.unimi.it/~boccignone/GiuseppeBoccignone_webpage/MatDidatSOD2009_files/pthread-Tutorial.pdf
 ***************************************************************/
 void p_thread2() {
     pthread_t thread2;
     pthread_mutex_init(&mutex, NULL);
     pthread_mutex_lock(&mutex);
     //thread creation
     int tid = pthread_create(&thread2, NULL, writeTime, NULL);
     pthread_mutex_unlock(&mutex);
     pthread_mutex_destroy(&mutex);
     //Sleep function for unlocking time
     usleep(50);
 }

/******************************************************
 * Function: getDir()
 * Description:  Returns a pointer to the newest created directory
 * Sources: http://stackoverflow.com/questions/25310066/getting-the-last-created-modified-file
 * http://stackoverflow.com/questions/12489/how-do-you-get-a-directory-listing-in-c
**********************************************************/
char *getDir() {
    time_t t;
    int i = 0;
    char *buffer = malloc(sizeof(char) * 64);
    DIR* dirp = opendir("./");
    struct dirent* dp;
    struct stat statbuf;

    if(dirp != NULL)
    {
        while(dp = readdir(dirp)) {

        //I had some trouble identifying only directories at first
        //Using S_ISDIR to verify the object is a directory instead of a file
        if(stat(dp->d_name, &statbuf) == 0 && S_ISDIR(statbuf.st_mode) &&
           strncmp(dp->d_name, "beyerda.rooms", 13)==0) {

            t = statbuf.st_mtime;
            if(t>i) {
                strcpy(buffer, dp->d_name);
                i = t;

            }
        }
    }
    closedir(dirp);

    }
    return buffer;
}



/**************************************************************
 * Function: getRoomData()
 * Description: Reads in room data from individual room files
 * Parameters: struct Room, roomName of start room
 * Pre-Conditions: Valid room files in directory
 * Post-Conditions: Room information returned to calling function
*******************************************************************/
int getRoomData(struct Room* room, char roomName[50]) {
    int i,j;

    char dir[100];
    sprintf(dir, getDir());

    char filename[100];
    //Concatenate full directory name using starting room
    sprintf(filename, "./%s/%s", dir, roomName);

    //Opening file
    FILE *fp;
    fp = fopen(filename, "r");
    if(fp == NULL)
        return -1;

    //Initializing initial data values to zero
    room->numConnections = 0;
    room->name = roomNames[0];

    char line[100];

    //Finding and assigning room name value using strtok using space as delimiter
    fgets(line, 100, fp);

    char *name = strtok(line, " ");
    name = strtok(NULL, " ");
    name = strtok(NULL, "\n");

    for(i = 0; i<10; i++) {
        if(strcmp(name, roomNames[i]) == 0) {
            room->name = roomNames[i];
            break;
        }
    }

    //Finding and assigning connections using space as delimiter
    while(fgets(line, 100, fp) != NULL) {
        char *token = strtok(line, " ");
        if(strcmp(token, "CONNECTION") == 0) {
            token = strtok(NULL, " ");
            token = strtok(NULL, "\n");

            for(i = 0; i<10; i++) {
                if(strcmp(token, roomNames[i]) == 0) {
                    room->connections[room->numConnections] = i;
                    break;
                }
            }
            room->numConnections++;
        }

        //Finally finding room type in a similar manner
        else {
            token = strtok(NULL, " ");
            token = strtok(NULL, "\n");

            for(i = 0; i<3; i++) {
                if(strcmp(token, roomType[i]) == 0) {
                    room->type = roomType[i];
                    break;
                }
            }
        }
    }
   return 0;

}

/*********************************************************************
 * Function: getStartRoom()
 * Description: This function returns as char array the starting room
 * Parameters: None
 * Pre-Conditions: Valid starting room file
 * Post-Conditions: None
*********************************************************************/
const char* getStartRoom() {

    char *directory = getDir();
    char *startRoom = malloc(sizeof(char) * 20);

    DIR *dir;
    struct dirent *ent;

    //I sometimes get an error stating:
    //"return makes integer from pointer without a cast", not sure how to solve it yet
    if((dir = opendir(directory)) != NULL) {
        while((ent = readdir(dir)) != NULL) {

            if(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")) {
                continue;
            }

            //The way the files were saved, the first file will always be the starting room
            strcpy(startRoom,ent->d_name);
            return startRoom;
            printf("%s", ent->d_name);
            break;

        }
        closedir(dir);
        free(directory);

    }else {
    perror("");
    return EXIT_FAILURE;}

}

/***********************************************************************
* Function: play_game()
* Description: This is the main user interface for the game.  It calls individual
* room files for interpretation
* Parameters: char array for starting room
* Pre-Conditions: Directory has valid room files
* Post-Conditions: Game concludes
***********************************************************************/
void play_game(char startRoom[50]) {

    int i, steps = 0, endingRoom = 0;
    char roomName[50];
    const char* path[500];

    // Allocate memory for room
    struct Room* room = malloc(sizeof(struct Room));

    // Get initial starting room data
    i = getRoomData(room,startRoom);
    do {

        int conTest = 0;
        //Print user interface and room data
        printf("\nCURRENT LOCATION: %s\n",room->name);
        printf("POSSIBLE CONNECTIONS:");
        printf(" %s",roomNames[room->connections[0]]);
        for (i = 1; i < room->numConnections; i++) {
            printf(", %s",roomNames[room->connections[i]]);
        }


        //Prompting user for new room with validity checks
        printf(".\nWHERE TO? >");
        scanf("%s",roomName);


       if(strcmp(roomName, "time") == 0) {
            p_thread2();
            printTime();
        }

         else if (isConnected(room, roomName) == 0)
            printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n");

        // Add room name to path array and incrememnt steps
        else {
           path[steps] = room->name;
           steps++;
           i = getRoomData(room,roomName);
       }

        //If room type is End Room, set endingRoom to 1 to exit while loop
        if(room->type == "END_ROOM"){
            endingRoom = 1;

        }
    } while(!endingRoom);

    //End of game message
    printf("\nYOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
    printf("YOU TOOK %i STEPS. YOUR PATH TO VICTORY WAS:\n",steps);
    for(i = 0; i < steps; i++)
        printf("%s\n",path[i]);

    // Free up room memory
    free(room);


}

/***********************************************************
* Function: isConnected()
* Description: Helper function to facilitate finding if user entered a valid connection
********************************************************************/
int isConnected(struct Room *room, char roomName[30]) {
    	int j;
	for (j = 0; j < room->numConnections; j++) {
		if (strcmp(roomNames[room->connections[j]], roomName) == 0)
			return 1;
	}

	// If not found, return false
	return 0;
}


int main(){

//Simple introduction
printf("Welcome to Dan Beyer's Adventure game!\n");

//Finding name of starting room
char startRoom[30];
strcpy(startRoom,getStartRoom());

//User interface
play_game(startRoom);


return 0;
}
