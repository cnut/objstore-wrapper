#include <cstddef>
#include <fstream>
#include <stdio.h>

#include "benchmark/benchmark.h"

#include "obj_store.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <sys/errno.h>

std::string_view kBucketName = "glenn-wp";
std::string_view kRegionName = "cn-northwest-1";

std::string formatBytes(uint64_t bytes) {
  double size = static_cast<double>(bytes);
  const char *units[] = {"B", "K", "M", "G", "T"};
  char buf[64];

  int index = 0;
  while (size >= 1024 && index < sizeof(units) / sizeof(units[0])) {
    size /= 1024;
    index++;
  }

  int ret = snprintf(buf, sizeof(buf), "%.2f%s", size, units[index]);
  assert(ret > 0 && ret < sizeof(buf));
  return std::string(buf, ret);
}

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

int RemoveFile(std::string_view path) {
  bool deleted = std::filesystem::remove(path.data());
  return deleted ? 0 : -ENOENT;
}

void CreateFilePutToS3DeleteFile(std::string_view prefix, size_t fsize,
                                 benchmark::State &state) {
  std::string fsize_str = formatBytes(fsize);
  const std::string filepath(std::string(prefix) + "_" + fsize_str);
  std::string objkey = std::string(prefix) + "_" + fsize_str;
  int ret = CreateFile(filepath, fsize);
  assert(ret == 0);

  objstore::ObjectStore *obj_store =
      objstore::create_object_store("aws", kRegionName, nullptr, false);
  assert(obj_store != nullptr);

  for ([[maybe_unused]] auto _ : state) {
    obj_store->put_object(kBucketName, objkey, filepath.c_str());
  }

  delete obj_store;
  RemoveFile(filepath);
}

void Benchmark_Put32B(benchmark::State &state) {
  CreateFilePutToS3DeleteFile("test_put", 32, state);
}

void Benchmark_Put4K(benchmark::State &state) {
  CreateFilePutToS3DeleteFile("test_put", 4096, state);
}

void Benchmark_Put2M(benchmark::State &state) {
  CreateFilePutToS3DeleteFile("test_put", 2 * 1024 * 1024, state);
}

void Benchmark_Put128M(benchmark::State &state) {
  CreateFilePutToS3DeleteFile("test_put", 128 * 1024 * 1024, state);
}

void Benchmark_Put2G(benchmark::State &state) {
  CreateFilePutToS3DeleteFile("test_put", 2ULL * 1024 * 1024 * 1024, state);
}

BENCHMARK(Benchmark_Put32B)->Iterations(10);
BENCHMARK(Benchmark_Put4K)->Iterations(10);
BENCHMARK(Benchmark_Put2M)->Iterations(10);
BENCHMARK(Benchmark_Put128M)->Iterations(10);
BENCHMARK(Benchmark_Put2G)->Iterations(10);