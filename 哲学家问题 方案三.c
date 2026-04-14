#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#define N 5
pthread_mutex_t chopsticks[N];

void *philosopher(void *num) {
    int id = *(int *)num;
    int left = id;
    int right = (id + 1) % N;

    while (1) {
        printf("AGV/哲学家 %d 正在工作/思考\n", id);
        sleep(rand() % 2);

        // 非对称拿取：核心
        if (id % 2 == 1) {
            // 奇数：先左后右
            pthread_mutex_lock(&chopsticks[left]);
            pthread_mutex_lock(&chopsticks[right]);
        } else {
            // 偶数：先右后左
            pthread_mutex_lock(&chopsticks[right]);
            pthread_mutex_lock(&chopsticks[left]);
        }

        printf("AGV/哲学家 %d 拿到两根充电桩，开始充电\n", id);
        sleep(rand() % 2);

        // 释放
        pthread_mutex_unlock(&chopsticks[left]);
        pthread_mutex_unlock(&chopsticks[right]);
        printf("AGV/哲学家 %d 充电完成\n", id);
    }
    return NULL;
}

int main() {
    pthread_t tid[N];
    int id[N];

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
