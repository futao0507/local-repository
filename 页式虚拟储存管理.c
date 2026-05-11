#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define INSTRUCTION_COUNT 320
#define PAGE_SIZE 10  // 10条指令为1页，共32页
#define MIN_FRAMES 4
#define MAX_FRAMES 32

// 生成题目要求的指令序列
void generate_instructions(int *inst) {
    int i = 0;
    int s, m;
    srand((unsigned)time(NULL));
    while (i < INSTRUCTION_COUNT) {
        // (1) 随机选取起点s
        s = rand() % INSTRUCTION_COUNT;
        // (2) 执行指令s
        if (i < INSTRUCTION_COUNT) inst[i++] = s;
        // (3) 顺序执行s+1
        if (i < INSTRUCTION_COUNT) inst[i++] = (s + 1) % INSTRUCTION_COUNT;
        // (4) 在前地址[0,s]中随机选m
        if (s > 0) m = rand() % (s + 1);
        else m = 0;
        // (5) 执行m
        if (i < INSTRUCTION_COUNT) inst[i++] = m;
        // (6) 顺序执行m+1
        if (i < INSTRUCTION_COUNT) inst[i++] = (m + 1) % INSTRUCTION_COUNT;
        // (7) 在后地址[m+2, 319]中随机选s
        if (m + 2 <= 319) s = m + 2 + rand() % (319 - (m + 2) + 1);
        else s = rand() % INSTRUCTION_COUNT;
    }
}

// 将指令序列转换为页地址流
void get_page_stream(int *inst, int *pages) {
    for (int i = 0; i < INSTRUCTION_COUNT; i++) {
        pages[i] = inst[i] / PAGE_SIZE;
    }
}

// 生成完全随机的指令序列（扩展用）
void generate_random_instructions(int *inst) {
    srand((unsigned)time(NULL));
    for (int i = 0; i < INSTRUCTION_COUNT; i++) {
        inst[i] = rand() % INSTRUCTION_COUNT;
    }
}

// 生成循环顺序指令序列：0,1,2,...,319,0,1,...（扩展用）
void generate_loop_instructions(int *inst) {
    for (int i = 0; i < INSTRUCTION_COUNT; i++) {
        inst[i] = i % INSTRUCTION_COUNT;
    }
}

// OPT算法：最优置换
int opt(int *pages, int page_count, int frame_num) {
    int frames[frame_num];
    memset(frames, -1, sizeof(frames));
    int fault = 0;

    for (int i = 0; i < page_count; i++) {
        int current = pages[i];
        int found = 0;
        // 检查是否命中
        for (int j = 0; j < frame_num; j++) {
            if (frames[j] == current) {
                found = 1;
                break;
            }
        }
        if (found) continue;

        // 缺页
        fault++;
        int empty = -1;
        for (int j = 0; j < frame_num; j++) {
            if (frames[j] == -1) {
                empty = j;
                break;
            }
        }
        if (empty != -1) {
            frames[empty] = current;
            continue;
        }

        // 找未来最久不使用的页
        int replace_idx = 0;
        int farthest = i;
        for (int j = 0; j < frame_num; j++) {
            int k;
            for (k = i + 1; k < page_count; k++) {
                if (pages[k] == frames[j]) break;
            }
            if (k > farthest) {
                farthest = k;
                replace_idx = j;
            }
        }
        frames[replace_idx] = current;
    }
    return fault;
}

// FIFO算法：先进先出
int fifo(int *pages, int page_count, int frame_num) {
    int frames[frame_num];
    memset(frames, -1, sizeof(frames));
    int fault = 0;
    int pointer = 0;

    for (int i = 0; i < page_count; i++) {
        int current = pages[i];
        int found = 0;
        for (int j = 0; j < frame_num; j++) {
            if (frames[j] == current) {
                found = 1;
                break;
            }
        }
        if (found) continue;

        fault++;
        int empty = -1;
        for (int j = 0; j < frame_num; j++) {
            if (frames[j] == -1) {
                empty = j;
                break;
            }
        }
        if (empty != -1) {
            frames[empty] = current;
            continue;
        }

        // 置换队首
        frames[pointer] = current;
        pointer = (pointer + 1) % frame_num;
    }
    return fault;
}

