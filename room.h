#ifndef ROOM_H
#define ROOM_H

#include "defs.h"

//Room Functions
void room_init(Room* room, const char* name, bool is_exit);
void room_connect(Room* a, Room* b);


//RoomStack functions
void roomstack_init(RoomStack* stack);
void roomstack_push(RoomStack* stack, Room* room);
Room* roomstack_pop(RoomStack* stack);
bool roomstack_is_empty(RoomStack* stack);
void roomstack_cleanup(RoomStack* stack);



#endif