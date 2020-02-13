#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

sem_t counter; /** semaphores **/
sem_t monitor;
sem_t collector;
sem_t s;
sem_t e;
int mcounter =0;
int mmonitor =0;
int mcollector=0;
int SIZE=2; /** size of buffer **/
int N=20; /** number of messages **/

class Queue
{
public:
    int* array;
    int front;
    int rear;
    int size;
    int limit;
};

Queue* create_Q()
{
    Queue* queue = new Queue();
    queue->limit = SIZE;
    queue->front = 0;
    queue->rear = SIZE - 1;
    queue->size = 0;
    queue->array = new int[(SIZE*sizeof(int))];
    return queue;
}

Queue* queue = create_Q(); /** create buffer **/

int isFull()
{
    if(queue->size == queue->limit)
        return(1);
    else
        return(0);
}

bool isEmpty()
{
    if(queue->size == 0)
        return(1);
    else
        return(0);
}

void enqueue()
{
    if (isFull() == 1)
    {
        std::cout <<"Monitor thread  : Buffer full!!" << std::endl;
        return;
    }
    queue->rear = (queue->rear + 1) % SIZE;
    queue->array[queue->rear] = 1; /** insert 1 msg in the buffer **/
    queue->size = queue->size + 1;
    mmonitor--; /** decrease the sum of msgs **/
    std::cout << "Monitor thread  : writing to buffer at position " << queue->rear << " Monitor now " << mmonitor <<std::endl;
}

void dequeue()
{
    if (isEmpty() == 1)
    {
        std::cout <<"Collector thread: Nothing is in the buffer!!" << std::endl ;
        return;
    }

    int position = queue->front;
    queue->front = (queue->front + 1) % SIZE;
    mcollector ++;
    queue->size = queue->size - 1;
    std::cout <<"Collector thread: reading from the buffer at position "<< position << std::endl;
}

void produce()
{
    //std::cout << "Producer waiting for the buffer .." << std::endl;
    sem_wait(&e);
    sem_wait(&s);
    sem_wait(&monitor);
    enqueue();
    sem_post(&monitor);
    sem_post(&s);
    //std::cout << "Producer left .." << std::endl;
}

void consume()
{
    //std::cout << "Consumer waiting for the buffer .." << std::endl;
    sem_wait(&s);
    sem_wait(&collector);
    dequeue();
    sem_post(&collector);
    sem_post(&s);
    sem_post(&e);
    //std::cout << "Consumer left .." << std::endl;
}

void check_counter()
{
    std::cout << "Monitor thread  : waiting to read counter " << std::endl;
    sem_wait(&counter);
    sem_wait(&monitor);
    if(mcounter != 0)
    {
        mmonitor += mcounter;
        std::cout << "Monitor thread  : reading a count value of " << mcounter << std::endl;
        mcounter =0;
    }else
    {
        std::cout << "Monitor thread  : reading a count value of " << mcounter << std::endl;
    }
    sem_post(&monitor);
    sem_post(&counter);
}

void* thread_mcounter(void* arg)
{
    int thread_id = *((int*) arg);
    std::cout<< "Counter thread " << thread_id << ": received a message" << std::endl;
    std::cout<< "Counter thread " << thread_id << ": Waiting to write.."<< std::endl;
    sem_wait(&counter);
    sleep(4);
    mcounter++;
    std::cout<< "Counter thread " << thread_id <<": Now adding to counter .. counter value is " <<mcounter<<std::endl;
    sem_post(&counter);
}

void* thread_mmonitor(void* arg)
{
    while(true)
    {
        check_counter();
        float t1 = rand() % 10;
        if(mmonitor !=0 )
        {
            produce(); /** add to buffer **/
        }
        sleep(t1);
    }
}

void* thread_mcollector(void* arg)
{
    while(mcollector < N)
    {
        float t1 = rand() % 10;
        consume(); /** take from buffer **/
        sleep(t1);
    }
}

int main(void)
{
    pthread_t monitorr,collectorr;
    pthread_create(&monitorr,NULL,thread_mmonitor,NULL); /** monitor thread **/
    pthread_create(&collectorr,NULL,thread_mcollector,NULL); /** collector thread **/

    sem_init(&counter, 0, 1);
    sem_init(&monitor, 0, 1);
    sem_init(&collector, 0, 1);
    sem_init(&s, 0, 1);
    sem_init(&e, 0, SIZE);
    pthread_t t[N];

    for(int i=1; i<=N; i++) /** send messages **/
    {
        float t1 = rand() % 10;
        pthread_create(&t[i],NULL,thread_mcounter,(void *) &i);
        sleep(t1);
    }

    for(int i=1; i<=N; i++)
    {
        pthread_join(t[i],NULL);
    }

    pthread_join(collectorr,NULL);

    sem_destroy(&counter);
    sem_destroy(&monitor);
    sem_destroy(&collector);
    sem_destroy(&s);
    sem_destroy(&e);

    //std::cout<< "mcounter is: " << mcounter << std::endl;
    //std::cout<< "mmonitor is: " << mmonitor << std::endl;
    //std::cout<< "mcollector is: " << mcollector << std::endl;

    return 0;
}
