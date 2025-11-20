#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

// Reads snakes and ladders from a CSV file.
// Returns true if file loaded successfully, false otherwise.
bool loadSnakesAndLadders(const char* filename,
                          int snakes[][2], int& numSnakes, int maxSnakes,
                          int ladders[][2], int& numLadders, int maxLadders);

#endif // BOARD_CONFIG_H
