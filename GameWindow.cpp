#include "GameWindow.h"

#include "BoardWidget.h"
#include "BoardEngine.h"
#include "RandomBoard.h"
#include "BoardEditor.h"
#include "StatsFormatter.h"

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QMessageBox>
#include <QStringList>
#include <QComboBox>
#include <QTimer>

#include <cstdlib>
#include <ctime>
#include <vector>

using namespace std;

// ====================== Constructor / Destructor ======================

GameWindow::GameWindow(QWidget *parent)
    : QMainWindow(parent),
    numSnakes(0),
    numLadders(0),
    boardEngine(nullptr),
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
    boardWidget(nullptr),
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
    delete boardEngine;
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

    boardWidget = new BoardWidget(boardContainer);
    boardContainerLayout->addWidget(boardWidget);
    boardContainerLayout->addStretch();

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

    if (boardEngine)
    {
        boardEngine->rebuild(snakes, numSnakes, ladders, numLadders);
    }
    else
    {
        boardEngine = new BoardEngine(snakes, numSnakes, ladders, numLadders);
    }

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
    statsText->setText(
        StatsFormatter::formatStats(numPlayers, turnsTaken, snakesHit, laddersClimbed)
        );
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
    if (boardWidget)
    {
        boardWidget->updateBoard(playerPos, playerFinished, numPlayers,
                                 snakes, numSnakes, ladders, numLadders);
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

    int finalPos = newPos;
    if (boardEngine)
        finalPos = boardEngine->applySnakesAndLadders(newPos, hitSnake, hitLadder);

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
    if (!boardEngine)
        return;

    std::vector<int> squares;
    std::vector<int> diceRollsNeeded;

    bool ok = boardEngine->shortestPathFrom(playerPos[currentPlayer],
                                            squares,
                                            diceRollsNeeded);
    if (!ok)
    {
        QMessageBox::warning(this, "Shortest Path",
                             QString("No path found from Player %1's position to 100.")
                                 .arg(currentPlayer + 1));
        return;
    }

    QStringList squareStrs;
    for (int s : squares)
        squareStrs << QString::number(s);

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

    // Delegate all dialog + parsing work to BoardEditor
    bool changed = BoardEditor::editBoard(this,
                                          snakes,  numSnakes,  MAX_SNAKES,
                                          ladders, numLadders, MAX_LADDERS,
                                          logText);
    if (!changed)
        return;

    // Rebuild board engine with edited board
    if (boardEngine)
        boardEngine->rebuild(snakes, numSnakes, ladders, numLadders);
    else
        boardEngine = new BoardEngine(snakes, numSnakes, ladders, numLadders);

    // Save to CSV (same path as random board)
    BoardEditor::saveBoardToCsv("C:/Users/yassi_b74iao3/Downloads/ADS v qt/sl/board.csv",
                                snakes, numSnakes, ladders, numLadders);

    updateBoard();

    if (logText)
        logText->append("Custom board applied from editor.");
}
