# CSC3060 Homework 4 Report

## 1. Student / Team Information
- Name: YIQI LI (Seed Student), JUNHONG JIANG
- ID: 124090324 (Seed ID), 124090252

## 2. Implementation Summary
This project implements a configurable multi-level cache simulator with support for different replacement policies and prefetching techniques. The simulator is divided into three main tasks.

- **Task 1:** Implemented a single-level L1 cache connected directly to main memory. The system supports configurable cache size, associativity, and block size. Core functionalities include address decomposition, hit/miss detection, replacement policy integration, and write-back handling for dirty blocks.
- **Task 2:** Extended the simulator to a two-level hierarchy (L1 → L2 → Main Memory). The L2 cache is configured to be four times larger than L1 and has four times the latency. Misses in L1 are forwarded to L2, and misses in L2 are forwarded to main memory. Latencies are accumulated across levels.
- **Task 3:** Implemented and optimized multiple replacement policies (LRU, SRRIP, BIP) and prefetchers (None, NextLine, Stride). The goal was to minimize the Average Memory Access Time (AMAT) below 1.9 cycles through systematic tuning.

## 3. Address Mapping Explanation
Each memory address is divided into three components:

- **Block Offset:** Determines the position within a cache block.  
  `offset_bits = log₂(block size)`
- **Set Index:** Determines which cache set the block maps to.  
  `index = (addr >> offset_bits) & ((1 << index_bits) - 1)`
- **Tag:** Identifies the specific block within a set.  
  `tag = addr >> (offset_bits + index_bits)`

### Effect of Cache Geometry Changes
- Increasing block size → increases offset bits
- Increasing cache size → increases number of sets → increases index bits
- Increasing associativity → reduces number of sets → reduces index bits

This flexible design ensures the simulator works correctly for various configurations.

## 4. Task 1 Testing
To verify correctness, multiple cache configurations were tested using trace_sanity.txt.

### Example Configuration:
- 32KB cache
- 8-way associativity
- 64B block size
- LRU replacement
- No prefetching

### Result:
- AMAT ≈ 79.57 cycles

This high AMAT is expected due to the absence of prefetching and the reliance on main memory.

### Validation Methods
- Manual verification of hit/miss behavior
- Checking correct handling of dirty write-backs
- Testing different cache sizes and associativity

## 5. Task 2 Hierarchy Explanation
### Cache Interaction
The two-level hierarchy operates as follows:
- L1 hit → return data immediately
- L1 miss → access L2
- L2 hit → return data
- L2 miss → access main memory

### Impact of Adding L2
- Reduced main memory accesses
- Improved overall hit rate
- Lower AMAT compared to single-level cache

## 6. Task 3 Design Choices
### Replacement Policies
- **LRU (Least Recently Used):** Replaces the least recently accessed block.
- **SRRIP (Static Re-Reference Interval Prediction):** Uses RRPV values to predict reuse distance.
- **BIP (Bimodal Insertion Policy):** Inserts most blocks with low priority to reduce pollution.

### Prefetchers
- **None:** No prefetching
- **NextLine:** Prefetches subsequent sequential blocks
- **Stride:** Detects stride patterns and prefetches accordingly

### Design Improvements
- Fixed incorrect hardcoded block size in NextLine prefetcher
- Modified prefetch triggering to occur only on cache misses
- Tuned prefetch depth to balance accuracy and cache pollution

## 7. Trace Analysis
### Observed Access Pattern
The personalized trace (my_trace.txt) exhibits:
- Sequential memory access pattern
- Fixed stride of one cache block (64 bytes)

### Impact on Design
- Stride prefetching is theoretically suitable but sensitive to noise
- NextLine prefetching performs better due to simplicity and stability
- Deep prefetching significantly improves performance for sequential access

### Trace Seed
The trace was generated using student ID: 124090324

## 8. Experimental Results
### Cache Size Tuning
| L1 Size | AMAT |
|---------|------|
| 32KB    | ~14.15 |
| 64KB    | ~12.65 |
| 128KB   | ~12.11 |

### Prefetch Depth Tuning (NextLine)
| Prefetch Depth | AMAT |
|----------------|------|
| 1              | ~12.11 |
| 10             | ~5.x |
| 30             | ~2.x |
| 70             | ~1.75 |

### Results
- Total Instructions: 6532
- L1 Hit Rate: 99.36%
- L2 Hit Rate: 57.58%
- Memory Accesses: 42
- Prefetches Issued: 2940
- AMAT: 1.67 cycles

## 9. Best Configuration and Discussion
### Best Configuration
- L1: 128KB, 8-way, 64B block, 1 cycle, LRU + NextLine (70 blocks)
- L2: 512KB, 8-way, 64B block, 4 cycles, LRU + NextLine (70 blocks)
- Main Memory: 100 cycles

### Why It Performs Well
- Sequential access pattern perfectly matches NextLine prefetching
- Large prefetch depth ensures data is available before demand
- Increased cache size reduces eviction of useful data
- LRU performs well due to strong temporal locality

### Performance Analysis
AMAT = 1 + (miss rate × miss penalty)

Estimated:
- miss penalty ≈ 4 + (1 - 0.58) × 100 ≈ 46.8
- miss rate ≈ 0.0064

AMAT ≈ 1 + 0.0064 × 46.8 ≈ 1.30

Measured AMAT is 1.67 cycles, slightly higher due to prefetch overhead and imperfect prediction.

### Limitations
- Performance heavily depends on access pattern
- Deep prefetching may cause cache pollution in irregular workloads
- Not suitable for highly random memory access patterns

### Conclusion
Through systematic optimization, the AMAT was reduced from approximately 80 cycles to 1.67 cycles, successfully meeting the project requirement.

Key contributions include:
- Correct implementation of cache hierarchy
- Fixing prefetcher design issues
- Optimizing prefetch timing and depth
- Matching prefetch strategy to access pattern

This project demonstrates that accurate prediction and minimizing cache pollution are more important than aggressive prefetching.

## External Resources and AI usage
- https://chatgpt.com/share/69dc8209-f5a8-832d-8b6b-e0162659c639
- https://chatgpt.com/share/69dc8221-9120-832b-828b-6f6a0248c023