//
//  MacchinaMapGenerator.cpp
//  CarlaUE4_Index
//
//  Created by Thierry Deruyttere on 26/09/18.
//  Copyright © 2018 Epic Games, Inc. All rights reserved.
//


// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "MacchinaGenerator.h"
#include "MapGen/GraphGenerator.h"
#include "MapGen/RoadMap.h"
#include "Game/Tagger.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/World.h"
#include "Paths.h"
#include "Async.h"
#include <vector>
#include "json2.h"

#include <algorithm>
#include <unordered_set>


using json = nlohmann::json;

#ifdef CARLA_ROAD_GENERATOR_EXTRA_LOG
#include <sstream>
#endif // CARLA_ROAD_GENERATOR_EXTRA_LOG

// =============================================================================
// -- Constructor and destructor -----------------------------------------------
// =============================================================================

AMacchinaCityMapGenerator::AMacchinaCityMapGenerator(const FObjectInitializer& ObjectInitializer)
        : Super(ObjectInitializer)
{
    RoadMap = ObjectInitializer.CreateDefaultSubobject<URoadMap>(this, TEXT("RoadMap"));
    LoadMeshes();
}

AMacchinaCityMapGenerator::~AMacchinaCityMapGenerator() {}

// =============================================================================
// -- Overriden from UObject ---------------------------------------------------
// =============================================================================

void AMacchinaCityMapGenerator::PreSave(const ITargetPlatform *TargetPlatform)
{
#if WITH_EDITOR
    if (bGenerateRoadMapOnSave) {
        // Generate road map only if we are not cooking.
        FCoreUObjectDelegates::OnObjectSaved.Broadcast(this);
        if (!GIsCookerLoadingPackage) {
            check(RoadMap != nullptr);
            GenerateRoadMap();
        }
    }
#endif // WITH_EDITOR

    Super::PreSave(TargetPlatform);
}

// =============================================================================
// -- Overriden from ACityMapMeshHolder ----------------------------------------
// =============================================================================

void AMacchinaCityMapGenerator::UpdateMap()
{
    //UpdateSeeds();
    //GenerateGraph();
    //GenerateMacchina();
    /*
    if (bGenerateRoads) {
        GenerateRoads();
    }
    if (bTriggerRoadMapGeneration) {
        bTriggerRoadMapGeneration = false;
        GenerateRoadMap();
    }*/
}

// =============================================================================
// -- Map construction and update related methods ------------------------------
// =============================================================================

void AMacchinaCityMapGenerator::UpdateSeeds()
{
    if (!bUseFixedSeed) {
        FRandomStream randomStream;
        randomStream.GenerateNewSeed();
        Seed = randomStream.GetCurrentSeed();
    }
}

void AMacchinaCityMapGenerator::GenerateGraph()
{
    if ((MapSizeX < 5u) || (MapSizeY < 5u)) {
        MapSizeX = 5u;
        MapSizeY = 5u;
        UE_LOG(LogCarla, Warning, TEXT("Map size changed, was too small"));
    }
#ifdef CARLA_ROAD_GENERATOR_EXTRA_LOG
    // Delete the dcel before the new one is created so indices are restored.
    Dcel.Reset(nullptr);
#endif // CARLA_ROAD_GENERATOR_EXTRA_LOG
    Dcel = MapGen::GraphGenerator::Generate(MapSizeX, MapSizeY, Seed);
    UE_LOG(LogCarla, Log,
           TEXT("Generated DCEL with: { %d vertices, %d half-edges, %d faces }"),
           Dcel->CountNodes(),
           Dcel->CountHalfEdges(),
           Dcel->CountFaces());
    DcelParser = MakeUnique<MapGen::GraphParser>(*Dcel);
#ifdef CARLA_ROAD_GENERATOR_EXTRA_LOG
    { // print the results of the parser.
        std::wstringstream sout;
        sout << "\nGenerated " << DcelParser->CityAreaCount() << " city areas: ";
        for (auto i = 0u; i < DcelParser->CityAreaCount(); ++i) {
            sout << "{ ";
            auto &cityArea = DcelParser->GetCityAreaAt(i);
            for (size_t j = 0u; j < cityArea.NodeCount(); ++j) {
                sout << cityArea.GetNodeAt(j) << " ";
            }
            sout << "} ";
        }
        sout << "\nGenerated " << DcelParser->RoadSegmentCount() << " road segments: ";
        for (auto i = 0u; i < DcelParser->RoadSegmentCount(); ++i) {
            sout << "{ ";
            auto &roadSegment = DcelParser->GetRoadSegmentAt(i);
            for (size_t j = 0u; j < roadSegment.Size(); ++j) {
                sout << roadSegment[j] << " ";
            }
            sout << "} ";
        }
        UE_LOG(LogCarla, Log, TEXT("\n%s"), sout.str().c_str());
    }
#endif // CARLA_ROAD_GENERATOR_EXTRA_LOG
}

