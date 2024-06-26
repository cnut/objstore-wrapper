#ifndef MY_OBJSTORE_S3_H_INCLUDED
#define MY_OBJSTORE_S3_H_INCLUDED

#include <string>

#include <aws/s3/S3Client.h>

#include "objstore.h"

namespace objstore {

class S3ObjectStore : public ObjectStore {
 public:
  explicit S3ObjectStore(const std::string_view region,
                         Aws::S3::S3Client &&s3_client)
      : region_(region), s3_client_(s3_client) {}
  virtual ~S3ObjectStore() = default;

  Status create_bucket(const std::string_view &bucket) override;

  Status delete_bucket(const std::string_view &bucket) override;

  Status put_object_from_file(const std::string_view &bucket,
                              const std::string_view &key,
                              const std::string_view &data_file_path) override;
  Status get_object_to_file(const std::string_view &bucket,
                            const std::string_view &key,
                            const std::string_view &output_file_path) override;

  Status put_object(const std::string_view &bucket, const std::string_view &key,
                    const std::string_view &data) override;
  Status get_object(const std::string_view &bucket, const std::string_view &key,
                    std::string &input) override;
  Status get_object(const std::string_view &bucket, const std::string_view &key,
                    size_t off, size_t len, std::string &body) override;
  Status get_object_meta(const std::string_view &bucket,
                         const std::string_view &key,
                         ObjectMeta &meta) override;

  Status list_object(const std::string_view &bucket,
                     const std::string_view &prefix,
                     std::vector<ObjectMeta> &objects) override;

  Status delete_object(const std::string_view &bucket,
                       const std::string_view &key) override;

 private:
  std::string region_;
  Aws::S3::S3Client s3_client_;
};

S3ObjectStore *create_s3_objstore(const std::string_view region,
                                  const std::string_view *endpoint,
                                  bool useHttps = true);

void destroy_s3_objstore(S3ObjectStore *s3_obj_store);

}  // namespace objstore

#endif  // MY_OBJSTORE_S3_H_INCLUDED
