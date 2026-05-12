#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define INSTRUCTION_COUNT 320
#define PAGE_SIZE 10  // 每10条指令为1页
#define VIRTUAL_PAGES 32  // 虚存容量32页
#define MIN_FRAMES 4   // 最小物理内存页面数
#define MAX_FRAMES 32  // 最大物理内存页面数

/* 生成320条指令地址 */
void generate_instructions(int *instructions) {
    int s = rand() % 320;  // 随机选择起始地址

    for (int i = 0; i < INSTRUCTION_COUNT; i++) {
        instructions[i] = s;

        // 根据概率分布生成下一条指令地址
        int r = rand() % 100;

        if (r < 50) {
            // 50%概率：顺序执行下一条指令
            s = (s + 1) % 320;
        } else if (r < 75) {
            // 25%概率：在前地址[0, s]中随机选取
            if (s > 0) {
                s = rand() % (s + 1);
            }
        } else {
            // 25%概率：在后地址[s+2, 319]中随机选取
            if (s + 2 <= 319) {
                s = (s + 2) + rand() % (320 - s - 1);
            } else {
                s = (s + 1) % 320;
            }
        }
    }
}

/* 将指令地址转换为页地址流 */
void convert_to_pages(int *instructions, int *pages) {
    for (int i = 0; i < INSTRUCTION_COUNT; i++) {
        pages[i] = instructions[i] / PAGE_SIZE;
    }
}

/* 查找页面是否在内存中 */
bool find_page(int *frames, int frame_count, int page) {
    for (int i = 0; i < frame_count; i++) {
        if (frames[i] == page) {
            return true;
        }
    }
    return false;
}

/* 获取空闲帧位置，如果没有空闲帧返回-1 */
int get_free_frame(int *frames, int frame_count) {
    for (int i = 0; i < frame_count; i++) {
        if (frames[i] == -1) {
            return i;
        }
    }
    return -1;
}

/*
 * FIFO（先进先出）页面置换算法
 * 返回缺页次数
 */
int fifo_algorithm(int *pages, int frame_count) {
    int frames[32];
    int front = 0;
    int page_fault = 0;

    // 初始化帧数组
    for (int i = 0; i < frame_count; i++) {
        frames[i] = -1;
    }

    // 遍历所有页面引用
    for (int i = 0; i < INSTRUCTION_COUNT; i++) {
        int page = pages[i];

        // 检查页面是否已在内存中
        if (!find_page(frames, frame_count, page)) {
            page_fault++;

            // 查找空闲帧
            int free_frame = get_free_frame(frames, frame_count);

            if (free_frame != -1) {
                // 有空闲帧，直接放入
                frames[free_frame] = page;
            } else {
                // 需要置换：选择队首的页面换出
                frames[front] = page;
                front = (front + 1) % frame_count;
            }
        }
    }

    return page_fault;
}

/*
 * LRU（最近最少使用）页面置换算法
 * 返回缺页次数
 */
int lru_algorithm(int *pages, int frame_count) {
    int frames[32];
    int use_time[32];  // 记录每页的最后使用时间
    int page_fault = 0;
    int current_time = 0;

    // 初始化帧数组
    for (int i = 0; i < frame_count; i++) {
        frames[i] = -1;
        use_time[i] = 0;
    }

    // 遍历所有页面引用
    for (int i = 0; i < INSTRUCTION_COUNT; i++) {
        int page = pages[i];
        current_time++;

        // 检查页面是否已在内存中
        if (!find_page(frames, frame_count, page)) {
            page_fault++;

            // 查找空闲帧
            int free_frame = get_free_frame(frames, frame_count);

            if (free_frame != -1) {
                // 有空闲帧，直接放入
                frames[free_frame] = page;
                use_time[free_frame] = current_time;
            } else {
                // 需要置换：选择最久未使用的页面换出
                int lru_index = 0;
                for (int j = 1; j < frame_count; j++) {
                    if (use_time[j] < use_time[lru_index]) {
                        lru_index = j;
                    }
                }
                frames[lru_index] = page;
                use_time[lru_index] = current_time;
            }
        } else {
            // 页面已在内存中，更新其使用时间
            for (int j = 0; j < frame_count; j++) {
                if (frames[j] == page) {
                    use_time[j] = current_time;
                    break;
                }
            }
        }
    }

    return page_fault;
}

/*
 * OPT（最优）页面置换算法
 * 返回缺页次数
 */
