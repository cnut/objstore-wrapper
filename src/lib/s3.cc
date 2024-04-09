#include "s3.h"

#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentials.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/CreateBucketRequest.h>
#include <aws/s3/model/DeleteBucketRequest.h>
#include <aws/s3/model/DeleteObjectRequest.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/HeadObjectRequest.h>
#include <aws/s3/model/ListObjectsRequest.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <errno.h>
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

}  // namespace

Status S3ObjectStore::create_bucket(const std::string_view &bucket) {
  Aws::S3::Model::CreateBucketRequest request;
  request.SetBucket(std::string(bucket));

  Aws::S3::Model::CreateBucketConfiguration createBucketConfig;
  createBucketConfig.SetLocationConstraint(
      Aws::S3::Model::BucketLocationConstraintMapper::
          GetBucketLocationConstraintForName(region_));
  request.SetCreateBucketConfiguration(createBucketConfig);

  Aws::S3::Model::CreateBucketOutcome outcome =
      s3_client_.CreateBucket(request);
  if (!outcome.IsSuccess()) {
    const Aws::S3::S3Error &err = outcome.GetError();
    return Status(static_cast<int>(err.GetResponseCode()), err.GetMessage());
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
    return Status(static_cast<int>(err.GetResponseCode()), err.GetMessage());
  }

  return Status();
}

Status S3ObjectStore::put_object_from_file(
    const std::string_view &bucket, const std::string_view &key,
    const std::string_view &data_file_path) {
  Aws::S3::Model::PutObjectRequest request;
  request.SetBucket(Aws::String(bucket));
  request.SetKey(Aws::String(key));

  std::shared_ptr<Aws::IOStream> input_data = Aws::MakeShared<Aws::FStream>(
      "IOStreamAllocationTag", data_file_path.data(),
      std::ios_base::in | std::ios_base::binary);

  if (!*input_data) {
    return Status(EIO, "Error unable to open input file");
  }

  request.SetBody(input_data);

  Aws::S3::Model::PutObjectOutcome outcome = s3_client_.PutObject(request);

  if (!outcome.IsSuccess()) {
    const Aws::S3::S3Error &err = outcome.GetError();
    return Status(static_cast<int>(err.GetResponseCode()), err.GetMessage());
  }

  return Status();
}

Status S3ObjectStore::get_object_to_file(
    const std::string_view &bucket, const std::string_view &key,
    const std::string_view &output_file_path) {
  std::string result;
  Status status = get_object(bucket, key, result);

  std::shared_ptr<Aws::IOStream> output_stream = Aws::MakeShared<Aws::FStream>(
      "IOStreamAllocationTag", output_file_path.data(),
      std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
  if (!*output_stream) {
    return Status(EIO, "Error unable to open output file");
  }

  bool fail =
      output_stream->write(result.c_str(), result.length()).flush().fail();
  if (fail) {
    return Status(EIO, "unable to write key's value into file");
  }

  return Status();
}

Status S3ObjectStore::put_object(const std::string_view &bucket,
                                 const std::string_view &key,
                                 const std::string_view &data) {

  Aws::S3::Model::PutObjectRequest request;
  request.SetBucket(Aws::String(bucket));
  request.SetKey(Aws::String(key));

  const std::shared_ptr<Aws::IOStream> data_stream =
      Aws::MakeShared<Aws::StringStream>("SStreamAllocationTag");
  if (!*data_stream) {
    return Status(EIO, "unable to create data stream to hold input data");
  }

  *data_stream << data;
  if (!*data_stream) {
    return Status(EIO, "unable to write data into data stream");
  }

  request.SetBody(data_stream);

  Aws::S3::Model::PutObjectOutcome outcome = s3_client_.PutObject(request);
  if (!outcome.IsSuccess()) {
    const Aws::S3::S3Error &err = outcome.GetError();
    return Status(static_cast<int>(err.GetResponseCode()), err.GetMessage());
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
    return Status(static_cast<int>(err.GetResponseCode()), err.GetMessage());
  }

  std::ostringstream oss;
  oss << outcome.GetResult().GetBody().rdbuf();
  if (!oss) {
    return Status(EIO, "unable to read data from response stream");
  }

  body = oss.str();

  return Status();
}

Status S3ObjectStore::get_object(const std::string_view &bucket,
                                 const std::string_view &key, size_t off,
                                 size_t len, std::string &body) {
  Aws::S3::Model::GetObjectRequest request;
  request.SetBucket(Aws::String(bucket));
  request.SetKey(Aws::String(key));
  std::string byte_range =
      "bytes=" + std::to_string(off) + "-" + std::to_string(off + len - 1);
  request.SetRange(byte_range);
  Aws::S3::Model::GetObjectOutcome outcome = s3_client_.GetObject(request);

  if (!outcome.IsSuccess()) {
    const Aws::S3::S3Error &err = outcome.GetError();
    return Status(static_cast<int>(err.GetResponseCode()), err.GetMessage());
  }

  std::ostringstream oss;
  oss << outcome.GetResult().GetBody().rdbuf();
  if (!oss) {
    return Status(EIO, "unable to read data from response stream");
  }

  body = oss.str();

  return Status();
}

Status S3ObjectStore::get_object_meta(const std::string_view &bucket,
                                      const std::string_view &key,
                                      ObjectMeta &meta) {
  Aws::S3::Model::HeadObjectRequest request;
  request.SetBucket(Aws::String(bucket));
  request.SetKey(Aws::String(key));
  Aws::S3::Model::HeadObjectOutcome outcome = s3_client_.HeadObject(request);

  if (!outcome.IsSuccess()) {
    const Aws::S3::S3Error &err = outcome.GetError();
    return Status(static_cast<int>(err.GetResponseCode()), err.GetMessage());
  }

  meta.key = key;
  meta.last_modified = outcome.GetResult().GetLastModified().Millis();
  meta.size = outcome.GetResult().GetContentLength();

  return Status();
}

Status S3ObjectStore::list_object(const std::string_view &bucket,
                                  const std::string_view &prefix,
                                  std::vector<ObjectMeta> &objects) {
  Aws::S3::Model::ListObjectsRequest request;
  request.SetBucket(Aws::String(bucket));
  request.SetPrefix(Aws::String(prefix));
  Aws::S3::Model::ListObjectsOutcome outcome = s3_client_.ListObjects(request);

  if (!outcome.IsSuccess()) {
    const Aws::S3::S3Error &err = outcome.GetError();
    return Status(static_cast<int>(err.GetResponseCode()), err.GetMessage());
  }

  for (auto obj : outcome.GetResult().GetContents()) {
    ObjectMeta meta;
    meta.key = obj.GetKey();
    meta.last_modified = obj.GetLastModified().Millis();
    meta.size = obj.GetSize();
    objects.push_back(meta);
  }

  return Status();
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
    return Status(static_cast<int>(err.GetResponseCode()), err.GetMessage());
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

void destroy_s3_objstore(S3ObjectStore *s3_objstore) {
  if (s3_objstore) {
    delete s3_objstore;
  }
  return;
}

}  // namespace objstore
