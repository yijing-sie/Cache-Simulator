# Cache-Simulator

Intro to Computer Systems assignment :

1. [csim.c](csim.c) is a small C program that simulates the behavior of a hardware cache memory


*  Given a series of memory operations, [csim.c](csim.c) simulates the behavior of a cache with arbitrary size and associativity. 



*  It uses the LRU (least-recently used) replacement policy and a write-back, write-allocate policy.

* All the smaple trace files can be found in [traces/csim](traces/csim) for testing the [csim.c](csim.c)


* Usage:


```
Usage: ./csim -s <s> -E <E> -b <b> -t <tracefile>

-s <s>: Number of set index bits (S = 2s is the number of sets) 
-E <E>: Associativity (number of lines per set) 
-b <b>: Number of block bits (B = 2b is the block size) 
-t <tracefile>: Name of the memory trace to replay


For example:
./csim -s 0 -E 1 -b 0 -t traces/csim/wide.trace
```
(The command-line arguments are based on the notation (s, E, and b) from page 617 of the CS:APP3e textbook.)
* The output includes the total number of hits, misses, evictions, the number of dirty bytes that have been evicted and the number of dirty bytes in the cache at the end of the simulation.

For example:

```
hits:4 misses:5 evictions:3 dirty_bytes_in_cache:32 dirty_bytes_evicted:16
```
---
2. [trans.c](trans.c) optimizes a `transpose_submit`, a matrix transpose function, with the goal of minimizing the number of cache misses and minimizes the number of clock cycles under several restrictions

* It computes the transpose of `N`×`M` matrix `A` and store the results in `M`×`N` matrix `B`, where `tmp` is a pointer to an array of 256 elements that can be used to hold data as an intermediate step between reading from `A` and writing to `B`.



* When M = N = 32 or M = N = 1024, trans.c optimizes a matrix transpose function for the cache; otherwise, it works like a regular matrix transpose function



An example of one such function is:


```
 void transpose_submit(size_t M, size_t N, double A[N][M], double B[M][N], double *tmp);
```


