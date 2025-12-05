#ifndef BOARDGRID_H
#define BOARDGRID_H

#include <QWidget>
#include <QVector>
#include "GameEngine.h" // Needs to know engine state to draw

class QGridLayout;
class QLabel;

class BoardGrid : public QWidget
{
    Q_OBJECT
public:
    explicit BoardGrid(QWidget *parent = nullptr);

    // Main function called by GameWindow to redraw everything
    void updateVisuals(const GameEngine& engine);

private:
    QGridLayout *layout;
    QVector<QLabel*> cellLabels;

    void setupGrid();
    QString getPlayerColor(int index) const;
};

#endif // BOARDGRID_H
