#include "room.h"

// Room Functions
void room_init(Room* room, const char* name, bool is_exit) {
    strncpy(room->roomName, name, MAX_ROOM_NAME);
    room->roomName[MAX_ROOM_NAME - 1] = '\0';

    room->ghost = NULL;
    for (int i = 0; i < MAX_CONNECTIONS; ++i) {
        room->connectedRooms[i] = NULL;
        room->hunters[i]        = NULL;
    }

    room->numOfHunters = 0;
    room->numOfConnectedRooms = 0;
    room->isExit = is_exit;
    room->roomEvidence = 0;

    if (sem_init(&(room->roomSemaphore), 0, 1) < 0) {
        printf("ERROR ON ROOM SEMAPHORE INIT\n");
    }
}


void room_connect(Room* a, Room* b) {
    a->connectedRooms[a->numOfConnectedRooms++] = b;
    b->connectedRooms[b->numOfConnectedRooms++] = a;
}



// RoomStack functions
void roomstack_init(RoomStack* stack) {
    stack->head = NULL;
}


void roomstack_push(RoomStack* stack, Room* room) {
    RoomNode* newNode = calloc(1, sizeof(RoomNode));
    if (!newNode) {
        printf("RoomNode Allocation failed in roomstack_push!\n");
        return;
    } else {
        newNode->room = room;
        newNode->next = NULL;
    }
    
    if (roomstack_is_empty(stack)) {
        stack->head = newNode;
        return;
    } 

    newNode->next = stack->head;
    stack->head = newNode;
    return;

}

Room* roomstack_pop(RoomStack* stack) {
    if (roomstack_is_empty(stack)) {
        return NULL;
    }

    Room* room = stack->head->room;

    if (stack->head->next == NULL) {
        free(stack->head);
        stack->head = NULL;
        return room;
    }

    RoomNode* next = stack->head->next;
    free(stack->head);
    stack->head = next;

    return room;
}

bool roomstack_is_empty(RoomStack* stack) {
    if (stack->head == NULL) return true;
    return false;
}

void roomstack_cleanup(RoomStack* stack) {
    if (roomstack_is_empty(stack)) return;
    RoomNode* next = NULL;

    while(stack->head != NULL) {
        next = stack->head->next;
        free(stack->head);
        stack->head = next;
    }
}
