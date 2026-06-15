#include "house.h"
#include "hunter.h"


void initHouse(House* house) {
    if (!initHunterArray(&(house->hunterArray))) {
        printf("initHunterArray failed (inside of initHouse)\n");
        return;
    };

    // Populate rooms
    house_populate_rooms(house);

    // Initialize case file
    house->casefile.collected = 0;
    house->casefile.solved = false;
    if (sem_init(&(house->casefile.mutex), 0, 1) < 0) {
        printf("ERROR ON SEM INIT FOR CASEFILE INSIDE HOUSE\n");
    }
}



bool cleanupHouse(House* house) {
    if(!cleanupHunterArray(&(house->hunterArray))) return false;
    for (int i = 0; i<house->room_count; i++) {
        sem_destroy(&(house->rooms[i].roomSemaphore));
    }
    sem_destroy (&(house->casefile.mutex));

    return true;
}
