#ifndef HUNTER_H
#define HUNTER_H

#include "defs.h"
#include "helpers.h"

// Hunter Functions
void initHunter(Hunter* hunter, House* house, const char* name, int id);
EvidenceType selectRandomEvidenceDevice(void);

void addToRoomStack(Hunter* hunter, Room* room);
Room* popFromRoomStack(Hunter* hunter);

bool removeHunterFromRoom(Room* room, Hunter* hunter);

void hunterAction(Hunter* hunter);
void moveToNewRoom(Hunter* hunter);
void checkForEvidence(Hunter* hunter);
bool insideExitProcedure(Hunter* hunter);
bool evidenceIsValid(Hunter* hunter);
bool checkBoredomAndFear(Hunter* hunter);

bool initHunterArray(HunterArray* array);
Hunter* addHunterToArray(HunterArray* array);
bool cleanupHunterArray(HunterArray* array);

#endif
