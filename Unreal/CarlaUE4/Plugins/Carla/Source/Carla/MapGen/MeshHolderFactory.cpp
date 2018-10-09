// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/MapGen/MeshHolderFactory.h"

#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla/MacchinaGenerator.h"

TArray<FActorDefinition> AMeshHolderFactory::GetDefinitions()
{
  FActorDefinition Mesh;
  bool Success = false;
  UActorBlueprintFunctionLibrary::MakeMeshHolderDefinition(
      {TEXT("mesh"), AMacchinaCityMapGenerator::StaticClass()},
      Success,
      Mesh);
  check(Success);
  return {Mesh};
}

FActorSpawnResult AMeshHolderFactory::SpawnActor(
    const FTransform &Transform,
    const FActorDescription &Description)
{
  FActorSpawnParameters Params;
  auto *World = GetWorld();
  if (World == nullptr)
  {
    return {};
  }

  auto *MeshHolder = World->SpawnActorDeferred<AMacchinaCityMapGenerator>(
      Description.Class,
      Transform,
      this,
      nullptr,
      ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
  UE_LOG(LogCarla, Log, TEXT("Created Mesh'"));

  UGameplayStatics::FinishSpawningActor(MeshHolder, Transform);
  return FActorSpawnResult{MeshHolder};
}
