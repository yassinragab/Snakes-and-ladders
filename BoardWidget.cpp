#include "BoardWidget.h"

#include <QPainter>
#include <QPaintEvent>
#include <QPainterPath>
#include <QVector2D>
#include <QtMath>

BoardWidget::BoardWidget(QWidget *parent)
    : QWidget(parent),
    m_numPlayers(0),
    m_celebrating(false)
{
    setMinimumSize(500, 500);
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

void BoardWidget::startCelebration()
{
    m_celebrating = true;                 // turn on celebration
    update();
}

void BoardWidget::clearCelebration()
{
    m_celebrating = false;
    update();        // trigger repaint without the star
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
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int rows = 10;
    int cols = 10;
    int cellW = width()  / cols;
    int cellH = height() / rows;

    //  Draw coloured background squares (alternating colours)
    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            QRect cellRect(c * cellW, r * cellH, cellW, cellH);

            bool isDark = ((r + c) % 2 == 0); // checker pattern

            QColor lightColor("#aee6a5"); // light green
            QColor darkColor ("#6cc96f"); // medium green
            painter.fillRect(cellRect, isDark ? lightColor : darkColor);

            // optional border
            painter.setPen(QPen(Qt::black, 1));
            painter.drawRect(cellRect);
        }
    }
    // Draw square numbers using snake pattern (1→10 left→right)
    painter.setFont(QFont("Arial", 12, QFont::Bold));

    for (int rowFromTop = 0; rowFromTop < rows; rowFromTop++)
    {
        int rowFromBottom = rows - 1 - rowFromTop;   // convert

        for (int col = 0; col < cols; col++)
        {
            int colSnake;
            if (rowFromBottom % 2 == 0)
                colSnake = col;           // even row → left to right
            else
                colSnake = cols - 1 - col; // odd row → right to left

            int cellNum = rowFromBottom * cols + colSnake + 1;

            QRect rect(col * cellW, rowFromTop * cellH, cellW, cellH);
            painter.drawText(rect, Qt::AlignCenter, QString::number(cellNum));
        }
    }
      //  Draw snakes
    drawSnakes(painter);

    //  Draw ladders
    drawLadders(painter);

    // Draw players
    drawPlayers(painter);

    //Celebration overlay (if someone won)
    drawCelebration(painter);
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

    QColor ladderColor(0, 0, 139);
    QPen railPen(ladderColor, 4);
    railPen.setCapStyle(Qt::RoundCap);
    railPen.setJoinStyle(Qt::RoundJoin);
    p.setPen(railPen);

    for (const auto &ladder : m_ladders)
    {
        int start = ladder.first;
        int end   = ladder.second;
        if (start < 1 || start > 100 || end < 1 || end > 100)
            continue;

        QPointF a = cellCenterFromNumber(start);
        QPointF b = cellCenterFromNumber(end);

        QVector2D dir(b - a);
        if (dir.length() == 0.0f) continue;

        dir.normalize();
        QVector2D perp(-dir.y(), dir.x());

        qreal halfWidth = 10.0;
        QPointF offset = perp.toPointF() * halfWidth;

        QPointF leftStart  = a - offset;
        QPointF leftEnd    = b - offset;
        QPointF rightStart = a + offset;
        QPointF rightEnd   = b + offset;

        p.drawLine(leftStart, leftEnd);
        p.drawLine(rightStart, rightEnd);

        int rungCount = 6;
        QPen rungPen(ladderColor, 3);
        rungPen.setCapStyle(Qt::RoundCap);
        p.setPen(rungPen);

        for (int i = 1; i < rungCount; ++i)
        {
            qreal t = qreal(i) / rungCount;
            QPointF midLeft  = leftStart  + (leftEnd  - leftStart)  * t;
            QPointF midRight = rightStart + (rightEnd - rightStart) * t;
            p.drawLine(midLeft, midRight);
        }

        p.setPen(railPen);
    }

    p.restore();  // <- CLEAN line 252
}

