#include "RandomBoard.h"
#include <fstream>
#include <cstdlib>
#include <ctime>

void generateRandomBoard(const char* filename,
                         int snakes[][2], int& numSnakes, int maxSnakes,
                         int ladders[][2], int& numLadders, int maxLadders,
                         int desiredSnakes, int desiredLadders)
{
    std::srand(static_cast<unsigned>(std::time(0)));

    // Clamp requested numbers to allowed ranges
    if (desiredSnakes < 0) desiredSnakes = 0;
    if (desiredLadders < 0) desiredLadders = 0;

    if (desiredSnakes  > maxSnakes)  desiredSnakes  = maxSnakes;
    if (desiredLadders > maxLadders) desiredLadders = maxLadders;

    bool used[101];
    for (int i = 0; i <= 100; i++)
        used[i] = false;

    numSnakes  = 0;
    numLadders = 0;

    const int MIN_SQUARE = 1;
    const int MAX_SQUARE = 99;

    // Generate snakes
    while (numSnakes < desiredSnakes)
    {
        int head = 5 + (std::rand() % (MAX_SQUARE - 4));
        int tail = MIN_SQUARE + (std::rand() % (head - MIN_SQUARE));

        if (head == tail)              continue;
        if (used[head] || used[tail])  continue;

        snakes[numSnakes][0] = head;
        snakes[numSnakes][1] = tail;
        used[head] = true;
        used[tail] = true;
        numSnakes++;
    }

    // Generate ladders
    while (numLadders < desiredLadders)
    {
        int bottom = MIN_SQUARE + (std::rand() % (MAX_SQUARE - 10));
        int top    = bottom + 1 + (std::rand() % (MAX_SQUARE - bottom));

        if (top == bottom)              continue;
        if (used[bottom] || used[top])  continue;

        ladders[numLadders][0] = bottom;
        ladders[numLadders][1] = top;
        used[bottom] = true;
        used[top]    = true;
        numLadders++;
    }

    std::ofstream file(filename);
    if (!file.is_open())
        return;

    for (int i = 0; i < numSnakes; i++)
        file << "S," << snakes[i][0] << "," << snakes[i][1] << "\n";

    for (int i = 0; i < numLadders; i++)
        file << "L," << ladders[i][0] << "," << ladders[i][1] << "\n";

    file.close();
}
