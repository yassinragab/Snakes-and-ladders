#include "BoardWidget.h"

#include <QPainter>
#include <QPaintEvent>
#include <QPainterPath>
#include <QVector2D>
#include <QtMath>

BoardWidget::BoardWidget(QWidget *parent)
    : QWidget(parent),
    m_numPlayers(0)
{
    setMinimumSize(500, 500);   // ensure a reasonable default size
}

QSize BoardWidget::sizeHint() const
{
    // preferred size the layout will try to give the board
    return QSize(700, 700);
}

void BoardWidget::updateBoard(const int playerPos[], const bool playerFinished[],
                              int numPlayers,
                              const int snakes[][2], int numSnakes,
                              const int ladders[][2], int numLadders)
{
    m_numPlayers = numPlayers;

    m_playerPos.clear();
    m_playerFinished.clear();
    for (int i = 0; i < numPlayers; ++i)
    {
        m_playerPos.push_back(playerPos[i]);
        m_playerFinished.push_back(playerFinished[i]);
    }

    m_snakes.clear();
    for (int i = 0; i < numSnakes; ++i)
        m_snakes.append(qMakePair(snakes[i][0], snakes[i][1]));

    m_ladders.clear();
    for (int i = 0; i < numLadders; ++i)
        m_ladders.append(qMakePair(ladders[i][0], ladders[i][1]));

    update();   // trigger repaint
}

void BoardWidget::computeBoardGeometry(int &cellSize, int &offsetX, int &offsetY) const
{
    const int rows = 10;
    const int cols = 10;

    // Square cells based on the smaller dimension
    int maxCellW = width()  / cols;
    int maxCellH = height() / rows;
    cellSize = qMin(maxCellW, maxCellH);

    if (cellSize <= 0)
    {
        cellSize = 1;
    }

    // Total pixel size of the board
    int boardW = cellSize * cols;
    int boardH = cellSize * rows;

    // Center the board in the widget
    offsetX = (width()  - boardW) / 2;
    offsetY = (height() - boardH) / 2;
}

QRect BoardWidget::cellRectFromNumber(int cellNum) const
{
    // cellNum is 1..100
    const int index = cellNum - 1;    // 0..99
    const int rowFromBottom = index / 10;  // 0 = bottom row
    const int colInRow      = index % 10;

    const int rows = 10;
    const int cols = 10;

    int cellSize, offsetX, offsetY;
    computeBoardGeometry(cellSize, offsetX, offsetY);

    int colFromLeft;
    if (rowFromBottom % 2 == 0)
        colFromLeft = colInRow;         // even row from bottom: left->right
    else
        colFromLeft = cols - 1 - colInRow;  // odd row from bottom: right->left

    const int rowFromTop = (rows - 1) - rowFromBottom;

    // Gap between cells
    int gap = cellSize / 8;                       // size of the gap
    int innerSize = cellSize - gap;               // actual drawable square size
    if (innerSize < 1) innerSize = 1;

    int x = offsetX + colFromLeft * cellSize + gap / 2;
    int y = offsetY + rowFromTop  * cellSize + gap / 2;

    return QRect(x, y, innerSize, innerSize);
}

QPointF BoardWidget::cellCenterFromNumber(int cellNum) const
{
    QRect r = cellRectFromNumber(cellNum);
    return r.center();
}

void BoardWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    drawGrid(p);
    drawLadders(p);
    drawSnakes(p);
    drawPlayers(p);
    drawNumbers(p);
}

void BoardWidget::drawGrid(QPainter &p)
{
    p.save();

    const int rows = 10;
    const int cols = 10;

    int cellSize, offsetX, offsetY;
    computeBoardGeometry(cellSize, offsetX, offsetY);

    int gap = cellSize / 8;
    int innerSize = cellSize - gap;
    if (innerSize < 1) innerSize = 1;

    p.setPen(QPen(Qt::black, 1));

    for (int r = 0; r < rows; ++r)
    {
        for (int c = 0; c < cols; ++c)
        {
            int x = offsetX + c * cellSize + gap / 2;
            int y = offsetY + r * cellSize + gap / 2;

            QRect cellRect(x, y, innerSize, innerSize);
            p.fillRect(cellRect, Qt::white);
            p.drawRect(cellRect);
        }
    }

    p.restore();
}

