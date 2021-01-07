#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
/*最大车辆数*/
#define MAXCARS 200
/*方向*/
#define SOUTH 's'
#define NORTH 'n'
#define EAST 'e'
#define WEST 'w'

/*各辆汽车的线程*/
pthread_t Cars[MAXCARS];
/*调度交通的线程*/
pthread_t Schedule;
/*解决死锁的线程*/
pthread_t solveDeadLovk;
/*各个象限的互斥锁*/
pthread_mutex_t A_quadrant_mutex;
pthread_mutex_t B_quadrant_mutex;
pthread_mutex_t C_quadrant_mutex;
pthread_mutex_t D_quadrant_mutex;
/*死锁的互斥锁*/
pthread_mutex_t deadLock_mutex;
/*最后一辆等待死锁解除的锁的互斥锁*/
pthread_mutex_t lastCarWaitForDeadLockSolved;
/*等待死锁解除的过程中不是最后一辆走的车的互斥锁*/
pthread_mutex_t south_wait_mutex;
pthread_mutex_t north_wait_mutex;
pthread_mutex_t west_wait_mutex;
pthread_mutex_t east_wait_mutex;

/*每条道路路口的互斥锁*/
pthread_mutex_t south_mutex;
pthread_mutex_t north_mutex;
pthread_mutex_t west_mutex;
pthread_mutex_t east_mutex;

/*检测到死锁的条件变量*/
pthread_cond_t deadLock_cond;
/*死锁处理完毕的条件变量*/
pthread_cond_t deadLockSolved_cond;

/*等待死锁解除的过程中不是最后一辆走的车的条件变量*/
pthread_cond_t south_wait_cond;
pthread_cond_t north_wait_cond;
pthread_cond_t west_wait_cond;
pthread_cond_t east_wait_cond;
/*A, B, C, D 四个象限是否是空的*/ 
bool AquadrantEmpty = true;
bool BquadrantEmpty = true;
bool CquadrantEmpty = true;
bool DquadrantEmpty = true;
/*东南西北 四个等待区是否是空的*/ 
bool waitingAreaEmpty_s=true;
bool waitingAreaEmpty_e=true;
bool waitingAreaEmpty_n=true;
bool waitingAreaEmpty_w=true;

/*是否死锁*/
bool deadLock;

/*最新进来的车的方向*/
char lastCar;

/*造成死锁的车的方向*/
char BadCar;

/*车的总数，各个方向车的数量*/
int carNum = 0;
int carNum_e=0;
int carNum_n=0;
int carNum_s=0;
int carNum_w=0;

/*优先队列*/
typedef struct QUEUE
{
    int front;
    int tail;
    int size;
    int id[MAXCARS];
    int ID;
} queue;
void queueInit(queue * Q)
{
    Q->front=0;
    Q->size=0;
    Q->tail=0;
    Q->ID=0;
}
void queueEnqueue(queue * Q, int i)
{
    Q->size++;
    Q->id[Q->tail]=i;
    Q->tail=(Q->tail+1)%MAXCARS;
}
void queueDequeue(queue * Q)
{
    Q->size--;
    if(Q->size<0)printf("Error: The queue is alreay empty!\n");
    Q->front=(Q->front+1)%MAXCARS;
}
int queueTop(queue * Q)
{
    if(Q->front==Q->tail)printf("Error: The queue is alreay empty!\n");
    return Q->id[Q->front];
}

/*四个方向等待的车辆的队列*/
queue queueNorth;
queue queueEast;
queue queueSouth;
queue queueWest;

/*车子行驶*/
void car_driving()
{
    usleep(30000);
}

/*死锁检测*/
bool isDeadLockDetected()
{
    return waitingAreaEmpty_s==false&&
        waitingAreaEmpty_e==false&&
        waitingAreaEmpty_n==false&&
        waitingAreaEmpty_w==false;
}

