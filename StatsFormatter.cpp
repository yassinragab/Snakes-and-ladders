#include "StatsFormatter.h"

QString StatsFormatter::formatStats(int numPlayers,
                                    const int turnsTaken[],
                                    const int snakesHit[],
                                    const int laddersClimbed[])
{
    QString s;
    for (int p = 0; p < numPlayers; ++p)
    {
        s += QString("Player %1:\n").arg(p + 1);
        s += QString("  Turns: %1\n").arg(turnsTaken[p]);
        s += QString("  Snakes hit: %1\n").arg(snakesHit[p]);
        s += QString("  Ladders climbed: %1\n\n").arg(laddersClimbed[p]);
    }
    return s;
}
