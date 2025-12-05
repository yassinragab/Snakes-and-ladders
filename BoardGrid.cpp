#include "BoardGrid.h"
#include <QGridLayout>
#include <QLabel>
#include <QString>

BoardGrid::BoardGrid(QWidget *parent) : QWidget(parent)
{
    layout = new QGridLayout(this);
    layout->setSpacing(1);
    layout->setContentsMargins(0,0,0,0);
    setupGrid();
}

void BoardGrid::setupGrid()
{
    cellLabels.resize(100);
    int visualRow = 0;

    // 10x10 ZigZag Logic
    for (int row = 9; row >= 0; --row, ++visualRow) {
        bool leftToRight = (row % 2 == 0);
        for (int col = 0; col < 10; ++col) {
            int realCol = leftToRight ? col : (9 - col);
            int cellNum = row * 10 + realCol + 1; // 1..100

            QLabel *lbl = new QLabel(QString::number(cellNum));
            lbl->setAlignment(Qt::AlignCenter);
            lbl->setFrameShape(QFrame::Box);
            lbl->setFixedSize(50, 50);

            layout->addWidget(lbl, visualRow, col);
            cellLabels[cellNum - 1] = lbl;
        }
    }
}

QString BoardGrid::getPlayerColor(int index) const
{
    switch (index) {
    case 0: return "#1e90ff"; // Blue
    case 1: return "#ff5555"; // Red
    case 2: return "#55dd55"; // Green
    case 3: return "#ffaa00"; // Orange
    default: return "black";
    }
}

void BoardGrid::updateVisuals(const GameEngine& engine)
{
    QString normalColor   = "white";
    QString snakeColor    = "#ff6666";
    QString ladderColor   = "#66ff66";
    QString playerColorBg = "#ffff66"; // Yellow highlight for player

    auto snakes = engine.getSnakes();
    auto ladders = engine.getLadders();
    int nSnakes = engine.getNumSnakes();
    int nLadders = engine.getNumLadders();

    for (int i = 0; i < 100; ++i) {
        int cellNum = i + 1;
        QLabel *lbl = cellLabels[i];

        // Reset Text
        QString text = QString::number(cellNum);

        // Check Snakes/Ladders for text info
        int jumpTo = -1;
        bool isSnake = false;

        for(int s=0; s<nSnakes; ++s)
            if(snakes[s][0] == cellNum) { jumpTo = snakes[s][1]; isSnake=true; break; }

        if (jumpTo == -1) {
            for(int l=0; l<nLadders; ++l)
                if(ladders[l][0] == cellNum) { jumpTo = ladders[l][1]; isSnake=false; break; }
        }

        if (jumpTo != -1)
            text += QString("\n%1→%2").arg(isSnake?"S":"L").arg(jumpTo);

        // Check Players
        QString playersHere = "";
        bool anyPlayer = false;
        for(int p=0; p < engine.getNumPlayers(); ++p) {
            if (!engine.isPlayerFinished(p) && engine.getPlayerPos(p) == cellNum) {
                playersHere += QString("P%1 ").arg(p+1);
                anyPlayer = true;
            }
        }
        if (anyPlayer) text = playersHere.trimmed() + "\n" + text;

        lbl->setText(text);

        // Set Colors
        QString style = "border: 1px solid black; ";
        if (anyPlayer) style += "background-color: " + playerColorBg + ";";
        else if (isSnake) style += "background-color: " + snakeColor + ";";
        else if (jumpTo != -1) style += "background-color: " + ladderColor + ";";
        else style += "background-color: " + normalColor + ";";

        lbl->setStyleSheet(style);
    }
}
