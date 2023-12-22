#include <string>

#include <aws/s3/S3Client.h>

class ObjectStore {
public:
  ObjectStore();
  ~ObjectStore();
};

Aws::Client::ClientConfiguration
CreateClientConf(const std::string_view region,
                 const std::string_view *endpoint, bool useHttps = true);

bool GetObject(const Aws::Client::ClientConfiguration &clientConfig,
               const std::string &fromBucket, const std::string_view &objectKey,
               const std::string_view &filepath);

bool PutObject(const Aws::Client::ClientConfiguration &clientConfig,
               const std::string_view &Bucket,
               const std::string_view &objectKey,
               const std::string_view &filepath);