#include "Queue.h"

Queue::Queue(int cap) : front(0), rear(-1), capacity(cap), size(0)
{
    arr = new int[cap];
}

Queue::~Queue()
{
    delete[] arr;
}

void Queue::enqueue(int x)
{
    if (isFull())
        return;
    rear = (rear + 1) % capacity;
    arr[rear] = x;
    size++;
}

int Queue::dequeue()
{
    if (isEmpty())
        return -1;
    int item = arr[front];
    front = (front + 1) % capacity;
    size--;
    return item;
}

bool Queue::isEmpty() const
{
    return size == 0;
}

bool Queue::isFull() const
{
    return size == capacity;
}