/*各变量初始化*/
void init()
{
    deadLock=false;

    queueInit(&queueNorth);
    queueInit(&queueSouth);
    queueInit(&queueEast);
    queueInit(&queueWest);
    pthread_mutex_init(&A_quadrant_mutex, NULL);
    pthread_mutex_init(&B_quadrant_mutex, NULL);
    pthread_mutex_init(&C_quadrant_mutex, NULL);
    pthread_mutex_init(&D_quadrant_mutex, NULL);
    pthread_mutex_init(&deadLock_mutex, NULL);
    pthread_mutex_init(&lastCarWaitForDeadLockSolved, NULL);
    pthread_mutex_init(&south_wait_mutex, NULL);
    pthread_mutex_init(&north_wait_mutex, NULL);
    pthread_mutex_init(&west_wait_mutex, NULL);
    pthread_mutex_init(&east_wait_mutex, NULL);

    pthread_mutex_init(&south_mutex, NULL);
    pthread_mutex_init(&north_mutex, NULL);
    pthread_mutex_init(&west_mutex, NULL);
    pthread_mutex_init(&east_mutex, NULL);

    pthread_cond_init(&deadLock_cond, NULL);
    pthread_cond_init(&deadLockSolved_cond, NULL);
    pthread_cond_init(&south_wait_cond,NULL);
    pthread_cond_init(&north_wait_cond, NULL);
    pthread_cond_init(&west_wait_cond, NULL);
    pthread_cond_init(&east_wait_cond, NULL);
}

/*车子在A象限行驶*/
void go_to_A(int carID, char direction)
{
    pthread_mutex_lock(&A_quadrant_mutex);
    car_driving();
    /*printf("car %d from ", carID);
    switch (direction)
    {
    case SOUTH:
        printf("South drive into A quadrant\n");
        break;
    case NORTH:
        printf("North drive into A quadrant\n");
        break;
    case WEST:
        printf("West drive into A quadrant\n");
        break;
    case EAST:
        printf("East drive into A quadrant\n");
        break;
    default:
        break;
    }*/
    pthread_mutex_unlock(&A_quadrant_mutex);
}

/*车子在B象限行驶*/
void go_to_B(int carID, char direction)
{
    pthread_mutex_lock(&B_quadrant_mutex);
    car_driving();
    /*printf("car %d from ", carID);
    switch (direction)
    {
    case SOUTH:
        printf("South drive into B quadrant\n");
        break;
    case NORTH:
        printf("North drive into B quadrant\n");
        break;
    case WEST:
        printf("West drive into B quadrant\n");
        break;
    case EAST:
        printf("East drive into B quadrant\n");
        break;
    default:
        break;
    }*/
    pthread_mutex_unlock(&B_quadrant_mutex);
}

/*车子在C象限行驶*/
void go_to_C(int carID, char direction)
{
    pthread_mutex_lock(&C_quadrant_mutex);
    car_driving();
    /*printf("car %d from ", carID);
    switch (direction)
    {
    case SOUTH:
        printf("South drive into C quadrant\n");
        break;
    case NORTH:
        printf("North drive into C quadrant\n");
        break;
    case WEST:
        printf("West drive into C quadrant\n");
        break;
    case EAST:
        printf("East drive into C quadrant\n");
        break;
    default:
        break;
    }*/
    pthread_mutex_unlock(&C_quadrant_mutex);
}

/*车子在D象限行驶*/
void go_to_D(int carID, char direction)
{
    pthread_mutex_lock(&D_quadrant_mutex);
    car_driving();
    /*printf("car %d from ", carID);
    switch (direction)
    {
    case SOUTH:
        printf("South drive into D quadrant\n");
        break;
    case NORTH:
        printf("North drive into D quadrant\n");
        break;
    case WEST:
        printf("West drive into D quadrant\n");
        break;
    case EAST:
        printf("East drive into D quadrant\n");
        break;
    default:
        break;
    }*/
    pthread_mutex_unlock(&D_quadrant_mutex);
}

