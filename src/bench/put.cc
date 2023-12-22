#include <stdio.h>

#include "benchmark/benchmark.h"

#include "s3.h"

void Benchmark_CreateConfigure(benchmark::State &state) {
  for ([[maybe_unused]] auto _ : state) {
    Aws::Client::ClientConfiguration config =
        CreateClientConf("glenn-test", nullptr, false);
  }
}

void Benchmark_Put(benchmark::State &state) {
  int i = 0;
  for ([[maybe_unused]] auto _ : state) {
    ++i;
  }
}

BENCHMARK(Benchmark_CreateConfigure);
BENCHMARK(Benchmark_Put);