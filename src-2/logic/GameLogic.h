#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include "GameTypes.h"

void ResetGame(GameState& gs);
void RevealLineOfSight(GameState& gs);
void TryMove(GameState& gs, int forwardDelta, bool dash = false);
void ComputePlayerDistMap(const GameState& gs, int distMap[MH][MW]);

void TriggerArchitect(GameState& gs, int anger, int aggress, int malice, const std::string& text, int waitTimeMs);

#endif // GAME_LOGIC_H
