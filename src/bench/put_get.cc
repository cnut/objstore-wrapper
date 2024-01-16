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

std::string format_bytes(uint64_t bytes) {
  uint64_t size = static_cast<double>(bytes);
  const char *units[] = {"B", "K", "M", "G", "T"};
  char buf[64];

  int index = 0;
  while (size >= 1024 && index < sizeof(units) / sizeof(units[0])) {
    size /= 1024;
    index++;
  }

  int ret = snprintf(buf, sizeof(buf), "%lld%s", size, units[index]);
  assert(ret > 0 && ret < sizeof(buf));
  return std::string(buf, ret);
}

std::string assemble_file_path(std::string_view prefix, size_t fsize) {
  std::string fsize_str = format_bytes(fsize);
  return std::string(prefix) + "_" + fsize_str;
}

int create_file(std::string_view path, size_t fsize) {
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

int remove_file(std::string_view path) {
  bool deleted = std::filesystem::remove(path.data());
  return deleted ? 0 : -ENOENT;
}

void create_file_put_to_s3_delete_file(std::string_view prefix, size_t fsize,
                                       benchmark::State &state) {
  const std::string filepath(assemble_file_path(prefix, fsize));
  const std::string objkey = filepath;
  int ret = create_file(filepath, fsize);
  assert(ret == 0);

  objstore::ObjectStore *obj_store =
      objstore::create_object_store("aws", kRegionName, nullptr, false);
  assert(obj_store != nullptr);

  for ([[maybe_unused]] auto _ : state) {
    obj_store->put_object_from_file(kBucketName, objkey, filepath.c_str());
  }

  delete obj_store;
  remove_file(filepath);
}

void get_from_s3_put_file(std::string_view prefix, size_t fsize,
                          benchmark::State &state) {
  const std::string obj_key(assemble_file_path(prefix, fsize));
  const std::string filepath = obj_key + ".s3";

  objstore::ObjectStore *obj_store =
      objstore::create_object_store("aws", kRegionName, nullptr, false);
  assert(obj_store != nullptr);

  for ([[maybe_unused]] auto _ : state) {
    obj_store->get_object_to_file(kBucketName, obj_key, filepath.c_str());
  }

  delete obj_store;
}

void Benchmark_Put32B(benchmark::State &state) {
  create_file_put_to_s3_delete_file("object", 32, state);
}

void Benchmark_Get32B(benchmark::State &state) {
  get_from_s3_put_file("object", 32, state);
}

void Benchmark_Put4K(benchmark::State &state) {
  create_file_put_to_s3_delete_file("object", 4096, state);
}

void Benchmark_Get4K(benchmark::State &state) {
  get_from_s3_put_file("object", 4096, state);
}

void Benchmark_Put2M(benchmark::State &state) {
  create_file_put_to_s3_delete_file("object", 2 * 1024 * 1024, state);
}

void Benchmark_Get2M(benchmark::State &state) {
  get_from_s3_put_file("object", 2 * 1024 * 1024, state);
}

void Benchmark_Put128M(benchmark::State &state) {
  create_file_put_to_s3_delete_file("object", 128 * 1024 * 1024, state);
}

void Benchmark_Get128M(benchmark::State &state) {
  get_from_s3_put_file("object", 128 * 1024 * 1024, state);
}

void Benchmark_Put2G(benchmark::State &state) {
  create_file_put_to_s3_delete_file("object", 2ULL * 1024 * 1024 * 1024, state);
}

void Benchmark_Get2G(benchmark::State &state) {
  get_from_s3_put_file("object", 2ULL * 1024 * 1024 * 1024, state);
}

BENCHMARK(Benchmark_Put32B)->Iterations(10);
BENCHMARK(Benchmark_Get32B)->Iterations(10);
BENCHMARK(Benchmark_Put4K)->Iterations(10);
BENCHMARK(Benchmark_Get4K)->Iterations(10);
BENCHMARK(Benchmark_Put2M)->Iterations(10);
BENCHMARK(Benchmark_Get2M)->Iterations(10);
BENCHMARK(Benchmark_Put128M)->Iterations(10);
BENCHMARK(Benchmark_Get128M)->Iterations(10);
BENCHMARK(Benchmark_Put2G)->Iterations(10);
BENCHMARK(Benchmark_Get2G)->Iterations(10);