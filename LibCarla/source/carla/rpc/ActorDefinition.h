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

  class ActorDefinition {
  public:

    ActorDefinition() = default;

    uint32_t uid = 0u;

    uint32_t class_id = 0u;

    std::string id;

    std::string tags;

    std::vector<ActorAttribute> attributes;

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    ActorDefinition(const FActorDefinition &Definition)
      : uid(Definition.UId),
        class_id(Definition.Class_id),
        id(FromFString(Definition.Id)),
        tags(FromFString(Definition.Tags)){
      attributes.reserve(Definition.Variations.Num() + Definition.Attributes.Num());
      for (const auto &Item : Definition.Variations) {
        attributes.push_back(Item);
      }
      for (const auto &Item : Definition.Attributes) {
        attributes.push_back(Item);
      }
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4

    MSGPACK_DEFINE_ARRAY(uid, class_id, id, tags, attributes);
  };

} // namespace rpc
} // namespace carla
