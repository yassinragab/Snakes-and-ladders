#include "BFS.h"
#include "Queue.h"
#include <iostream>

using namespace std;

bool BFS::shortestPath(int startVertex, int targetVertex, std::vector<int> &pathOut)
{
    const int N = 100;
    int *visited = new int[N];
    int *parent  = new int[N];

    for (int i = 0; i < N; i++)
    {
        visited[i] = 0;
        parent[i]  = -1;
    }

    Queue q(N);
    visited[startVertex] = 1;
    q.enqueue(startVertex);

    bool found = false;

    while (!q.isEmpty())
    {
        int current = q.dequeue();

        if (current == targetVertex)
        {
            found = true;
            break;
        }

        int *neighbors = graph->getNeighbors(current);
        int count      = graph->getDegree(current);

        for (int i = 0; i < count; i++)
        {
            int next = neighbors[i];

            if (!visited[next])
            {
                visited[next] = 1;
                parent[next]  = current;
                q.enqueue(next);
            }
        }
    }

    if (found)
    {
        pathOut.clear();
        int crawl = targetVertex;
        while (crawl != -1)
        {
            pathOut.push_back(crawl);
            crawl = parent[crawl];
        }
        // pathOut is from target back to start; caller can reverse
    }

    delete[] visited;
    delete[] parent;

    return found;
}


