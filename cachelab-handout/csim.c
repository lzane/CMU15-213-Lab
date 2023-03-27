#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include "cachelab.h"

unsigned long ts = 0;
typedef struct
{
    char valid;
    unsigned int tag;
    unsigned long timestamp;
} line;
typedef struct
{
    unsigned int offset, setIndex, tag;
} addressPart;

addressPart splitAddress(unsigned int address, int s, int b)
{
    unsigned int offset = address & ((1 << b) - 1);
    unsigned int setIndex = (address >> b) & ((1 << s) - 1);
    unsigned int tag = address >> (b + s);
    addressPart part = {offset, setIndex, tag};
    return part;
}

typedef struct
{
    int hit, miss, evict;
} res;

// search whether address hit
// return *line when hit, NULL when miss
// update timestamp when hit
line *search(int E, line (*cache)[E], addressPart part)
{
    line *set = cache[part.setIndex];
    for (int i = 0; i < E; i++)
    {
        // hit
        if (set[i].tag == part.tag && set[i].valid)
        {
            // update timestamp
            set[i].timestamp = ts;
            return &set[i];
        }
    }

    return NULL;
}

// load to cache, return 1 if eviction happen
// using LRU eviction policy
int load(int E, line (*cache)[E], addressPart part)
{
    line *set = cache[part.setIndex];
    int leastUsedLineIndex = 0;

    line newLine;
    newLine.tag = part.tag;
    newLine.timestamp = ts;
    newLine.valid = 1;

    // has free space
    for (int i = 0; i < E; i++)
    {
        if (!set[i].valid)
        {
            set[i] = newLine;
            return 0;
        }

        if (set[i].timestamp < set[leastUsedLineIndex].timestamp)
        {
            leastUsedLineIndex = i;
        }
    }

    // eviction
    set[leastUsedLineIndex] = newLine;
    return 1;
}

// L or S
res cacheLS(int E, line (*cache)[E], addressPart part)
{
    res r = {0, 0, 0};

    line *l = search(E, cache, part);
    if (l == NULL)
    {
        // miss
        r.miss++;
        // load new line
        int evict = load(E, cache, part);
        if (evict)
        {
            r.evict++;
        }

        return r;
    }

    // hit
    r.hit++;
    return r;
}

// M
res cacheM(int E, line (*cache)[E], addressPart part)
{
    res r1 = cacheLS(E, cache, part);
    res r2 = cacheLS(E, cache, part);

    res r;
    r.evict = r1.evict + r2.evict;
    r.hit = r1.hit + r2.hit;
    r.miss = r1.miss + r2.miss;
    return r;
}

int main(int argc, char **argv)
{
    int opt, s, E, b;
    char v = 0; // verbose flag
    char *t;

    // parse the arguments
    while ((opt = getopt(argc, argv, "s:E:b:t:v")) != -1)
    {
        switch (opt)
        {
        case 's':
            s = atoi(optarg);
            break;
        case 'E':
            E = atoi(optarg);
            break;
        case 'b':
            b = atoi(optarg);
            break;
        case 't':
            t = optarg;
            break;
        case 'v':
            v = 1;
            break;
        default:
            printf("wrong argument\n");
            break;
        }
    }
    // if (v)
    // {
    //     printf("args s=%d, E=%d, b=%d, t=%s\n", s, E, b, t);
    // }

    int S = 1 << s;
    line(*cache)[E] = malloc(sizeof(line) * S * E);
    memset(cache, 0, sizeof(line)*S*E);
    // line cache[S][E];
    // memset(cache, 0, sizeof(line)*S*E);

    // reading trace file
    FILE *traceFile;
    traceFile = fopen(t, "r");
    char identifier;
    unsigned int address;
    int size;

    res r = {0, 0, 0};

    while (fscanf(traceFile, " %c %x,%d", &identifier, &address, &size) == 3)
    {
        if (identifier == 'I')
        {
            continue;
        }

        addressPart part = splitAddress(address, s, b);
        if (v)
        {
            printf("reading line: %c %x,%d split to: tag=%x setIndex=%x offset=%x\n", identifier, address, size, part.tag, part.setIndex, part.offset);
        }

        ts++;
        res r1;
        if (identifier == 'L' || identifier == 'S')
        {
            r1 = cacheLS(E, cache, part);
        }
        else if (identifier == 'M')
        {
            r1 = cacheM(E, cache, part);
        }

        if (v)
        {
            printf("%c %x,%d", identifier, address, size);
            if (r1.miss)
            {
                printf(" miss");
            }
            if (r1.evict)
            {
                printf(" eviction");
            }
            if (r1.hit)
            {
                printf(" hit");
            }
            printf("\n");
        }

        r.evict += r1.evict;
        r.hit += r1.hit;
        r.miss += r1.miss;
    }

    free(cache);
    fclose(traceFile);
    printSummary(r.hit, r.miss, r.evict);
    return 0;
}
