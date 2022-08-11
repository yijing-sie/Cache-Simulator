/**
 * @author Yi-Jing Sie <ysie@andrew.cmu.edu>
 *
 */
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cachelab.h"

/* cache_line contains all necessary info about a cacheline */
typedef struct {
    unsigned long tag;
    bool dirty_bit;
    unsigned long hits; // number of encounters
    int valid;
} cache_line;

static int set_bits;
static unsigned long line;
static int block_bits;
static char *tracefile;
static unsigned long dirty_bytes =
    0; /* number of dirty bytes in cache at end of simulation */
static int verbal = 0;
static unsigned long time = 0;
/**
 * perform load for cache
 */
void load(unsigned long cache_size, unsigned long set, unsigned long tag,
          unsigned long line, cache_line *cache, csim_stats_t *csim_stats,
          unsigned long block_bytes) {
    // printf("set = %lx, line = %lu,tag = %lx\n", set, line, tag);
    unsigned long minimial_time = (unsigned long)(0 - 1);
    unsigned long least_hit_idx =
        0; // line index for least recently used cache line
    unsigned long miss = 0;
    unsigned long hit = 0;
    bool finished_load = false; // indicate whether load is finished
    for (unsigned long i = 0; i < line; i++) {
        if ((cache[set * line + i].valid == 1) &&
            (cache[set * line + i].tag == tag)) {
            if (verbal) {
                printf("load hit\n");
            }
            cache[set * line + i].hits = time;
            hit = 1;
            break;
        }
    }
    /* miss cases */
    if (!hit) {
        miss = 1;
        // hit_least = cache[set * line + 0].hits;
        for (unsigned long i = 0; i < line; i++) {
            if (cache[set * line + i].valid == 0) {
                cache[set * line + i].valid = 1;
                cache[set * line + i].tag = tag;
                cache[set * line + i].hits = time;
                finished_load = true;
                // cache[set * line + i].dirty_bit = false;
                if (verbal) {
                    printf("load miss\n");
                }
                break;
            }
            if (cache[set * line + i].hits < minimial_time) {
                minimial_time = cache[set * line + i].hits;
                least_hit_idx = i;
            }
        }
        /* eviction */
        if (!finished_load) {
            csim_stats->evictions += 1;
            // printf("least_hit_idx = %lu\n", least_hit_idx);
            cache[set * line + least_hit_idx].valid = 1;
            cache[set * line + least_hit_idx].tag = tag;
            cache[set * line + least_hit_idx].hits = time;
            if (cache[set * line + least_hit_idx].dirty_bit) {
                cache[set * line + least_hit_idx].dirty_bit = false;
                csim_stats->dirty_evictions += block_bytes;
                dirty_bytes -= block_bytes;
                if (verbal) {
                    printf("dirty ");
                }
            }
            if (verbal) {
                printf("load eviction miss\n");
            }
        }
    }
    csim_stats->hits += hit;
    csim_stats->misses += miss;
}
/**
 * perform store for cache
 */
void store(unsigned long cache_size, unsigned long set, unsigned long tag,
           unsigned long line, cache_line *cache, csim_stats_t *csim_stats,
           unsigned long block_bytes) {
    // printf("set = %lx, line = %lu,tag = %lx\n", set, line, tag);
    unsigned long minimial_time = (unsigned long)(0 - 1);
    unsigned long least_hit_idx =
        0; // line index for least recently used cache line
    unsigned long miss = 0;
    unsigned long hit = 0;
    bool finished_load = false; // indicate whether load is finished
    for (unsigned long i = 0; i < line; i++) {
        if ((cache[set * line + i].valid == 1) &&
            (cache[set * line + i].tag == tag)) {
            if (verbal) {
                printf("write hit\n");
            }
            cache[set * line + i].valid = 1;
            cache[set * line + i].hits = time;
            hit = 1;
            if (!cache[set * line + i].dirty_bit) {
                dirty_bytes += block_bytes;
                cache[set * line + i].dirty_bit = true;
            }
            break;
        }
    }
    /* miss cases */
    if (!hit) {
        miss = 1;
        for (unsigned long i = 0; i < line; i++) {
            if (cache[set * line + i].valid == 0) {
                cache[set * line + i].hits = time;
                cache[set * line + i].valid = 1;
                cache[set * line + i].tag = tag;
                finished_load = true;
                dirty_bytes += block_bytes;
                cache[set * line + i].dirty_bit = true;
                if (verbal) {
                    printf("write miss\n");
                }
                break;
            }
            if (cache[set * line + i].hits < minimial_time) {
                minimial_time = cache[set * line + i].hits;
                least_hit_idx = i;
            }
        }
        /* eviction */
        if (!finished_load) {
            csim_stats->evictions += 1;
            cache[set * line + least_hit_idx].valid = 1;
            cache[set * line + least_hit_idx].tag = tag;
            cache[set * line + least_hit_idx].hits = time;
            if (cache[set * line + least_hit_idx].dirty_bit) {
                csim_stats->dirty_evictions += block_bytes;
                if (verbal) {
                    printf("dirty ");
                }
            } else {
                dirty_bytes += block_bytes;
                cache[set * line + least_hit_idx].dirty_bit = true;
            }
            if (verbal) {
                printf("write eviction miss\n");
            }
        }
    }
    csim_stats->hits += hit;
    csim_stats->misses += miss;
}

