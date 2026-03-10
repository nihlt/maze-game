#ifndef ENEMY_LOGIC_H
#define ENEMY_LOGIC_H

#include "GameTypes.h"
#include <vector>

void UpdateEntities(GameState& gs);
void ComputePlayerDistMap(const GameState& gs, int distMap[MH][MW]);

#endif // ENEMY_LOGIC_H
