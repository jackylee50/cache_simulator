#include "repl_policy.h"

// =========================================================
// TODO: Task 1 / Task 3 replacement policies
// Implement LRU first, then extend with SRRIP / BIP.
// =========================================================

void LRUPolicy::onHit(std::vector<CacheLine>& set, int way, uint64_t cycle) {
    (void)set;
    (void)way;
    (void)cycle;
    // TODO: mark the hit line as most recently used.
    set[way].last_access = cycle;
}

void LRUPolicy::onMiss(std::vector<CacheLine>& set, int way, uint64_t cycle) {
    (void)set;
    (void)way;
    (void)cycle;
    // TODO: initialize a newly inserted line as MRU.
    set[way].last_access = cycle;
}

int LRUPolicy::getVictim(std::vector<CacheLine>& set) {
    uint64_t min_time = UINT64_MAX;
    int victim = 0;

    for (int i = 0; i < (int)set.size(); i++) {
        // 只考虑 valid line（如果你有 valid bit）
        if (!set[i].valid) {
            return i; // 直接用空位置
        }

        if (set[i].last_access < min_time) {
            min_time = set[i].last_access;
            victim = i;
        }
    }

    return victim;
}

void SRRIPPolicy::onHit(std::vector<CacheLine>& set, int way, uint64_t cycle) {
    (void)set;
    (void)way;
    (void)cycle;
    // TODO: typically promote the line to RRPV=0.
    // HIT → most recently used behavior
    set[way].rrpv = 0;
}

void SRRIPPolicy::onMiss(std::vector<CacheLine>& set, int way, uint64_t cycle) {
    (void)set;
    (void)way;
    (void)cycle;
    // TODO: insert with a long re-reference interval, e.g. RRPV=2.
    // insert with long re-reference interval
    set[way].rrpv = 2;   // standard SRRIP insertion point
}

int SRRIPPolicy::getVictim(std::vector<CacheLine>& set) {
    (void)set;
    // TODO: search for RRPV==3, otherwise age all lines and retry.
    while (true) {

        // 1. try find RRPV = 3
        for (int i = 0; i < (int)set.size(); i++) {
            if (set[i].valid && set[i].rrpv == 3) {
                return i;
            }
        }

        // 2. age all lines (increment RRPV)
        for (int i = 0; i < (int)set.size(); i++) {
            if (set[i].valid && set[i].rrpv < 3) {
                set[i].rrpv++;
            }
        }
    }

    return 0; // unreachable
}

void BIPPolicy::onHit(std::vector<CacheLine>& set, int way, uint64_t cycle) {
    (void)set;
    (void)way;
    (void)cycle;
    // TODO: hits still become MRU.
    // LRU-style promotion → update timestamp
    set[way].last_access = cycle;
}

void BIPPolicy::onMiss(std::vector<CacheLine>& set, int way, uint64_t cycle) {
    (void)set;
    (void)way;
    (void)cycle;
    // TODO: mostly insert at LRU position, but occasionally insert at MRU.
    // Hint: use insertion_counter and throttle.
    insertion_counter++;

    bool insert_at_mru = (insertion_counter % 32 == 0);

    if (insert_at_mru) {
        // rare MRU insertion → recent use
        set[way].last_access = cycle;
    } else {
        // normal insertion → make it very old
        set[way].last_access = 0;
    }
}

int BIPPolicy::getVictim(std::vector<CacheLine>& set) {
    (void)set;
    // TODO: BIP usually uses the same victim selection as LRU.
    int victim = 0;
    uint64_t oldest = UINT64_MAX;

    for (int i = 0; i < (int)set.size(); i++) {

        if (!set[i].valid) return i;

        if (set[i].last_access < oldest) {
            oldest = set[i].last_access;
            victim = i;
        }
    }

    return victim;
}

ReplacementPolicy* createReplacementPolicy(std::string name) {
    if (name == "SRRIP") return new SRRIPPolicy();
    if (name == "BIP") return new BIPPolicy();
    return new LRUPolicy();
}
