#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>

#define N 5    // 5个哲学家/AGV
#define LIMIT 4 // 最多允许4人同时拿筷子

// 筷子/充电桩：互斥锁
pthread_mutex_t chopsticks[N];
// 限制并发数的信号量
sem_t limit_sem;

// 哲学家线程函数
void *philosopher(void *num) {
    int id = *(int *)num;
    int left = id;
    int right = (id + 1) % N;

    while (1) {
        // 思考
        printf("AGV/哲学家 %d 正在思考(工作)\n", id);
        sleep(rand() % 2);

        // 申请进入：最多4人
        sem_wait(&limit_sem);

        // 饥饿，拿左筷子
        printf("AGV/哲学家 %d 饥饿，尝试拿左充电桩/筷子 %d\n", id, left);
        pthread_mutex_lock(&chopsticks[left]);
        printf("AGV/哲学家 %d 拿到左充电桩/筷子 %d\n", id, left);

        // 拿右筷子
        printf("AGV/哲学家 %d 尝试拿右充电桩/筷子 %d\n", id, right);
        pthread_mutex_lock(&chopsticks[right]);
        printf("AGV/哲学家 %d 拿到右充电桩/筷子 %d，开始充电/就餐\n", id, right);

        // 充电/就餐
        sleep(rand() % 2);

        // 放下筷子
        pthread_mutex_unlock(&chopsticks[right]);
        pthread_mutex_unlock(&chopsticks[left]);
        printf("AGV/哲学家 %d 结束充电/就餐，放下两根充电桩/筷子\n", id);

        // 释放名额
        sem_post(&limit_sem);
    }
    return NULL;
}

int main() {
    pthread_t tid[N];
    int id[N];

    // 初始化互斥锁和信号量
    sem_init(&limit_sem, 0, LIMIT);
    for (int i = 0; i < N; i++) {
        pthread_mutex_init(&chopsticks[i], NULL);
        id[i] = i;
    }

    // 创建线程
    for (int i = 0; i < N; i++) {
        pthread_create(&tid[i], NULL, philosopher, &id[i]);
    }

    // 等待线程
    for (int i = 0; i < N; i++) {
        pthread_join(tid[i], NULL);
    }

    return 0;
}