/*south的车行驶*/
void *south_road(void *arg)
{
    /*车子从远处驶进队列*/
    car_driving();
    /*车子停在队列里*/
    pthread_mutex_lock(&south_mutex);

    /*车子进入等待区，等待队列里dequeue出一辆车*/
    int currentCarID=queueTop(&queueSouth);
    queueDequeue(&queueSouth);
    /*最新进入的车的方向更新*/
    lastCar=SOUTH;
    printf("car %d from South arrives at crossing\n",currentCarID);
    /*当前道路等待区状态更新*/
    waitingAreaEmpty_s=false;

    /*如果检测到死锁*/
    if(isDeadLockDetected())
    {
        /*造成死锁的车状态更新*/
        BadCar=SOUTH;
        /*死锁状态为真*/
        deadLock=true;
        /*发出死锁信号给死锁处理线程*/
        pthread_cond_signal(&deadLock_cond);

        /*等待其他3辆车走了这辆车才能走*/
        pthread_mutex_lock(&lastCarWaitForDeadLockSolved);
        pthread_cond_wait(&deadLockSolved_cond,&lastCarWaitForDeadLockSolved);
        pthread_mutex_unlock(&lastCarWaitForDeadLockSolved);

        /*死锁状态为假*/
        deadLock=false;
    }
    /*如果右手边有车*/
    else if(!waitingAreaEmpty_e)
    {
        /*等待右边的车将其唤醒*/
        pthread_mutex_lock(&south_wait_mutex);
        while(!waitingAreaEmpty_e)
        {
            pthread_cond_wait(&south_wait_cond, &south_wait_mutex);
        }
        pthread_mutex_unlock(&south_wait_mutex);
    }
    

    /*进入A象限*/
    go_to_A(currentCarID,SOUTH);
    waitingAreaEmpty_s=true;
    AquadrantEmpty=false;

    /*进入B象限*/
    go_to_B(currentCarID,SOUTH);
    AquadrantEmpty=true;
    BquadrantEmpty=false;
    BquadrantEmpty=true;

    /*离开*/
    printf("car %d from South leaving crossing\n",currentCarID);

    /*当前道路解锁*/
    pthread_mutex_unlock(&south_mutex);

    /*如果是死锁状态*/
    if(deadLock)
    {
        /*车子进入等待区，等待队列里dequeue出一辆车*/
        if(BadCar==WEST)
        {
            /*通知造成死锁的车可以走了，标志着死锁的结束*/
            pthread_cond_signal(&deadLockSolved_cond);
        }else
        {
            /*通知造成左边的车可以走了*/
            pthread_cond_signal(&west_wait_cond);
        }
    }else
    {
        /*通知造成左边的车可以走了*/
        pthread_cond_signal(&west_wait_cond);
    }
    
    return NULL;
}

/*north的车行驶*/
void *north_road(void *arg)
{
    /*车子从远处驶进队列*/
    car_driving();
    /*车子停在队列里*/
    pthread_mutex_lock(&north_mutex);

    /*车子进入等待区，等待队列里dequeue出一辆车*/
    int currentCarID=queueTop(&queueNorth);
    queueDequeue(&queueNorth);
    /*最新进入的车的方向更新*/
    lastCar=NORTH;
    printf("car %d from North arrives at crossing\n",currentCarID);
    /*当前道路等待区状态更新*/
    waitingAreaEmpty_n=false;

    /*如果检测到死锁*/
    if(isDeadLockDetected())
    {
        /*造成死锁的车状态更新*/
        BadCar=NORTH;
        /*死锁状态为真*/
        deadLock=true;
        /*发出死锁信号给死锁处理线程*/
        pthread_cond_signal(&deadLock_cond);

        /*等待其他3辆车走了这辆车才能走*/
        pthread_mutex_lock(&lastCarWaitForDeadLockSolved);
        pthread_cond_wait(&deadLockSolved_cond,&lastCarWaitForDeadLockSolved);
        pthread_mutex_unlock(&lastCarWaitForDeadLockSolved);

        /*死锁状态为假*/
        deadLock=false;
    }
    /*如果右手边有车*/
    else if(!waitingAreaEmpty_w)
    {
        /*等待右边的车将其唤醒*/
        pthread_mutex_lock(&north_wait_mutex);
        while(!waitingAreaEmpty_w)
        {
            pthread_cond_wait(&north_wait_cond, &north_wait_mutex);
        }
        pthread_mutex_unlock(&north_wait_mutex);
    }

    /*进入C象限*/
    waitingAreaEmpty_n=true;
    CquadrantEmpty=false;
    go_to_C(currentCarID,NORTH);
    
    /*进入D象限*/
    CquadrantEmpty=true;
    DquadrantEmpty=false;
    go_to_D(currentCarID,NORTH);
    DquadrantEmpty=true;
    
    /*离开*/
    printf("car %d from North leaving crossing\n",currentCarID);

    /*当前道路解锁*/
    pthread_mutex_unlock(&north_mutex);

    /*如果是死锁状态*/
    if(deadLock)
    {
        /*车子进入等待区，等待队列里dequeue出一辆车*/
        if(BadCar==EAST)
        {
            /*通知造成死锁的车可以走了，标志着死锁的结束*/
            pthread_cond_signal(&deadLockSolved_cond);
        }else
        {
            /*通知造成左边的车可以走了*/
            pthread_cond_signal(&east_wait_cond);
        }
    }else
    {
        /*通知造成左边的车可以走了*/
        pthread_cond_signal(&east_wait_cond);
    }
    
    return NULL;
}