int opt_algorithm(int *pages, int frame_count) {
    int frames[32];
    int page_fault = 0;

    // 初始化帧数组
    for (int i = 0; i < frame_count; i++) {
        frames[i] = -1;
    }

    // 遍历所有页面引用
    for (int i = 0; i < INSTRUCTION_COUNT; i++) {
        int page = pages[i];

        // 检查页面是否已在内存中
        if (!find_page(frames, frame_count, page)) {
            page_fault++;

            // 查找空闲帧
            int free_frame = get_free_frame(frames, frame_count);

            if (free_frame != -1) {
                // 有空闲帧，直接放入
                frames[free_frame] = page;
            } else {
                // 需要置换：选择未来最久不会被使用的页面换出
                int furthest_use = -1;
                int replace_index = 0;

                for (int j = 0; j < frame_count; j++) {
                    // 查找frames[j]在未来何时会被使用
                    int next_use = -1;
                    for (int k = i + 1; k < INSTRUCTION_COUNT; k++) {
                        if (pages[k] == frames[j]) {
                            next_use = k;
                            break;
                        }
                    }

                    // 如果该页面未来不再被使用，则选择它置换
                    if (next_use == -1) {
                        replace_index = j;
                        break;
                    }

                    // 记录最晚被使用的页面
                    if (next_use > furthest_use) {
                        furthest_use = next_use;
                        replace_index = j;
                    }
                }

                frames[replace_index] = page;
            }
        }
    }

    return page_fault;
}

/* 打印页地址流（用于调试） */
void print_page_stream(int *pages) {
    printf("页地址流:\n");
    for (int i = 0; i < INSTRUCTION_COUNT; i++) {
        if (i % 20 == 0) {
            printf("\n%3d: ", i);
        }
        printf("%2d ", pages[i]);
    }
    printf("\n\n");
}

/* 打印算法结果表格 */
void print_results(int *fifo_results, int *lru_results, int *opt_results) {
    printf("========================================================\n");
    printf("                    页面置换算法命中率比较\n");
    printf("========================================================\n");
    printf("框架数  |  FIFO缺页  |  FIFO命中率  |  LRU缺页  |  LRU命中率  |  OPT缺页  |  OPT命中率\n");
    printf("------------------------------------------------------------------------\n");

    for (int frames = MIN_FRAMES; frames <= MAX_FRAMES; frames++) {
        double fifo_hit = 1.0 - (double)fifo_results[frames - MIN_FRAMES] / INSTRUCTION_COUNT;
        double lru_hit = 1.0 - (double)lru_results[frames - MIN_FRAMES] / INSTRUCTION_COUNT;
        double opt_hit = 1.0 - (double)opt_results[frames - MIN_FRAMES] / INSTRUCTION_COUNT;

        printf("  %2d    |    %3d     |   %6.2f%%    |    %3d    |   %6.2f%%   |    %3d    |   %6.2f%%\n",
               frames,
               fifo_results[frames - MIN_FRAMES], fifo_hit * 100,
               lru_results[frames - MIN_FRAMES], lru_hit * 100,
               opt_results[frames - MIN_FRAMES], opt_hit * 100);
    }

    printf("======================================================\n");
}

int main() {
    // 初始化随机数种子
    srand((unsigned int)time(NULL));

    // 生成指令序列
    int instructions[INSTRUCTION_COUNT];
    generate_instructions(instructions);

    // 转换为页地址流
    int pages[INSTRUCTION_COUNT];
    convert_to_pages(instructions, pages);

    // 打印页地址流（可选）
    // print_page_stream(pages);

    // 存储各算法的缺页次数
    int fifo_results[MAX_FRAMES - MIN_FRAMES + 1];
    int lru_results[MAX_FRAMES - MIN_FRAMES + 1];
    int opt_results[MAX_FRAMES - MIN_FRAMES + 1];

    // 对每个物理内存容量运行各算法
    for (int frames = MIN_FRAMES; frames <= MAX_FRAMES; frames++) {
        fifo_results[frames - MIN_FRAMES] = fifo_algorithm(pages, frames);
        lru_results[frames - MIN_FRAMES] = lru_algorithm(pages, frames);
        opt_results[frames - MIN_FRAMES] = opt_algorithm(pages, frames);
    }

    // 打印结果
    print_results(fifo_results, lru_results, opt_results);

    // 性能分析
    printf("\n=====================================================\n");
    printf("                         算法性能分析\n");
    printf("========================================================\n");
    printf("1. OPT算法（最优算法）：\n");
    printf("   - 具有最高的命中率，是理论最优值\n");
    printf("   - 需要知道未来的页面访问顺序，实际中难以实现\n");
    printf("   - 作为评估其他算法性能的基准\n\n");

    printf("2. FIFO算法（先进先出）：\n");
    printf("   - 实现简单，开销最小\n");
    printf("   - 可能会出现Belady现象（增加帧数反而增加缺页）\n");
    printf("   - 适用于页面访问具有明显时间局部性的场景\n\n");

    printf("3. LRU算法（最近最少使用）：\n");
    printf("   - 近似OPT算法，性能较好\n");
    printf("   - 需要维护使用记录，开销较大\n");
    printf("   - 在实际系统中广泛应用\n\n");

    printf("结论：在本实验的指令序列中，LRU算法表现最接近OPT算法，\n");
    printf("      在帧数较少时性能明显优于FIFO算法。\n");
    printf("========================================================\n");
    return 0;
}

