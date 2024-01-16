#include <string>

#include <aws/s3/S3Client.h>

#include "obj_store.h"

namespace objstore {

class S3ObjectStore : public ObjectStore {
public:
  explicit S3ObjectStore(const std::string_view region,
                         Aws::S3::S3Client &&s3_client)
      : region_(region), s3_client_(s3_client){};
  virtual ~S3ObjectStore() = default;

  Status create_bucket(const std::string_view &bucket);

  Status delete_bucket(const std::string_view &bucket);

  Status put_object_from_file(const std::string_view &bucket,
                              const std::string_view &key,
                              const std::string_view &data_file_name) override;
  Status get_object_to_file(const std::string_view &bucket,
                            const std::string_view &key,
                            const std::string_view &output_file_name) override;

  Status put_object(const std::string_view &bucket, const std::string_view &key,
                    const std::string_view &data) override;
  Status get_object(const std::string_view &bucket, const std::string_view &key,
                    std::string &input) override;

  Status list_object(const std::string_view &bucket,
                     const std::string_view &key,
                     std::vector<std::string> objects) override;

  Status delete_object(const std::string_view &bucket,
                       const std::string_view &key) override;

private:
  std::string region_;
  Aws::S3::S3Client s3_client_;
};

S3ObjectStore *create_s3_objstore(const std::string_view region,
                                  const std::string_view *endpoint,
                                  bool useHttps = true);

S3ObjectStore *create_s3_objstore(const std::string_view &access_key,
                                  const std::string_view &secret_key,
                                  const std::string_view region,
                                  const std::string_view *endpoint,
                                  bool useHttps = true);

}; // namespace objstore