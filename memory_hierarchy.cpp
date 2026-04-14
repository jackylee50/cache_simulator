#include "memory_hierarchy.h"
#include "prefetcher.h"
#include "repl_policy.h"
#include <cmath>
#include <iomanip>
#include <iostream>

using namespace std;

MainMemory::MainMemory(int lat) : latency(lat) {}

int MainMemory::access(uint64_t addr, char type, uint64_t cycle) {
    (void)addr;
    (void)type;
    (void)cycle;
    access_count++;
    return latency;
}

void MainMemory::printStats() {
    cout << "  [Main Memory] Total Accesses: " << access_count << endl;
}

CacheLevel::CacheLevel(string name, CacheConfig cfg, MemoryObject* next)
    : level_name(name), config(cfg), next_level(next) {
    policy = createReplacementPolicy(config.policy_name);
    prefetcher = createPrefetcher(config.prefetcher, config.block_size);

    uint64_t total_bytes = (uint64_t)config.size_kb * 1024;
    num_sets = total_bytes / (config.block_size * config.associativity);

    offset_bits = log2(config.block_size);
    index_bits = log2(num_sets);

    sets.resize(num_sets, vector<CacheLine>(config.associativity));

    cout << "Constructed " << level_name << ": "
         << config.size_kb << "KB, " << config.associativity << "-way, "
         << config.latency << "cyc, "
         << "[" << config.policy_name << " + " << prefetcher->getName() << "]" << endl;
}

CacheLevel::~CacheLevel() {
    delete policy;
    delete prefetcher;
}

uint64_t CacheLevel::get_index(uint64_t addr) {
    uint64_t offset_bits = log2(config.block_size);

    uint64_t cache_size_bytes = config.size_kb * 1024;
    uint64_t num_sets = cache_size_bytes / (config.block_size * config.associativity);
    uint64_t index_bits = log2(num_sets);

    return (addr >> offset_bits) & ((1ULL << index_bits) - 1);
}

uint64_t CacheLevel::get_tag(uint64_t addr) {
    uint64_t offset_bits = (uint64_t)log2(config.block_size);

    uint64_t cache_size_bytes = config.size_kb * 1024;
    uint64_t num_sets = cache_size_bytes / (config.block_size * config.associativity);
    uint64_t index_bits = (uint64_t)log2(num_sets);

    return addr >> (offset_bits + index_bits);
}

uint64_t CacheLevel::reconstruct_addr(uint64_t tag, uint64_t index) {
    uint64_t offset_bits = __builtin_ctz(config.block_size);

    uint64_t cache_size_bytes = config.size_kb * 1024;
    uint64_t num_sets = cache_size_bytes / (config.block_size * config.associativity);
    uint64_t index_bits = __builtin_ctz(num_sets);

    uint64_t addr = (tag << (index_bits + offset_bits)) 
                  | (index << offset_bits);

    return addr;
}

void CacheLevel::write_back_victim(const CacheLine& line, uint64_t index, uint64_t cycle) {
    // TODO: Task 1 / Task 2
    // Move dirty write-back logic into this helper.
    // Suggested steps:
    // 1. If the victim is not dirty, return immediately.
    // 2. If there is no next level, return immediately.
    // 3. Increment the write-back counter.
    // 4. Reconstruct the evicted block address from tag + index.
    // 5. Send a write access to the next level.
    (void)line;
    (void)index;
    (void)cycle;
    // 1. not dirty → no write back
    if (!line.dirty) {
        return;
    }

    // 2. no next level → stop here
    if (!next_level) {
        return;
    }

    // 3. update stats
    write_backs++;

    // 4. reconstruct address from tag + index
    uint64_t addr = reconstruct_addr(line.tag, index);


    // 5. send write to next level
    next_level->access(addr, 'w', cycle);
}

int CacheLevel::access(uint64_t addr, char type, uint64_t cycle) {
    int lat = config.latency;

    uint64_t index = get_index(addr);
    uint64_t tag = get_tag(addr);

    auto &set = sets[index];

    int hit_way = -1;

    for (int i = 0; i < config.associativity; i++) {
        if (set[i].valid && set[i].tag == tag) {
            hit_way = i;
            break;
        }
    }

    // HIT
    if (hit_way != -1) {
        hits++;

        policy->onHit(set, hit_way, cycle);

        if (type == 'w') {
            set[hit_way].dirty = true;
        }

        if (set[hit_way].is_prefetched) {
            set[hit_way].is_prefetched = false;
        }

        // =======================
        // Prefetcher Hook (Task 3)
        // =======================
        if (prefetcher) {
            bool is_miss = (hit_way == -1);

            std::vector<uint64_t> pf_addrs = prefetcher->calculatePrefetch(addr, is_miss);

            for (auto pf_addr : pf_addrs) {
                install_prefetch(pf_addr, cycle);
                prefetch_issued++;
            }
        }

        return lat;
    }

    // MISS
    misses++;

    int victim = -1;

    for (int i = 0; i < config.associativity; i++) {
        if (!set[i].valid) {
            victim = i;
            break;
        }
    }

    if (victim == -1) {
        victim = policy->getVictim(set);
    }

    // write-back
    if (set[victim].valid && set[victim].dirty) {
        write_back_victim(set[victim], index, cycle);
    }


    if (next_level) {
        lat += next_level->access(addr, type, cycle);
    }

    set[victim].valid = true;
    set[victim].tag = tag;
    set[victim].dirty = (type == 'w');
    set[victim].is_prefetched = false;

    policy->onMiss(set, victim, cycle);

    // =======================
    // Prefetcher Hook (Task 3)
    // =======================
    if (prefetcher) {
        bool is_miss = (hit_way == -1);

        std::vector<uint64_t> pf_addrs = prefetcher->calculatePrefetch(addr, is_miss);

        for (auto pf_addr : pf_addrs) {
            install_prefetch(pf_addr, cycle);
            prefetch_issued++;
        }
    }

    return lat;
}

void CacheLevel::install_prefetch(uint64_t addr, uint64_t cycle) {

    uint64_t index = get_index(addr);
    uint64_t tag   = get_tag(addr);

    auto& set = sets[index];

    // 1. avoid duplicate install
    for (auto& line : set) {
        if (line.valid && line.tag == tag) {
            return;
        }
    }

    // 2. choose victim via replacement policy
    int victim = policy->getVictim(set);

    CacheLine& line = set[victim];

    // 3. write-back if needed (reuse helper!)
    if (line.valid && line.dirty) {
        write_back_victim(line, index, cycle);
    }

    // 4. install prefetched line
    line.valid = true;
    line.tag = tag;

    line.dirty = false;              // prefetch is clean
    line.last_access = cycle;

    line.is_prefetched = true;

    // 5. important: initialize replacement metadata
    line.rrpv = 3;                   // SRRIP default low priority

    // optional: if your policy uses LRU timestamps only
    // line.last_access = cycle;
}

void CacheLevel::printStats() {
    uint64_t total = hits + misses;
    cout << "  [" << level_name << "] "
         << "Hit Rate: " << fixed << setprecision(2) << (total ? (double)hits / total * 100.0 : 0) << "% "
         << "(Access: " << total << ", Miss: " << misses << ", WB: " << write_backs << ")" << endl;
    cout << "      Prefetches Issued: " << prefetch_issued << endl;
}
