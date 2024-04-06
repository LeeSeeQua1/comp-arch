#include <iostream>
#include "Constants.h"

using namespace std;

#define M 64
#define N 60
#define K 32
char a[M][K];
short b[K][N];
int c[M][N];

int lru_tags[Constants::CACHE_SETS_COUNT][Constants::CACHE_WAY];
int plru_tags[Constants::CACHE_SETS_COUNT][Constants::CACHE_WAY];
int age[Constants::CACHE_SETS_COUNT][Constants::CACHE_WAY];
bool mru[Constants::CACHE_SETS_COUNT][Constants::CACHE_WAY];
bool lru_modified[Constants::CACHE_SETS_COUNT][Constants::CACHE_WAY];
bool plru_modified[Constants::CACHE_SETS_COUNT][Constants::CACHE_WAY];

int lru_time = 0;
int lru_hits = 0;
int lru_misses = 0;
int plru_time = 0;
int plru_hits = 0;
int plru_misses = 0;
int local_time = 0;
int mod = 0;

int get_idx(int addr) {
    return (addr >> Constants::CACHE_OFFSET_LEN) % Constants::CACHE_SETS_COUNT;
}

int get_tag(int addr) {
    return addr >> (Constants::CACHE_OFFSET_LEN + Constants::CACHE_IDX_LEN);
}

void update_age(int set_idx, int line_idx) {
    int val = age[set_idx][line_idx];
    age[set_idx][line_idx] = 0;
    for (int i = 0; i < Constants::CACHE_WAY; i++) {
        if (i != line_idx && age[set_idx][i] < val) {
            age[set_idx][i]++;
        }
    }
}

int hit_time(int size, bool r) {
    return r ? 7 + (size + 15) / 16 : 8;
}

int miss_time(int size, bool r) {
    return r ? 5 + (size + 15) / 16 : 6;
}

int mem_write_time() {
    return 102;
}

int mem_read_time() {
    return 101 + Constants::CACHE_LINE_SIZE / 2;
}

int lru_get(int addr, int size) {
    int idx = get_idx(addr);
    for (int i = 0; i < Constants::CACHE_WAY; i++) {
        if (lru_tags[idx][i] == get_tag(addr)) {
            lru_time += hit_time(size, true);
            lru_hits++;
            update_age(idx, i);
            return 1;
        }
    }
    lru_misses++;
    lru_time += miss_time(size, true);
    for (int i = 0; i < Constants::CACHE_WAY; i++) {
        if (age[idx][i] == Constants::CACHE_WAY - 1) {
            if (lru_modified[idx][i]) {
                lru_time += mem_write_time();
            }
            lru_time += mem_read_time();
            update_age(idx, i);
            lru_tags[idx][i] = get_tag(addr);
            lru_modified[idx][i] = false;
            return 0;
        }
    }
}

void lru_put(int addr, int size) {
    int idx = get_idx(addr);
    for (int i = 0; i < Constants::CACHE_WAY; i++) {
        if (lru_tags[idx][i] == get_tag(addr)) {
            lru_time += hit_time(size, false);
            lru_hits++;
            lru_modified[idx][i] = true;
            update_age(idx, i);
            return;
        }
    }
    lru_time += miss_time(size, false);
    lru_misses++;
    for (int i = 0; i < Constants::CACHE_WAY; i++) {
        if (age[idx][i] == Constants::CACHE_WAY - 1) {
            if (lru_modified[idx][i]) {
                lru_time += mem_write_time();
            }
            lru_time += mem_read_time();
            update_age(idx, i);
            lru_tags[idx][i] = get_tag(addr);
            lru_modified[idx][i] = true;
            return;
        }
    }
}

void update_mru(int set_idx, int line_idx) {
    mru[set_idx][line_idx] = true;
    int count = 0;
    for (int i = 0; i < Constants::CACHE_WAY; i++) {
        if (mru[set_idx][i]) count++;
    }
    if (count == Constants::CACHE_WAY) {
        for (int i = 0; i < Constants::CACHE_WAY; i++) {
            if (i != line_idx) mru[set_idx][i] = false;
        }
    }
}

