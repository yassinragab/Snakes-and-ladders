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
#include <QComboBox>
#include <QTimer>
#include <QDialog>

#include <cstdlib>
#include <ctime>
#include <vector>
#include <fstream>

using namespace std;

// ====================== Constructor / Destructor ======================

GameWindow::GameWindow(QWidget *parent)
    : QMainWindow(parent),
    numSnakes(0),
    numLadders(0),
    graph(nullptr),
    solver(nullptr),
    numPlayers(1),          // base config: 1 player
    desiredSnakes(8),       // base config: 8 snakes
    desiredLadders(8),      // base config: 8 ladders
    currentPlayer(0),
    gameFinished(false),
    isAnimating(false),
    animPlayer(0),
    animCurrent(1),
    animEnd(1),
    animTimer(nullptr),
    centralWidget(nullptr),
    boardLayout(nullptr),
    createGameButton(nullptr),
    rollButton(nullptr),
    resetButton(nullptr),
    shortestPathButton(nullptr),
    editBoardButton(nullptr),
    diceLabel(nullptr),
    positionLabel(nullptr),
    turnLabel(nullptr),
    logText(nullptr),
    statsText(nullptr),
    playerCountCombo(nullptr),
    snakeCountCombo(nullptr),
    ladderCountCombo(nullptr)
{
    srand(static_cast<unsigned>(time(nullptr)));
    setupUi();
    initGameLogic();     // uses base config (1,8,8)
    updateBoard();
}

GameWindow::~GameWindow()
{
    delete animTimer;
    delete solver;
    delete graph;
}

// ====================== UI Setup ======================

