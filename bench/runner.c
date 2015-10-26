#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "timer.h"
#include "generate.h"

static void print_help() {
    fprintf(stderr,"usage: bench [OPTIONS]");
    fprintf(stderr,"options:");
    fprintf(stderr, "-b 200\n");
    fprintf(stderr, "\tbench size in MBs\n");
    fprintf(stderr, "-e 2\n");
    fprintf(stderr, "\tenlarge bench size by repeating it x times\n");
    fprintf(stderr, "-c 6\n");
    fprintf(stderr, "\tcolumns to generate\n");
    fprintf(stderr, "-r 5\n");
    fprintf(stderr, "\tnumber of measure runs\n");
    fprintf(stderr, "-s 42\n");
    fprintf(stderr, "\tseed for random generator\n");
}

static void run(const char* restrict command, const char* restrict buffer, size_t buffer_size, unsigned int buffer_copy, unsigned int repeats, double* results) {
    for (unsigned int r = 0; r < repeats; r++) {
        FILE* target = popen(command, "w");
        if (!target) {
            fprintf(stderr, "Can't start \"%s\"\n", command);
            results[r] = -1;
        }
        double start = getRealTime();
        for (unsigned int b = 0; b < buffer_copy; b++) {
            fwrite(buffer, sizeof(char), buffer_size, target);
            fflush(target);
        }
        if (pclose(target) != 0) {
            fprintf(stderr, "\"%s\" had an error.\n", command);
            results[r] = -1;
        }
        double stop = getRealTime();
        results[r] = (stop - start);
    }
}

static void print_run(const char* restrict command, const char* restrict buffer, size_t buffer_size, unsigned int buffer_copy, unsigned int repeats) {
    double* results = calloc(repeats, sizeof(double));
    run(command, buffer, buffer_size, buffer_copy, repeats, results);
    fprintf(stdout, "\"%s\"", command);
    for (unsigned int r = 0; r < repeats; r++) {
        fprintf(stdout, ",%f", ( (buffer_size * buffer_copy) / results[r]) / (1024*1024) );
    }
    fprintf(stdout, "\n");
    free(results);
}

// based on xxhash avalanche
#define PRIME1   2654435761U
#define PRIME2   2246822519U
#define PRIME3   3266489917U

static unsigned int xxh_mix(unsigned int x, unsigned int seed) {
    unsigned int crc = x  + seed + PRIME1;
    crc ^= crc >> 15;
    crc *= PRIME2;
    crc ^= crc >> 13;
    crc *= PRIME3;
    crc ^= crc >> 16;
    return crc;
}

int main(int argc, char** argv) {
    size_t bench_size = 200 * 1024 * 1024; 
    unsigned int columns = 6;
    unsigned int repeats = 5;
    unsigned int bench_copy = 2;
    unsigned int seed1 = xxh_mix(29, 42);
    unsigned int seed2 = xxh_mix(13, 11);

    char c;
    while ((c = getopt (argc, argv, "b:c:r:e:s:h")) != -1) {
        switch (c) {
            case 'b':
                sscanf(optarg, "%zd", &bench_size);
                bench_size *= 1024 * 1024;
                break;
            case 'c':
                sscanf(optarg, "%u", &columns);
                break;
            case 'r':
                sscanf(optarg, "%u", &repeats);
                break;
            case 'e':
                sscanf(optarg, "%u", &bench_copy);
                break;
            case 's':
                sscanf(optarg, "%u", &seed1);
                seed1 = xxh_mix(seed1, 42);
                break;
            case '?':
            case 'h':
            default:
                print_help();
                exit(1);
                break;
        }
    }
    char* buffer = calloc(bench_size, sizeof(char));
    fprintf(stderr, "Preparing data (%zd bytes)\n",bench_size); 
    size_t data_filled = generate_csv(buffer, bench_size, seed1, seed2, columns);
    fprintf(stderr, "Data ready (%zd bytes)\n",data_filled); 
    fprintf(stdout, "command");
    for (unsigned int r = 0; r < repeats; r++) {
        fprintf(stdout, ",run%u", r);
    }
    fprintf(stdout, "\n");
    fprintf(stderr, "Running csvgrep\n");
    print_run("wc -l > /dev/null", buffer, data_filled, bench_copy, repeats);
    print_run("wc -l > /dev/null", buffer, data_filled, bench_copy, repeats);
    
    return 0;
}