/*west的车行驶*/
void *west_road(void *arg)
{
    /*车子从远处驶进队列*/
    car_driving();
    /*车子停在队列里*/
    pthread_mutex_lock(&west_mutex);

    /*车子进入等待区，等待队列里dequeue出一辆车*/
    int currentCarID=queueTop(&queueWest);
    queueDequeue(&queueWest);
    /*最新进入的车的方向更新*/
    lastCar=WEST;
    printf("car %d from West arrives at crossing\n",currentCarID);
    /*当前道路等待区状态更新*/
    waitingAreaEmpty_w=false;

    /*如果检测到死锁*/
    if(isDeadLockDetected())
    {
        /*造成死锁的车状态更新*/
        BadCar=WEST;
        /*死锁状态为真*/
        deadLock=true;
        /*发出死锁信号给死锁处理线程*/
        pthread_cond_signal(&deadLock_cond);

        /*等待其他3辆车走了这辆车才能走*/
        pthread_mutex_lock(&lastCarWaitForDeadLockSolved);
        pthread_cond_wait(&deadLockSolved_cond,&lastCarWaitForDeadLockSolved);
        pthread_mutex_unlock(&lastCarWaitForDeadLockSolved);

        /*死锁状态为假*/
        deadLock=false;
    }
    /*如果右手边有车*/
    else if(!waitingAreaEmpty_s)
    {
        /*等待右边的车将其唤醒*/
        pthread_mutex_lock(&west_wait_mutex);
        while(!waitingAreaEmpty_s)
        {
            pthread_cond_wait(&west_wait_cond, &west_wait_mutex);
        }
        pthread_mutex_unlock(&west_wait_mutex);
    }
    
    /*进入D象限*/
    waitingAreaEmpty_w=true;
    DquadrantEmpty=false;
    go_to_D(currentCarID,WEST);

    /*进入A象限*/
    DquadrantEmpty=true;
    AquadrantEmpty=false;
    go_to_A(currentCarID,WEST);
    AquadrantEmpty=true;

    /*离开*/
    printf("car %d from West leaving crossing\n",currentCarID);

    /*当前道路解锁*/
    pthread_mutex_unlock(&west_mutex);

    /*如果是死锁状态*/
    if(deadLock)
    {
        /*车子进入等待区，等待队列里dequeue出一辆车*/
        if(BadCar==NORTH)
        {
            /*通知造成死锁的车可以走了，标志着死锁的结束*/
            pthread_cond_signal(&deadLockSolved_cond);
        }else
        {
            /*通知造成左边的车可以走了*/
            pthread_cond_signal(&north_wait_cond);
        }
    }else
    {
        /*通知造成左边的车可以走了*/
        pthread_cond_signal(&north_wait_cond);
    }
    
    return NULL;
}

/*east的车行驶*/
void *east_road(void *arg)
{
    /*车子从远处驶进队列*/
    car_driving();
    /*车子停在队列里*/
    pthread_mutex_lock(&east_mutex);

    /*车子进入等待区，等待队列里dequeue出一辆车*/
    int currentCarID=queueTop(&queueEast);
    queueDequeue(&queueEast);
    /*最新进入的车的方向更新*/
    lastCar=EAST;
    printf("car %d from East arrives at crossing\n",currentCarID);
    /*当前道路等待区状态更新*/
    waitingAreaEmpty_e=false;

    /*如果检测到死锁*/
    if(isDeadLockDetected())
    {
        /*造成死锁的车状态更新*/
        BadCar=EAST;
        /*死锁状态为真*/
        deadLock=true;
        /*发出死锁信号给死锁处理线程*/
        pthread_cond_signal(&deadLock_cond);

        /*等待其他3辆车走了这辆车才能走*/
        pthread_mutex_lock(&lastCarWaitForDeadLockSolved);
        pthread_cond_wait(&deadLockSolved_cond,&lastCarWaitForDeadLockSolved);
        pthread_mutex_unlock(&lastCarWaitForDeadLockSolved);

        /*死锁状态为假*/
        deadLock=false;
    }
    /*如果右手边有车*/
    else if(!waitingAreaEmpty_n)
    {
        /*等待右边的车将其唤醒*/
        pthread_mutex_lock(&east_wait_mutex);
        while(!waitingAreaEmpty_n)
        {
            pthread_cond_wait(&east_wait_cond, &east_wait_mutex);
        }
        pthread_mutex_unlock(&east_wait_mutex);
    }
    
    /*进入B象限*/
    waitingAreaEmpty_e=true;
    BquadrantEmpty=false;
    go_to_B(currentCarID,EAST);
    
    /*进入C象限*/
    BquadrantEmpty=true;
    CquadrantEmpty=false;
    go_to_C(currentCarID,EAST);
    CquadrantEmpty=true;
    
    /*离开*/
    printf("car %d from East leaving crossing\n",currentCarID);

    /*当前道路解锁*/
    pthread_mutex_unlock(&east_mutex);

    /*如果是死锁状态*/
    if(deadLock)
    {
        /*车子进入等待区，等待队列里dequeue出一辆车*/
        if(BadCar==SOUTH)
        {
            /*通知造成死锁的车可以走了，标志着死锁的结束*/
            pthread_cond_signal(&deadLockSolved_cond);
        }else
        {
            /*通知造成左边的车可以走了*/
            pthread_cond_signal(&south_wait_cond);
        }
    }else
    {
        /*通知造成左边的车可以走了*/
        pthread_cond_signal(&south_wait_cond);
    }
    
    return NULL;
}

