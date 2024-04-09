#include "local.h"

#include <assert.h>
#include <cerrno>
#include <chrono>
#include <sys/errno.h>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <system_error>

namespace objstore {

namespace fs = std::filesystem;

namespace {

int mkdir_p(std::string_view path) {
  std::error_code errcode;
  fs::create_directories(path, errcode);
  // bool created = fs::create_directories(path, errcode);
  //  assert(created);
  return errcode.value();
}

int rm_f(std::string_view path) {
  std::error_code errcode;
  fs::remove_all(path, errcode);
  // bool deleted = fs::remove_all(path, errcode);
  // assert(deleted);
  return errcode.value();
}

// if the path is invalid, an execption will be throw
bool is_dir_empty(std::string_view path) { return fs::is_empty(path); }

int get_obj_meta_from_file(fs::path path, ObjectMeta &meta) {
  std::error_code errcode;

  fs::file_time_type ftime = fs::last_write_time(path, errcode);
  if (errcode.value() != 0) {
    return errcode.value();
  }
  int64_t epoch_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                            ftime.time_since_epoch())
                            .count();

  errcode.clear();
  std::uintmax_t fsize = fs::file_size(path, errcode);
  if (errcode.value() != 0) {
    return errcode.value();
  }

  meta.last_modified = epoch_in_ms;
  meta.size = fsize;

  return 0;
}

}  // anonymous namespace

Status LocalObjectStore::create_bucket(const std::string_view &bucket) {
  const std::lock_guard<std::mutex> _(mutex_);

  if (!is_valid_key(bucket)) {
    return Status(EINVAL, "invalid bucket");
  }

  int ret = mkdir_p(generate_path(bucket));
  return Status(ret, std::generic_category().message(ret));
}

Status LocalObjectStore::delete_bucket(const std::string_view &bucket) {
  const std::lock_guard<std::mutex> _(mutex_);

  if (!is_valid_key(bucket)) {
    return Status(EINVAL, "invalid bucket");
  }

  int ret = rm_f(generate_path(bucket));
  return Status(ret, std::generic_category().message(ret));
}

Status LocalObjectStore::put_object_from_file(
    const std::string_view &bucket, const std::string_view &key,
    const std::string_view &data_file_path) {
  const std::lock_guard<std::mutex> _(mutex_);

  if (!is_valid_key(key)) {
    return Status(EINVAL, "invalid key");
  }

  std::string key_path = generate_path(bucket, key);
  // key may contains '/', so if its parent directory does not exists, we create
  // for it.
  int ret = mkdir_p(fs::path(key_path).parent_path().native());
  assert(!ret);
  std::error_code errcode;
  fs::copy(data_file_path, key_path, fs::copy_options::overwrite_existing,
           errcode);
  return Status(errcode.value(), errcode.message());
}

Status LocalObjectStore::get_object_to_file(
    const std::string_view &bucket, const std::string_view &key,
    const std::string_view &output_file_path) {
  const std::lock_guard<std::mutex> _(mutex_);

  if (!is_valid_key(key)) {
    return Status(EINVAL, "invalid key");
  }

  std::string key_path = generate_path(bucket, key);
  std::error_code errcode;
  fs::copy(key_path, output_file_path, fs::copy_options::overwrite_existing,
           errcode);
  return Status(errcode.value(), errcode.message());
}

Status LocalObjectStore::put_object(const std::string_view &bucket,
                                    const std::string_view &key,
                                    const std::string_view &data) {
  const std::lock_guard<std::mutex> _(mutex_);

  if (!is_valid_key(key)) {
    return Status(EINVAL, "invalid key");
  }

  std::string key_path = generate_path(bucket, key);
  // key may contains '/', so if its parent directory does not exists, we create
  // for it.
  int ret = mkdir_p(fs::path(key_path).parent_path().native());
  assert(!ret);
  std::ofstream output_file(key_path, std::ios::binary | std::ios::trunc);
  if (!output_file) {
    return Status(EIO, "Couldn't open file");
  }

  bool fail = !output_file.write(data.data(), data.size());
  output_file.close();
  return fail ? Status(EIO, "write fail") : Status();
}

Status LocalObjectStore::get_object(const std::string_view &bucket,
                                    const std::string_view &key,
                                    std::string &body) {
  const std::lock_guard<std::mutex> _(mutex_);

  if (!is_valid_key(key)) {
    return Status(EINVAL, "invalid key");
  }

  std::string key_path = generate_path(bucket, key);
  std::ifstream input_file(key_path, std::ios::binary);
  if (!input_file) {
    return Status(EIO, "Couldn't open file");
  }

  input_file.seekg(0, std::ios::end);
  std::streamsize fileSize = input_file.tellg();
  input_file.seekg(0, std::ios::beg);

  body.resize(fileSize);
  bool fail = !input_file.read(body.data(), body.size());
  input_file.close();
  return fail ? Status(EIO, "read fail") : Status();
}

