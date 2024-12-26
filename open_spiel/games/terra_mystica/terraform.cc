#include "open_spiel/games/terra_mystica/terraform.h"
#include "open_spiel/games/terra_mystica/faction.h"

namespace open_spiel {
namespace terra_mystica {

int TerrainDistance::GetDistance(Terrain from, Terrain to) {
  // Returns number of spades needed to transform between terrains
  // The terrain wheel is circular, so we need to find the shorter path
  int from_idx = static_cast<int>(from);
  int to_idx = static_cast<int>(to);
  
  // Calculate both clockwise and counterclockwise distances
  int direct_distance = std::abs(from_idx - to_idx);
  int wrap_distance = static_cast<int>(Terrain::kRiver) - direct_distance;
  
  // Return the shorter distance
  return std::min(direct_distance, wrap_distance);
}

int TerrainDistance::GetWorkerCost(int distance, int terraforming_level, 
                                 const FactionInfo& faction,
                                 Terrain from_terrain) {
  // Giants always use 2 spades regardless of distance
  if (faction.GetId() == FactionId::kGiants) {
    return 2;
  }
  
  // Darklings always use 1 priest per spade
  if (faction.GetId() == FactionId::kDarklings) {
    return distance;  // 1 priest per spade
  }
  
  // Regular worker cost calculation
  int base_cost;
  switch (terraforming_level) {
    case 0: base_cost = BaseCosts::BaseSpadeWorkerCost(); break;  // 3 workers
    case 1: base_cost = 2; break;  // 2 workers
    case 2: base_cost = 1; break;  // 1 worker
    default: base_cost = BaseCosts::BaseSpadeWorkerCost(); break;
  }
  
  return base_cost * distance;
}

bool TerraformHelper::CanTransformTerrain(
    int hex_id,
    Terrain current_terrain,
    Terrain target_terrain,
    int terraforming_level,
    int available_workers,
    const FactionInfo& faction,
    bool has_connection) {
  // Basic validity checks
  if (hex_id < 0) return false;
  if (!IsTransformable(current_terrain)) return false;
  if (!IsTransformable(target_terrain)) return false;
  if (!has_connection) return false;
  
  // Giants can only transform to Mountains
  if (faction.GetId() == FactionId::kGiants && 
      target_terrain != Terrain::kMountains) {
    return false;
  }
  
  // Calculate distance and cost
  int distance = TerrainDistance::GetDistance(current_terrain, target_terrain);
  int resource_cost = TerrainDistance::GetWorkerCost(
      distance, terraforming_level, faction, current_terrain);
  
  // For Darklings, available_workers represents priests
  if (faction.GetId() == FactionId::kDarklings) {
    return available_workers >= resource_cost;  // Need 1 priest per spade
  }
  
  // For all other factions, check worker cost
  return available_workers >= resource_cost;
}

int TerraformHelper::GetTransformationCost(
    Terrain from,
    Terrain to,
    int terraforming_level,
    const FactionInfo& faction) {
  if (!IsTransformable(from) || !IsTransformable(to)) return -1;
  
  // Giants can only transform to Mountains
  if (faction.GetId() == FactionId::kGiants && to != Terrain::kMountains) {
    return -1;
  }
  
  int distance = TerrainDistance::GetDistance(from, to);
  return TerrainDistance::GetWorkerCost(distance, terraforming_level, faction, from);
}

}  // namespace terra_mystica
}  // namespace open_spiel