/*调度线程*/
void * CarSchedule(void *arg)
{
    printf("there is total %d cars\n",carNum);
    int num=carNum;
    while(carNum>0)
    {
        /*如果队列里还有车且等待区没车*/
        if(carNum_s>0&&waitingAreaEmpty_s)
        {
            carNum--;
            carNum_s--;
            pthread_create(&Cars[carNum+1], NULL, south_road, NULL);
        }
        if(carNum_n>0&&waitingAreaEmpty_n)
        {
            carNum--;
            carNum_n--;
            pthread_create(&Cars[carNum+1], NULL, north_road, NULL);
        }
        if(carNum_w>0&&waitingAreaEmpty_w)
        {
            carNum--;
            carNum_w--;
            pthread_create(&Cars[carNum+1], NULL, west_road, NULL);
        }
        if(carNum_e>0&&waitingAreaEmpty_e)
        {
            carNum--;
            carNum_e--;
            pthread_create(&Cars[carNum+1], NULL, east_road, NULL);
        }
    }

    /*等待所有线程结束*/
    for(int i=0;i<num;i++)
    {
        pthread_join(Cars[i],NULL);
    }
    printf("Byebye!\n");
}

/*解决死锁的线程*/
void *solve_DeadLock(void *arg)
{
    while(true)
    {
        /*死锁的互斥锁*/
        pthread_mutex_lock(&deadLock_mutex); 
        /*等待deadLock_cond的信号解锁*/
        pthread_cond_wait(&deadLock_cond,&deadLock_mutex); 
        /*死锁状态为真*/
        deadLock=true;
        printf("DEADLOCK: car jam detected, signalling");
        /*死锁处理，让最新进入的车的左边那辆车先走*/
        if(lastCar==WEST)
        {
            pthread_cond_signal(&north_wait_cond);
            printf(" North to go\n"); 
        }else if(lastCar==EAST)
        {
            pthread_cond_signal(&south_wait_cond);
            printf(" South to go\n"); 
        }else if(lastCar==SOUTH)
        {
            pthread_cond_signal(&west_wait_cond);
            printf(" West to go\n");
        }else if(lastCar==NORTH)
        {
            pthread_cond_signal(&east_wait_cond);
            printf(" East to go\n");
        }
        /*解锁*/
        pthread_mutex_unlock(&deadLock_mutex); 
    }
}

int main()
{
    printf("This is a Traffic DeadLock Problem!:)\n");
    printf("print s to push a car on the road to the south \n");
    printf("print n to push a car on the road to the north \n");
    printf("print e to push a car on the road to the east \n");
    printf("print w to push a car on the road to the west \n");

    char input[MAXCARS];
    scanf("%s",input);
    char c;
    carNum=strlen(input);
    for(int i=0;i<carNum;i++)
    {
        c=input[i];
        switch (c)
        {
        case SOUTH:
            carNum_s++;
            queueEnqueue(&queueSouth,i+1);
            break;
        case NORTH:
            carNum_n++;
            queueEnqueue(&queueNorth,i+1);
            break;
        case WEST:
            carNum_w++;
            queueEnqueue(&queueWest,i+1);
            break;
        case EAST:
            carNum_e++;
            queueEnqueue(&queueEast,i+1);
            break;
        }
        
    }
    pthread_create(&Schedule,NULL,CarSchedule,NULL);
    pthread_create(&solveDeadLovk,NULL,solve_DeadLock,NULL);

    /*等待schedule线程结束*/
    pthread_join(Schedule,NULL);
}