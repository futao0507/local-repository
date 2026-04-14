#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#define N 5

pthread_mutex_t chopsticks[N];
// 全局互斥锁：原子拿取两根筷子
pthread_mutex_t take_mutex;

void *philosopher(void *num) {
    int id = *(int *)num;
    int left = id;
    int right = (id + 1) % N;

    while (1) {
        printf("AGV/哲学家 %d 正在思考(工作)\n", id);
        sleep(rand() % 2);

        // 加锁：原子拿取
        pthread_mutex_lock(&take_mutex);

        printf("AGV/哲学家 %d 饥饿，拿左充电桩 %d\n", id, left);
        pthread_mutex_lock(&chopsticks[left]);
        printf("AGV/哲学家 %d 拿右充电桩 %d\n", id, right);
        pthread_mutex_lock(&chopsticks[right]);

        // 解锁拿取权限
        pthread_mutex_unlock(&take_mutex);

        printf("AGV/哲学家 %d 开始充电/就餐\n", id);
        sleep(rand() % 2);

        // 释放
        pthread_mutex_unlock(&chopsticks[right]);
        pthread_mutex_unlock(&chopsticks[left]);
        printf("AGV/哲学家 %d 结束充电\n", id);
    }
    return NULL;
}

int main() {
    pthread_t tid[N];
    int id[N];
    pthread_mutex_init(&take_mutex, NULL);

    for (int i = 0; i < N; i++) {
        pthread_mutex_init(&chopsticks[i], NULL);
        id[i] = i;
        pthread_create(&tid[i], NULL, philosopher, &id[i]);
    }

    for (int i = 0; i < N; i++) {
        pthread_join(tid[i], NULL);
    }
    return 0;
}
