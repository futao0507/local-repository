#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define N 5 // 哲学家数量

// 状态定义
#define THINKING 0
#define HUNGRY 1
#define EATING 2

// 全局变量
int state[N]; // 存储每个哲学家的状态
sem_t mutex;  // 保护状态变量的互斥锁
sem_t chopstick[N]; // 信号量数组，代表5根筷子
sem_t room;   // 【关键】限制进入房间的人数，最多4人

// 随机休眠函数，模拟思考或吃饭的时间 (0-2秒)
void random_sleep() {
    int sec = rand() % 3;
    sleep(sec);
}

// 打印日志函数
void log_action(int id, const char* action) {
    printf("哲学家 %d: %s\n", id, action);
}

// 哲学家线程函数
void* philosopher(void* arg) {
    int id = *(int*)arg;
    free(arg); // 释放动态分配的内存

    while (1) {
        // 1. 思考阶段
        log_action(id, "正在思考 ??");
        random_sleep();

        // 2. 感到饥饿，准备拿筷子
        log_action(id, "感到饥饿 ??");
        
        // 【核心逻辑A】进入房间前先申请名额 (最多4个)
        sem_wait(&room);
        
        // 申请互斥锁，保护状态修改
        sem_wait(&mutex);
        state[id] = HUNGRY;
        sem_post(&mutex); // 立即释放锁，让其他哲学家能检查状态

        // 尝试拿左右筷子
        // 拿左边筷子
        sem_wait(&chopstick[id]);
        // 拿右边筷子 (id+1)%N
        sem_wait(&chopstick[(id+1)%N]);

        // 3. 开始吃饭
        sem_wait(&mutex);
        state[id] = EATING;
        log_action(id, "拿起左右筷子，开始吃饭 ??");
        sem_post(&mutex);

        random_sleep(); // 吃饭时间

        // 4. 放下筷子
        sem_post(&chopstick[id]);
        sem_post(&chopstick[(id+1)%N]);
        log_action(id, "放下筷子，结束用餐");

        // 【核心逻辑A】吃完离开，释放名额 (V操作)
        sem_post(&room);

        sem_wait(&mutex);
        state[id] = THINKING;
        sem_post(&mutex);
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t tid[N];
    int i;

    // 初始化随机数种子
    srand((unsigned)time(NULL));

    // 初始化信号量
    sem_init(&mutex, 0, 1);      // 互斥锁，初始值1
    sem_init(&room, 0, 4);       // 限制4人，初始值4 (方案A的核心)
    
    for(i = 0; i < N; i++) {
        sem_init(&chopstick[i], 0, 1); // 每根筷子初始值1
        state[i] = THINKING;     // 初始状态为思考
    }

    // 创建哲学家线程
    for(i = 0; i < N; i++) {
        int* p = (int*)malloc(sizeof(int));
        *p = i;
        pthread_create(&tid[i], NULL, philosopher, p);
    }

    // 等待线程结束 (实际上会无限循环，需手动Ctrl+C终止)
    for(i = 0; i < N; i++) {
        pthread_join(tid[i], NULL);
    }

    // 销毁信号量 (理论上执行不到)
    sem_destroy(&mutex);
    sem_destroy(&room);
    for(i = 0; i < N; i++) {
        sem_destroy(&chopstick[i]);
    }

    return 0;
}