// LRU算法：最近最少使用
int lru(int *pages, int page_count, int frame_num) {
    int frames[frame_num];
    int last_used[frame_num];
    memset(frames, -1, sizeof(frames));
    memset(last_used, -1, sizeof(last_used));
    int fault = 0;

    for (int i = 0; i < page_count; i++) {
        int current = pages[i];
        int found = 0;
        int hit_idx = -1;
        for (int j = 0; j < frame_num; j++) {
            if (frames[j] == current) {
                found = 1;
                hit_idx = j;
                break;
            }
        }
        if (found) {
            last_used[hit_idx] = i;
            continue;
        }

        fault++;
        int empty = -1;
        for (int j = 0; j < frame_num; j++) {
            if (frames[j] == -1) {
                empty = j;
                break;
            }
        }
        if (empty != -1) {
            frames[empty] = current;
            last_used[empty] = i;
            continue;
        }

        // 找最久未用的页
        int lru_idx = 0;
        int min_used = last_used[0];
        for (int j = 1; j < frame_num; j++) {
            if (last_used[j] < min_used) {
                min_used = last_used[j];
                lru_idx = j;
            }
        }
        frames[lru_idx] = current;
        last_used[lru_idx] = i;
    }
    return fault;
}

int main() {
    int inst[INSTRUCTION_COUNT];
    int pages[INSTRUCTION_COUNT];

    // 生成指令序列和页地址流
    generate_instructions(inst);
    get_page_stream(inst, pages);

    printf("=== 页式虚拟存储管理 - 页面置换算法测试 ===\n");
    printf("指令总数：%d，页大小：%d条指令/页，总页数：%d\n\n",
           INSTRUCTION_COUNT, PAGE_SIZE, INSTRUCTION_COUNT / PAGE_SIZE);

    // 遍历内存容量4~32页
    for (int frame = 4; frame <= 32; frame++) {
        int opt_fault = opt(pages, INSTRUCTION_COUNT, frame);
        int fifo_fault = fifo(pages, INSTRUCTION_COUNT, frame);
        int lru_fault = lru(pages, INSTRUCTION_COUNT, frame);

        double opt_hit = 1.0 - (double)opt_fault / INSTRUCTION_COUNT;
        double fifo_hit = 1.0 - (double)fifo_fault / INSTRUCTION_COUNT;
        double lru_hit = 1.0 - (double)lru_fault / INSTRUCTION_COUNT;

        printf("内存容量：%2d 页 | OPT 缺页：%3d 次 命中率：%.2f%% | FIFO 缺页：%3d 次 命中率：%.2f%% | LRU 缺页：%3d 次 命中率：%.2f%%\n",
               frame,
               opt_fault, opt_hit * 100,
               fifo_fault, fifo_hit * 100,
               lru_fault, lru_hit * 100);
    }
	  printf("\n");

	   // ---------- 扩展分析：随机序列 ----------
    int random_inst[INSTRUCTION_COUNT];
    int random_pages[INSTRUCTION_COUNT];
    generate_random_instructions(random_inst);
    get_page_stream(random_inst, random_pages);

    printf("********** 扩展分析 **********\n");
    printf("\n====== 随机序列 ======\n");
    printf("内存容量(页)\tOPT命中率\tFIFO命中率\tLRU命中率\n");
    for (int frame = MIN_FRAMES; frame <= MAX_FRAMES; frame++) {
        int opt_fault = opt(random_pages, INSTRUCTION_COUNT, frame);
        int fifo_fault = fifo(random_pages, INSTRUCTION_COUNT, frame);
        int lru_fault = lru(random_pages, INSTRUCTION_COUNT, frame);

        double opt_hit = 1.0 - (double)opt_fault / INSTRUCTION_COUNT;
        double fifo_hit = 1.0 - (double)fifo_fault / INSTRUCTION_COUNT;
        double lru_hit = 1.0 - (double)lru_fault / INSTRUCTION_COUNT;

        printf("%2d\t\t%.4f\t\t%.4f\t\t%.4f\n",
               frame, opt_hit, fifo_hit, lru_hit);
    }

    // ---------- 扩展分析：循环序列 ----------
    int loop_inst[INSTRUCTION_COUNT];
    int loop_pages[INSTRUCTION_COUNT];
    generate_loop_instructions(loop_inst);
    get_page_stream(loop_inst, loop_pages);

    printf("\n====== 循环序列 ======\n");
    printf("内存容量(页)\tOPT命中率\tFIFO命中率\tLRU命中率\n");
    for (int frame = MIN_FRAMES; frame <= MAX_FRAMES; frame++) {
        int opt_fault = opt(loop_pages, INSTRUCTION_COUNT, frame);
        int fifo_fault = fifo(loop_pages, INSTRUCTION_COUNT, frame);
        int lru_fault = lru(loop_pages, INSTRUCTION_COUNT, frame);

        double opt_hit = 1.0 - (double)opt_fault / INSTRUCTION_COUNT;
        double fifo_hit = 1.0 - (double)fifo_fault / INSTRUCTION_COUNT;
        double lru_hit = 1.0 - (double)lru_fault / INSTRUCTION_COUNT;

        printf("%2d\t\t%.4f\t\t%.4f\t\t%.4f\n",
               frame, opt_hit, fifo_hit, lru_hit);
    }

    return 0;
}

