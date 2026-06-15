#include "hunter.h"
#include <string.h>


void initHunter(Hunter* hunter, House* house, const char* name, int id) {
    if (name) {
        strncpy(hunter->hunterName, name, MAX_HUNTER_NAME);
        hunter->hunterName[MAX_HUNTER_NAME - 1] = '\0';
    } else {
        hunter->hunterName[0] = '\0';
    }

    hunter->hunterId = id;
    hunter->fear = 0;
    hunter->boredom = 0;

    hunter->currRoom = house->starting_room;
    hunter->caseFile = &(house->casefile);

    hunter->returnToVan = false;
    hunter->reasonForExiting = 100;
    hunter->hunterDevice = selectRandomEvidenceDevice();
    hunter->Exited = false;

    roomstack_init(&(hunter->vistitedRoomsStack));

    Room* van = house->starting_room;
    if (van->numOfHunters < MAX_ROOM_OCCUPANCY) {
        van->hunters[van->numOfHunters++] = hunter;
    }

    log_hunter_init(hunter->hunterId,
                    hunter->currRoom->roomName,
                    hunter->hunterName,
                    hunter->hunterDevice);
}

EvidenceType selectRandomEvidenceDevice(void) {
    const EvidenceType* device_list;
    int size = get_all_evidence_types(&device_list);
    int i    = rand_int_threadsafe(0, size - 1);
    return device_list[i];
}

void addToRoomStack(Hunter* hunter, Room* room) {
    roomstack_push(&(hunter->vistitedRoomsStack), room);
}

Room* popFromRoomStack(Hunter* hunter) {
    return roomstack_pop(&(hunter->vistitedRoomsStack));
}


bool checkBoredomAndFear(Hunter* hunter) {
    if (hunter->boredom >= ENTITY_BOREDOM_MAX ||
        hunter->fear >= HUNTER_FEAR_MAX) {

        hunter->Exited = true;

        if (hunter->boredom >= ENTITY_BOREDOM_MAX) hunter->reasonForExiting = LR_BORED;
        
        if (hunter->fear >= HUNTER_FEAR_MAX) hunter->reasonForExiting = LR_AFRAID;
        

        sem_wait(&(hunter->currRoom->roomSemaphore));
        removeHunterFromRoom(hunter->currRoom, hunter);
        sem_post(&(hunter->currRoom->roomSemaphore));

        log_exit(hunter->hunterId,
                 hunter->boredom,
                 hunter->fear,
                 hunter->currRoom->roomName,
                 hunter->hunterDevice,
                 hunter->reasonForExiting);

        return true;
    }
    return false;
}

bool evidenceIsValid(Hunter* hunter) {
    bool solved = false;

    sem_wait(&(hunter->caseFile->mutex));
    if (hunter->caseFile->solved ||
        evidence_is_valid_ghost(hunter->caseFile->collected)) {

        hunter->caseFile->solved = true;
        solved = true;
    }
    sem_post(&(hunter->caseFile->mutex));

    if (!solved) {
        return false;
    }

    hunter->Exited = true;
    hunter->reasonForExiting = LR_EVIDENCE;

    sem_wait(&(hunter->currRoom->roomSemaphore));
    removeHunterFromRoom(hunter->currRoom, hunter);
    sem_post(&(hunter->currRoom->roomSemaphore));

    log_exit(hunter->hunterId, hunter->boredom, hunter->fear, hunter->currRoom->roomName, hunter->hunterDevice, hunter->reasonForExiting);

    return true;
}

bool insideExitProcedure(Hunter* hunter) {

    roomstack_cleanup(&(hunter->vistitedRoomsStack));

    if (hunter->returnToVan) {
        log_return_to_van(hunter->hunterId,
                          hunter->boredom,
                          hunter->fear,
                          hunter->currRoom->roomName,
                          hunter->hunterDevice,
                          false); 
        hunter->returnToVan = false;
    }

    if (evidenceIsValid(hunter)) {
        return true;
    }

    EvidenceType newDevice = selectRandomEvidenceDevice();
    log_swap(hunter->hunterId,
             hunter->boredom,
             hunter->fear,
             hunter->hunterDevice,
             newDevice);

    hunter->hunterDevice = newDevice;
    return false;
}


void checkForEvidence(Hunter* hunter) {
    sem_wait(&(hunter->caseFile->mutex));
    sem_wait(&(hunter->currRoom->roomSemaphore));

    bool inExit = hunter->currRoom->isExit;

    if (hunter->currRoom->roomEvidence & hunter->hunterDevice) {
        log_evidence(hunter->hunterId,
                     hunter->boredom,
                     hunter->fear,
                     hunter->currRoom->roomName,
                     hunter->hunterDevice);

        clearEvidence(&(hunter->currRoom->roomEvidence), hunter->hunterDevice);
        SetEvidence(&(hunter->caseFile->collected), hunter->hunterDevice);

        if (evidence_is_valid_ghost(hunter->caseFile->collected)) {
            hunter->caseFile->solved = true;
        }

        if (!inExit) {
            hunter->returnToVan = true;
        }

        sem_post(&(hunter->currRoom->roomSemaphore));
        sem_post(&(hunter->caseFile->mutex));

        if (!inExit) {
            log_return_to_van(hunter->hunterId,
                              hunter->boredom,
                              hunter->fear,
                              hunter->currRoom->roomName,
                              hunter->hunterDevice,
                              true);
        }
    } 
    else {

        int random = rand_int_threadsafe(0, 20);
        if (random == 1 && !hunter->currRoom->isExit) {
            hunter->returnToVan = true;

            sem_post(&(hunter->currRoom->roomSemaphore));
            sem_post(&(hunter->caseFile->mutex));

            log_return_to_van(hunter->hunterId,
                              hunter->boredom,
                              hunter->fear,
                              hunter->currRoom->roomName,
                              hunter->hunterDevice,
                              true);
        } else {
            sem_post(&(hunter->currRoom->roomSemaphore));
            sem_post(&(hunter->caseFile->mutex));
        }
    }
}


