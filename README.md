# Introduction
The S3 interface has become the de facto standard for object storage,
and most object storage will be compatible with the S3 interface.

AWS provides [aws-sdk-cpp](https://github.com/aws/aws-sdk-cpp.git) to help application programs to manipulate S3 objects.
But it is relatively cumbersome to use aws-sdk-cpp directly.
This repository provides an S3 wrapper to reduce the development complexity for C++ programmer when he/she tries to use S3-like object storage.

At the same time, in order to facilitate development and testing in local environment, this repository provides a local object store emulated by local file.

In addition, this repository also provides a benchmark to facilitate testing the performance of object storage.

# build

```bash
# prepare the build directory
mkdir build
cd build

# config by CMake then build it
cmake ..
make -j
```

# run unit test
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

# run the benchmark

```bash
cd ${build_path}

export AWS_ACCESS_KEY_ID=${replace_with_your_access_key}
export AWS_SECRET_ACCESS_KEY=${replace_with_your_secret_key}

./src/run_put_get
2024-03-19T11:24:40+08:00
Running ./src/run_put_get
Run on (8 X 2199.99 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x4)
  L1 Instruction 64 KiB (x4)
  L2 Unified 512 KiB (x4)
  L3 Unified 8192 KiB (x1)
Load Average: 0.09, 0.14, 0.30
--------------------------------------------------------------------------
Benchmark                                Time             CPU   Iterations
--------------------------------------------------------------------------
Benchmark_Put32B/iterations:10    22860820 ns      2062632 ns           10
Benchmark_Get32B/iterations:10    12312219 ns      1923766 ns           10
Benchmark_Put4K/iterations:10     29301010 ns      1804016 ns           10
Benchmark_Get4K/iterations:10     14785381 ns      1911869 ns           10
Benchmark_Put2M/iterations:10     71184075 ns     11047759 ns           10
Benchmark_Get2M/iterations:10     31855399 ns     11215472 ns           10
Benchmark_Put128M/iterations:10 1228492543 ns    474431809 ns           10
Benchmark_Get128M/iterations:10 1780746023 ns    603051582 ns           10
```