int main(int argc, char *argv[]) {
    char action;
    unsigned long address;
    unsigned long bytes;
    /**
     * Parsing inputs to get the number of set index bits (set_bits),
     * Associativity (E),
     * Number of block bits (block_bits),
     * and the name of tracefile
     */
    int option;
    while ((option = getopt(argc, argv, ":s:E:b:t:v")) != -1) {
        switch (option) {
        case 's':
            set_bits = atoi(optarg);
            // printf("s = %d\n", set_bits);
            break;
        case 'E':
            line = (unsigned long)atoi(optarg);
            // printf("E = %llu\n", line);
            break;
        case 'b':
            block_bits = atoi(optarg);
            // printf("b = %d\n", block_bits);
            break;
        case 't':
            tracefile = (char *)malloc(strlen(optarg) + 1);
            if (!tracefile) {
                free(tracefile);
                printf("Fail to allocate memory for tracefile.");
                return false;
            }
            strcpy(tracefile, optarg);
            // printf("t = %s\n", tracefile);
            break;
        case 'v':
            verbal = 1;
            break;
        case '?':
            printf("Unknown option: %c\n", optopt);
            break;
        case ':':
            printf("Missing arguments for %c\n", optopt);
            break;
        }
    }
    unsigned long tag_mask = (unsigned long)(-1 << (set_bits + block_bits));
    unsigned long set_mask = (unsigned long)(-1 << (block_bits)) ^ tag_mask;
    unsigned long set_size = (1 << set_bits);      // number of sets
    unsigned long block_bytes = (1 << block_bits); // number of bytes per block
    /* Initialiize a cache whose structure is based on the input */
    unsigned long cache_size = (set_size * line);

    cache_line *cache = calloc(cache_size, sizeof(cache_line));
    if (!cache) {
        printf("Fail to allocate memory for cache");
        free(cache);
        exit(1);
    }
    /* Open tracefile */
    FILE *file = fopen(tracefile, "r");
    if (!file) {
        printf("Cannot Open the tracefile.");
        exit(1);
    }
    csim_stats_t *csim_stats = calloc(1, sizeof(csim_stats_t));
    if (!csim_stats) {
        printf("Fail to allocate memory for csim_stats");
        free(csim_stats);
        exit(1);
    }
    while (EOF != fscanf(file, "%c\t%lx,%lu\n", &action, &address, &bytes)) {
        // printf("The action is %c, with address = %lx\n", action, address);
        unsigned long set = (address & set_mask) >> block_bits;
        unsigned long tag = (tag_mask & address);
        switch (action) {
        case 'L':
            load(cache_size, set, tag, line, cache, csim_stats, block_bytes);
            break;
        case 'S':
            store(cache_size, set, tag, line, cache, csim_stats, block_bytes);
            break;
        }
        time++;
    }
    csim_stats->dirty_bytes = dirty_bytes;
    free(tracefile);
    free(cache);
    const csim_stats_t *stats;
    stats = csim_stats;
    printSummary(stats);
    free(csim_stats);
    fclose(file);
    return 0;
}
