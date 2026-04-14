#include "prefetcher.h"

std::vector<uint64_t> NextLinePrefetcher::calculatePrefetch(uint64_t current_addr, bool miss) {
    std::vector<uint64_t> prefetches;

    if (!miss) return prefetches;  // only prefetch on miss

    // prefetch 3 next lines
    uint64_t base_block_addr = current_addr / block_size;

    for (int i = 1; i <= 70; ++i) {
        uint64_t next_block = base_block_addr + i;
        uint64_t prefetch_addr = next_block * block_size;
        prefetches.push_back(prefetch_addr);
    }

    return prefetches;
}

std::vector<uint64_t> StridePrefetcher::calculatePrefetch(uint64_t current_addr, bool miss) {
    std::vector<uint64_t> prefetches;

    if (!miss) return prefetches;  // only prefetch on miss

    const uint64_t block = block_size;

    uint64_t curr_block = current_addr / block;

    // first access
    if (!has_last_block) {
        last_block = curr_block;
        has_last_block = true;
        return prefetches;
    }

    // compute stride (in blocks)
    int64_t stride = (int64_t)curr_block - (int64_t)last_block;

    // update confidence
    if (stride == last_stride) {
        confidence++;
    } else {
        confidence = 1;
        last_stride = stride;
    }

    last_block = curr_block;

    // threshold (typical 2~3)
    const uint32_t threshold = 2;

    if (confidence >= threshold && stride != 0) {
        uint64_t next_block = curr_block + stride;
        uint64_t prefetch_addr = next_block * block;

        prefetches.push_back(prefetch_addr);
    }

    return prefetches;
}

Prefetcher* createPrefetcher(std::string name, uint32_t block_size) {
    if (name == "NextLine") return new NextLinePrefetcher(block_size);
    if (name == "Stride") return new StridePrefetcher(block_size);
    return new NoPrefetcher();
}