void GameWindow::setupUi()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto *rootLayout = new QHBoxLayout(centralWidget);

    // ===== LEFT: BOARD =====
    QWidget *boardContainer = new QWidget(centralWidget);
    auto *boardContainerLayout = new QVBoxLayout(boardContainer);
    boardContainerLayout->setContentsMargins(0, 0, 0, 0);
    boardContainerLayout->setSpacing(0);

    boardLayout = new QGridLayout();
    boardLayout->setSpacing(1);
    boardContainerLayout->addLayout(boardLayout);
    boardContainerLayout->addStretch();

    setupBoardGrid();

    // ===== RIGHT: SETTINGS + INFO + BUTTONS + LOG + STATS =====
    auto *sideLayout = new QVBoxLayout();

    // --- SETTINGS ROW (drop-downs) ---
    auto *settingsLayout = new QHBoxLayout();

    auto *playersLabel = new QLabel("Players:");
    playerCountCombo   = new QComboBox();
    for (int i = 1; i <= MAX_PLAYERS; ++i)
        playerCountCombo->addItem(QString::number(i));

    auto *snakesLabel = new QLabel("Snakes:");
    snakeCountCombo   = new QComboBox();
    for (int i = 0; i <= MAX_SNAKES; ++i)
        snakeCountCombo->addItem(QString::number(i));

    auto *laddersLabel = new QLabel("Ladders:");
    ladderCountCombo   = new QComboBox();
    for (int i = 0; i <= MAX_LADDERS; ++i)
        ladderCountCombo->addItem(QString::number(i));

    // Set dropdowns to base config values
    playerCountCombo->setCurrentIndex(numPlayers - 1); // 1-based to index
    snakeCountCombo->setCurrentIndex(desiredSnakes);   // 0..20
    ladderCountCombo->setCurrentIndex(desiredLadders); // 0..20

    settingsLayout->addWidget(playersLabel);
    settingsLayout->addWidget(playerCountCombo);
    settingsLayout->addWidget(snakesLabel);
    settingsLayout->addWidget(snakeCountCombo);
    settingsLayout->addWidget(laddersLabel);
    settingsLayout->addWidget(ladderCountCombo);
    settingsLayout->addStretch();

    // Top: dice + positions + turn
    auto *infoLayout = new QHBoxLayout();
    diceLabel     = new QLabel("Dice: -");
    positionLabel = new QLabel("Positions: all at 1");
    turnLabel     = new QLabel("Current: Player 1");

    infoLayout->addWidget(diceLabel);
    infoLayout->addWidget(positionLabel);
    infoLayout->addWidget(turnLabel);
    infoLayout->addStretch();

    // Middle: buttons
    auto *buttonLayout = new QHBoxLayout();
    createGameButton   = new QPushButton("Create Game");
    rollButton         = new QPushButton("Roll Dice");
    resetButton        = new QPushButton("Reset Game");
    shortestPathButton = new QPushButton("Show Shortest Path");
    editBoardButton    = new QPushButton("Edit Board");

    buttonLayout->addWidget(createGameButton);
    buttonLayout->addWidget(rollButton);
    buttonLayout->addWidget(resetButton);
    buttonLayout->addWidget(shortestPathButton);
    buttonLayout->addWidget(editBoardButton);

    // Log box
    logText = new QTextEdit();
    logText->setReadOnly(true);
    logText->setMinimumWidth(260);

    // Stats box
    statsText = new QTextEdit();
    statsText->setReadOnly(true);
    statsText->setMinimumWidth(260);

    sideLayout->addLayout(settingsLayout);
    sideLayout->addLayout(infoLayout);
    sideLayout->addLayout(buttonLayout);
    sideLayout->addWidget(new QLabel("Game Log:"));
    sideLayout->addWidget(logText, 1);
    sideLayout->addWidget(new QLabel("Statistics:"));
    sideLayout->addWidget(statsText, 1);

    // Put left and right in the root layout
    rootLayout->addWidget(boardContainer);
    rootLayout->addLayout(sideLayout);

    rootLayout->setStretch(0, 3);  // board
    rootLayout->setStretch(1, 2);  // side panel

    // Connections
    connect(createGameButton,   &QPushButton::clicked,
            this,               &GameWindow::onCreateGameClicked);
    connect(rollButton,         &QPushButton::clicked,
            this,               &GameWindow::onRollDiceClicked);
    connect(resetButton,        &QPushButton::clicked,
            this,               &GameWindow::onResetClicked);
    connect(shortestPathButton, &QPushButton::clicked,
            this,               &GameWindow::onShowShortestPathClicked);
    connect(editBoardButton,    &QPushButton::clicked,
            this,               &GameWindow::onEditBoardClicked);

    setWindowTitle("Snakes and Ladders - Qt GUI (Multi-Player)");
    resize(1200, 700);
}

void GameWindow::setupBoardGrid()
{
    cellLabels.clear();
    cellLabels.resize(100);   // index by (cell number - 1)

    const int ROWS = 10;
    const int COLS = 10;

    int visualRow = 0; // layout row: 0 = top
    for (int row = 9; row >= 0; --row, ++visualRow)
    {
        bool leftToRight = (row % 2 == 0);

        for (int col = 0; col < COLS; ++col)
        {
            int realCol = leftToRight ? col : (COLS - 1 - col);
            int cellNum = row * COLS + realCol + 1;  // 1..100

            QLabel *label = new QLabel(QString::number(cellNum));
            label->setAlignment(Qt::AlignCenter);
            label->setFrameShape(QFrame::Box);

            label->setFixedSize(50, 50);
            label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

            boardLayout->addWidget(label, visualRow, col);
            cellLabels[cellNum - 1] = label;
        }
    }
}

// ====================== Game Initialization ======================

