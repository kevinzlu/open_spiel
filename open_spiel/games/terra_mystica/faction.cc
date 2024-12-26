#include "open_spiel/games/terra_mystica/faction.h"
#include "open_spiel/games/terra_mystica/power.h"

namespace open_spiel {
namespace terra_mystica {

BuildingCost BuildingCost::DwellingCost() {
  return {1, 2};  // Base cost: 1 worker, 2 coins
}

BuildingCost BuildingCost::TradingHouseCost() {
  return {2, 3};  // Base cost: 2 workers, 3 coins
}

BuildingCost BuildingCost::TempleCost() {
  return {2, 5};  // Base cost: 2 workers, 5 coins
}

BuildingCost BuildingCost::StrongholdCost() {
  return {4, 6};  // Base cost: 4 workers, 6 coins
}

BuildingCost BuildingCost::SanctuaryCost() {
  return {4, 6};  // Base cost: 4 workers, 6 coins
}

FactionInfo FactionInfo::Create(FactionId id) {
  switch (id) {
    case FactionId::kChaosmagicians:
      return FactionInfo(id, "Chaos Magicians", Terrain::kWasteland,
                        BuildingCostOverrides{
                          .stronghold = {4, 4}  // Special SH cost
                        },
                        /*starting_cult_levels=*/{2, 0, 0, 0});  // 2 fire
      
    case FactionId::kGiants:
      return FactionInfo(id, "Giants", Terrain::kMountains,
                        BuildingCostOverrides{},
                        /*starting_cult_levels=*/{1, 0, 0, 1});  // 1 fire, 1 air
      
    case FactionId::kFakirs:
      return FactionInfo(id, "Fakirs", Terrain::kDesert,
                        BuildingCostOverrides{
                          .stronghold = {4, 10}
                        },
                        /*starting_cult_levels=*/{1, 0, 0, 1});  // 1 fire, 1 air
      
    case FactionId::kNomads:
      return FactionInfo(id, "Nomads", Terrain::kDesert,
                        BuildingCostOverrides{
                          .stronghold = {4, 8}
                        },
                        /*starting_cult_levels=*/{1, 0, 1, 0});  // 1 fire, 1 earth
      
    case FactionId::kHalflings:
      return FactionInfo(id, "Halflings", Terrain::kPlains,
                        BuildingCostOverrides{
                          .stronghold = {4, 8}
                        },
                        /*starting_cult_levels=*/{0, 0, 1, 1});  // 1 earth, 1 air
      
    case FactionId::kCultists:
      return FactionInfo(id, "Cultists", Terrain::kWasteland,
                        BuildingCostOverrides{
                          .stronghold = {4, 8},
                          .sanctuary = {4, 8}
                        },
                        /*starting_cult_levels=*/{1, 0, 1, 0});  // 1 earth, 1 fire
      
    case FactionId::kAlchemists:
      return FactionInfo(id, "Alchemists", Terrain::kSwamp,
                        BuildingCostOverrides{},
                        /*starting_cult_levels=*/{1, 1, 0, 0});  // 1 fire, 1 water
      
    case FactionId::kDarklings:
      return FactionInfo(id, "Darklings", Terrain::kSwamp,
                        BuildingCostOverrides{
                          .sanctuary = {4, 10}
                        },
                        /*starting_cult_levels=*/{0, 1, 1, 0});  // 1 water, 1 earth
      
    case FactionId::kMermaids:
      return FactionInfo(id, "Mermaids", Terrain::kLakes,
                        BuildingCostOverrides{
                          .sanctuary = {4, 8}
                        },
                        /*starting_cult_levels=*/{0, 2, 0, 0});  // 2 water
      
    case FactionId::kSwarmlings:
      return FactionInfo(id, "Swarmlings", Terrain::kLakes,
                        BuildingCostOverrides{
                          .dwelling = {2, 3},
                          .trading_house = {3, 4},
                          .temple = {3, 6},
                          .stronghold = {5, 8},
                          .sanctuary = {5, 8}
                        },
                        /*starting_cult_levels=*/{1, 1, 1, 1});  // 1 of each
      
    case FactionId::kAuren:
      return FactionInfo(id, "Auren", Terrain::kForest,
                        BuildingCostOverrides{
                          .sanctuary = {4, 8}
                        },
                        /*starting_cult_levels=*/{0, 1, 0, 1});  // 1 air, 1 water
      
    case FactionId::kWitches:
      return FactionInfo(id, "Witches", Terrain::kForest,
                        BuildingCostOverrides{},
                        /*starting_cult_levels=*/{0, 0, 0, 2});  // 2 air
      
    case FactionId::kDwarves:
      return FactionInfo(id, "Dwarves", Terrain::kMountains,
                        BuildingCostOverrides{},
                        /*starting_cult_levels=*/{0, 0, 2, 0});  // 2 earth
      
    case FactionId::kEngineers:
      return FactionInfo(id, "Engineers", Terrain::kMountains,
                        BuildingCostOverrides{
                          .dwelling = {1, 1},
                          .trading_house = {1, 2},
                          .temple = {1, 4},
                          .stronghold = {3, 6},
                          .sanctuary = {3, 6}
                        },
                        /*starting_cult_levels=*/{0, 0, 0, 0});  // 0 on each
      
    default:
      return FactionInfo(id, "Unknown", Terrain::kPlains);
  }
}

BuildingCost FactionInfo::GetModifiedBuildingCost(Building building) const {
  // First check for faction-specific override
  switch (building) {
    case Building::kDwelling:
      if (cost_overrides_.dwelling.has_value()) return *cost_overrides_.dwelling;
      return BaseCosts::DwellingCost();
      
    case Building::kTradingHouse:
      if (cost_overrides_.trading_house.has_value()) return *cost_overrides_.trading_house;
      return BaseCosts::TradingHouseCost();
      
    case Building::kTemple:
      if (cost_overrides_.temple.has_value()) return *cost_overrides_.temple;
      return BaseCosts::TempleCost();
      
    case Building::kStronghold:
      if (cost_overrides_.stronghold.has_value()) return *cost_overrides_.stronghold;
      return BaseCosts::StrongholdCost();
      
    case Building::kSanctuary:
      if (cost_overrides_.sanctuary.has_value()) return *cost_overrides_.sanctuary;
      return BaseCosts::SanctuaryCost();
      
    default:
      return {0, 0};
  }
}

int FactionInfo::GetSpadeWorkerCost(int terraforming_level) const {
  // Base cost depends on terraforming level (0-2)
  int base_cost;
  switch (terraforming_level) {
    case 0: base_cost = BaseCosts::BaseSpadeWorkerCost(); break;  // 3 workers
    case 1: base_cost = 2; break;  // 2 workers
    case 2: base_cost = 1; break;  // 1 worker
    default: base_cost = BaseCosts::BaseSpadeWorkerCost(); break;
  }
  
  // Apply faction-specific modifier
  return std::max(1, base_cost);  // Minimum cost of 1
}

PowerBowls FactionInfo::GetStartingPowerBowls() const {
  PowerBowls bowls;
  
  // Most factions start with 5 power in bowl 1 and 7 in bowl 2
  bowls.bowl1 = 5;
  bowls.bowl2 = 7;
  bowls.bowl3 = 0;
  
  // Special cases
  switch (faction_type_) {
    case FactionType::kEngineers:
    case FactionType::kMermaids:
    case FactionType::kSwarmlings:
    case FactionType::kHalflings:
      bowls.bowl1 = 3;
      bowls.bowl2 = 9;
      break;
    case FactionType::kFakirs:
      bowls.bowl1 = 7;
      bowls.bowl2 = 5;
      break;
    default:
      break;  // Use default values
  }
  
  return bowls;
}

}  // namespace terra_mystica
}  // namespace open_spiel
