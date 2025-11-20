#ifndef BFS_H
#define BFS_H

#include "adj_list.h"
#include <vector>

class BFS
{
private:
    adj_list *graph;

public:
    explicit BFS(adj_list *g) : graph(g) {}

    // Old behavior (prints to console)
    void bfs(int startVertex);

    // New: compute shortest path and return it in pathOut (0-based indices)
    bool shortestPath(int startVertex, int targetVertex, std::vector<int> &pathOut);
};

#endif // BFS_H
