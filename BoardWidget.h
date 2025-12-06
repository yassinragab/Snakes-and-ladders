#ifndef BOARDWIDGET_H
#define BOARDWIDGET_H

#include <QWidget>
#include <QVector>
#include <QPair>

class BoardWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BoardWidget(QWidget *parent = nullptr);

    QSize sizeHint() const override;

    // Copy game state into the widget & trigger repaint
    void updateBoard(const int playerPos[], const bool playerFinished[],
                     int numPlayers,
                     const int snakes[][2], int numSnakes,
                     const int ladders[][2], int numLadders);

    void startCelebration();
    void clearCelebration();


protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int m_numPlayers;
    QVector<int>  m_playerPos;
    QVector<bool> m_playerFinished;
    QVector<QPair<int,int>> m_snakes;   // (start, end)
    QVector<QPair<int,int>> m_ladders;  // (start, end)

    bool m_celebrating = false;

    // Helpers
    void  computeBoardGeometry(int &cellSize, int &offsetX, int &offsetY) const;
    QRect cellRectFromNumber(int cellNum) const;   // 1..100
    QPointF cellCenterFromNumber(int cellNum) const;

    void drawGrid(QPainter &p);
    void drawLadders(QPainter &p);
    void drawSnakes(QPainter &p);
    void drawPlayers(QPainter &p);
    void drawNumbers(QPainter &p);
    void drawCelebration(QPainter &p);


    QColor playerColor(int idx) const;
};

#endif // BOARDWIDGET_H
