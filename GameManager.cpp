#include "GameManager.h"
#include "BoardEngine.h"
#include "RandomBoard.h"

#include <cstdlib>
#include <ctime>
#include <sstream>

GameManager::GameManager()
    : numPlayers(1),
    desiredSnakes(8),
    desiredLadders(8),
    currentPlayer(0),
    gameFinished(false),
    numSnakes(0),
    numLadders(0),
    boardEngine(nullptr)
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    resetGame();
}

GameManager::~GameManager()
{
    delete boardEngine;
}

int GameManager::clampPlayers(int players) const
{
    if (players < 1) return 1;
    if (players > MAX_PLAYERS) return MAX_PLAYERS;
    return players;
}

int GameManager::clampCount(int count, int max) const
{
    if (count < 0) return 0;
    if (count > max) return max;
    return count;
}

void GameManager::configure(int players, int desiredSnakesIn, int desiredLaddersIn)
{
    numPlayers     = clampPlayers(players);
    desiredSnakes  = clampCount(desiredSnakesIn,  MAX_SNAKES);
    desiredLadders = clampCount(desiredLaddersIn, MAX_LADDERS);
}

void GameManager::ensureBoardEngine()
{
    if (!boardEngine)
    {
        boardEngine = new BoardEngine(snakes, numSnakes, ladders, numLadders);
    }
    else
    {
        boardEngine->rebuild(snakes, numSnakes, ladders, numLadders);
    }
}

void GameManager::initRandomBoard(const char* filename)
{
    numSnakes  = 0;
    numLadders = 0;

    generateRandomBoard(filename,
                        snakes,  numSnakes,  MAX_SNAKES,
                        ladders, numLadders, MAX_LADDERS,
                        desiredSnakes, desiredLadders);

    ensureBoardEngine();
}

void GameManager::setCustomBoard(const int snakesIn[][2], int numSnakesIn,
                                 const int laddersIn[][2], int numLaddersIn)
{
    numSnakes  = clampCount(numSnakesIn,  MAX_SNAKES);
    numLadders = clampCount(numLaddersIn, MAX_LADDERS);

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

    ensureBoardEngine();
}

void GameManager::resetGame()
{
    gameFinished  = false;
    currentPlayer = 0;

    for (int i = 0; i < MAX_PLAYERS; ++i)
    {
        playerPos[i]      = 1;
        playerFinished[i] = (i >= numPlayers);
        diceRolls[i]      = 0;
        snakesHit[i]      = 0;
        laddersClimbed[i] = 0;
        turnsTaken[i]     = 0;
    }
}

GameManager::RollResult GameManager::rollDice()
{
    RollResult r{};
    r.moveMade  = false;
    r.playerWon = false;

    if (gameFinished || numPlayers <= 0)
        return r;

    // skip finished players if needed
    int safety = 0;
    while (playerFinished[currentPlayer] && safety < numPlayers)
    {
        currentPlayer = (currentPlayer + 1) % numPlayers;
        ++safety;
    }

    r.playerIndex = currentPlayer;

    int &pos = playerPos[currentPlayer];
    if (pos >= 100)
    {
        playerFinished[currentPlayer] = true;
        gameFinished = true;
        r.fromPos = pos;
        r.finalPos = pos;
        return r;
    }

    r.dice = (std::rand() % 6) + 1;
    r.fromPos = pos;
    diceRolls[currentPlayer]++;
    turnsTaken[currentPlayer]++;

    int newPos = pos + r.dice;
    r.newPos = newPos;

    if (newPos > 100)
    {
        // overshoot: stay in place
        r.moveExceeded = true;
        r.finalPos     = pos;
        r.moveMade     = true;

        // advance turn
        currentPlayer = (currentPlayer + 1) % numPlayers;
        return r;
    }

    bool hitSnake  = false;
    bool hitLadder = false;
    int finalPos   = newPos;

    if (boardEngine)
        finalPos = boardEngine->applySnakesAndLadders(newPos, hitSnake, hitLadder);

    r.hitSnake  = hitSnake;
    r.hitLadder = hitLadder;
    r.finalPos  = finalPos;
    r.moveMade  = true;

    if (hitSnake)       snakesHit[currentPlayer]++;
    else if (hitLadder) laddersClimbed[currentPlayer]++;

    pos = finalPos;

    if (pos == 100)
    {
        playerFinished[currentPlayer] = true;
        gameFinished = true;
        r.playerWon = true;
    }

    // advance to next player if game not finished
    if (!gameFinished)
        currentPlayer = (currentPlayer + 1) % numPlayers;

    return r;
}

bool GameManager::shortestPathFromCurrent(std::vector<int> &squaresOut,
                                          std::vector<int> &diceRollsOut) const
{
    if (!boardEngine || currentPlayer < 0 || currentPlayer >= numPlayers)
        return false;

    int currentPos = playerPos[currentPlayer];
    return boardEngine->shortestPathFrom(currentPos, squaresOut, diceRollsOut);
}

void GameManager::setPlayerPositionForAnimation(int playerIndex, int pos)
{
    if (playerIndex < 0 || playerIndex >= numPlayers)
        return;
    playerPos[playerIndex] = pos;
}

std::string GameManager::buildStatsString() const
{
    std::ostringstream oss;
    for (int p = 0; p < numPlayers; ++p)
    {
        oss << "Player " << (p + 1) << ":\n";
        oss << "  Turns: "          << turnsTaken[p]     << "\n";
        oss << "  Snakes hit: "      << snakesHit[p]      << "\n";
        oss << "  Ladders climbed: " << laddersClimbed[p] << "\n\n";
    }
    return oss.str();
}
