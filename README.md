# Cache Simulator

A simple multi-level cache simulator program.

## What is this?

Simulates a computer's cache system, including:
- L1 and L2 two-level caches
- Three replacement policies: LRU, SRRIP, BIP
- Two prefetching methods: NextLine, Stride
- Calculate hit rate and average access time (AMAT)

## Files

- `main.cpp` - Main program
- `memory_hierarchy.cpp/h` - Cache core logic
- `repl_policy.cpp/h` - Replacement policies
- `prefetcher.cpp/h` - Prefetchers
- `defs.h` - Data structures
- `interfaces.h` - Interfaces
- `Makefile` - Build configuration

## Build

```bash
make          # Compile
make clean    # Clean
```

## Run

### Task 1 (L1 cache only)

```bash
make task1
```
output: 
make task1
./cache_sim trace_sanity.txt 32 8 64 1 100
Constructed L1: 32KB, 8-way, 1cyc, [LRU + None]

=== Starting Simulation ===

=== Simulation Results ===
  [L1] Hit Rate: 21.43% (Access: 56, Miss: 44, WB: 2)
      Prefetches Issued: 0
  [Main Memory] Total Accesses: 46

Metrics:
  Total Instructions: 56
  Total Cycles:       4456
  AMAT:               79.57 cycles


### Task 2 (L1 + L2 cache)

```bash
make task2
```
output: 
make task2
./cache_sim trace_sanity.txt 32 8 64 1 100 --enable-l2
Constructed L2: 128KB, 8-way, 4cyc, [LRU + None]
Constructed L1: 32KB, 8-way, 1cyc, [LRU + None]

=== Starting Simulation ===

=== Simulation Results ===
  [L1] Hit Rate: 21.43% (Access: 56, Miss: 44, WB: 2)
      Prefetches Issued: 0
  [L2] Hit Rate: 50.00% (Access: 46, Miss: 23, WB: 0)
      Prefetches Issued: 0
  [Main Memory] Total Accesses: 23

Metrics:
  Total Instructions: 56
  Total Cycles:       2532
  AMAT:               45.21 cycles


### Task 3 (Optimized version)

```bash
make task3
```

output: 
make task3
./cache_sim my_trace.txt 128 8 64 1 100 LRU NextLine --enable-l2 LRU NextLine
Constructed L2: 512KB, 8-way, 4cyc, [LRU + NextLine]
Constructed L1: 128KB, 8-way, 1cyc, [LRU + NextLine]

=== Starting Simulation ===

=== Simulation Results ===
  [L1] Hit Rate: 99.36% (Access: 6532, Miss: 42, WB: 57)
      Prefetches Issued: 2940
  [L2] Hit Rate: 57.58% (Access: 99, Miss: 42, WB: 0)
      Prefetches Issued: 2940
  [Main Memory] Total Accesses: 42

Metrics:
  Total Instructions: 6532
  Total Cycles:       10900
  AMAT:               1.67 cycles


## Command Line

General format:
```bash
./cache_sim <trace_file> <L1_KB> <assoc> <block> <L1_lat> <memory_lat> \
    [L1_policy] [L1_prefetch] [--enable-l2 [L2_policy] [L2_prefetch]]
```

Examples:
```bash
./cache_sim my_trace.txt 32 8 64 1 100 LRU None
./cache_sim my_trace.txt 128 8 64 1 100 LRU NextLine --enable-l2 SRRIP None
```

Policies: LRU, SRRIP, BIP
Prefetchers: None, NextLine, Stride

## Trace File Format

Each line represents one memory access:
```
<type> <address>
```

Example:
```
r 0x10000140
w 0x10000180
r 0x1000c0c0
```

## Generate Personalized Trace

```bash
make trace_gen
./trace_generator/workload_gen student <student_id> > my_trace.txt
```

## Three Replacement Policies

- LRU: Replace the least recently used cache line
- SRRIP: Prediction based on reuse distance
- BIP: Bimodal insertion policy to reduce cache pollution

## Two Prefetching Methods

- NextLine: Prefetch next sequential blocks
- Stride: Detect access patterns and predict prefetches

## Useful Commands

```bash
# Analyze your trace file
python3 trace_analyzer.py my_trace.txt

# Clean and rebuild
make clean && make

# Test basic functionality
make task1
```

