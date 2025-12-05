#ifndef STATSFORMATTER_H
#define STATSFORMATTER_H

#include <QString>

class StatsFormatter
{
public:
    static QString formatStats(int numPlayers,
                               const int turnsTaken[],
                               const int snakesHit[],
                               const int laddersClimbed[]);
};

#endif // STATSFORMATTER_H
