#ifndef GHOST_H
#define GHOST_H

#include "defs.h"
#include "helpers.h"

void initGhost(Ghost* ghost, House* house);
GhostType pickGhostType();
EvidenceByte pickRandomGhostEvidence(Ghost* ghost);
void getAllGhostEvidence (GhostType ghostType, EvidenceByte* seperatedBits);

Room* pickRandomRoom(House* house);

void ghostAction(Ghost* ghost);


#endif