#include "local.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sys/errno.h>
#include <system_error>

namespace objstore {

namespace fs = std::filesystem;

namespace {

int mkdir_p(std::string_view path) {
  std::error_code errcode;
  bool created = fs::create_directories(path, errcode);
  return errcode.value();
}

int rm_f(std::string_view path) {
  std::error_code errcode;
  bool created = fs::remove_all(path, errcode);
  return errcode.value();
}

} // anonymous namespace

Status LocalObjectStore::create_bucket(const std::string_view &bucket) {
  if (!is_valid_key(bucket)) {
    return Status(EINVAL, "invalid bucket");
  }

  int ret = mkdir_p(generate_path(bucket));
  return Status(ret, std::generic_category().message(ret));
}

Status LocalObjectStore::delete_bucket(const std::string_view &bucket) {
  if (!is_valid_key(bucket)) {
    return Status(EINVAL, "invalid bucket");
  }

  int ret = rm_f(generate_path(bucket));
  return Status(ret, std::generic_category().message(ret));
}

Status
LocalObjectStore::put_object_from_file(const std::string_view &bucket,
                                       const std::string_view &key,
                                       const std::string_view &data_file_name) {
  if (!is_valid_key(key)) {
    return Status(EINVAL, "invalid key");
  }

  std::string key_path = generate_path(bucket, key);
  std::error_code errcode;
  fs::copy(data_file_name, key_path, fs::copy_options::overwrite_existing,
           errcode);
  return Status(errcode.value(), errcode.message());
}

Status
LocalObjectStore::get_object_to_file(const std::string_view &bucket,
                                     const std::string_view &key,
                                     const std::string_view &output_file_name) {
  if (!is_valid_key(key)) {
    return Status(EINVAL, "invalid key");
  }

  std::string key_path = generate_path(bucket, key);
  std::error_code errcode;
  fs::copy(key_path, output_file_name, fs::copy_options::overwrite_existing,
           errcode);
  return Status(errcode.value(), errcode.message());
}

Status LocalObjectStore::put_object(const std::string_view &bucket,
                                    const std::string_view &key,
                                    const std::string_view &data) {
  if (!is_valid_key(key)) {
    return Status(EINVAL, "invalid key");
  }

  std::string key_path = generate_path(bucket, key);
  std::ofstream outputFile(key_path, std::ios::binary | std::ios::trunc);
  if (!outputFile) {
    return Status(EIO, "Couldn't open file");
  }

  bool ok = !outputFile.write(data.data(), data.size());
  outputFile.close();
  return ok ? Status() : Status(EIO, "write fail");
}

Status LocalObjectStore::get_object(const std::string_view &bucket,
                                    const std::string_view &key,
                                    std::string &body) {
  if (!is_valid_key(key)) {
    return Status(EINVAL, "invalid key");
  }

  std::string key_path = generate_path(bucket, key);
  std::ifstream inputFile(key_path, std::ios::binary);
  if (!inputFile) {
    return Status(EIO, "Couldn't open file");
  }

  inputFile.seekg(0, std::ios::end);
  std::streamsize fileSize = inputFile.tellg();
  inputFile.seekg(0, std::ios::beg);

  body.resize(fileSize);
  // if error, just let stl throw exception.
  bool ok = !inputFile.read(body.data(), body.size());
  inputFile.close();
  return ok ? Status() : Status(EIO, "write fail");
}

Status LocalObjectStore::list_object(const std::string_view &bucket,
                                     const std::string_view &key,
                                     std::vector<std::string> objects) {
  std::string bucket_path = generate_path(bucket);
  objects.clear();
  for (const auto &entry : fs::directory_iterator(bucket_path)) {
    if (fs::is_directory(entry)) {
      std::cerr << "encounter a directory: " << entry.path()
                << ", some error happened" << std::endl;
      abort();
    } else if (fs::is_regular_file(entry)) {
      objects.push_back(entry.path().filename().string());
    }
  }
  return Status(-1, "not implemented");
}

Status LocalObjectStore::delete_object(const std::string_view &bucket,
                                       const std::string_view &key) {
  if (!is_valid_key(key)) {
    return Status(EINVAL, "invalid key");
  }

  std::string key_path = generate_path(bucket, key);
  int ret = rm_f(key_path);
  return Status(ret, std::generic_category().message(ret));
}

bool LocalObjectStore::is_valid_key(const std::string_view &key) {
  return key.find('/') == std::string::npos;
}

std::string LocalObjectStore::generate_path(const std::string_view &bucket) {
  return std::string(basepath_) + "/" + std::string(bucket);
}

std::string LocalObjectStore::generate_path(const std::string_view &bucket,
                                            const std::string_view &key) {
  std::string key_buf(key);
  return std::string(basepath_) + "/" + std::string(bucket) + '/' +
         std::string(key_buf);
}

LocalObjectStore *create_local_objstore(const std::string_view region,
                                        const std::string_view *endpoint,
                                        bool use_https) {
  int ret = mkdir_p(region);
  if (ret != 0) {
    return nullptr;
  }

  LocalObjectStore *lobs =
      new LocalObjectStore(region /* use region parameter as basepath */);

  if (lobs == nullptr) {
    rm_f(region);
  }

  return lobs;
}

LocalObjectStore *create_local_objstore(const std::string_view &access_key,
                                        const std::string_view &secret_key,
                                        const std::string_view region,
                                        const std::string_view *endpoint,
                                        bool use_https) {
  return create_local_objstore(region, endpoint, use_https);
}

void destroy_local_objstore(LocalObjectStore *local_objstore) {
  delete local_objstore;
  // keep the data there.
  return;
}
}; // namespace objstore
