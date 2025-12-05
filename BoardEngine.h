#ifndef BOARDENGINE_H
#define BOARDENGINE_H

#include "adj_list.h"
#include "BFS.h"
#include <vector>

class BoardEngine
{
public:
    static const int MAX_SNAKES  = 20;
    static const int MAX_LADDERS = 20;
    static const int BOARD_SIZE  = 100;

    BoardEngine(const int snakes[][2], int numSnakes,
                const int ladders[][2], int numLadders);
    ~BoardEngine();

    // Rebuild internal board + graph when snakes/ladders change
    void rebuild(const int snakes[][2], int numSnakes,
                 const int ladders[][2], int numLadders);

    // Apply snakes/ladders effect to a position (1..100)
    int applySnakesAndLadders(int pos, bool &hitSnake, bool &hitLadder) const;

    // Shortest path from current position (1..100) to 100
    // Returns squares on path (1-based) and required dice rolls
    bool shortestPathFrom(int currentPos,
                          std::vector<int> &squaresOut,
                          std::vector<int> &diceRollsOut) const;

    int getNumSnakes()  const { return numSnakes;  }
    int getNumLadders() const { return numLadders; }

    const int (*getSnakes() const)[2]  { return snakes;  }
    const int (*getLadders() const)[2] { return ladders; }

private:
    int snakes[MAX_SNAKES][2];
    int ladders[MAX_LADDERS][2];
    int numSnakes;
    int numLadders;

    adj_list *graph;
    BFS      *solver;

    void buildGraph();
    void destroyGraph();
};

#endif // BOARDENGINE_H
