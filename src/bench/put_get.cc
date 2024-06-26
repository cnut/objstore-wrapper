#include <cstddef>
#include <fstream>
#include <stdio.h>

#include "benchmark/benchmark.h"
#include "gflags/gflags.h"

#include "objstore.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <sys/errno.h>

DEFINE_string(provider, "local",
              "provider of objstore, only support local or aws");
DEFINE_string(region, "/tmp/",
              "region of the objstore, direcotry for local objstore");
DEFINE_string(endpoint, "",
              "endpoint, which will be ignoreed by local objstore");
DEFINE_bool(
    use_https, false,
    "whether to use https or not, which will be ignored by local objstore");
DEFINE_string(bucket, "test_bucket",
              "bucket, which will be used for this test");

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

  objstore::ObjectStore *obj_store = objstore::create_object_store(
      FLAGS_provider, FLAGS_region, nullptr, false);
  assert(obj_store != nullptr);

  for ([[maybe_unused]] auto _ : state) {
    obj_store->put_object_from_file(FLAGS_bucket, objkey, filepath.c_str());
  }

  destroy_object_store(obj_store);
  remove_file(filepath);
}

void get_from_s3_put_file(std::string_view prefix, size_t fsize,
                          benchmark::State &state) {
  const std::string obj_key(assemble_file_path(prefix, fsize));
  const std::string filepath = obj_key + ".s3";

  objstore::ObjectStore *obj_store = objstore::create_object_store(
      FLAGS_provider, FLAGS_region, nullptr, false);
  assert(obj_store != nullptr);

  for ([[maybe_unused]] auto _ : state) {
    obj_store->get_object_to_file(FLAGS_bucket, obj_key, filepath.c_str());
  }

  destroy_object_store(obj_store);
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

int main(int argc, char **argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  benchmark::RunSpecifiedBenchmarks();
  return 0;
}