void GameWindow::initGameLogic()
{
    // Clamp config
    if (numPlayers < 1) numPlayers = 1;
    if (numPlayers > MAX_PLAYERS) numPlayers = MAX_PLAYERS;
    if (desiredSnakes < 0) desiredSnakes = 0;
    if (desiredSnakes > MAX_SNAKES) desiredSnakes = MAX_SNAKES;
    if (desiredLadders < 0) desiredLadders = 0;
    if (desiredLadders > MAX_LADDERS) desiredLadders = MAX_LADDERS;

    // Sync dropdowns with current config
    if (playerCountCombo)
        playerCountCombo->setCurrentIndex(numPlayers - 1);
    if (snakeCountCombo)
        snakeCountCombo->setCurrentIndex(desiredSnakes);
    if (ladderCountCombo)
        ladderCountCombo->setCurrentIndex(desiredLadders);

    currentPlayer = 0;
    gameFinished  = false;

    // Initialize players & stats
    for (int i = 0; i < MAX_PLAYERS; ++i)
    {
        playerPos[i]      = 1;
        playerFinished[i] = (i >= numPlayers);
        diceRolls[i]      = 0;
        snakesHit[i]      = 0;
        laddersClimbed[i] = 0;
        turnsTaken[i]     = 0;
    }

    // Generate board
    numSnakes  = 0;
    numLadders = 0;

    generateRandomBoard("C:/Users/yassi_b74iao3/Downloads/ADS v qt/sl/board.csv",
                        snakes,  numSnakes,  MAX_SNAKES,
                        ladders, numLadders, MAX_LADDERS,
                        desiredSnakes, desiredLadders);

    delete graph;
    delete solver;
    graph  = new adj_list(numSnakes, snakes, numLadders, ladders);
    solver = new BFS(graph);

    if (positionLabel)
    {
        QString posStr = "Positions: ";
        for (int p = 0; p < numPlayers; ++p)
            posStr += QString("P%1=%2 ").arg(p + 1).arg(playerPos[p]);
        positionLabel->setText(posStr.trimmed());
    }

    if (diceLabel) diceLabel->setText("Dice: -");
    if (turnLabel) turnLabel->setText("Current: Player 1");

    if (logText)
    {
        logText->clear();
        logText->append(
            QString("New random board generated for %1 player(s), %2 snake(s), %3 ladder(s).")
                .arg(numPlayers)
                .arg(numSnakes)
                .arg(numLadders)
            );
        logText->append("Reach square 100 to win!");
    }

    updateStatsPanel();
}

// ====================== Utility Helpers ======================

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

QString GameWindow::playerColor(int playerIndex) const
{
    switch (playerIndex)
    {
    case 0: return "#1e90ff"; // blue
    case 1: return "#ff5555"; // red
    case 2: return "#55dd55"; // green
    case 3: return "#ffaa00"; // orange
    default: return "black";
    }
}

void GameWindow::updateStatsPanel()
{
    if (!statsText) return;

    QString s;
    for (int p = 0; p < numPlayers; ++p)
    {
        s += QString("Player %1:\n").arg(p + 1);
        s += QString("  Turns: %1\n").arg(turnsTaken[p]);
        s += QString("  Snakes hit: %1\n").arg(snakesHit[p]);
        s += QString("  Ladders climbed: %1\n\n").arg(laddersClimbed[p]);
    }
    statsText->setText(s);
}

void GameWindow::animateMove(int player, int start, int end)
{
    if (!animTimer)
    {
        animTimer = new QTimer(this);
        connect(animTimer, &QTimer::timeout,
                this,      &GameWindow::onAnimateStep);
    }

    isAnimating = true;
    animPlayer  = player;
    animCurrent = start;
    animEnd     = end;

    // Faster animation (e.g. 50ms per step)
    animTimer->start(50);
}

// ====================== Board Redraw ======================

