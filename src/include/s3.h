#include <string>

#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>

bool GetObject(const Aws::Client::ClientConfiguration &clientConfig,
               const std::string &fromBucket, const std::string &objectKey,
               const std::string &filepath);

bool PutObject(const Aws::Client::ClientConfiguration &clientConfig,
               const std::string &fromBucket, const std::string &objectKey,
               const std::string &filepath);