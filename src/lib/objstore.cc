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

#include "objstore.h"

#include "mysys/objstore/local.h"
#include "mysys/objstore/s3.h"

namespace objstore {

ObjectStore *create_object_store(const std::string_view &provider,
                                 const std::string_view region,
                                 const std::string_view *endpoint,
                                 bool use_https) {
  if (provider == "aws") {
    return create_s3_objstore(region, endpoint, use_https);
  } else if (provider == "local") {
    return create_local_objstore(region, endpoint, use_https);
  } else {
    return nullptr;
  }
}

void destroy_object_store(ObjectStore *obj_store) {
  // provide a register mechanism to create/destroy the object store
  delete obj_store;
}

}  // namespace objstore