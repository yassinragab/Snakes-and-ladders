#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include <vector>
#include <string>
#include "GameConstants.h"
#include "adj_list.h"
#include "BFS.h"

struct PlayerStats {
    int diceRolls = 0;
    int snakesHit = 0;
    int laddersClimbed = 0;
    int turnsTaken = 0;
};

class GameEngine
{
public:
    GameEngine();
    ~GameEngine();

    // Core Game Actions
    void initGame(int nPlayers, int nSnakes, int nLadders);
    void reset();

    // Returns the final position after move logic
    // hitType output: 0=none, 1=snake, 2=ladder
    int rollDiceForPlayer(int playerIndex, int& diceValue, int& hitType, int& startPos);

    // Updates specific player position (used for animation)
    void setPlayerPosition(int playerIndex, int pos);

    // Board Editing
    bool addSnake(int head, int tail);
    bool addLadder(int bottom, int top);

    // Getters for UI to read
    int getPlayerPos(int playerIndex) const;
    bool isPlayerFinished(int playerIndex) const;
    int getNumPlayers() const { return numPlayers; }
    int getNumSnakes() const { return numSnakes; }
    int getNumLadders() const { return numLadders; }

    // Data access for rendering
    const int (*getSnakes())[2] { return snakes; }
    const int (*getLadders())[2] { return ladders; }
    const PlayerStats& getStats(int playerIndex) const { return stats[playerIndex]; }

    // Solver integration
    bool getShortestPathToWin(int playerIndex, std::vector<int>& pathOut, std::vector<int>& diceNeeded);

private:
    void generateBoard();
    int applySnakesAndLadders(int pos, bool &hitSnake, bool &hitLadder);

    int numPlayers;
    int desiredSnakes;
    int desiredLadders;

    int snakes[MAX_SNAKES][2];
    int ladders[MAX_LADDERS][2];
    int numSnakes;
    int numLadders;

    int playerPos[MAX_PLAYERS];
    bool playerFinished[MAX_PLAYERS];
    PlayerStats stats[MAX_PLAYERS];

    adj_list *graph;
    BFS *solver;
};

#endif // GAMEENGINE_H
