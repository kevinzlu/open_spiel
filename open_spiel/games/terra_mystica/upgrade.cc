#include "open_spiel/games/terra_mystica/upgrade.h"
#include "open_spiel/games/terra_mystica/faction.h"

namespace open_spiel {
namespace terra_mystica {

bool BuildingHelper::CanUpgrade(Building from, Building to) {
  switch (from) {
    case Building::kDwelling:
      return to == Building::kTradingHouse;
    case Building::kTradingHouse:
      return to == Building::kTemple || to == Building::kStronghold;
    case Building::kTemple:
      return to == Building::kSanctuary;
    default:
      return false;
  }
}

BuildingCost BuildingHelper::GetTradingHouseCost(const FactionInfo& faction,
                                               bool has_adjacent_opponent) {
  // Get the base or faction-specific trading house cost
  BuildingCost base_cost = faction.GetModifiedBuildingCost(Building::kTradingHouse);
  
  // Double the coin cost if not adjacent to opponent
  if (!has_adjacent_opponent) {
    return base_cost.WithDoubleCoinCost();
  }
  
  return base_cost;
}

BuildingCost BuildingHelper::GetUpgradeCost(Building from, Building to,
                                          const FactionInfo& faction,
                                          bool has_adjacent_opponent) {
  if (!CanUpgrade(from, to)) return {0, 0};  // Invalid upgrade
  
  // Special handling for trading house upgrades
  if (to == Building::kTradingHouse) {
    return GetTradingHouseCost(faction, has_adjacent_opponent);
  }
  
  // All other upgrades use standard costs
  return faction.GetModifiedBuildingCost(to);
}

std::vector<Building> BuildingHelper::GetPossibleUpgrades(Building current) {
  std::vector<Building> upgrades;
  for (Building b : {Building::kTradingHouse, Building::kTemple, 
                    Building::kStronghold, Building::kSanctuary}) {
    if (CanUpgrade(current, b)) {
      upgrades.push_back(b);
    }
  }
  return upgrades;
}

}  // namespace terra_mystica
}  // namespace open_spiel