void BoardWidget::drawLadders(QPainter &p)
{
    p.save();

    QPen railPen(Qt::green, 4);
    railPen.setCapStyle(Qt::RoundCap);
    railPen.setJoinStyle(Qt::RoundJoin);
    p.setPen(railPen);

    for (const auto &ladder : m_ladders)
    {
        int start = ladder.first;   // bottom
        int end   = ladder.second;  // top
        if (start < 1 || start > 100 || end < 1 || end > 100)
            continue;

        QPointF a = cellCenterFromNumber(start);
        QPointF b = cellCenterFromNumber(end);

        // Direction from bottom to top
        QVector2D dir(b - a);
        if (dir.length() == 0.0f)
            continue;

        dir.normalize();
        QVector2D perp(-dir.y(), dir.x());  // perpendicular

        // Distance between the two side rails
        qreal halfWidth = 10.0;  // half the ladder width in pixels
        QPointF offset = perp.toPointF() * halfWidth;

        QPointF leftStart  = a - offset;
        QPointF leftEnd    = b - offset;
        QPointF rightStart = a + offset;
        QPointF rightEnd   = b + offset;

        // Draw side rails
        p.drawLine(leftStart, leftEnd);
        p.drawLine(rightStart, rightEnd);

        // Draw rungs
        int rungCount = 6;
        QPen rungPen(Qt::green, 3);
        rungPen.setCapStyle(Qt::RoundCap);
        p.setPen(rungPen);

        for (int i = 1; i < rungCount; ++i)
        {
            qreal t = qreal(i) / rungCount;   // between 0 and 1 along the ladder
            QPointF midLeft  = leftStart  + (leftEnd  - leftStart)  * t;
            QPointF midRight = rightStart + (rightEnd - rightStart) * t;
            p.drawLine(midLeft, midRight);
        }

        // Restore rail pen for next ladder
        p.setPen(railPen);
    }

    p.restore();
}

void BoardWidget::drawSnakes(QPainter &p)
{
    p.save();
    QPen pen(Qt::red, 5);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    p.setPen(pen);

    for (const auto &snake : m_snakes)
    {
        int head = snake.first;   // usually bigger number
        int tail = snake.second;  // smaller number
        if (head < 1 || head > 100 || tail < 1 || tail > 100)
            continue;

        QPointF a = cellCenterFromNumber(head);
        QPointF b = cellCenterFromNumber(tail);

        // Build a curvy path from head to tail
        QPainterPath path(a);

        QVector2D dir(b - a);
        QVector2D perp(-dir.y(), dir.x());
        if (dir.length() > 0.0f)
        {
            dir.normalize();
            perp.normalize();
        }

        // Control points for bezier to make a snake-like curve
        qreal amplitude = 20.0;  // how "curvy" the snake is

        QPointF c1 = a + (b - a) * 0.33 + perp.toPointF() * amplitude;
        QPointF c2 = a + (b - a) * 0.66 - perp.toPointF() * amplitude;

        path.cubicTo(c1, c2, b);

        p.drawPath(path);

        // Draw a simple snake "head" at the start (triangle)
        p.setBrush(Qt::red);
        QVector2D headDir = (dir.length() > 0.0f) ? dir : QVector2D(0, 1);
        headDir.normalize();
        QVector2D side(-headDir.y(), headDir.x());

        qreal headSize = 10.0;
        QPointF tip   = a + headDir.toPointF() * headSize;
        QPointF left  = a + side.toPointF() * headSize * 0.7;
        QPointF right = a - side.toPointF() * headSize * 0.7;

        QPolygonF headPoly;
        headPoly << tip << left << right;
        p.drawPolygon(headPoly);
    }

    p.restore();
}

void BoardWidget::drawPlayers(QPainter &p)
{
    p.save();

    for (int i = 0; i < m_numPlayers; ++i)
    {
        if (i >= m_playerPos.size() || i >= m_playerFinished.size())
            continue;

        if (m_playerFinished[i])
            continue;

        int cellNum = m_playerPos[i];
        if (cellNum < 1 || cellNum > 100)
            continue;

        QRect cell = cellRectFromNumber(cellNum);
        QPointF center = cell.center();

        // Slight vertical offset so multiple players don't overlap perfectly
        qreal radius = qMin(cell.width(), cell.height()) * 0.18;
        if (radius < 2.0) radius = 2.0;

        QPointF offset(0, - (i - (m_numPlayers - 1) / 2.0) * (radius * 1.8));

        p.setBrush(playerColor(i));
        p.setPen(Qt::black);
        p.drawEllipse(center + offset, radius, radius);
    }

    p.restore();
}

void BoardWidget::drawNumbers(QPainter &p)
{
    p.save();
    p.setPen(Qt::black);
    QFont f = p.font();
    f.setPointSize(8);
    p.setFont(f);

    for (int cellNum = 1; cellNum <= 100; ++cellNum)
    {
        QRect cell = cellRectFromNumber(cellNum);
        QRect textRect = cell.adjusted(2, 2, -2, -cell.height()/2); // top-left-ish

        p.drawText(textRect, Qt::AlignLeft | Qt::AlignTop,
                   QString::number(cellNum));
    }

    p.restore();
}

QColor BoardWidget::playerColor(int idx) const
{
    switch (idx)
    {
    case 0: return QColor("#1e90ff"); // blue
    case 1: return QColor("#ff5555"); // red
    case 2: return QColor("#55dd55"); // green
    case 3: return QColor("#ffaa00"); // orange
    default: return Qt::black;
    }
}
