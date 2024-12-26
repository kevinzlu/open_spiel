#ifndef OPEN_SPIEL_GAMES_TERRA_MYSTICA_FACTION_H_
#define OPEN_SPIEL_GAMES_TERRA_MYSTICA_FACTION_H_

#include <optional>
#include <string>
#include <array>
#include "open_spiel/games/terra_mystica/terraform.h"
#include "open_spiel/games/terra_mystica/upgrade.h"
#include "open_spiel/games/terra_mystica/cult.h"

namespace open_spiel {
namespace terra_mystica {

// Faction IDs
enum class FactionId {
  kNone = -1,
  kChaosmagicians,
  kGiants,
  kFakirs,
  kNomads,
  kHalflings,
  kCultists,
  kAlchemists,
  kDarklings,
  kMermaids,
  kSwarmlings,
  kAuren,
  kWitches,
  kDwarves,
  kEngineers
};

// Resource types for terraforming
enum class TerraformResource {
  kWorker,
  kPriest
};

// Base costs for buildings and terraforming
struct BaseCosts {
  static BuildingCost DwellingCost() { return {1, 2}; }
  static BuildingCost TradingHouseCost() { return {2, 3}; }
  static BuildingCost TempleCost() { return {2, 5}; }
  static BuildingCost StrongholdCost() { return {4, 6}; }
  static BuildingCost SanctuaryCost() { return {4, 6}; }
  
  static int BaseSpadeWorkerCost() { return 3; }
  static int BasePriestCost() { return 1; }  // For Darklings
};

// Structure to hold faction-specific building cost overrides
struct BuildingCostOverrides {
  std::optional<BuildingCost> dwelling;
  std::optional<BuildingCost> trading_house;
  std::optional<BuildingCost> temple;
  std::optional<BuildingCost> stronghold;
  std::optional<BuildingCost> sanctuary;
};

// Starting cult levels for each faction
struct StartingCultLevels {
  int fire;
  int water;
  int earth;
  int air;
  
  // Helper to get level for a specific cult type
  int GetLevel(CultType type) const {
    switch (type) {
      case CultType::kFire: return fire;
      case CultType::kWater: return water;
      case CultType::kEarth: return earth;
      case CultType::kAir: return air;
    }
    return 0;
  }
};

// Forward declaration of PowerBowls
struct PowerBowls;

// Faction-specific information and modifiers
class FactionInfo {
 public:
  static FactionInfo Create(FactionId id);
  
  // Getters
  FactionId GetId() const { return id_; }
  std::string GetName() const { return name_; }
  Terrain GetHomeTerrain() const { return home_terrain_; }
  const StartingCultLevels& GetStartingCultLevels() const { return starting_cult_levels_; }
  
  // Get starting power bowl configuration for this faction
  PowerBowls GetStartingPowerBowls() const;
  
  // Cost modifiers
  BuildingCost GetModifiedBuildingCost(Building building) const;
  
 private:
  FactionInfo(FactionId id, 
              const std::string& name,
              Terrain home_terrain,
              BuildingCostOverrides cost_overrides = BuildingCostOverrides{},
              StartingCultLevels starting_cult_levels = {0, 0, 0, 0})
      : id_(id),
        name_(name),
        home_terrain_(home_terrain),
        cost_overrides_(cost_overrides),
        starting_cult_levels_(starting_cult_levels) {}

  FactionId id_;
  std::string name_;
  Terrain home_terrain_;
  BuildingCostOverrides cost_overrides_;
  StartingCultLevels starting_cult_levels_;
};

}  // namespace terra_mystica
}  // namespace open_spiel

#endif  // OPEN_SPIEL_GAMES_TERRA_MYSTICA_FACTION_H_
