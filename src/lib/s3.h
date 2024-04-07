/*
   Copyright (c) 2024, ApeCloud Inc Holding Limited.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

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

  Status list_object(const std::string_view &bucket,
                     const std::string_view &key,
                     std::vector<std::string> &objects) override;

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