void AMacchinaCityMapGenerator::LoadMeshes() {
#define SET_STATIC_MESH(Tag, Folder, FileName) \
{ \
static const ConstructorHelpers::FObjectFinder<UStaticMesh> MeshObj(TEXT("StaticMesh'" Folder "/" FileName "." FileName "'")); \
SetStaticMesh(ECityMapMeshTag:: Tag, MeshObj.Object); \
}

#define PREFIX_FOLDER "/Game/Carla/Static/"

    SET_STATIC_MESH(RoadTwoLanes_LaneLeft,           PREFIX_FOLDER "Road",        "St_Road_TileRoad_RoadL");
    SET_STATIC_MESH(RoadTwoLanes_LaneRight,          PREFIX_FOLDER "Road",        "St_Road_TileRoad_RoadR");
    SET_STATIC_MESH(RoadTwoLanes_SidewalkLeft,       PREFIX_FOLDER "SideWalk",    "St_Road_TileRoad_SidewalkL");
    SET_STATIC_MESH(RoadTwoLanes_SidewalkRight,      PREFIX_FOLDER "SideWalk",    "St_Road_TileRoad_SidewalkR");
    SET_STATIC_MESH(RoadTwoLanes_LaneMarkingSolid,   PREFIX_FOLDER "RoadLines",   "St_Road_TileRoad_LaneMarkingSolid");
    SET_STATIC_MESH(RoadTwoLanes_LaneMarkingBroken,  PREFIX_FOLDER "RoadLines",   "St_Road_TileRoad_LaneMarkingBroken");

    SET_STATIC_MESH(Road90DegTurn_Lane0,             PREFIX_FOLDER "Road",        "St_Road_Curve_Road0");
    SET_STATIC_MESH(Road90DegTurn_Lane1,             PREFIX_FOLDER "Road",        "St_Road_Curve_Road1");
    SET_STATIC_MESH(Road90DegTurn_Lane2,             PREFIX_FOLDER "Road",        "St_Road_Curve_Road2");
    SET_STATIC_MESH(Road90DegTurn_Lane3,             PREFIX_FOLDER "Road",        "St_Road_Curve_Road3");
    SET_STATIC_MESH(Road90DegTurn_Lane4,             PREFIX_FOLDER "Road",        "St_Road_Curve_Road4");
    SET_STATIC_MESH(Road90DegTurn_Lane5,             PREFIX_FOLDER "Road",        "St_Road_Curve_Road5");
    SET_STATIC_MESH(Road90DegTurn_Lane6,             PREFIX_FOLDER "Road",        "St_Road_Curve_Road6");
    SET_STATIC_MESH(Road90DegTurn_Lane7,             PREFIX_FOLDER "Road",        "St_Road_Curve_Road7");
    SET_STATIC_MESH(Road90DegTurn_Lane8,             PREFIX_FOLDER "Road",        "St_Road_Curve_Road8");
    SET_STATIC_MESH(Road90DegTurn_Lane9,             PREFIX_FOLDER "Road",        "St_Road_Curve_Road9");
    SET_STATIC_MESH(Road90DegTurn_Sidewalk0,         PREFIX_FOLDER "SideWalk",    "St_Road_Curve_Sidewalk1");
    SET_STATIC_MESH(Road90DegTurn_Sidewalk1,         PREFIX_FOLDER "SideWalk",    "St_Road_Curve_Sidewalk2");
    SET_STATIC_MESH(Road90DegTurn_Sidewalk2,         PREFIX_FOLDER "SideWalk",    "St_Road_Curve_Sidewalk3");
    SET_STATIC_MESH(Road90DegTurn_Sidewalk3,         PREFIX_FOLDER "SideWalk",    "St_Road_Curve_Sidewalk4");
    SET_STATIC_MESH(Road90DegTurn_LaneMarking,       PREFIX_FOLDER "RoadLines",   "St_Road_Curve_LaneMarking");

    SET_STATIC_MESH(RoadTIntersection_Lane0,         PREFIX_FOLDER "Road",        "St_Road_TCross_Road0");
    SET_STATIC_MESH(RoadTIntersection_Lane1,         PREFIX_FOLDER "Road",        "St_Road_TCross_Road1");
    SET_STATIC_MESH(RoadTIntersection_Lane2,         PREFIX_FOLDER "Road",        "St_Road_TCross_Road2");
    SET_STATIC_MESH(RoadTIntersection_Lane3,         PREFIX_FOLDER "Road",        "St_Road_TCross_Road3");
    SET_STATIC_MESH(RoadTIntersection_Lane4,         PREFIX_FOLDER "Road",        "St_Road_TCross_Road4");
    SET_STATIC_MESH(RoadTIntersection_Lane5,         PREFIX_FOLDER "Road",        "St_Road_TCross_Road5");
    SET_STATIC_MESH(RoadTIntersection_Lane6,         PREFIX_FOLDER "Road",        "St_Road_TCross_Road6");
    SET_STATIC_MESH(RoadTIntersection_Lane7,         PREFIX_FOLDER "Road",        "St_Road_TCross_Road7");
    SET_STATIC_MESH(RoadTIntersection_Lane8,         PREFIX_FOLDER "Road",        "St_Road_TCross_Road8");
    SET_STATIC_MESH(RoadTIntersection_Lane9,         PREFIX_FOLDER "Road",        "St_Road_TCross_Road9");
    SET_STATIC_MESH(RoadTIntersection_Sidewalk0,     PREFIX_FOLDER "SideWalk",    "St_Road_TCross_Sidewalk1");
    SET_STATIC_MESH(RoadTIntersection_Sidewalk1,     PREFIX_FOLDER "SideWalk",    "St_Road_TCross_Sidewalk2");
    SET_STATIC_MESH(RoadTIntersection_Sidewalk2,     PREFIX_FOLDER "SideWalk",    "St_Road_TCross_Sidewalk3");
    SET_STATIC_MESH(RoadTIntersection_Sidewalk3,     PREFIX_FOLDER "SideWalk",    "St_Road_TCross_Sidewalk4");
    SET_STATIC_MESH(RoadTIntersection_LaneMarking,   PREFIX_FOLDER "RoadLines",   "St_Road_TCross_LaneMarking");

    SET_STATIC_MESH(RoadXIntersection_Lane0,         PREFIX_FOLDER "Road",        "St_Road_XCross_Road0");
    SET_STATIC_MESH(RoadXIntersection_Lane1,         PREFIX_FOLDER "Road",        "St_Road_XCross_Road1");
    SET_STATIC_MESH(RoadXIntersection_Lane2,         PREFIX_FOLDER "Road",        "St_Road_XCross_Road2");
    SET_STATIC_MESH(RoadXIntersection_Lane3,         PREFIX_FOLDER "Road",        "St_Road_XCross_Road3");
    SET_STATIC_MESH(RoadXIntersection_Lane4,         PREFIX_FOLDER "Road",        "St_Road_XCross_Road4");
    SET_STATIC_MESH(RoadXIntersection_Lane5,         PREFIX_FOLDER "Road",        "St_Road_XCross_Road5");
    SET_STATIC_MESH(RoadXIntersection_Lane6,         PREFIX_FOLDER "Road",        "St_Road_XCross_Road6");
    SET_STATIC_MESH(RoadXIntersection_Lane7,         PREFIX_FOLDER "Road",        "St_Road_XCross_Road7");
    SET_STATIC_MESH(RoadXIntersection_Lane8,         PREFIX_FOLDER "Road",        "St_Road_XCross_Road8");
    SET_STATIC_MESH(RoadXIntersection_Lane9,         PREFIX_FOLDER "Road",        "St_Road_XCross_Road9");
    SET_STATIC_MESH(RoadXIntersection_Sidewalk0,     PREFIX_FOLDER "SideWalk",    "St_Road_XCross_Sidewalk1");
    SET_STATIC_MESH(RoadXIntersection_Sidewalk1,     PREFIX_FOLDER "SideWalk",    "St_Road_XCross_Sidewalk2");
    SET_STATIC_MESH(RoadXIntersection_Sidewalk2,     PREFIX_FOLDER "SideWalk",    "St_Road_XCross_Sidewalk3");
    SET_STATIC_MESH(RoadXIntersection_Sidewalk3,     PREFIX_FOLDER "SideWalk",    "St_Road_XCross_Sidewalk4");
    SET_STATIC_MESH(RoadXIntersection_LaneMarking,   PREFIX_FOLDER "RoadLines",   "St_Road_XCross_LaneMarking");

    /*
     * Objects
     */
    SET_STATIC_MESH(House_AmerSuburb002_N2,   PREFIX_FOLDER "Buildings/Bl_House_AmerSuburb002_N2",   "Bl_House_AmerSuburb002_N2");
    SET_STATIC_MESH(House_AmerSuburb003_N2,   PREFIX_FOLDER "Buildings/Bl_House_AmerSuburb003_N2",   "Bl_House_AmerSuburb003");
    SET_STATIC_MESH(House_AmerSuburb004_N2,   PREFIX_FOLDER "Buildings/Bl_House_AmerSuburb004_N2",   "Bl_House_AmerSuburb004_N2");
    SET_STATIC_MESH(House_AmerSuburb005_N5,   PREFIX_FOLDER "Buildings/Bl_House_AmerSuburb005_N5",   "Bl_House_AmerSuburb005_N5");
    SET_STATIC_MESH(House_AmerSuburb006_N2,   PREFIX_FOLDER "Buildings/Bl_House_AmerSuburb006_N2",   "Bl_House_AmerSuburb006_N2");

    SET_STATIC_MESH(Props_Bench,              PREFIX_FOLDER "Props/Bench",   "prop_bench");
    SET_STATIC_MESH(Props_Trashcan,           PREFIX_FOLDER "Props/MetallicTrashCan",   "prop_Trashcan");
    SET_STATIC_MESH(Props_BusStop,            PREFIX_FOLDER "Props/BusStop",   "prop_bus_stop");
    SET_STATIC_MESH(Props_FireHydrant,        PREFIX_FOLDER "Pole/FireHydrant",   "prop__firehydrant");

    /*
     * Vehicles
     */
    SET_STATIC_MESH(Vehicle_Toyota_prius,            PREFIX_FOLDER "Vehicles/4Wheeled/Toyota_Prius",   "Vh_Car_ToyotaPrius_NOrig");
    SET_STATIC_MESH(Vehicle_mini,                    PREFIX_FOLDER "Vehicles/4Wheeled/MIni",   "Vh_Car_MiniCooperS");


#undef PREFIX_FOLDER
#undef SET_STATIC_MESH
}

