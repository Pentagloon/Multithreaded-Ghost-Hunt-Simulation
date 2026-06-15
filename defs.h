#ifndef DEFS_H
#define DEFS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <semaphore.h>
#include <pthread.h>


/*
    You are free to rename all of the types and functions defined here.

    The ghost ID must remain the same for the validator to work correctly.
*/

#define MAX_ROOM_NAME 64
#define MAX_HUNTER_NAME 64
#define MAX_ROOMS 24
#define MAX_ROOM_OCCUPANCY 8
#define MAX_CONNECTIONS 8
#define ENTITY_BOREDOM_MAX 15
#define HUNTER_FEAR_MAX 15
#define DEFAULT_GHOST_ID 68057

typedef unsigned char EvidenceByte; // Just giving a helpful name to unsigned char for evidence bitmasks

typedef struct CaseFile CaseFile;
typedef struct Ghost Ghost;
typedef struct Room Room;
typedef struct RoomNode RoomNode;
typedef struct RoomStack RoomStack;
typedef struct Hunter Hunter;
typedef struct HunterArray HunterArray;
typedef struct House House;


typedef enum LogReason {
    LR_EVIDENCE = 0,
    LR_BORED = 1,
    LR_AFRAID = 2
} LogReason;

typedef enum EvidenceType {
    EV_EMF          = 1 << 0,
    EV_ORBS         = 1 << 1,
    EV_RADIO        = 1 << 2,
    EV_TEMPERATURE  = 1 << 3,
    EV_FINGERPRINTS = 1 << 4,
    EV_WRITING      = 1 << 5,
    EV_INFRARED     = 1 << 6,
} EvidenceType;

typedef enum GhostType {
    GH_POLTERGEIST  = EV_FINGERPRINTS | EV_TEMPERATURE | EV_WRITING,
    GH_THE_MIMIC    = EV_FINGERPRINTS | EV_TEMPERATURE | EV_RADIO,
    GH_HANTU        = EV_FINGERPRINTS | EV_TEMPERATURE | EV_ORBS,
    GH_JINN         = EV_FINGERPRINTS | EV_TEMPERATURE | EV_EMF,
    GH_PHANTOM      = EV_FINGERPRINTS | EV_INFRARED    | EV_RADIO,
    GH_BANSHEE      = EV_FINGERPRINTS | EV_INFRARED    | EV_ORBS,
    GH_GORYO        = EV_FINGERPRINTS | EV_INFRARED    | EV_EMF,
    GH_BULLIES      = EV_FINGERPRINTS | EV_WRITING     | EV_RADIO,
    GH_MYLING       = EV_FINGERPRINTS | EV_WRITING     | EV_EMF,
    GH_OBAKE        = EV_FINGERPRINTS | EV_ORBS        | EV_EMF,
    GH_YUREI        = EV_TEMPERATURE  | EV_INFRARED    | EV_ORBS,
    GH_ONI          = EV_TEMPERATURE  | EV_INFRARED    | EV_EMF,
    GH_MOROI        = EV_TEMPERATURE  | EV_WRITING     | EV_RADIO,
    GH_REVENANT     = EV_TEMPERATURE  | EV_WRITING     | EV_ORBS,
    GH_SHADE        = EV_TEMPERATURE  | EV_WRITING     | EV_EMF,
    GH_ONRYO        = EV_TEMPERATURE  | EV_RADIO       | EV_ORBS,
    GH_THE_TWINS    = EV_TEMPERATURE  | EV_RADIO       | EV_EMF,
    GH_DEOGEN       = EV_INFRARED     | EV_WRITING     | EV_RADIO,
    GH_THAYE        = EV_INFRARED     | EV_WRITING     | EV_ORBS,
    GH_YOKAI        = EV_INFRARED     | EV_RADIO       | EV_ORBS,
    GH_WRAITH       = EV_INFRARED     | EV_RADIO       | EV_EMF,
    GH_RAIJU        = EV_INFRARED     | EV_ORBS        | EV_EMF,
    GH_MARE         = EV_WRITING      | EV_RADIO       | EV_ORBS,
    GH_SPIRIT       = EV_WRITING      | EV_RADIO       | EV_EMF,
} GhostType;


typedef struct CaseFile {
    EvidenceByte collected;
    bool         solved;
    sem_t        mutex; 
} CaseFile;


struct Room {
    char roomName[MAX_ROOM_NAME];
    Ghost* ghost;

    Room* connectedRooms[MAX_CONNECTIONS];
    int numOfConnectedRooms;

    Hunter* hunters[MAX_CONNECTIONS];
    int numOfHunters;

    bool isExit;
    EvidenceByte roomEvidence;

    sem_t roomSemaphore;
};


struct RoomNode {
    Room* room;
    struct RoomNode* next;
};


struct RoomStack {
    RoomNode* head;
};


struct Ghost {
    GhostType ghostType;
    Room* currRoom;
    CaseFile* caseFile; 

    int ghostId;
    int boredomLevel;
    bool Exited;
};


struct Hunter {
    char hunterName[MAX_HUNTER_NAME];
    int hunterId;
    int fear;
    int boredom;
    Room* currRoom;
    CaseFile* caseFile; 

    EvidenceType hunterDevice;
    RoomStack vistitedRoomsStack;
    LogReason reasonForExiting;
    bool returnToVan;
    bool Exited;

};


struct HunterArray {
    Hunter* hunters;
    int size;
    int capacity;
}; 




struct House { 
    Ghost ghost;
    HunterArray hunterArray;
    Room rooms[MAX_ROOMS];
    CaseFile casefile;

    Room* starting_room;
    int room_count;
};


/* The provided `house_populate_rooms()` function requires the following functions.
   You are free to rename them and change their parameters and modify house_populate_rooms()
   as needed as long as the house has the correct rooms and connections after calling it.
*/



#endif
