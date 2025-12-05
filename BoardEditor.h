#ifndef BOARDEDITOR_H
#define BOARDEDITOR_H

#include <QString>

class QWidget;
class QTextEdit;

class BoardEditor
{
public:
    // Shows the dialog, edits snakes/ladders arrays, logs invalid lines.
    // Returns true if the user pressed "Apply" and at least tried to edit.
    static bool editBoard(QWidget *parent,
                          int snakes[][2], int &numSnakes, int maxSnakes,
                          int ladders[][2], int &numLadders, int maxLadders,
                          QTextEdit *logText);

    // Save current snakes/ladders arrays to a CSV file.
    static void saveBoardToCsv(const char *filename,
                               const int snakes[][2], int numSnakes,
                               const int ladders[][2], int numLadders);
};

#endif // BOARDEDITOR_H
