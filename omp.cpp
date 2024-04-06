#include <iostream>
#include <random>
#include <omp.h>
#include <cstdio>
#include <cmath>
#include <string>
using namespace std;

typedef unsigned int ui;

const int mod = (1 << 21) - 1;

double dist(double x1, double y1, double z1, double x2, double y2, double z2) {
    return sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2) + (z1-z2)*(z1-z2));
}

bool equals(double x, double y) {
    double eps = 1e-8;
    return x - y >= -eps && x - y <= eps;
}

uint64_t rol64(uint64_t x, int k)
{
    return (x << k) | (x >> (64 - k));
}

struct xoshiro256ss_state {
    uint64_t s[4];
};

uint64_t xoshiro256ss(struct xoshiro256ss_state *state)
{
    uint64_t *s = state->s;
    uint64_t const result = rol64(s[1] * 5, 7) * 9;
    uint64_t const t = s[1] << 17;

    s[2] ^= s[0];
    s[3] ^= s[1];
    s[1] ^= s[2];
    s[0] ^= s[3];

    s[2] ^= t;
    s[3] = rol64(s[3], 45);

    return result;
}

struct splitmix64_state {
    uint64_t s;
};

uint64_t splitmix64(struct splitmix64_state *state) {
    uint64_t result = (state->s += 0x9E3779B97f4A7C15);
    result = (result ^ (result >> 30)) * 0xBF58476D1CE4E5B9;
    result = (result ^ (result >> 27)) * 0x94D049BB133111EB;
    return result ^ (result >> 31);
}

void xoshiro256ss_init(struct xoshiro256ss_state *state, uint64_t seed) {
    struct splitmix64_state smstate = {seed};

    uint64_t tmp = splitmix64(&smstate);
    state->s[0] = (uint32_t)tmp;
    state->s[1] = (uint32_t)(tmp >> 32);

    tmp = splitmix64(&smstate);
    state->s[2] = (uint32_t)tmp;
    state->s[3] = (uint32_t)(tmp >> 32);
}

int main(int argc, char *argv[]) {
    int threads_in;
    try {
        threads_in = stoi(argv[1]);
    }
    catch (...) {
        cerr << "Incorrect number of threads" << '\n';
        return 0;
    }
    int n;
    double x1, y1, z1, x2, y2, z2, x3, y3, z3;
    try {
        FILE *in = fopen(argv[2], "r");
        fscanf(in, "%d\n(%lf %lf %lf) \n(%lf %lf %lf) \n(%lf %lf %lf)", &n,
               &x1, &y1, &z1, &x2, &y2, &z2, &x3, &y3, &z3);
        fclose(in);
    }
    catch (...) {
        cerr << "Error while reading a file" << '\n';
        return 0;
    }
    double side;
    double d1 = dist(x1, y1, z1, x2, y2, z2);
    double d2 = dist(x1, y1, z1, x3, y3, z3);
    if (!equals(d1, d2)) {
        side = dist(x2, y2, z2, x3, y3, z3);
    }
    else {
        side = d1;
    }
    double d = side * sqrt(2) / 2;
    int points_count = 0;
    double tstart = omp_get_wtime();
    if (threads_in == -1) {
        random_device rd;
        xoshiro256ss_state state;
        xoshiro256ss_init(&state, rd());
        int local_count = 0;
        for (int i = 0; i < n; i++) {
            long long tmp = xoshiro256ss(&state);
            long long x = tmp & mod;
            long long y = (tmp >> 21) & mod;
            long long z = (tmp >> 42) & mod;
            if (x + y + z < mod) {
                local_count++;
            }
        }
        points_count += local_count;
    }
    else {
        if (threads_in > omp_get_max_threads()) {
            cerr << "Number of threads is greater than it's maximum value" << '\n';
            return 0;
        }
        if (threads_in > 0) {
            omp_set_num_threads(threads_in);
        }
#pragma omp parallel
        {
            random_device rd;
            xoshiro256ss_state state;
            xoshiro256ss_init(&state, rd());
            int local_count = 0;
    #pragma omp for schedule(static, 1000)
            for (int i = 0; i < n; i++) {
                long long tmp = xoshiro256ss(&state);
                long long x = tmp & mod;
                long long y = (tmp >> 21) & mod;
                long long z = (tmp >> 42) & mod;
                if (x + y + z < mod) {
                    local_count++;
                }
            }
#pragma omp atomic
            points_count += local_count;
        }
    }

    double tend = omp_get_wtime();
    try {
        FILE *out = fopen(argv[3], "w");
        fprintf(out, "%g %g\n", 2 * side * side * d / 3, 8 * d * d * d * points_count / n);
        fclose(out);
    }
    catch (...) {
        cerr << "Error while writing to a file" << '\n';
        return 0;
    }
    printf("Time (%i thread(s)): %g ms\n",
           threads_in != -1 ? omp_get_max_threads() : 0, (tend - tstart) * 1000);
}