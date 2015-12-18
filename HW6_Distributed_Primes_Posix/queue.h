#define MAX_QUEUE_NUMBERS 1000


struct queue{
    int front;
    int end;
    int numbers[MAX_QUEUE_NUMBERS];
    int size;
};

void enqueue(struct queue *q, int value)
{
    if(q->size < MAX_QUEUE_NUMBERS)
    {
        if(q->size == 0)
        {
            (q->front)++;
        }
        (q->end)++;
        q->numbers[q->end] = value;
        (q->size)++;
    }    
}

int dequeue(struct queue *q)
{
    if(q->size > 0)
    {
        int val = q->numbers[q->front];
        (q->front)++;
        (q->size)--;
        return val;
    }
    else
        return -1;
}

bool isempty(struct queue *q)
{
    return q->size == 0;
}