void GameWindow::updateBoard()
{
    if (cellLabels.size() != 100)
        return;

    QString normalColor   = "white";
    QString snakeColor    = "#ff6666";
    QString ladderColor   = "#66ff66";
    QString playerColorBg = "#ffff66";

    for (int i = 0; i < 100; ++i)
    {
        int cellNum = i + 1;
        QLabel *label = cellLabels[i];
        if (!label) continue;

        int snakeTo  = -1;
        int ladderTo = -1;

        for (int s = 0; s < numSnakes; ++s)
        {
            if (snakes[s][0] == cellNum)
            {
                snakeTo = snakes[s][1];
                break;
            }
        }
        for (int l = 0; l < numLadders; ++l)
        {
            if (ladders[l][0] == cellNum)
            {
                ladderTo = ladders[l][1];
                break;
            }
        }

        QString playersHere;
        for (int p = 0; p < numPlayers; ++p)
        {
            if (!playerFinished[p] && playerPos[p] == cellNum)
            {
                playersHere += QString("P%1 ").arg(p + 1);
            }
        }
        bool anyPlayerHere = !playersHere.isEmpty();

        QString text;
        if (anyPlayerHere)
        {
            text += playersHere.trimmed();
            text += "\n";
        }

        text += QString::number(cellNum);

        if (snakeTo != -1)
            text += QString("\nS→%1").arg(snakeTo);
        if (ladderTo != -1)
            text += QString("\nL→%1").arg(ladderTo);

        label->setText(text);

        QString style;
        if (anyPlayerHere)
        {
            style = QString("background-color: %1; color: black; border: 1px solid black;")
            .arg(playerColorBg);
        }
        else if (snakeTo != -1)
        {
            style = QString("background-color: %1; color: black; border: 1px solid black;")
            .arg(snakeColor);
        }
        else if (ladderTo != -1)
        {
            style = QString("background-color: %1; color: black; border: 1px solid black;")
            .arg(ladderColor);
        }
        else
        {
            style = QString("background-color: %1; color: black; border: 1px solid black;")
            .arg(normalColor);
        }

        label->setStyleSheet(style);
    }

    if (positionLabel)
    {
        QString posStr = "Positions: ";
        for (int p = 0; p < numPlayers; ++p)
            posStr += QString("P%1=%2 ").arg(p + 1).arg(playerPos[p]);
        positionLabel->setText(posStr.trimmed());
    }

    if (turnLabel)
    {
        QString color = playerColor(currentPlayer);
        turnLabel->setText(QString("Current: Player %1").arg(currentPlayer + 1));
        turnLabel->setStyleSheet(QString("font-weight: bold; color: %1;").arg(color));
    }

    updateStatsPanel();
}

// ====================== Slots: Config ======================

void GameWindow::onCreateGameClicked()
{
    if (isAnimating)
    {
        QMessageBox::information(this, "Busy",
                                 "Please wait until animation finishes.");
        return;
    }

    if (playerCountCombo)
        numPlayers = playerCountCombo->currentText().toInt();
    if (snakeCountCombo)
        desiredSnakes = snakeCountCombo->currentText().toInt();
    if (ladderCountCombo)
        desiredLadders = ladderCountCombo->currentText().toInt();

    initGameLogic();
    updateBoard();

    if (logText)
    {
        logText->append("");
        logText->append("Game created with selected configuration.");
    }
}

void GameWindow::onResetClicked()
{
    if (isAnimating)
    {
        QMessageBox::information(this, "Busy",
                                 "Please wait until animation finishes.");
        return;
    }

    numPlayers     = 1;
    desiredSnakes  = 8;
    desiredLadders = 8;

    initGameLogic();
    updateBoard();

    if (logText)
    {
        logText->append("");
        logText->append("Game reset to base configuration (1 player, 8 snakes, 8 ladders).");
    }
}

// ====================== Slots: Rolling & Movement ======================

