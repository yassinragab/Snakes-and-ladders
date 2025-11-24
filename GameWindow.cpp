#include "GameWindow.h"

#include <QWidget>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QMessageBox>
#include <QStringList>

#include <cstdlib>
#include <ctime>
#include <vector>

GameWindow::GameWindow(QWidget *parent)
    : QMainWindow(parent),
    numSnakes(0),
    numLadders(0),
    graph(nullptr),
    solver(nullptr),
    playerPos(1),
    centralWidget(nullptr),
    boardLayout(nullptr),
    rollButton(nullptr),
    resetButton(nullptr),
    shortestPathButton(nullptr),
    diceLabel(nullptr),
    positionLabel(nullptr),
    logText(nullptr)
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    setupUi();
    initGameLogic();
    updateBoard();
}

GameWindow::~GameWindow()
{
    delete solver;
    delete graph;
}

void GameWindow::setupUi()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto *mainLayout = new QVBoxLayout(centralWidget);

    // --- Board grid ---
    boardLayout = new QGridLayout();
    boardLayout->setSpacing(2);
    mainLayout->addLayout(boardLayout);

    setupBoardGrid();

    // --- Info ---
    auto *infoLayout  = new QHBoxLayout();
    diceLabel         = new QLabel("Dice: -");
    positionLabel     = new QLabel("Position: 1");
    infoLayout->addWidget(diceLabel);
    infoLayout->addWidget(positionLabel);
    infoLayout->addStretch();
    mainLayout->addLayout(infoLayout);

    // --- Buttons ---
    auto *buttonLayout = new QHBoxLayout();
    rollButton         = new QPushButton("Roll Dice");
    resetButton        = new QPushButton("Reset Game");
    shortestPathButton = new QPushButton("Show Shortest Path");

    buttonLayout->addWidget(rollButton);
    buttonLayout->addWidget(resetButton);
    buttonLayout->addWidget(shortestPathButton);
    buttonLayout->addStretch();

    mainLayout->addLayout(buttonLayout);

    // --- Log ---
    logText = new QTextEdit();
    logText->setReadOnly(true);
    logText->setMinimumHeight(130);
    mainLayout->addWidget(logText);

    connect(rollButton, &QPushButton::clicked, this, &GameWindow::onRollDiceClicked);
    connect(resetButton, &QPushButton::clicked, this, &GameWindow::onResetClicked);
    connect(shortestPathButton,&QPushButton::clicked,this, &GameWindow::onShowShortestPathClicked);

    setWindowTitle("Snakes and Ladders ");
    resize(800, 700);
}

void GameWindow::setupBoardGrid()
{
    cellLabels.clear();
    cellLabels.resize(100);   // index by cell number - 1

    const int ROWS = 10;
    const int COLS = 10;

    int visualRow = 0; // row in the layout (0 = top)
    for (int row = 9; row >= 0; --row, ++visualRow)
    {
        // even row (0,2,4..) -> left to right
        // odd  row (1,3,5..) -> right to left
        bool leftToRight = (row % 2 == 0);

        for (int col = 0; col < COLS; ++col)
        {
            int realCol = leftToRight ? col : (COLS - 1 - col);
            int cellNum = row * COLS + realCol + 1; // 1..100

            QLabel *label = new QLabel(QString::number(cellNum));
            label->setAlignment(Qt::AlignCenter);
            label->setFrameShape(QFrame::Box);
            label->setMinimumSize(40, 40);

            // Place visually: visualRow is from top (0) to bottom (9)
            boardLayout->addWidget(label, visualRow, col);

            // Store by logical cell index (1..100 -> 0..99)
            cellLabels[cellNum - 1] = label;
        }
    }
}

void GameWindow::initGameLogic()
{
    // Generate random board
    numSnakes  = 0;
    numLadders = 0;

    generateRandomBoard("C:/Users/yassi_b74iao3/Downloads/ADS v qt/sl/board.csv",
                        snakes, numSnakes, MAX_SNAKES,
                        ladders, numLadders, MAX_LADDERS);


    // Rebuild graph & BFS
    delete graph;
    delete solver;

    graph  = new adj_list(numSnakes, snakes, numLadders, ladders);
    solver = new BFS(graph);

    playerPos = 1;
    if (positionLabel)
        positionLabel->setText("Position: 1");
    if (diceLabel)
        diceLabel->setText("Dice: -");
    if (logText)
    {
        logText->clear();
        logText->append("New random board generated.");
        logText->append("Reach square 100 to win!");
    }
}