void moveToNewRoom(Hunter* hunter) {
    Room* from = hunter->currRoom;
    Room* to = NULL;
    Room* breadcrumb = NULL;

    if (hunter->returnToVan) {
        breadcrumb = popFromRoomStack(hunter);

        if (breadcrumb == NULL) {
            hunter->returnToVan = false;
            return;
        }
        to = breadcrumb;
    } else {
        to = pickConnectedRoom(from);
    }

    if (to == NULL || to == from) return;
    

    Room* first  = from;
    Room* second = to;
    if (!pickRoomsAlphabetically(from, to)) {
        first  = to;
        second = from;
    }

    sem_wait(&(first->roomSemaphore));
    sem_wait(&(second->roomSemaphore));

    if (to->numOfHunters >= MAX_ROOM_OCCUPANCY) {
        if (hunter->returnToVan && breadcrumb != NULL) {
            roomstack_push(&(hunter->vistitedRoomsStack), breadcrumb);
        }

        sem_post(&(second->roomSemaphore));
        sem_post(&(first->roomSemaphore));
        return;
    }

    if (!hunter->returnToVan) {
        roomstack_push(&(hunter->vistitedRoomsStack), from);
    }

    removeHunterFromRoom(from, hunter); 
    to->hunters[to->numOfHunters++] = hunter;   

    log_move(hunter->hunterId,
             hunter->boredom,
             hunter->fear,
             from->roomName,
             to->roomName,
             hunter->hunterDevice);

    hunter->currRoom = to;

    sem_post(&(second->roomSemaphore));
    sem_post(&(first->roomSemaphore));
}

void hunterAction(Hunter* hunter) {
    sem_wait(&(hunter->currRoom->roomSemaphore));
    if (hunter->currRoom->ghost != NULL) {
        hunter->boredom = 0;
        hunter->fear++;
    } else {
        hunter->boredom++;
    }
    sem_post(&(hunter->currRoom->roomSemaphore));

    if (hunter->currRoom->isExit) {
        if (insideExitProcedure(hunter)) {
            return;
        }
    }

    if (checkBoredomAndFear(hunter)) {
        return;
    }

    if (!hunter->returnToVan) {
        checkForEvidence(hunter);
    }

    moveToNewRoom(hunter);
}


bool removeHunterFromRoom(Room* room, Hunter* hunter) {
    int removeIndex = -1;

    for (int i = 0; i < room->numOfHunters; i++) {
        if (room->hunters[i] == hunter) {
            removeIndex = i;
            break;
        }
    }

    if (removeIndex == -1)  {
        if (room->isExit) {
            return true;
        }

        return false; // hunter not found in a non-exit room
    }

    for (int i = removeIndex; i < room->numOfHunters - 1; i++) {
        room->hunters[i] = room->hunters[i + 1];
    }

    room->hunters[room->numOfHunters - 1] = NULL;
    room->numOfHunters--;

    return true;
}


bool initHunterArray(HunterArray* array) {
    array->size     = 0;
    array->capacity = 1;

    array->hunters = calloc(array->capacity, sizeof(Hunter));
    if (array->hunters == NULL) {
        array->capacity = 0;
        return false;
    }
    return true;
}

static void resizeHunterArray(HunterArray* array) {
    int newCapacity = array->capacity * 2;
    Hunter* tmpArr  = calloc(newCapacity, sizeof(Hunter));
    if (!tmpArr) {
        printf("resizeHunterArray: allocation failed\n");
        return;
    }

    for (int i = 0; i < array->size; i++) {
        tmpArr[i] = array->hunters[i];
    }

    free(array->hunters);
    array->hunters  = tmpArr;
    array->capacity = newCapacity;
}

Hunter* addHunterToArray(HunterArray* array) {
    if (array->size == array->capacity) {
        resizeHunterArray(array);
    }

    Hunter* slot = &(array->hunters[array->size]);
    memset(slot, 0, sizeof(Hunter));
    array->size++;

    return slot;
}

bool cleanupHunterArray(HunterArray* array) {
    for (int i = 0; i < array->size; i++) {
        roomstack_cleanup(&(array->hunters[i].vistitedRoomsStack));
    }
    free(array->hunters);
    array->hunters = NULL;
    array->size    = 0;
    array->capacity = 0;

    return true;
}
