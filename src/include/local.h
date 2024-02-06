#include <string>

#include "obj_store.h"

namespace objstore {

class LocalObjectStore : public ObjectStore {
public:
  explicit LocalObjectStore(const std::string_view basepath)
      : basepath_(basepath){};
  virtual ~LocalObjectStore() = default;

  Status create_bucket(const std::string_view &bucket) override;

  Status delete_bucket(const std::string_view &bucket) override;

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
  bool is_valid_key(const std::string_view &key);
  std::string generate_path(const std::string_view &bucket);
  std::string generate_path(const std::string_view &bucket,
                            const std::string_view &key);

private:
  std::string basepath_;
};

LocalObjectStore *create_local_objstore(const std::string_view region,
                                        const std::string_view *endpoint,
                                        bool useHttps = true);

LocalObjectStore *create_local_objstore(const std::string_view &access_key,
                                        const std::string_view &secret_key,
                                        const std::string_view region,
                                        const std::string_view *endpoint,
                                        bool useHttps = true);

void destroy_local_objstore(LocalObjectStore *local_obj_store);

}; // namespace objstore