void AMacchinaCityMapGenerator::GenerateScene(const std::string& scene) {

    AsyncTask(ENamedThreads::GameThread, [this, scene]() {
        /*FString JsonRaw = FString(scene.c_str());
        TSharedPtr<FJsonObject> JsonParsed;
        TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(JsonRaw);*/
        json JsonRaw = json::parse(scene);
        json map = JsonRaw["map"].get<std::vector<json>>();
        for(auto &item : map){
            int obj = item["obj"].get<int>();
            if(obj < CityMapMeshTag::GetNumberOfTags()){
                float x = item["x"].get<float>();
                float y = item["y"].get<float>();
                float angle = item["angle"].get<float>();

                ECityMapMeshTag tag = static_cast<ECityMapMeshTag>(obj);
                AddInstance(tag, x, y, angle);
            }
        }
        //GenerateRoadMap();
        // code to execute on game thread here
        //GenerateMacchina();
        //UE_LOG(LogCarla, Log, TEXT("%s"), *FString(map.c_str()));
    });
    //GenerateMacchina();
    UE_LOG(LogCarla, Log,
           TEXT("Generated macchina map"));
    //UE_LOG(LogCarla, Info, TEXT("Finished creating map"));

}

void AMacchinaCityMapGenerator::GenerateMacchina(){
    //check(Dcel != nullptr);

    auto x = 0;
    auto y = 0;
    auto end = 2;

    // First road
    AddInstance(ECityMapMeshTag::RoadTwoLanes_LaneLeft,          x, y, HALF_PI);
    AddInstance(ECityMapMeshTag::RoadTwoLanes_SidewalkLeft,      x, y, HALF_PI);
    AddInstance(ECityMapMeshTag::RoadTwoLanes_SidewalkRight,     -400, y, HALF_PI);

    // second road
    y = 810.0;
    AddInstance(ECityMapMeshTag::RoadTwoLanes_LaneLeft,          x, y, HALF_PI);
    AddInstance(ECityMapMeshTag::RoadTwoLanes_SidewalkLeft,      x, y, HALF_PI);
    AddInstance(ECityMapMeshTag::RoadTwoLanes_SidewalkRight,     -400, y, HALF_PI);

    // Horizontal
    y = 1202.0;
    AddInstance(ECityMapMeshTag::RoadTwoLanes_LaneLeft,          -200, y);
    AddInstance(ECityMapMeshTag::RoadTwoLanes_LaneLeft,          610, y);
    AddInstance(ECityMapMeshTag::RoadTwoLanes_LaneLeft,          -1010, y);

    y = 1623.0;
    AddInstance(ECityMapMeshTag::RoadTwoLanes_SidewalkRight,     430, y);
    AddInstance(ECityMapMeshTag::RoadTwoLanes_SidewalkRight,     -810, y);

    // second road
    y = 2031.0;
    AddInstance(ECityMapMeshTag::RoadTwoLanes_LaneLeft,          x, y, HALF_PI);
    AddInstance(ECityMapMeshTag::RoadTwoLanes_SidewalkLeft,      x, y, HALF_PI);
    AddInstance(ECityMapMeshTag::RoadTwoLanes_SidewalkRight,     -410, y, HALF_PI);


    y = 1213.0;
    AddInstance(ECityMapMeshTag::RoadTwoLanes_SidewalkLeft,      400, y, 0);
    AddInstance(ECityMapMeshTag::RoadTwoLanes_SidewalkRight,     -830, y, PI);


    // Road sign
    //AddInstance(ECityMapMeshTag::,      210, 920, 0);


}

