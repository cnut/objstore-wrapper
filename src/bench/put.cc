#include <stdio.h>

#include "benchmark/benchmark.h"

#include "s3.h"

#include <fstream>
#include <string>

std::string_view kBucketName = "glenn-wp";
std::string_view kRegionName = "cn-northwest-1";

int CreateFile(std::string_view path, size_t fsize) {
  std::ofstream file(path.data(), std::ios::binary);
  if (!file) {
    return -1; // Return an appropriate error code if file creation fails
  }

  // Set the file size
  file.seekp(fsize - 1);
  file.write("", 1);
  file.close();

  return 0; // Return 0 on success
}

void Benchmark_CreateConfigure(benchmark::State &state) {
  for ([[maybe_unused]] auto _ : state) {
    Aws::Client::ClientConfiguration config =
        CreateClientConf(kRegionName, nullptr, false);
  }
}

void Benchmark_Put32B(benchmark::State &state) {
  std::string_view filepath = "some_file";
  int ret = CreateFile(filepath, 32);
  assert(ret == 0);

  Aws::Client::ClientConfiguration config =
      CreateClientConf(kRegionName, nullptr, false);

  int i = 0;

  for ([[maybe_unused]] auto _ : state) {
    PutObject(config, kBucketName, "small" + std::to_string(i++), filepath);
  }
}

void Benchmark_Put2M(benchmark::State &state) {
  std::string_view filepath = "some_file";
  int ret = CreateFile(filepath, 2 * 1024 * 1024);
  assert(ret == 0);

  Aws::Client::ClientConfiguration config =
      CreateClientConf(kRegionName, nullptr, false);

  int i = 0;

  for ([[maybe_unused]] auto _ : state) {
    PutObject(config, kBucketName, "big" + std::to_string(i++), filepath);
  }
}

BENCHMARK(Benchmark_CreateConfigure);
BENCHMARK(Benchmark_Put32B);
BENCHMARK(Benchmark_Put2M);