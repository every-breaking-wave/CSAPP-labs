// id : 520030910021 name: Feng Yifei

#define _POSIX_C_SOURCE 2

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "cachelab.h"
#define MaxLineNum 100

enum locateStat
{
    HIT = 0,
    COLD,
    CONFLICT
};


typedef struct hitQueue
{
    int size;
    int curSize;
    int lineNum;
    int data[MaxLineNum];
    int front;
    int rear;
} hitQueue;

//创建队列
hitQueue initQueue(int line)
{
    hitQueue *sq = (hitQueue *)malloc(sizeof(hitQueue));
    sq->lineNum = line;
    sq->size = line * 2;
    sq->curSize = 0;
    memset(sq->data, 0, sizeof(int) * sq->size);
    sq->rear = sq->front = 0;
    return *sq;
}

//判断循环队列是否为空
int isEmpty(hitQueue *qu)
{
    return qu->curSize == 0;
}

//元素出循环队列,不关心出的是谁
int deQueue(hitQueue *qu)
{
    if (isEmpty(qu))
    {
        return 0;
    }
    qu->front = (qu->front + 1) % qu->size;
    qu->curSize--;
    return 1;
}

//元素进循环队列
int enQueue(hitQueue *qu, int newHitLine)
{
    if (qu->curSize == qu->size)
    {
        deQueue(qu);
    }
    qu->data[qu->rear] = newHitLine;
    qu->rear = (qu->rear + 1) % qu->size;

    qu->curSize++;
    return 1;
}

int get_LRU_line(hitQueue *qu)
{
    int table[qu->lineNum];
    memset(table, 0, sizeof(table));

    for (int i = 0; i < qu->curSize; i++)
    {
        // 为每个line赋予访问的标记
        table[qu->data[(qu->front + i) % qu->size]] = i + 1;
    }
    int min = table[0];
    int minId = 0;
    // 获取最小标记的line
    for (int i = 1; i < qu->lineNum; i++)
    {
        if (table[i] < min)
        {
            min = table[i];
            minId = i;
        }
    }
    return minId;
}

typedef struct Line_t
{
    char validBit;
    // if is enough?
    long tag;
    int data;
} Line;

typedef Line *Set;

typedef struct Cache_t
{
    int S;
    int E;
    int B;
    Set *set;
    hitQueue *queue;
    //    que<int> hitQueue;
} Cache;

void initLine(Line *line)
{
    line->validBit = 0;
    line->tag = 0;
    line->data = 0;
}

Cache *initCache(int setNum, int lineNum, int blockNum)
{
    Cache *cache = (Cache *)malloc(sizeof(Cache));
    cache->S = setNum;
    cache->E = lineNum;
    cache->B = blockNum;
    cache->set = (Set *)malloc(sizeof(Set) * setNum);

    cache->queue = (hitQueue *)malloc(sizeof(hitQueue) * setNum);
    memset(cache->queue, 0, sizeof(hitQueue) * lineNum);

    for (int i = 0; i < setNum; i++)
    {
        cache->queue[i] = initQueue(cache->E);
        cache->set[i] = (Line *)malloc(sizeof(Line) * lineNum);
        for (int j = 0; j < lineNum; j++)
        {
            initLine(&cache->set[i][j]);
        }
    }
    return cache;
}


enum locateStat locate(long addr, Cache *cache)
{
    int setId = (addr / cache->B) % cache->S;
    long tag = addr / cache->B / cache->S;

    Line *line = cache->set[setId];
    // int EmptyId = 0;
    int *Empty = (int *)malloc(cache->E);
    memset(Empty, 0, cache->E);
    // check each line in set[setId]
    for (int i = 0; i < cache->E; i++)
    {
        if (!line[i].validBit)
        {
            Empty[i] = 1;
            continue;
        }
        if (line[i].tag == tag)
        {
            enQueue(&cache->queue[setId], i);
            return HIT;
        }
    }

    // two cases:
    //  cold miss
    for (int i = 0; i < cache->E; i++)
    {
        if (Empty[i])
        {
            line[i].validBit = 1;
            enQueue(&cache->queue[setId], i);
            cache->set[setId][i].tag = tag;
            return COLD;
        }
    }

    int LRU_id = get_LRU_line(&cache->queue[setId]);
    cache->set[setId][LRU_id].tag = tag;
    enQueue(&cache->queue[setId], LRU_id);
    return CONFLICT;
}

int main(int argc, char *argv[])
{
    int ch = 0;
    int S, E, B;
    int miss_cnt = 0, hit_cnt = 0, eviction_cnt = 0;
    FILE *file;
    while ((ch = getopt(argc, argv, "vs:E:b:t:")) != -1)
        switch (ch)
        {
            case 'v':
                break;
            case 's':
                S = 1 << atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                B = 1 << atoi(optarg);
                break;
            case 't':
                file = fopen(optarg, "r");
                break;

            default:
                printf("wrong opt");
                break;
        }

    if (S <= 0 || E <= 0 || B <= 0 || file == NULL)
    {
        printf("err ins\n");
    }

    Cache *cache = initCache(S, E, B);

    char id;
    long addr;
    int size;
    while (fscanf(file, " %c %lx,%d", &id, &addr, &size) > 0)
    {
        int mem_cnt;
        if (id == 'L' || id == 'S')
        {
            mem_cnt = 1;
        }
        else if (id == 'M')
        {
            mem_cnt = 2;
        }
        else
        {
            continue;
        }
        for (int i = 0; i < mem_cnt; i++)
        {
            int m = locate(addr, cache);
            switch (m)
            {
                case HIT:
                    hit_cnt += 1;
                    break;
                case COLD:
                    miss_cnt += 1;
                    break;
                case CONFLICT:
                    miss_cnt += 1;
                    eviction_cnt += 1;
                    break;
            }
        }
    }
    printSummary(hit_cnt, miss_cnt, eviction_cnt);
    return 0;
}