int GameWindow::applySnakesAndLadders(int pos, bool &hitSnake, bool &hitLadder)
{
    hitSnake  = false;
    hitLadder = false;

    for (int i = 0; i < numSnakes; i++)
    {
        if (snakes[i][0] == pos)
        {
            hitSnake = true;
            return snakes[i][1];
        }
    }

    for (int i = 0; i < numLadders; i++)
    {
        if (ladders[i][0] == pos)
        {
            hitLadder = true;
            return ladders[i][1];
        }
    }

    return pos;
}

void GameWindow::updateBoard()
{
    if (cellLabels.size() != 100)
        return;

    for (int i = 0; i < 100; ++i)
    {
        int cellNum = i + 1;
        QLabel *label = cellLabels[i];
        if (!label) continue;

        int snakeTo  = -1;
        int ladderTo = -1;

        // Check snakes
        for (int s = 0; s < numSnakes; ++s)
        {
            if (snakes[s][0] == cellNum)
            {
                snakeTo = snakes[s][1];
                break;
            }
        }

        // Check ladders
        for (int l = 0; l < numLadders; ++l)
        {
            if (ladders[l][0] == cellNum)
            {
                ladderTo = ladders[l][1];
                break;
            }
        }

        bool isPlayerHere = (cellNum == playerPos);

        // ----- Text inside the box -----
        QString text;
        if (isPlayerHere)
            text += "P\n";  // show player at top

        text += QString::number(cellNum);

        if (snakeTo != -1)
            text += QString("\nS→%1").arg(snakeTo);
        if (ladderTo != -1)
            text += QString("\nL→%1").arg(ladderTo);

        label->setText(text);

        // ----- Colors (always black font) -----
        QString style;
        if (isPlayerHere)
        {
            // Player cell: bright yellow
            style = "background-color: #ffff66; color: black; border: 1px solid black;";
        }
        else if (snakeTo != -1)
        {
            // Brighter red for snakes
            style = "background-color: #ff6666; color: black; border: 1px solid black;";
        }
        else if (ladderTo != -1)
        {
            // Brighter green for ladders
            style = "background-color: #66ff66; color: black; border: 1px solid black;";
        }
        else
        {
            // Normal cell
            style = "background-color: white; color: black; border: 1px solid black;";
        }

        label->setStyleSheet(style);
    }

    if (positionLabel)
        positionLabel->setText(QString("Position: %1").arg(playerPos));
}


void GameWindow::onRollDiceClicked()
{
    if (playerPos >= 100)
    {
        QMessageBox::information(this, "Game finished",
                                 "You already reached 100. Press Reset to start a new game.");
        return;
    }

    int dice = (std::rand() % 6) + 1;
    diceLabel->setText(QString("Dice: %1").arg(dice));

    int newPos = playerPos + dice;

    // --- Case 1: roll would exceed 100 ---
    if (newPos > 100)
    {
        if (logText)
        {
            QString msg = QString("Rolled %1, but move exceeds 100. Stay at %2.")
            .arg(dice).arg(playerPos);
            logText->append(msg);
            logText->append("");   // empty line between this roll and the next one
        }

        updateBoard();
        return;
    }

    // --- Case 2: normal move ---
    bool hitSnake = false;
    bool hitLadder = false;

    int finalPos = applySnakesAndLadders(newPos, hitSnake, hitLadder);

    // Build ONE line that may include snake/ladder info
    QString msg = QString("Rolled %1, moving from %2 to %3.")
                      .arg(dice).arg(playerPos).arg(newPos);

    if (hitSnake)
    {
        msg += QString(" Oh no! Snake! Slide down to %1.").arg(finalPos);
    }
    else if (hitLadder)
    {
        msg += QString(" Nice! Ladder! Climb up to %1.").arg(finalPos);
    }

    playerPos = finalPos;
    updateBoard();

    if (logText)
    {
        logText->append(msg);  // whole story in one line
        logText->append("");   // blank line after each roll
    }

    if (playerPos == 100)
    {
        if (logText)
            logText->append(" Congratulations! You reached 100 and won the game!");
        QMessageBox::information(this, "You win!", "Congratulations! You reached square 100.");
    }
}

void GameWindow::onResetClicked()
{
    initGameLogic();
    updateBoard();
}

void GameWindow::onShowShortestPathClicked()
{
    if (!solver)
        return;

    std::vector<int> path;
    bool ok = solver->shortestPath(0, 99, path);

    if (!ok)
    {
        QMessageBox::warning(this, "Shortest Path", "No path found from 1 to 100.");
        return;
    }

    // path is target..start, so reverse and convert to 1-based
    QStringList list;
    for (int i = static_cast<int>(path.size()) - 1; i >= 0; --i)
        list << QString::number(path[i] + 1);

    QString msg = "Shortest path (from 1 to 100):\n" + list.join(" -> ");

    if (logText)
        logText->append(msg);

    QMessageBox::information(this, "Shortest Path", msg);
}
