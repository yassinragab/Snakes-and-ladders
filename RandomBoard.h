#ifndef RANDOM_BOARD_H
#define RANDOM_BOARD_H

void generateRandomBoard(const char* filename,
                         int snakes[][2], int& numSnakes, int maxSnakes,
                         int ladders[][2], int& numLadders, int maxLadders);

#endif // RANDOM_BOARD_H