void GameWindow::onRollDiceClicked()
{
    if (gameFinished)
    {
        QMessageBox::information(this, "Game finished",
                                 "The game is already finished. Reset or create a new game.");
        return;
    }
    if (isAnimating)
    {
        QMessageBox::information(this, "Busy",
                                 "Please wait until animation finishes.");
        return;
    }

    int safety = 0;
    while (playerFinished[currentPlayer] && safety < numPlayers)
    {
        currentPlayer = (currentPlayer + 1) % numPlayers;
        ++safety;
    }

    int &pos = playerPos[currentPlayer];

    if (pos >= 100)
    {
        playerFinished[currentPlayer] = true;
        QMessageBox::information(this, "Game finished",
                                 QString("Player %1 already reached 100.").arg(currentPlayer + 1));
        return;
    }

    int dice = (rand() % 6) + 1;
    diceLabel->setText(QString("Dice: %1").arg(dice));

    diceRolls[currentPlayer]++;
    turnsTaken[currentPlayer]++;

    int newPos = pos + dice;

    if (newPos > 100)
    {
        if (logText)
        {
            QString msg = QString("Player %1 rolled %2, but move exceeds 100. Stays at %3.")
            .arg(currentPlayer + 1)
                .arg(dice)
                .arg(pos);
            logText->append(msg);
            logText->append("");
        }

        updateBoard();
        currentPlayer = (currentPlayer + 1) % numPlayers;
        updateBoard();
        return;
    }

    bool hitSnake  = false;
    bool hitLadder = false;

    int finalPos = applySnakesAndLadders(newPos, hitSnake, hitLadder);

    if (hitSnake)       snakesHit[currentPlayer]++;
    else if (hitLadder) laddersClimbed[currentPlayer]++;

    QString msg = QString("Player %1 rolled %2, moving from %3 to %4.")
                      .arg(currentPlayer + 1)
                      .arg(dice)
                      .arg(pos)
                      .arg(newPos);

    if (hitSnake)
        msg += QString(" Oh no! Snake! Slide down to %1.").arg(finalPos);
    else if (hitLadder)
        msg += QString(" Nice! Ladder! Climb up to %1.").arg(finalPos);

    pos = finalPos;

    if (hitSnake || hitLadder)
        animateMove(currentPlayer, newPos, finalPos);

    updateBoard();

    if (logText)
    {
        logText->append(msg);
        logText->append("");
    }

    if (pos == 100)
    {
        playerFinished[currentPlayer] = true;
        gameFinished = true;

        if (logText)
            logText->append(
                QString("🎉 Congratulations! Player %1 reached 100 and won the game!")
                    .arg(currentPlayer + 1));

        QMessageBox::information(this, "You win!",
                                 QString("Congratulations! Player %1 reached square 100.")
                                     .arg(currentPlayer + 1));
        return;
    }

    currentPlayer = (currentPlayer + 1) % numPlayers;
    updateBoard();
}

void GameWindow::onAnimateStep()
{
    if (animCurrent == animEnd)
    {
        if (animTimer)
            animTimer->stop();
        isAnimating = false;
        return;
    }

    if (animCurrent < animEnd)
        animCurrent++;
    else
        animCurrent--;

    playerPos[animPlayer] = animCurrent;
    updateBoard();
}

// ====================== Slots: Shortest Path (+ dice) ======================

void GameWindow::onShowShortestPathClicked()
{
    if (!solver || !graph)
        return;

    int startVertex = playerPos[currentPlayer] - 1;  // 0-based
    vector<int> path;
    bool ok = solver->shortestPath(startVertex, 99, path); // 99 = square 100

    if (!ok)
    {
        QMessageBox::warning(this, "Shortest Path",
                             QString("No path found from Player %1's position to 100.")
                                 .arg(currentPlayer + 1));
        return;
    }

    // path is [target, ..., start]; reverse it to [start..target]
    vector<int> squares;
    squares.reserve(path.size());
    for (int i = static_cast<int>(path.size()) - 1; i >= 0; --i)
        squares.push_back(path[i]);

    // compute dice rolls
    vector<int> diceRollsNeeded;
    diceRollsNeeded.reserve(squares.size() - 1);

    for (size_t i = 0; i + 1 < squares.size(); ++i)
    {
        int u = squares[i];       // current vertex (0-based)
        int v = squares[i + 1];   // next vertex (0-based)

        int *neighbors = graph->getNeighbors(u);
        int degree     = graph->getDegree(u);

        int dice = -1;
        for (int j = 0; j < degree; ++j)
        {
            if (neighbors[j] == v)
            {
                dice = j + 1;     // neighbors[0] => roll 1, etc.
                break;
            }
        }

        if (dice == -1)
            dice = 0; // should not happen; 0 marks an issue

        diceRollsNeeded.push_back(dice);
    }

    QStringList squareStrs;
    for (int s : squares)
        squareStrs << QString::number(s + 1);

    QStringList diceStrs;
    for (int d : diceRollsNeeded)
        diceStrs << QString::number(d);

    QString msg = QString("Shortest path for Player %1 (from %2 to 100):\n")
                      .arg(currentPlayer + 1)
                      .arg(playerPos[currentPlayer]);

    msg += "Squares: " + squareStrs.join(" -> ") + "\n";
    msg += "Dice rolls: " + diceStrs.join(", ");

    if (logText)
        logText->append(msg);

    QMessageBox::information(this, "Shortest Path", msg);
}

