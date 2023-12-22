#include <stdio.h>

#include "benchmark/benchmark.h"

void Benchmark_Get(benchmark::State &state) {
  int i = 0;
  for ([[maybe_unused]] auto _ : state) {
    printf("i %d\n", i++);
  }
}

BENCHMARK(Benchmark_Get);