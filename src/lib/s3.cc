#include "s3.h"

#include <aws/core/Aws.h>
#include <aws/core/auth/awscredentials.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

namespace objstore {

namespace { // anonymous namespace

class S3ApiGlobalOption {
public:
  S3ApiGlobalOption() {
    Aws::SDKOptions options;
    Aws::InitAPI(options);
  }
  ~S3ApiGlobalOption() {
    Aws::SDKOptions options;
    Aws::ShutdownAPI(options);
  }
};

S3ApiGlobalOption g_aws_api_option_initializor;

}; // namespace

Status S3ObjectStore::put_object(const std::string_view &bucket,
                                 const std::string_view &key,
                                 const std::string_view &file_name) {

  Aws::S3::Model::PutObjectRequest request;
  request.SetBucket(Aws::String(bucket));
  // We are using the name of the file as the key for the object in the
  // bucket. However, this is just a string_view and can be set according to
  // your retrieval needs.
  request.SetKey(Aws::String(key));

  std::shared_ptr<Aws::IOStream> inputData =
      Aws::MakeShared<Aws::FStream>("SampleAllocationTag", file_name,
                                    std::ios_base::in | std::ios_base::binary);

  if (!*inputData) {
    std::cerr << "Error unable to read file " << file_name << std::endl;
    return Status(-1, "Error unable to read file");
  }

  request.SetBody(inputData);

  Aws::S3::Model::PutObjectOutcome outcome = s3_client_.PutObject(request);

  if (!outcome.IsSuccess()) {
    std::cerr << "Error: PutObject: " << outcome.GetError().GetMessage()
              << std::endl;
    return Status(static_cast<int>(outcome.GetError().GetResponseCode()),
                  outcome.GetError().GetMessage());
  } else {
    // std::cout << "Added object '" << fileName << "' to bucket '" <<
    // bucketName
    //           << "'.";
  }

  return Status();
}

Status S3ObjectStore::get_object(const std::string_view &bucket,
                                 const std::string_view &key,
                                 std::string &body) {
  Aws::S3::Model::GetObjectRequest request;
  request.SetBucket(Aws::String(bucket));
  request.SetKey(Aws::String(key));
  Aws::S3::Model::GetObjectOutcome outcome = s3_client_.GetObject(request);

  if (!outcome.IsSuccess()) {
    const Aws::S3::S3Error &err = outcome.GetError();
    std::cerr << "Error: GetObject: " << err.GetExceptionName() << ": "
              << err.GetMessage() << std::endl;

    return Status(static_cast<int>(outcome.GetError().GetResponseCode()),
                  outcome.GetError().GetMessage());
  } else {
    // std::cout << "Successfully retrieved '" << objectKey << "' from '"
    //         << fromBucket << "'." << std::endl;

    std::ostringstream oss;
    oss << outcome.GetResult().GetBody().rdbuf();
    body = oss.str();
  }

  return Status();
}

Status S3ObjectStore::list_object(const std::string_view &bucket,
                                  const std::string_view &key,
                                  std::vector<std::string> objects) {
  return Status(-1, "not implemented");
}

S3ObjectStore *create_s3_objstore(const std::string_view region,
                                  const std::string_view *endpoint,
                                  bool use_https) {
  Aws::Client::ClientConfiguration clientConfig;
  clientConfig.region = region;
  if (endpoint != nullptr) {
    clientConfig.endpointOverride = *endpoint;
  }
  clientConfig.scheme =
      use_https ? Aws::Http::Scheme::HTTPS : Aws::Http::Scheme::HTTP;
  Aws::S3::S3Client client(clientConfig);
  return new S3ObjectStore(std::move(client));
}

S3ObjectStore *create_s3_objstore(const std::string_view &access_key,
                                  const std::string_view &secret_key,
                                  const std::string_view region,
                                  const std::string_view *endpoint,
                                  bool use_https) {
#if 0
  Aws::Auth::AWSCredentials credentials(std::string(access_key),
                                        std::string(secret_key));

  Aws::Client::ClientConfiguration clientConfig;
  clientConfig.region = region;
  if (endpoint != nullptr) {
    clientConfig.endpointOverride = *endpoint;
  }
  clientConfig.scheme =
      use_https ? Aws::Http::Scheme::HTTPS : Aws::Http::Scheme::HTTP;

  Aws::S3::S3Client s3_client(credentials, clientConfig);

  return new S3ObjectStore(std::move(s3_client));
#else
  return nullptr;
#endif
}

ObjectStore *create_object_store(const std::string_view &provider,
                                 const std::string_view region,
                                 const std::string_view *endpoint,
                                 bool use_https) {
  if (provider != "aws") {
    return nullptr;
  }

  return create_s3_objstore(region, endpoint, use_https);
}

ObjectStore *create_object_store(const std::string_view &provider,
                                 const std::string_view &access_key,
                                 const std::string_view &secret_key,
                                 const std::string_view region,
                                 const std::string_view *endpoint,
                                 bool use_https) {
  if (provider != "aws") {
    return nullptr;
  }

  return create_s3_objstore(access_key, secret_key, region, endpoint,
                            use_https);
}

}; // namespace objstore