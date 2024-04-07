#include "objstore.h"

#include "local.h"
#include "s3.h"

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
