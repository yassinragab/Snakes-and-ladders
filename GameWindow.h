#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QMainWindow>

class QLabel;
class QPushButton;
class QTextEdit;
class QComboBox;
class QTimer;
class QWidget;

class BoardWidget;
class BoardEngine;

class GameWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit GameWindow(QWidget *parent = nullptr);
    ~GameWindow();

private slots:
    void onCreateGameClicked();
    void onRollDiceClicked();
    void onResetClicked();
    void onShowShortestPathClicked();
    void onEditBoardClicked();
    void onAnimateStep();

private:
    void setupUi();
    void updateBoard();
    void initGameLogic();                  // Uses numPlayers/desiredSnakes/desiredLadders
    void updateStatsPanel();
    void animateMove(int player, int start, int end);
    QString playerColor(int playerIndex) const;

    static const int MAX_SNAKES  = 20;
    static const int MAX_LADDERS = 20;
    static const int MAX_PLAYERS = 4;

    // Board data (owned here, used by BoardEngine & BoardWidget)
    int snakes[MAX_SNAKES][2];
    int ladders[MAX_LADDERS][2];
    int numSnakes;
    int numLadders;

    BoardEngine *boardEngine;

    // Config
    int  numPlayers;                       // current configuration
    int  desiredSnakes;
    int  desiredLadders;

    // Game state
    int  currentPlayer;                    // 0..numPlayers-1
    int  playerPos[MAX_PLAYERS];           // positions 1..100 for each player
    bool playerFinished[MAX_PLAYERS];      // has this player reached 100?
    bool gameFinished;                     // true if someone already won

    // Stats
    int  diceRolls[MAX_PLAYERS];
    int  snakesHit[MAX_PLAYERS];
    int  laddersClimbed[MAX_PLAYERS];
    int  turnsTaken[MAX_PLAYERS];

    // Animation (snakes/ladders)
    bool   isAnimating;
    int    animPlayer;
    int    animCurrent;
    int    animEnd;
    QTimer *animTimer;

    // Widgets
    QWidget      *centralWidget;
    BoardWidget  *boardWidget;    // dedicated widget for board UI

    QPushButton *createGameButton;
    QPushButton *rollButton;
    QPushButton *resetButton;
    QPushButton *shortestPathButton;
    QPushButton *editBoardButton;

    QLabel      *diceLabel;
    QLabel      *positionLabel;
    QLabel      *turnLabel;
    QTextEdit   *logText;
    QTextEdit   *statsText;           // stats panel

    // Drop-downs for settings (players/snakes/ladders)
    QComboBox   *playerCountCombo;
    QComboBox   *snakeCountCombo;
    QComboBox   *ladderCountCombo;
};

#endif // GAMEWINDOW_H
