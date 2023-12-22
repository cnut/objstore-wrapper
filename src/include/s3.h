#include <string>

#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>

Aws::Client::ClientConfiguration
CreateClientConf(const std::string_view region,
                 const std::string_view *endpoint, bool useHttps = true);

bool GetObject(const Aws::Client::ClientConfiguration &clientConfig,
               const std::string &fromBucket, const std::string &objectKey,
               const std::string &filepath);

bool PutObject(const Aws::Client::ClientConfiguration &clientConfig,
               const std::string &fromBucket, const std::string &objectKey,
               const std::string &filepath);