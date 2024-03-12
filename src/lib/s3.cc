#include "s3.h"

#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentials.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/CreateBucketRequest.h>
#include <aws/s3/model/DeleteBucketRequest.h>
#include <aws/s3/model/DeleteObjectRequest.h>
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

Status S3ObjectStore::create_bucket(const std::string_view &bucket) {
  Aws::S3::Model::CreateBucketRequest request;
  request.SetBucket(std::string(bucket));

  if (region_ != "us-east-1") {
    Aws::S3::Model::CreateBucketConfiguration createBucketConfig;
    createBucketConfig.SetLocationConstraint(
        Aws::S3::Model::BucketLocationConstraintMapper::
            GetBucketLocationConstraintForName(region_));
    request.SetCreateBucketConfiguration(createBucketConfig);
  }

  Aws::S3::Model::CreateBucketOutcome outcome =
      s3_client_.CreateBucket(request);
  if (!outcome.IsSuccess()) {
    auto err = outcome.GetError();
    std::cerr << "Error: CreateBucket: " << err.GetExceptionName() << ": "
              << err.GetMessage() << std::endl;
    return Status(static_cast<int>(outcome.GetError().GetResponseCode()),
                  outcome.GetError().GetMessage());
  }

  return Status();
}

Status S3ObjectStore::delete_bucket(const std::string_view &bucket) {
  Aws::S3::Model::DeleteBucketRequest request;
  request.SetBucket(std::string(bucket));

  Aws::S3::Model::DeleteBucketOutcome outcome =
      s3_client_.DeleteBucket(request);

  if (!outcome.IsSuccess()) {
    const Aws::S3::S3Error &err = outcome.GetError();
    std::cerr << "Error: DeleteBucket: " << err.GetExceptionName() << ": "
              << err.GetMessage() << std::endl;
    return Status(static_cast<int>(outcome.GetError().GetResponseCode()),
                  outcome.GetError().GetMessage());
  } else {
    // std::cout << "The bucket was deleted" << std::endl;
  }

  return Status();
}

Status
S3ObjectStore::put_object_from_file(const std::string_view &bucket,
                                    const std::string_view &key,
                                    const std::string_view &data_file_name) {

  Aws::S3::Model::PutObjectRequest request;
  request.SetBucket(Aws::String(bucket));
  request.SetKey(Aws::String(key));

  std::shared_ptr<Aws::IOStream> inputData =
      Aws::MakeShared<Aws::FStream>("IOStreamAllocationTag", data_file_name.data(),
                                    std::ios_base::in | std::ios_base::binary);

  if (!*inputData) {
    std::cerr << "Error unable to read file " << data_file_name << std::endl;
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

Status
S3ObjectStore::get_object_to_file(const std::string_view &bucket,
                                  const std::string_view &key,
                                  const std::string_view &output_file_name) {
  std::string result;
  Status status = get_object(bucket, key, result);

  std::shared_ptr<Aws::IOStream> outputStream = Aws::MakeShared<Aws::FStream>(
      "IOStreamAllocationTag", output_file_name.data(),
      std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
  outputStream->write(result.c_str(), result.length());

  return Status();
}

Status S3ObjectStore::put_object(const std::string_view &bucket,
                                 const std::string_view &key,
                                 const std::string_view &data) {

  Aws::S3::Model::PutObjectRequest request;
  request.SetBucket(Aws::String(bucket));
  // We are using the name of the file as the key for the object in the
  // bucket. However, this is just a string_view and can be set according to
  // your retrieval needs.
  request.SetKey(Aws::String(key));

  const std::shared_ptr<Aws::IOStream> inputData =
      Aws::MakeShared<Aws::StringStream>("SStreamAllocationTag");

  if (!*inputData) {
    std::cerr << "Error to create string stream buf" << std::endl;
    return Status(-1, "Error unable to read file");
  }

  *inputData << data;
  request.SetBody(inputData);

  Aws::S3::Model::PutObjectOutcome outcome = s3_client_.PutObject(request);

  if (!outcome.IsSuccess()) {
    std::cerr << "Error: PutObject: " << outcome.GetError().GetMessage()
              << std::endl;
    return Status(static_cast<int>(outcome.GetError().GetResponseCode()),
                  outcome.GetError().GetMessage());
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

Status S3ObjectStore::delete_object(const std::string_view &bucket,
                                    const std::string_view &key) {
  Aws::S3::Model::DeleteObjectRequest request;
  request.SetBucket(Aws::String(bucket));
  request.SetKey(Aws::String(key));
  Aws::S3::Model::DeleteObjectOutcome outcome =
      s3_client_.DeleteObject(request);

  if (!outcome.IsSuccess()) {
    const Aws::S3::S3Error &err = outcome.GetError();
    std::cerr << "Error: DeleteObject: " << err.GetExceptionName() << ": "
              << err.GetMessage() << std::endl;

    return Status(static_cast<int>(outcome.GetError().GetResponseCode()),
                  outcome.GetError().GetMessage());
  }

  return Status();
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
  return new S3ObjectStore(region, std::move(client));
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

  return new S3ObjectStore(region, std::move(s3_client));
#else
  return nullptr;
#endif
}

void destroy_s3_objstore(S3ObjectStore *s3_objstore) {
  delete s3_objstore;
  return;
}

}; // namespace objstore
