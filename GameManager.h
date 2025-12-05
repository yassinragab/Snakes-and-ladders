#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include <vector>
#include <string>

class BoardEngine;

class GameManager
{
public:
    static const int MAX_PLAYERS = 4;
    static const int MAX_SNAKES  = 20;
    static const int MAX_LADDERS = 20;

    struct RollResult {
        bool moveMade;      // true if we actually processed a move
        int  playerIndex;   // which player rolled
        int  dice;
        int  fromPos;
        int  newPos;        // before snake/ladder
        int  finalPos;      // after snake/ladder
        bool hitSnake;
        bool hitLadder;
        bool moveExceeded;  // true if > 100 and we stayed in place
        bool playerWon;     // true if this move reached 100
    };

    GameManager();
    ~GameManager();

    // Configuration
    void configure(int players, int desiredSnakes, int desiredLadders);
    void initRandomBoard(const char* filename);
    void setCustomBoard(const int snakesIn[][2], int numSnakesIn,
                        const int laddersIn[][2], int numLaddersIn);

    void resetGame();   // players back to 1, stats reset, same board

    // Getters
    int  getNumPlayers() const { return numPlayers; }
    int  getCurrentPlayer() const { return currentPlayer; }
    bool isGameFinished() const { return gameFinished; }

    const int*  getPlayerPositions() const { return playerPos; }
    const bool* getPlayerFinished() const { return playerFinished; }

    int  getNumSnakes() const { return numSnakes; }
    int  getNumLadders() const { return numLadders; }

    const int (*getSnakes() const)[2]  { return snakes; }
    const int (*getLadders() const)[2] { return ladders; }

    const int* getSnakesHit() const { return snakesHit; }
    const int* getLaddersClimbed() const { return laddersClimbed; }
    const int* getTurnsTaken() const { return turnsTaken; }

    // Logic
    RollResult rollDice();   // advances currentPlayer internally (if game not finished)

    bool shortestPathFromCurrent(std::vector<int> &squaresOut,
                                 std::vector<int> &diceRollsOut) const;

    // For animation to temporarily change position
    void setPlayerPositionForAnimation(int playerIndex, int pos);

    // Helper text for stats
    std::string buildStatsString() const;

private:
    int  numPlayers;
    int  desiredSnakes;
    int  desiredLadders;

    int  currentPlayer;
    int  playerPos[MAX_PLAYERS];
    bool playerFinished[MAX_PLAYERS];
    bool gameFinished;

    int  diceRolls[MAX_PLAYERS];
    int  snakesHit[MAX_PLAYERS];
    int  laddersClimbed[MAX_PLAYERS];
    int  turnsTaken[MAX_PLAYERS];

    int  snakes[MAX_SNAKES][2];
    int  ladders[MAX_LADDERS][2];
    int  numSnakes;
    int  numLadders;

    BoardEngine *boardEngine;

    void ensureBoardEngine();
    int  clampPlayers(int players) const;
    int  clampCount(int count, int max) const;
};

#endif // GAMEMANAGER_H
