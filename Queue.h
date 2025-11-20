#ifndef QUEUE_H
#define QUEUE_H

class Queue
{
private:
    int *arr;
    int front, rear, capacity, size;

public:
    Queue(int cap);
    ~Queue();
    void enqueue(int x);
    int dequeue();
    bool isEmpty() const;
    bool isFull() const;
};

#endif // QUEUE_H