Status LocalObjectStore::get_object(const std::string_view &bucket,
                                    const std::string_view &key, size_t off,
                                    size_t len, std::string &body) {
  const std::lock_guard<std::mutex> _(mutex_);

  if (!is_valid_key(key)) {
    return Status(EINVAL, "invalid key");
  }

  std::string key_path = generate_path(bucket, key);
  std::ifstream input_file(key_path, std::ios::binary);
  if (!input_file) {
    return Status(EIO, "Couldn't open file");
  }

  input_file.seekg(0, std::ios::end);
  std::streamsize fileSize = input_file.tellg();
  if (off >= fileSize) {
    return Status(ERANGE, "offset out of range");
  }

  input_file.seekg(off);
  body.resize(len);
  bool fail = !input_file.read(body.data(), len);
  body.resize(input_file.gcount());
  if (input_file.eof() && input_file.gcount() > 0) {
    fail = false;
  }
  input_file.close();
  return fail ? Status(EIO, "read fail") : Status();
}

Status LocalObjectStore::get_object_meta(const std::string_view &bucket,
                                         const std::string_view &key,
                                         ObjectMeta &meta) {
  const std::lock_guard<std::mutex> _(mutex_);

  if (!is_valid_key(key)) {
    return Status(EINVAL, "invalid key");
  }
  fs::path key_path = fs::path(generate_path(bucket, key));

  int ret = get_obj_meta_from_file(key_path, meta);
  if (ret != 0) {
    return Status(ret, "fail to get object meta");
  }

  std::string bucket_path = generate_path(bucket);
  // use lexically_relative() to remove the bucket prefix
  // example: entry: bucket/key_prefix_dir/key -> key_prefix_dir/key
  meta.key = key_path.lexically_relative(bucket_path).c_str();
  return Status();
}

Status LocalObjectStore::list_object(const std::string_view &bucket,
                                     const std::string_view &prefix
                                     [[maybe_unused]],
                                     std::vector<ObjectMeta> &objects) {
  const std::lock_guard<std::mutex> _(mutex_);

  std::string bucket_path = generate_path(bucket);
  objects.clear();
  for (const auto &entry : fs::recursive_directory_iterator(bucket_path)) {
    if (fs::is_directory(entry)) {
      // when we encounter an dir, it should have some child, otherwise there
      // is some error.
      assert(!is_dir_empty(std::string_view(entry.path().c_str())));
    } else if (fs::is_regular_file(entry)) {
      ObjectMeta meta;
      // use lexically_relative() to remove the bucket prefix
      // example: entry: bucket/key_prefix_dir/key -> key_prefix_dir/key
      meta.key = entry.path().lexically_relative(bucket_path).c_str();
      if (prefix.size() > 0 && std::string_view(meta.key).find(prefix) != 0) {
        continue;
      }
      int ret = get_obj_meta_from_file(entry.path(), meta);
      if (ret != 0) {
        return Status(ret, "fail to get object meta");
      }
      objects.push_back(meta);
    }
  }
  return Status();
}

Status LocalObjectStore::delete_object(const std::string_view &bucket,
                                       const std::string_view &key) {
  const std::lock_guard<std::mutex> _(mutex_);

  if (!is_valid_key(key)) {
    return Status(EINVAL, "invalid key");
  }

  const std::string key_path_str = generate_path(bucket, key);
  const std::string bucket_path_str = generate_path(bucket);
  bool key_parent = false;

  fs::path key_path = fs::path(key_path_str);
  while (key_path.native().size() > bucket_path_str.size()) {
    assert(fs::is_directory(key_path) == key_parent);
    int ret = rm_f(key_path.c_str());
    if (ret != 0) {
      if (key_parent) {
        // delete the child file successfully, but failed to delete the parent.
        abort();
      } else {
        return Status(ret, std::generic_category().message(ret));
      }
    }

    key_path = key_path.parent_path();
    key_parent = true;

    // if this entry is the last entry in the parent directory, we need to
    // remove the parent directory in a recursive way.
    if (!is_dir_empty(std::string_view(key_path.c_str()))) {
      break;
    }
  }
  return Status();
}

bool LocalObjectStore::is_valid_key(const std::string_view &key) {
  // key in s3, should be no more than 1024 bytes.
  return key.size() > 0 && key.size() <= 1024;
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
                                        const std::string_view *endpoint
                                        [[maybe_unused]],
                                        bool use_https [[maybe_unused]]) {
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

LocalObjectStore *create_local_objstore(const std::string_view &access_key
                                        [[maybe_unused]],
                                        const std::string_view &secret_key
                                        [[maybe_unused]],
                                        const std::string_view region,
                                        const std::string_view *endpoint,
                                        bool use_https) {
  return create_local_objstore(region, endpoint, use_https);
}

void destroy_local_objstore(LocalObjectStore *local_objstore) {
  if (local_objstore) {
    delete local_objstore;
  }
  // keep the data there.
  return;
}

}  // namespace objstore
