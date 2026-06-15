#include "ghost.h"

void initGhost(Ghost* ghost, House* house) {
    Room* room = pickRandomRoom(house);
    ghost->currRoom = room;
    room->ghost = ghost;

    ghost->ghostType = pickGhostType();
    ghost->ghostId = DEFAULT_GHOST_ID;
    ghost->boredomLevel = 0;
    ghost->Exited = false;

    ghost->caseFile = &(house->casefile);

    log_ghost_init(ghost->ghostId, ghost->currRoom->roomName, ghost->ghostType);
}


Room* pickRandomRoom(House* house) {
    int i = rand_int_threadsafe(0, house->room_count-1);
    return house->rooms+i;
}


GhostType pickGhostType() {
    const GhostType *list;
    int size = get_all_ghost_types(&list);
    int i = rand_int_threadsafe(0, size-1);
    return list[i];
}

void ghostAction(Ghost* ghost) {
    if (ghost->caseFile != NULL) {
        bool solved = false;
        sem_wait(&(ghost->caseFile->mutex));
        solved = ghost->caseFile->solved;
        sem_post(&(ghost->caseFile->mutex));

        if (solved) {
            if (ghost->currRoom != NULL) {
                sem_wait(&(ghost->currRoom->roomSemaphore));
                if (ghost->currRoom->ghost == ghost) {
                    ghost->currRoom->ghost = NULL;
                }
                sem_post(&(ghost->currRoom->roomSemaphore));
            }

            ghost->Exited = true;
            log_ghost_exit(ghost->ghostId,
                           ghost->boredomLevel,
                           ghost->currRoom ? ghost->currRoom->roomName : "");
            return;
        }
    }

    sem_wait(&(ghost->currRoom->roomSemaphore));
    printf("Ghost check: room %s has %d hunters\n",
           ghost->currRoom->roomName,
           ghost->currRoom->numOfHunters);

    if (ghost->currRoom->numOfHunters != 0) {
        ghost->boredomLevel = 0;
    } else {
        ghost->boredomLevel++;
    }
    sem_post(&(ghost->currRoom->roomSemaphore));

    if (ghost->boredomLevel >= ENTITY_BOREDOM_MAX) {
        if (ghost->currRoom != NULL) {
            sem_wait(&(ghost->currRoom->roomSemaphore));
            if (ghost->currRoom->ghost == ghost) {
                ghost->currRoom->ghost = NULL;
            }
            sem_post(&(ghost->currRoom->roomSemaphore));
        }

        ghost->Exited = true;
        log_ghost_exit(ghost->ghostId,
                       ghost->boredomLevel,
                       ghost->currRoom ? ghost->currRoom->roomName : "");
        return;
    }
    
    int decision = rand_int_threadsafe(0, 2);

    // 0: IDLE
    if (decision == 0) {
        log_ghost_idle(ghost->ghostId,
                       ghost->boredomLevel,
                       ghost->currRoom->roomName);
    }
    // 1: HAUNT (drop evidence)
    else if (decision == 1) {
        EvidenceByte evidence = pickRandomGhostEvidence(ghost);
        sem_wait(&(ghost->currRoom->roomSemaphore));
        SetEvidence(&(ghost->currRoom->roomEvidence), evidence);
        log_ghost_evidence(ghost->ghostId,
                           ghost->boredomLevel,
                           ghost->currRoom->roomName,
                           evidence);
        sem_post(&(ghost->currRoom->roomSemaphore));
    }
    // 2: MOVE (only if room has no hunters; lock both rooms alphabetically)
    else {
        sem_wait(&(ghost->currRoom->roomSemaphore));
        if (ghost->currRoom->numOfHunters != 0) {
            sem_post(&(ghost->currRoom->roomSemaphore));
            return;
        }
        sem_post(&(ghost->currRoom->roomSemaphore));

        Room* newRoom = pickConnectedRoom(ghost->currRoom);

        if (pickRoomsAlphabetically(ghost->currRoom, newRoom)) {
            sem_wait(&(ghost->currRoom->roomSemaphore));
            sem_wait(&(newRoom->roomSemaphore));
        } else {
            sem_wait(&(newRoom->roomSemaphore));
            sem_wait(&(ghost->currRoom->roomSemaphore));
        }

        ghost->currRoom->ghost = NULL;
        log_ghost_move(ghost->ghostId,
                       ghost->boredomLevel,
                       ghost->currRoom->roomName,
                       newRoom->roomName);
        Room* oldCurrRoom = ghost->currRoom;
        ghost->currRoom = newRoom;
        ghost->currRoom->ghost = ghost;

        sem_post(&(oldCurrRoom->roomSemaphore));
        sem_post(&(ghost->currRoom->roomSemaphore));
    }
}



EvidenceByte pickRandomGhostEvidence(Ghost* ghost) {
    EvidenceByte allBits[3];
    getAllGhostEvidence(ghost->ghostType, allBits);

    int i = rand_int_threadsafe(0, 2);

    return allBits[i];
}

void getAllGhostEvidence (GhostType ghostType, EvidenceByte* seperatedBits) {
    int currI = 0;
    const EvidenceType* list;
    int size = get_all_evidence_types(&list);
    
    for (int i=0; i<size; i++) {
        if ((list[i] & ghostType) != 0) seperatedBits[currI++] = list[i];
    }

}


