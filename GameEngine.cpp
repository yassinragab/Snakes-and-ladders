#include "GameEngine.h"
#include "RandomBoard.h"
#include <cstdlib>
#include <ctime>

GameEngine::GameEngine() : graph(nullptr), solver(nullptr)
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    initGame(1, 8, 8); // Default
}

GameEngine::~GameEngine()
{
    delete solver;
    delete graph;
}

void GameEngine::initGame(int nPlayers, int nSnakes, int nLadders)
{
    numPlayers = nPlayers;
    desiredSnakes = nSnakes;
    desiredLadders = nLadders;
    reset();
}

void GameEngine::reset()
{
    // Reset players
    for (int i = 0; i < MAX_PLAYERS; ++i) {
        playerPos[i] = 1;
        playerFinished[i] = (i >= numPlayers);
        stats[i] = PlayerStats(); // reset stats
    }

    generateBoard();
}

void GameEngine::generateBoard()
{
    // Re-use your existing RandomBoard logic
    numSnakes = 0;
    numLadders = 0;

    // Note: You might want to adjust the path in your RandomBoard.cpp or pass it in
    generateRandomBoard("board.csv",
                        snakes, numSnakes, MAX_SNAKES,
                        ladders, numLadders, MAX_LADDERS,
                        desiredSnakes, desiredLadders);

    delete solver;
    delete graph;
    graph = new adj_list(numSnakes, snakes, numLadders, ladders);
    solver = new BFS(graph);
}

void GameEngine::setPlayerPosition(int playerIndex, int pos)
{
    if (playerIndex >= 0 && playerIndex < numPlayers)
        playerPos[playerIndex] = pos;
}

int GameEngine::rollDiceForPlayer(int playerIndex, int& diceValue, int& hitType, int& startPos)
{
    startPos = playerPos[playerIndex];
    diceValue = (std::rand() % 6) + 1;

    stats[playerIndex].diceRolls++;
    stats[playerIndex].turnsTaken++;

    int newPos = startPos + diceValue;
    hitType = 0; // 0: normal, 1: snake, 2: ladder

    if (newPos > 100) {
        return startPos; // Bounce back logic or stay put? Your original code stayed put.
    }

    bool snake = false, ladder = false;
    int finalPos = applySnakesAndLadders(newPos, snake, ladder);

    if (snake) {
        hitType = 1;
        stats[playerIndex].snakesHit++;
    } else if (ladder) {
        hitType = 2;
        stats[playerIndex].laddersClimbed++;
    }

    playerPos[playerIndex] = finalPos;

    if (finalPos == 100) {
        playerFinished[playerIndex] = true;
    }

    return finalPos;
}

int GameEngine::applySnakesAndLadders(int pos, bool &hitSnake, bool &hitLadder)
{
    hitSnake = false;
    hitLadder = false;

    for (int i = 0; i < numSnakes; i++) {
        if (snakes[i][0] == pos) {
            hitSnake = true;
            return snakes[i][1];
        }
    }
    for (int i = 0; i < numLadders; i++) {
        if (ladders[i][0] == pos) {
            hitLadder = true;
            return ladders[i][1];
        }
    }
    return pos;
}

// Logic for manual board editing
bool GameEngine::addSnake(int head, int tail) {
    if (numSnakes >= MAX_SNAKES) return false;
    snakes[numSnakes][0] = head;
    snakes[numSnakes][1] = tail;
    numSnakes++;
    // Rebuild graph
    delete solver; delete graph;
    graph = new adj_list(numSnakes, snakes, numLadders, ladders);
    solver = new BFS(graph);
    return true;
}

// Getters
int GameEngine::getPlayerPos(int playerIndex) const { return playerPos[playerIndex]; }
bool GameEngine::isPlayerFinished(int playerIndex) const { return playerFinished[playerIndex]; }

bool GameEngine::getShortestPathToWin(int playerIndex, std::vector<int>& pathOut, std::vector<int>& diceNeeded)
{
    int startVertex = playerPos[playerIndex] - 1; // 0-based
    if (!solver->shortestPath(startVertex, 99, pathOut)) return false;

    // Convert path to dice logic
    // (Copying your logic from old GameWindow here to keep Engine pure)
    std::vector<int> squares;
    squares.reserve(pathOut.size());
    for (int i = (int)pathOut.size() - 1; i >= 0; --i)
        squares.push_back(pathOut[i]);

    diceNeeded.clear();
    for (size_t i = 0; i + 1 < squares.size(); ++i) {
        int u = squares[i];
        int v = squares[i + 1];
        int* neighbors = graph->getNeighbors(u);
        int degree = graph->getDegree(u);
        int dice = 0;
        for (int j = 0; j < degree; ++j) {
            if (neighbors[j] == v) {
                dice = j + 1;
                break;
            }
        }
        diceNeeded.push_back(dice);
    }
    return true;
}
