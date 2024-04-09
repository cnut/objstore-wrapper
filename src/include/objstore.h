#ifndef OBJSTORE_OBJSTORE_H_INCLUDED
#define OBJSTORE_OBJSTORE_H_INCLUDED

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace objstore {

// interfaces to manipulate object like aws S3.
// this interfaces will shield the differences between different object storages
// provider, such as aws S3, aliyun OSS, MinIO, etc.

class Status {
 public:
  Status() {}
  Status(int error_code, std::string_view error_msg)
      : error_code_(error_code), error_msg_(error_msg) {}
  ~Status() = default;

  bool is_succ() const { return error_code_ == 0; }

  void set_error_code(int error_code) { error_code_ = error_code; }
  int error_code() const { return error_code_; }

  void set_error_msg(std::string_view error_msg) { error_msg_ = error_msg; }
  std::string_view error_message() const { return error_msg_; }

 private:
  int error_code_{0};
  std::string error_msg_;
};

struct ObjectMeta {
  std::string key;
  int64_t last_modified; // timestamp in milliseconds since epoch.
  long long size;        // body size
};

class ObjectStore {
 public:
  virtual ~ObjectStore() = default;

  virtual Status create_bucket(const std::string_view &bucket) = 0;

  virtual Status delete_bucket(const std::string_view &bucket) = 0;

  virtual Status put_object_from_file(
      const std::string_view &bucket, const std::string_view &key,
      const std::string_view &data_file_path) = 0;
  virtual Status get_object_to_file(
      const std::string_view &bucket, const std::string_view &key,
      const std::string_view &output_file_path) = 0;

  virtual Status put_object(const std::string_view &bucket,
                            const std::string_view &key,
                            const std::string_view &data) = 0;
  virtual Status get_object(const std::string_view &bucket,
                            const std::string_view &key, std::string &body) = 0;
  virtual Status get_object(const std::string_view &bucket,
                            const std::string_view &key, size_t off, size_t len,
                            std::string &body) = 0;
  virtual Status get_object_meta(const std::string_view &bucket,
                                 const std::string_view &key,
                                 ObjectMeta &meta) = 0;

  virtual Status list_object(const std::string_view &bucket,
                             const std::string_view &prefix,
                             std::vector<ObjectMeta> &objects) = 0;

  virtual Status delete_object(const std::string_view &bucket,
                               const std::string_view &key) = 0;
};

// create ObjectStore based credentials in credentials dir or environment
// variables.
ObjectStore *create_object_store(const std::string_view &provider,
                                 const std::string_view region,
                                 const std::string_view *endpoint,
                                 bool use_https = true);

void destroy_object_store(ObjectStore *obj_store);

}  // namespace objstore

#endif  // OBJSTORE_OBJSTORE_H_INCLUDED