void BoardWidget::drawSnakes(QPainter &p)
{
    p.save();

    // 🟡 Dark yellow snake colour
    QColor snakeColor(204, 153, 0);          // golden dark yellow
    QPen pen(snakeColor, 5);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    p.setPen(pen);

    for (const auto &snake : m_snakes)
    {
        int head = snake.first;   // top
        int tail = snake.second;  // bottom
        if (head < 1 || head > 100 || tail < 1 || tail > 100)
            continue;

        QPointF a = cellCenterFromNumber(head);
        QPointF b = cellCenterFromNumber(tail);

        QVector2D dir(b - a);
        QVector2D perp(-dir.y(), dir.x());
        if (dir.length() > 0.0f)
        {
            dir.normalize();
            perp.normalize();
        }

        // 👉 Shift snake sideways so the head is not on the number
        QRect headCell = cellRectFromNumber(head);
        qreal sideOffset = headCell.width() * 0.35;         // tweak 0.30–0.45 if you like
        QPointF sideShift = perp.toPointF() * sideOffset;

        QPointF aShift = a + sideShift;
        QPointF bShift = b + sideShift;

        // Curvy snake path from shifted head to shifted tail
        QPainterPath path(aShift);

        qreal amplitude = 20.0;
        QPointF c1 = aShift + (bShift - aShift) * 0.33 + perp.toPointF() * amplitude;
        QPointF c2 = aShift + (bShift - aShift) * 0.66 - perp.toPointF() * amplitude;

        path.cubicTo(c1, c2, bShift);
        p.drawPath(path);

        // 🐍 Snake head triangle, also using shifted head position
        p.setBrush(snakeColor);

        QVector2D headDir = (dir.length() > 0.0f ? dir : QVector2D(0, 1));
        headDir.normalize();
        QVector2D side(-headDir.y(), headDir.x());

        qreal headSize = 10.0;
        QPointF base = aShift;   // shifted, not in the center

        QPointF tip   = base + headDir.toPointF() * headSize;
        QPointF left  = base + side.toPointF() * headSize * 0.7;
        QPointF right = base - side.toPointF() * headSize * 0.7;

        QPolygonF headPoly;
        headPoly << tip << left << right;
        p.drawPolygon(headPoly);
    }

    p.restore();
}


void BoardWidget::drawCelebration(QPainter &p)     // ADD NEW
{
    if (!m_celebrating)
        return;   // nothing to draw if no celebration

    p.save();

    // We celebrate on the last square (100)
    QRect cell = cellRectFromNumber(100);
    QPointF center = cell.center();

    // Big yellow star
    QPen pen(Qt::yellow, 3);
    p.setPen(pen);
    p.setBrush(QColor(255, 255, 180));

    const int points = 10;            // 10-point star
    const qreal outerR = cell.width() * 0.7;
    const qreal innerR = cell.width() * 0.35;

    QPolygonF star;
    for (int i = 0; i < points; ++i)
    {
        qreal angle = i * 2 * M_PI / points;   // needs <QtMath>, you already have it
        qreal r = (i % 2 == 0) ? outerR : innerR;
        QPointF pt(center.x() + r * qCos(angle),
                   center.y() + r * qSin(angle));
        star << pt;
    }

    p.drawPolygon(star);
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

        // Smaller radius so they fit nicely in corners
        qreal radius = qMin(cell.width(), cell.height()) * 0.12;
        if (radius < 2.0) radius = 2.0;

        // Distance from center to place the circles
        qreal dx = radius * 1.6;
        qreal dy = radius * 1.6;

        QPointF pos = center;

        // Place players in different corners of the cell
        switch (i)
        {
        case 0: // Player 1 – top-left
            pos = QPointF(center.x() - dx * 1.6, center.y() - dy);
            break;

        case 1: // Player 2 – top-right
            pos = QPointF(center.x() + dx * 1.0, center.y() - dy);
            break;

        case 2: // Player 3 – bottom-left
            pos = QPointF(center.x() - dx * 1.6, center.y() + dy * 2.0);
            break;

        case 3: // Player 4 – bottom-right
            pos = QPointF(center.x() + dx * 1.0, center.y() + dy * 2.0);
            break;
        }

        p.setBrush(playerColor(i));
        p.setPen(Qt::black);
        p.drawEllipse(pos, radius, radius);
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
    case 3: return QColor("#8000ff"); // purple
    default: return Qt::black;
    }
}

