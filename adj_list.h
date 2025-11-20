#ifndef ADJ_LIST_H
#define ADJ_LIST_H

class adj_list
{
private:
    int v;        // number of vertices
    int **adj;    // adjacency list
    int *board;   // board representation with snakes and ladders

public:
    adj_list(int numSnakes, int snakes[][2], int numLadders, int ladders[][2]);
    ~adj_list();

    int* getNeighbors(int v);
    int getDegree(int v);
};

#endif // ADJ_LIST_H