void AMacchinaCityMapGenerator::GenerateRoads()
{
    check(Dcel != nullptr);
    using Graph = MapGen::DoublyConnectedEdgeList;
    const Graph &graph = *Dcel;

    const uint32 margin = CityMapMeshTag::GetRoadIntersectionSize() / 2u;

    FHalfEdgeCounter HalfEdgeCounter;

    // For each edge add road segment.
    for (auto &edge : graph.GetHalfEdges()) {
        if (HalfEdgeCounter.Insert(edge)) {
            auto source = Graph::GetSource(edge).GetPosition();
            auto target = Graph::GetTarget(edge).GetPosition();

            if (source.x == target.x) {
                // vertical
                auto y = 1u + margin + std::min(source.y, target.y);
                auto end = std::max(source.y, target.y) - margin;
                for (; y < end; ++y) {
                    AddInstance(ECityMapMeshTag::RoadTwoLanes_LaneLeft,          source.x, y, HALF_PI);
                    AddInstance(ECityMapMeshTag::RoadTwoLanes_LaneRight,         source.x, y, HALF_PI);
                    AddInstance(ECityMapMeshTag::RoadTwoLanes_SidewalkLeft,      source.x, y, HALF_PI);
                    AddInstance(ECityMapMeshTag::RoadTwoLanes_SidewalkRight,     source.x, y, HALF_PI);
                    AddInstance(ECityMapMeshTag::RoadTwoLanes_LaneMarkingBroken, source.x, y, HALF_PI);
                }
            } else if (source.y == target.y) {
                // horizontal
                auto x = 1u + margin + std::min(source.x, target.x);
                auto end = std::max(source.x, target.x) - margin;
                for (; x < end; ++x) {
                    AddInstance(ECityMapMeshTag::RoadTwoLanes_LaneLeft,          x, source.y);
                    AddInstance(ECityMapMeshTag::RoadTwoLanes_LaneRight,         x, source.y);
                    AddInstance(ECityMapMeshTag::RoadTwoLanes_SidewalkLeft,      x, source.y);
                    AddInstance(ECityMapMeshTag::RoadTwoLanes_SidewalkRight,     x, source.y);
                    AddInstance(ECityMapMeshTag::RoadTwoLanes_LaneMarkingBroken, x, source.y);
                }
            } else {
                UE_LOG(LogCarla, Warning, TEXT("Diagonal edge ignored"));
            }
        }
    }

#define ADD_INTERSECTION(tag, x, y, angle) \
AddInstance(tag ##_Lane0, x, y, angle); \
AddInstance(tag ##_Lane1, x, y, angle); \
AddInstance(tag ##_Lane2, x, y, angle); \
AddInstance(tag ##_Lane3, x, y, angle); \
AddInstance(tag ##_Lane4, x, y, angle); \
AddInstance(tag ##_Lane5, x, y, angle); \
AddInstance(tag ##_Lane6, x, y, angle); \
AddInstance(tag ##_Lane7, x, y, angle); \
AddInstance(tag ##_Lane8, x, y, angle); \
AddInstance(tag ##_Lane9, x, y, angle); \
AddInstance(tag ##_Sidewalk0, x, y, angle); \
AddInstance(tag ##_Sidewalk1, x, y, angle); \
AddInstance(tag ##_Sidewalk2, x, y, angle); \
AddInstance(tag ##_Sidewalk3, x, y, angle); \
AddInstance(tag ##_LaneMarking, x, y, angle);


    // For each node add the intersection.
    for (auto &node : graph.GetNodes()) {
        const auto coords = node.GetPosition();
        switch (node.IntersectionType) {
            case MapGen::EIntersectionType::Turn90Deg:
            ADD_INTERSECTION(ECityMapMeshTag::Road90DegTurn, coords.x, coords.y, node.Rotation);
                break;
            case MapGen::EIntersectionType::TIntersection:
            ADD_INTERSECTION(ECityMapMeshTag::RoadTIntersection, coords.x, coords.y, node.Rotation);
                break;
            case MapGen::EIntersectionType::XIntersection:
            ADD_INTERSECTION(ECityMapMeshTag::RoadXIntersection, coords.x, coords.y, node.Rotation);
                break;
            default:
                UE_LOG(LogCarla, Warning, TEXT("Intersection type not implemented"));
        }
    }

#undef ADD_INTERSECTION
}

void AMacchinaCityMapGenerator::GenerateRoadMap()
{
    UE_LOG(LogCarla, Log, TEXT("Generating road map..."));

    auto World = GetWorld();
    check(GetWorld() != nullptr);
    check(RoadMap != nullptr);

    ATagger::TagActorsInLevel(*GetWorld(), bTagForSemanticSegmentation); // We need the tags.

    const float IntersectionSize = CityMapMeshTag::GetRoadIntersectionSize();
    const uint32 Margin = IntersectionSize / 2u;
    const float Offset = GetMapScale() * Margin;

    const float CmPerPixel = GetMapScale() / static_cast<float>(PixelsPerMapUnit);

    const uint32 SizeX = PixelsPerMapUnit * (MapSizeX + 2u * Margin);
    const uint32 SizeY = PixelsPerMapUnit * (MapSizeY + 2u * Margin);

    const FTransform &ActorTransform = GetActorTransform();

    const FVector MapOffset(-Offset, -Offset, 0.0f);
    RoadMap->Reset(SizeX, SizeY, 1.0f / CmPerPixel, ActorTransform.Inverse(), MapOffset);

    for (uint32 PixelY = 0u; PixelY < SizeY; ++PixelY) {
        for (uint32 PixelX = 0u; PixelX < SizeX; ++PixelX) {
            const float X = static_cast<float>(PixelX) * CmPerPixel - Offset;
            const float Y = static_cast<float>(PixelY) * CmPerPixel - Offset;
            const FVector Start = ActorTransform.TransformPosition(FVector(X, Y, 50.0f));
            const FVector End = ActorTransform.TransformPosition(FVector(X, Y, -50.0f));

            // Do the ray tracing.
            FHitResult Hit;
            if (LineTrace(World, Start, End, Hit)) {
                auto StaticMeshComponent = Cast<UStaticMeshComponent>(Hit.Component.Get());
                if (StaticMeshComponent == nullptr) {
                    UE_LOG(LogCarla, Error, TEXT("Road component is not UInstancedStaticMeshComponent"));
                } else {
                    RoadMap->SetPixelAt(
                            PixelX,
                            PixelY,
                            GetTag(*StaticMeshComponent->GetStaticMesh()),
                            StaticMeshComponent->GetOwner()->GetTransform(),
                            bLeftHandTraffic);
                }
            }
        }
    }

#if WITH_EDITOR
    RoadMap->Log();
#endif // WITH_EDITOR

    if (bSaveRoadMapToDisk) {
        RoadMap->SaveAsPNG(FPaths::ProjectSavedDir(), World->GetMapName());
    }

#if WITH_EDITOR
    RoadMap->DrawDebugPixelsToLevel(GetWorld(), !bDrawDebugPixelsToLevel);
#endif // WITH_EDITOR
}
