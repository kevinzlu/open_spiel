#ifndef OPEN_SPIEL_GAMES_TERRA_MYSTICA_UPGRADE_H_
#define OPEN_SPIEL_GAMES_TERRA_MYSTICA_UPGRADE_H_

#include <vector>
#include "open_spiel/spiel.h"

namespace open_spiel {
namespace terra_mystica {

// Forward declarations
class FactionInfo;

// Building types
enum class Building {
  kNone,
  kDwelling,
  kTradingHouse,
  kTemple,
  kStronghold,
  kSanctuary
};

// Building costs
struct BuildingCost {
  int workers;
  int coins;
  
  BuildingCost& operator=(const BuildingCost& other) = default;
  
  BuildingCost WithDoubleCoinCost() const {
    return {workers, coins * 2};
  }
};

// Building upgrade helper
class BuildingHelper {
 public:
  // Check if a building can be upgraded to another type
  static bool CanUpgrade(Building from, Building to);
  
  // Get the cost to upgrade from one building type to another
  // For trading houses, the cost depends on whether it's adjacent to another player
  static BuildingCost GetUpgradeCost(Building from, Building to,
                                   const FactionInfo& faction,
                                   bool has_adjacent_opponent = false);
  
  // Get list of possible upgrades for a building
  static std::vector<Building> GetPossibleUpgrades(Building current);
  
  // Check if a building is a terminal upgrade (cannot be upgraded further)
  static bool IsTerminalBuilding(Building building) {
    return building == Building::kStronghold || 
           building == Building::kSanctuary;
  }
  
  // Get trading house cost based on adjacency
  static BuildingCost GetTradingHouseCost(const FactionInfo& faction,
                                        bool has_adjacent_opponent);
};

}  // namespace terra_mystica
}  // namespace open_spiel

#endif  // OPEN_SPIEL_GAMES_TERRA_MYSTICA_UPGRADE_H_
