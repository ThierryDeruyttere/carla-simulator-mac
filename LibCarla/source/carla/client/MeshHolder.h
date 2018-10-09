// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Logging.h"
#include "carla/client/Actor.h"

namespace carla {
namespace client {

  class MeshHolder : public Actor {
  public:

    void SetScene(std::string scene) {
      /// @todo should we check if we are already listening?
      log_debug("MeshHolder", GetId(), "type", GetTypeId(), ": generating map");
      GetWorld()->GetClient().SetScene(*this, scene);
    }

  private:

    friend class Client;

      MeshHolder(carla::rpc::Actor actor, SharedPtr<World> world)
      : Actor(actor, std::move(world)) {}

  };

} // namespace client
} // namespace carla
