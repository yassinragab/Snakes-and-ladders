#include "BoardEngine.h"

BoardEngine::BoardEngine(const int snakesIn[][2], int numSnakesIn,
                         const int laddersIn[][2], int numLaddersIn)
    : numSnakes(0),
    numLadders(0),
    graph(nullptr),
    solver(nullptr)
{
    rebuild(snakesIn, numSnakesIn, laddersIn, numLaddersIn);
}

BoardEngine::~BoardEngine()
{
    destroyGraph();
}

void BoardEngine::destroyGraph()
{
    delete solver;
    solver = nullptr;

    delete graph;
    graph = nullptr;
}

void BoardEngine::buildGraph()
{
    destroyGraph();
    graph  = new adj_list(numSnakes, snakes, numLadders, ladders);
    solver = new BFS(graph);
}

void BoardEngine::rebuild(const int snakesIn[][2], int numSnakesIn,
                          const int laddersIn[][2], int numLaddersIn)
{
    // Clamp to allowed limits
    if (numSnakesIn < 0) numSnakesIn = 0;
    if (numSnakesIn > MAX_SNAKES) numSnakesIn = MAX_SNAKES;
    if (numLaddersIn < 0) numLaddersIn = 0;
    if (numLaddersIn > MAX_LADDERS) numLaddersIn = MAX_LADDERS;

    numSnakes  = numSnakesIn;
    numLadders = numLaddersIn;

    for (int i = 0; i < numSnakes; ++i)
    {
        snakes[i][0] = snakesIn[i][0];
        snakes[i][1] = snakesIn[i][1];
    }
    for (int i = 0; i < numLadders; ++i)
    {
        ladders[i][0] = laddersIn[i][0];
        ladders[i][1] = laddersIn[i][1];
    }

    buildGraph();
}

int BoardEngine::applySnakesAndLadders(int pos, bool &hitSnake, bool &hitLadder) const
{
    hitSnake  = false;
    hitLadder = false;

    // snakes & ladders arrays use 1-based squares
    for (int i = 0; i < numSnakes; ++i)
    {
        if (snakes[i][0] == pos)
        {
            hitSnake = true;
            return snakes[i][1];
        }
    }
    for (int i = 0; i < numLadders; ++i)
    {
        if (ladders[i][0] == pos)
        {
            hitLadder = true;
            return ladders[i][1];
        }
    }
    return pos;
}

bool BoardEngine::shortestPathFrom(int currentPos,
                                   std::vector<int> &squaresOut,
                                   std::vector<int> &diceRollsOut) const
{
    if (!solver || !graph)
        return false;

    int startVertex = currentPos - 1;  // 0-based
    if (startVertex < 0 || startVertex >= BOARD_SIZE)
        return false;

    std::vector<int> path;
    bool ok = solver->shortestPath(startVertex, BOARD_SIZE - 1, path); // target = 99
    if (!ok)
        return false;

    // Convert path [target .. start] into squares [start .. target], 1-based
    squaresOut.clear();
    for (int i = static_cast<int>(path.size()) - 1; i >= 0; --i)
        squaresOut.push_back(path[i] + 1);

    diceRollsOut.clear();
    for (size_t i = 0; i + 1 < squaresOut.size(); ++i)
    {
        int uSquare = squaresOut[i];
        int vSquare = squaresOut[i + 1];
        int u = uSquare - 1;
        int v = vSquare - 1;

        int *neighbors = graph->getNeighbors(u);
        int degree     = graph->getDegree(u);

        int dice = -1;
        for (int j = 0; j < degree; ++j)
        {
            if (neighbors[j] == v)
            {
                dice = j + 1;   // neighbors[0] => roll 1
                break;
            }
        }
        if (dice == -1)
            dice = 0;          // should not happen; marks an issue

        diceRollsOut.push_back(dice);
    }

    return true;
}
