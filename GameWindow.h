#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QMainWindow>
#include <QVector>

class QLabel;
class QPushButton;
class QTextEdit;
class QGridLayout;

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
    void onRollDiceClicked();
    void onResetClicked();
    void onShowShortestPathClicked();

private:
    void setupUi();
    void setupBoardGrid();
    void updateBoard();
    void initGameLogic();
    int  applySnakesAndLadders(int pos, bool &hitSnake, bool &hitLadder);

    static const int MAX_SNAKES  = 20;
    static const int MAX_LADDERS = 20;

    int snakes[MAX_SNAKES][2];
    int ladders[MAX_LADDERS][2];
    int numSnakes;
    int numLadders;

    adj_list *graph;
    BFS      *solver;

    int playerPos; // 1..100

    QWidget      *centralWidget;
    QGridLayout  *boardLayout;
    QVector<QLabel*> cellLabels;

    QPushButton *rollButton;
    QPushButton *resetButton;
    QPushButton *shortestPathButton;
    QLabel      *diceLabel;
    QLabel      *positionLabel;
    QTextEdit   *logText;
};

#endif // GAMEWINDOW_H
