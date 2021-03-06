// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/rpc/ActorAttribute.h"
#include "carla/rpc/MsgPack.h"
#include "carla/rpc/String.h"

#include <vector>

namespace carla {
namespace rpc {

  class ActorDescription {
  public:

    ActorDescription() = default;

    uint32_t uid = 0u;

    std::string id;

    std::vector<ActorAttributeValue> attributes;

    //CUSTOM
    uint32_t class_id = 0u;

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    ActorDescription(const FActorDescription &Description)
      : uid(Description.UId),
        id(FromFString(Description.Id)), class_id(Description.Class_id) {
      attributes.reserve(Description.Variations.Num());
      for (const auto &Item : Description.Variations) {
        attributes.emplace_back(Item.Value);
      }
    }

    operator FActorDescription() const {
      FActorDescription Description;
      Description.UId = uid;
      Description.Id = ToFString(id);
      Description.Class_id = class_id;
      Description.Variations.Reserve(attributes.size());
      for (const auto &item : attributes) {
        Description.Variations.Emplace(ToFString(item.id), item);
      }
      return Description;
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4

    MSGPACK_DEFINE_ARRAY(uid, class_id, id, attributes);
  };

} // namespace rpc
} // namespace carla
