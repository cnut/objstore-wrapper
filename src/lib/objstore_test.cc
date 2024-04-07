#include "objstore.h"

#include <gflags/gflags.h>
#include <gtest/gtest.h>

namespace objstore {

DEFINE_string(provider, "file",
              "provider of objstore, only support local or aws");
DEFINE_string(region, "/tmp/",
              "region of the objstore, direcotry for local objstore");
DEFINE_string(endpoint, "",
              "endpoint, which will be ignoreed by local objstore");
DEFINE_bool(
    use_https, false,
    "whether to use https or not, which will be ignored by local objstore");
DEFINE_string(bucket, "test_bucket",
              "bucket, which will be used for this test");

class ObjstoreTest : public testing::Test {
protected:
  ObjstoreTest() = default;

  void SetUp() override {
    // Create a new object store
    std::string_view endpoint = FLAGS_endpoint;
    objstore_ = create_object_store(FLAGS_provider, FLAGS_region,
                                    endpoint.size() == 0 ? nullptr : &endpoint,
                                    FLAGS_use_https);
  }

  void TearDown() override {
    // Destroy the object store
    destroy_object_store(objstore_);
    objstore_ = nullptr;
  }

protected:
  ObjectStore *objstore_{nullptr};
};

TEST_F(ObjstoreTest, PutGetDeleteMeta) {
  std::string_view key = "test_obj_key";
  std::string_view value = "test_obj_value";
  Status st = objstore_->put_object(FLAGS_bucket, key, value);
  ASSERT_EQ(st.error_code(), 0) << "fail to put object " << st.error_message();

  std::string value_out;
  st = objstore_->get_object(FLAGS_bucket, key, value_out);
  ASSERT_EQ(st.error_code(), 0) << "fail to get object " << st.error_message();
  ASSERT_EQ(value_out, value);

  ObjectMeta meta;
  st = objstore_->get_object_meta(FLAGS_bucket, key, meta);
  ASSERT_EQ(st.error_code(), 0)
      << "fail to get object meta " << st.error_message();
  ASSERT_EQ(meta.key, key);
  ASSERT_GT(meta.last_modified, 0);
  ASSERT_GT(meta.size, value.size());

  st = objstore_->delete_object(FLAGS_bucket, key);
  ASSERT_EQ(st.error_code(), 0)
      << "fail to delete object " << st.error_message();
}

TEST_F(ObjstoreTest, List) {
  std::string key_prefix = "test_obj_key_";

  for (int i = 0; i < 100; ++i) {
    std::string kv = key_prefix + std::to_string(i);
    Status st = objstore_->put_object(FLAGS_bucket, kv, kv);
    ASSERT_EQ(st.error_code(), 0)
        << "fail to put object " << st.error_message();
  }

  std::vector<ObjectMeta> objects;
  Status st = objstore_->list_object(FLAGS_bucket, "", objects);
  ASSERT_EQ(st.error_code(), 0) << "fail to list object " << st.error_message();
  ASSERT_EQ(objects.size(), 100);
  for (auto &meta : objects) {
    ASSERT_EQ(meta.key.size(), meta.size);
    ASSERT_GT(meta.last_modified, 0);
  }

  objects.clear();
  st = objstore_->list_object(FLAGS_bucket, key_prefix, objects);
  ASSERT_EQ(st.error_code(), 0) << "fail to list object " << st.error_message();
  ASSERT_EQ(objects.size(), 100);
  for (auto &meta : objects) {
    ASSERT_EQ(meta.key.size(), meta.size);
    ASSERT_GT(meta.last_modified, 0);
  }

  objects.clear();
  st = objstore_->list_object(FLAGS_bucket, "not_existing_prefix", objects);
  ASSERT_EQ(st.error_code(), 0) << "fail to list object " << st.error_message();
  ASSERT_EQ(objects.size(), 0);

  for (int i = 0; i < 100; ++i) {
    std::string kv = key_prefix + std::to_string(i);
    Status st = objstore_->delete_object(FLAGS_bucket, kv);
    ASSERT_EQ(st.error_code(), 0)
        << "fail to delete object " << st.error_message();
  }
}

} // namespace objstore

int main(int argc, char **argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
