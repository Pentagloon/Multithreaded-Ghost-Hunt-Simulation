#include <signal.h>
#include <unistd.h>

#include "defs.h"
#include "helpers.h"
#include "house.h"
#include "ghost.h"
#include "hunter.h"
#include "room.h"

void* HunterThread(void *hunterPtr);
void* GhostThread(void *ghostPtr);

static bool hunters_won(const House* house);

int main(int argc, char* argv[]) {
    /*
      1. Initialize a House structure.
      2. Populate the House with rooms.
      3. Read all hunters (names + IDs) into a dynamic array, OR use default
         hunters when running in batch/report mode.
      4. Initialize the ghost and then each hunter.
      5. Create threads for the ghost and each hunter.
      6. Wait for all threads to complete.
      7. Print final results and (optionally) send SIGUSR1 / SIGUSR2 to the
         parent process when --report is used.
    */

    House house1 = {0};
    initHouse(&house1);

    bool reportMode = false;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--report") == 0) {
            reportMode = true;
        }
    }

    printf("Enter hunters 1 at a time. Type 'done' as the name to finish.\n");

    char name[MAX_HUNTER_NAME];
    int  id = 0;

    while (1) {
        printf("Enter hunter's name (max 63 chars): \n");
        if (scanf("%63s", name) != 1) {
            break;
        }

        if (!strcmp(name, "done") || !strcmp(name, "Done")) {
            break;
        }

        printf("Enter hunter's ID (int): \n");
        if (scanf("%d", &id) != 1) {
            break;
        }

        Hunter* specHunter = addHunterToArray(&(house1.hunterArray));
        strncpy(specHunter->hunterName, name, MAX_HUNTER_NAME);
        specHunter->hunterName[MAX_HUNTER_NAME - 1] = '\0';
        specHunter->hunterId = id;
    }

    if (house1.hunterArray.size == 0) {
        printf("no hunters specified. Exiting.\n");
        cleanupHouse(&house1);
        return 0;
    }

    initGhost(&(house1.ghost), &house1);

    printf("\n\n\n********************NOW, WE START THE GAME************************\n\n\n");

    for (int i = 0; i < house1.hunterArray.size; ++i) {
        Hunter* h = &(house1.hunterArray.hunters[i]);
        initHunter(h, &house1, h->hunterName, h->hunterId);
    }

    int numOfHunters = house1.hunterArray.size;
    pthread_t* hunterThreads = calloc(numOfHunters, sizeof(pthread_t));
    pthread_t ghostThread;

    pthread_create(&ghostThread, NULL, GhostThread, &(house1.ghost));

    for (int i = 0; i < numOfHunters; i++) {
        pthread_create(hunterThreads + i,
                       NULL,
                       HunterThread,
                       house1.hunterArray.hunters + i);
    }

    for (int i = 0; i < numOfHunters; i++) {
        pthread_join(hunterThreads[i], NULL);
    }
    pthread_join(ghostThread, NULL);

    displayInvestigationResults(&house1);
    displayVictoryResults(&house1);

    bool huntersWin = hunters_won(&house1);

    if (reportMode) {
        pid_t ppid = getppid();
        int sig = huntersWin ? SIGUSR1 : SIGUSR2;

        if (ppid > 1) {
            kill(ppid, sig);
        }
    }

    cleanupHouse(&house1);

    free(hunterThreads);

    return 0;
}

static bool hunters_won(const House* house) {
    EvidenceByte mask = house->casefile.collected;

    if (!evidence_is_valid_ghost(mask)) {
        return false;
    }

    if (mask != (EvidenceByte)house->ghost.ghostType) {
        return false;
    }

    for (int i = 0; i < house->hunterArray.size; ++i) {
        const Hunter* h = &(house->hunterArray.hunters[i]);
        if (h->reasonForExiting == LR_EVIDENCE) {
            return true;
        }
    }

    return false;
}

void* HunterThread(void *hunterPtr) {
    Hunter* hunter = (Hunter*)hunterPtr;
    while (!hunter->Exited) {
        hunterAction(hunter);
    }
    return NULL;
}

void* GhostThread(void *ghostPtr) {
    Ghost* ghost = (Ghost*)ghostPtr;
    while (!ghost->Exited) {
        ghostAction(ghost);
    }
    return NULL;
}