int plru_get(int addr, int size) {
    int idx = get_idx(addr);
    for (int i = 0; i < Constants::CACHE_WAY; i++) {
        if (plru_tags[idx][i] == get_tag(addr)) {
            plru_time += hit_time(size, true);
            plru_hits++;
            update_mru(idx, i);
            return 1;
        }
    }
    plru_time += miss_time(size, true);
    plru_misses++;
    for (int i = 0; i < Constants::CACHE_WAY; i++) {
        if (!mru[idx][i]) {
            if (plru_modified[idx][i]) {
                plru_time += mem_write_time();
                mod++;
            }
            plru_time += mem_read_time();
            plru_tags[idx][i] = get_tag(addr);
            update_mru(idx, i);
            plru_modified[idx][i] = false;
            return 0;
        }
    }
}

int plru_put(int addr, int size) {
    int idx = get_idx(addr);
    for (int i = 0; i < Constants::CACHE_WAY; i++) {
        if (plru_tags[idx][i] == get_tag(addr)) {
            plru_time += hit_time(size, false);
            plru_hits++;
            update_mru(idx, i);
            plru_modified[idx][i] = true;
            return 1;
        }
    }
    plru_time += miss_time(size, false);
    plru_misses++;
    for (int i = 0; i < Constants::CACHE_WAY; i++) {
        if (!mru[idx][i]) {
            if (plru_modified[idx][i]) {
                mod++;
                plru_time += mem_write_time();
            }
            plru_time += mem_read_time();
            plru_tags[idx][i] = get_tag(addr);
            update_mru(idx, i);
            plru_modified[idx][i] = true;
            return 0;
        }
    }
}

void init() {
    for (int i = 0; i < Constants::CACHE_SETS_COUNT; i++) {
        for (int j = 0; j < Constants::CACHE_WAY; j++) {
            lru_tags[i][j] = -1;
            plru_tags[i][j] = -1;
            age[i][j] = j;
            mru[i][j] = false;
            lru_modified[i][j] = false;
            plru_modified[i][j] = false;
        }
    }
}

void mmul() {
    int a_addr = 0x40000;
    int b_addr = a_addr + M * K;
    int c_addr = b_addr + 2 * K * N;
    local_time += 2; // assign pointers pa and pc
    local_time++; // entering loop
    for (int y = 0; y < M; y++) {
        local_time++; // entering inner loop
        for (int x = 0; x < N; x++) {
            int s = 0;
            b_addr = 0x40000 + M * K;
            local_time += 2; // two assignments;
            local_time++; // entering inner loop
            for (int k = 0; k < K ; k++) {
                s += lru_get(a_addr + k, 8)
                     * lru_get(b_addr + 2 * x, 16);
                s += plru_get(a_addr + k, 8)
                     * plru_get(b_addr + 2 * x, 16);
                local_time += 6; // addition & multiplication
                b_addr += 2 * N;
                local_time++; // pointer moving
                local_time += 2; // increment + new iteration
            }
            lru_put(c_addr + 4 * x, 32);
            plru_put(c_addr + 4 * x, 32);
            local_time += 2; // increment + new iteration
        }
        a_addr += K;
        c_addr += 4 * N;
        local_time += 2; // addition
        local_time += 2; // increment + new iteration
    }
    local_time += 1; // exiting function
}

int main() {
    init();
    mmul();
    printf("LRU:\thit perc. %3.4f%%\ttime: %d\npLRU:\thit perc. %3.4f%%\ttime: %d\n",
           (double)lru_hits * 100 / (lru_hits + lru_misses),
           lru_time + local_time,
           (double)plru_hits * 100 / (plru_hits + plru_misses),
           plru_time + local_time);
    return 0;
}