// ====================== Slots: Custom Board Editor ======================

void GameWindow::onEditBoardClicked()
{
    if (isAnimating)
    {
        QMessageBox::information(this, "Busy",
                                 "Please wait until animation finishes.");
        return;
    }

    QDialog dlg(this);
    dlg.setWindowTitle("Custom Board Editor");

    QVBoxLayout *layout = new QVBoxLayout(&dlg);

    QLabel *label = new QLabel("Enter snakes and ladders (one per line):\n"
                               "Format: S,head,tail  (head > tail)\n"
                               "        L,bottom,top (top > bottom)");
    QTextEdit *editor = new QTextEdit();

    QPushButton *okBtn = new QPushButton("Apply");
    QPushButton *cancelBtn = new QPushButton("Cancel");

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);

    layout->addWidget(label);
    layout->addWidget(editor);
    layout->addLayout(btnLayout);

    connect(okBtn,    &QPushButton::clicked, &dlg, &QDialog::accept);
    connect(cancelBtn,&QPushButton::clicked, &dlg, &QDialog::reject);

    if (dlg.exec() != QDialog::Accepted)
        return;

    QString text = editor->toPlainText();
    QStringList lines = text.split('\n', Qt::SkipEmptyParts);

    numSnakes  = 0;
    numLadders = 0;

    for (const QString &line : lines)
    {
        QStringList parts = line.split(',', Qt::SkipEmptyParts);
        if (parts.size() != 3) continue;

        QString type = parts[0].trimmed().toUpper();
        int a = parts[1].toInt();
        int b = parts[2].toInt();

        // Snake: S, head, tail -> must have head > tail
        if (type == "S" && numSnakes < MAX_SNAKES)
        {
            if (a > b)
            {
                snakes[numSnakes][0] = a;
                snakes[numSnakes][1] = b;
                numSnakes++;
            }
            else
            {
                if (logText)
                    logText->append(
                        QString("Ignored invalid snake: S,%1,%2 (head must be > tail).")
                            .arg(a).arg(b));
            }
        }
        // Ladder: L, bottom, top -> must have top > bottom
        else if (type == "L" && numLadders < MAX_LADDERS)
        {
            if (b > a)
            {
                ladders[numLadders][0] = a;
                ladders[numLadders][1] = b;
                numLadders++;
            }
            else
            {
                if (logText)
                    logText->append(
                        QString("Ignored invalid ladder: L,%1,%2 (top must be > bottom).")
                            .arg(a).arg(b));
            }
        }
    }

    delete graph;
    delete solver;
    graph  = new adj_list(numSnakes, snakes, numLadders, ladders);
    solver = new BFS(graph);

    // also write the edited board to CSV
    ofstream file("C:/Users/yassi_b74iao3/Downloads/ADS v qt/sl/board.csv");
    if (file.is_open())
    {
        for (int i = 0; i < numSnakes; ++i)
            file << "S," << snakes[i][0] << "," << snakes[i][1] << "\n";
        for (int i = 0; i < numLadders; ++i)
            file << "L," << ladders[i][0] << "," << ladders[i][1] << "\n";
        file.close();
    }

    updateBoard();

    if (logText)
        logText->append("Custom board applied from editor.");
}
