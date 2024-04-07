# Introduction
The S3 interface has become the de facto standard for object storage,
and most object storage will be compatible with the S3 interface.

AWS provides [aws-sdk-cpp](https://github.com/aws/aws-sdk-cpp.git) to help application programs to manipulate S3 objects.
But it is relatively cumbersome to use aws-sdk-cpp directly.
This repository provides an S3 wrapper to reduce the development complexity for C++ programmer when he/she tries to use S3-like object storage.

At the same time, in order to facilitate development and testing in local environment, this repository provides a local object store emulated by local file.

In addition, this repository also provides a benchmark to facilitate testing the performance of object storage.

# Build

```bash
# prepare the build directory
mkdir build
cd build

# config by CMake then build it
cmake ..
make -j
```

# Run unit test
```bash
cd ${build_path}

# run unittest on object store emulated by local filesystem
./src/objstore_test
[==========] Running 2 tests from 1 test suite.
[----------] Global test environment set-up.
[----------] 2 tests from ObjstoreTest
[ RUN      ] ObjstoreTest.PutGetDeleteMeta
[       OK ] ObjstoreTest.PutGetDeleteMeta (0 ms)
[ RUN      ] ObjstoreTest.List
[       OK ] ObjstoreTest.List (18 ms)
[----------] 2 tests from ObjstoreTest (18 ms total)

[----------] Global test environment tear-down
[==========] 2 tests from 1 test suite ran. (18 ms total)
[  PASSED  ] 2 tests.

# run unittest on real s3 object store
export AWS_ACCESS_KEY_ID=${replace_with_your_access_key}
export AWS_SECRET_ACCESS_KEY=${replace_with_your_secret_key}
export AWS_REGION=${replace_with_your_region}
export AWS_BUCKET=${replace_with_your_bucket}
./src/objstore_test --provider=aws --region=${AWS_REGION} --bucket=${AWS_BUCKET}
[==========] Running 2 tests from 1 test suite.
[----------] Global test environment set-up.
[----------] 2 tests from ObjstoreTest
[ RUN      ] ObjstoreTest.PutGetDeleteMeta
[       OK ] ObjstoreTest.PutGetDeleteMeta (662 ms)
[ RUN      ] ObjstoreTest.List
[       OK ] ObjstoreTest.List (4819 ms)
[----------] 2 tests from ObjstoreTest (5481 ms total)

[----------] Global test environment tear-down
[==========] 2 tests from 1 test suite ran. (5481 ms total)
[  PASSED  ] 2 tests.
```

# Run the benchmark

```bash
cd ${build_path}

# local file
./src/run_put_get
2024-04-07T20:22:41+08:00
Run on (8 X 2200 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x4)
  L1 Instruction 64 KiB (x4)
  L2 Unified 512 KiB (x4)
  L3 Unified 8192 KiB (x1)
Load Average: 0.12, 0.15, 0.17
--------------------------------------------------------------------------
Benchmark                                Time             CPU   Iterations
--------------------------------------------------------------------------
Benchmark_Put32B/iterations:10      673746 ns       116475 ns           10
Benchmark_Get32B/iterations:10      671033 ns       163809 ns           10
Benchmark_Put4K/iterations:10       549130 ns       112278 ns           10
Benchmark_Get4K/iterations:10       594254 ns       151080 ns           10
Benchmark_Put2M/iterations:10      6257329 ns      1921726 ns           10
Benchmark_Get2M/iterations:10      6632720 ns      1770679 ns           10
Benchmark_Put128M/iterations:10  836703031 ns     89330970 ns           10
Benchmark_Get128M/iterations:10  919419905 ns     91914068 ns           10
...


# aws s3
export AWS_ACCESS_KEY_ID=${replace_with_your_access_key}
export AWS_SECRET_ACCESS_KEY=${replace_with_your_secret_key}
export AWS_REGION=${replace_with_your_region}
export AWS_BUCKET=${replace_with_your_bucket}
./src/run_put_get --provider=aws --region=${AWS_REGION} --bucket=${AWS_BUCKET}
2024-04-07T20:25:56+08:00
Run on (8 X 2200 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x4)
  L1 Instruction 64 KiB (x4)
  L2 Unified 512 KiB (x4)
  L3 Unified 8192 KiB (x1)
Load Average: 0.17, 0.22, 0.20
--------------------------------------------------------------------------
Benchmark                                Time             CPU   Iterations
--------------------------------------------------------------------------
Benchmark_Put32B/iterations:10    22222298 ns      2051112 ns           10
Benchmark_Get32B/iterations:10    11883256 ns      1885276 ns           10
Benchmark_Put4K/iterations:10     31041540 ns      1886112 ns           10
Benchmark_Get4K/iterations:10     16202092 ns      1963213 ns           10
Benchmark_Put2M/iterations:10     83472982 ns     10209276 ns           10
Benchmark_Get2M/iterations:10     33561795 ns     11899454 ns           10
Benchmark_Put128M/iterations:10 1177425063 ns    526843469 ns           10
Benchmark_Get128M/iterations:10 1800414391 ns    578817779 ns           10
```
