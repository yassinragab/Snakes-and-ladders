#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <vector>

class QLabel;
class QPushButton;
class QTextEdit;
class QGridLayout;
class QComboBox;
class QTimer;

#include "adj_list.h"
#include "BFS.h"
#include "RandomBoard.h"

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
    void onReplayClicked();
    void onReplayStep();
    void onEditBoardClicked();
    void onAnimateStep();

private:
    void setupUi();
    void setupBoardGrid();
    void updateBoard();
    void initGameLogic();                  // Uses numPlayers/desiredSnakes/desiredLadders
    int  applySnakesAndLadders(int pos, bool &hitSnake, bool &hitLadder);

    void updateStatsPanel();
    void animateMove(int player, int start, int end);
    QString playerColor(int playerIndex) const;

    static const int MAX_SNAKES  = 20;
    static const int MAX_LADDERS = 20;
    static const int MAX_PLAYERS = 4;

    // Board data
    int snakes[MAX_SNAKES][2];
    int ladders[MAX_LADDERS][2];
    int numSnakes;
    int numLadders;

    adj_list *graph;
    BFS      *solver;

    // Config
    int  numPlayers;                       // current configuration
    int  desiredSnakes;
    int  desiredLadders;

    // Game state
    int  currentPlayer;                    // 0..numPlayers-1
    int  playerPos[MAX_PLAYERS];           // positions 1..100 for each player
    bool playerFinished[MAX_PLAYERS];      // has this player reached 100?
    bool gameFinished;                     // true if someone already won

    // Stats (feature 6)
    int diceRolls[MAX_PLAYERS];            // still tracked internally if needed
    int snakesHit[MAX_PLAYERS];
    int laddersClimbed[MAX_PLAYERS];
    int turnsTaken[MAX_PLAYERS];

    // History + replay (feature 8)
    struct Move
    {
        int player;    // 0..numPlayers-1
        int fromPos;   // 1..100
        int dice;      // 1..6
        int toPos;     // 1..100 (after snake/ladder)
        bool hitSnake;
        bool hitLadder;
    };

    std::vector<Move> history;
    bool   isReplaying;
    int    replayIndex;
    QTimer *replayTimer;

    // Animation (feature 12)
    bool   isAnimating;
    int    animPlayer;
    int    animCurrent;
    int    animEnd;
    QTimer *animTimer;

    // Widgets
    QWidget      *centralWidget;
    QGridLayout  *boardLayout;
    QVector<QLabel*> cellLabels;

    QPushButton *createGameButton;
    QPushButton *rollButton;
    QPushButton *resetButton;
    QPushButton *shortestPathButton;
    QPushButton *replayButton;
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
