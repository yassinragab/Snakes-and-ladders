// #include "BoardDisplay.h"

// BoardDisplay::BoardDisplay() {}
// #include "BoardDisplay.h"
// #include <iostream>
// using namespace std;

// void BoardDisplay::printBoard2D(int snakes[][2], int numSnakes,
//                                 int ladders[][2], int numLadders)
// {
//     cout << "\nSnakes and Ladders Board\n\n";

//     for (int row = 9; row >= 0; row--)
//     {
//         bool leftToRight = (row % 2 == 0);

//         for (int col = 0; col < 10; col++)
//         {
//             int realCol = leftToRight ? col : 9 - col;
//             int cell    = row * 10 + realCol + 1;
//             bool printed = false;

//             for (int s = 0; s < numSnakes; s++)
//             {
//                 if (snakes[s][0] == cell)
//                 {
//                     cout << cell << "(S->" << snakes[s][1] << ")\t";
//                     printed = true;
//                     break;
//                 }
//             }
//             if (printed) continue;

//             for (int l = 0; l < numLadders; l++)
//             {
//                 if (ladders[l][0] == cell)
//                 {
//                     cout << cell << "(L->" << ladders[l][1] << ")\t";
//                     printed = true;
//                     break;
//                 }
//             }
//             if (printed) continue;

//             cout << cell << "\t";
//         }
//         cout << "\n\n";
//     }
// }

// void BoardDisplay::printBoardWithPlayer(int snakes[][2], int numSnakes,
//                                         int ladders[][2], int numLadders,
//                                         int playerPos)
// {
//     cout << "\nSnakes and Ladders Board (Player at " << playerPos << ")\n\n";

//     for (int row = 9; row >= 0; row--)
//     {
//         bool leftToRight = (row % 2 == 0);

//         for (int col = 0; col < 10; col++)
//         {
//             int realCol = leftToRight ? col : 9 - col;
//             int cell    = row * 10 + realCol + 1;
//             bool printed = false;
//             bool isPlayerHere = (cell == playerPos);

//             for (int s = 0; s < numSnakes; s++)
//             {
//                 if (snakes[s][0] == cell)
//                 {
//                     if (isPlayerHere)
//                         cout << "[" << cell << "P(S->" << snakes[s][1] << ")]\t";
//                     else
//                         cout << cell << "(S->" << snakes[s][1] << ")\t";
//                     printed = true;
//                     break;
//                 }
//             }
//             if (printed) continue;

//             for (int l = 0; l < numLadders; l++)
//             {
//                 if (ladders[l][0] == cell)
//                 {
//                     if (isPlayerHere)
//                         cout << "[" << cell << "P(L->" << ladders[l][1] << ")]\t";
//                     else
//                         cout << cell << "(L->" << ladders[l][1] << ")\t";
//                     printed = true;
//                     break;
//                 }
//             }
//             if (printed) continue;

//             if (isPlayerHere)
//                 cout << "[" << cell << "P]\t";
//             else
//                 cout << cell << "\t";
//         }
//         cout << "\n\n";
//     }
// }
