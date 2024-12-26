#ifndef OPEN_SPIEL_GAMES_TERRA_MYSTICA_SHIPPING_H_
#define OPEN_SPIEL_GAMES_TERRA_MYSTICA_SHIPPING_H_

#include "open_spiel/games/terra_mystica/faction.h"

namespace open_spiel {
namespace terra_mystica {

// Constants for shipping
struct ShippingConstants {
  static constexpr int kBaseCostPriest = 1;
  static constexpr int kBaseCostCoins = 4;
  static constexpr int kDefaultMaxLevel = 3;
  static constexpr int kMermaidMaxLevel = 5;
  static constexpr int kDefaultStartLevel = 0;
  static constexpr int kMermaidStartLevel = 1;
};

// Points awarded for each shipping level advancement
struct ShippingPoints {
  static int GetPoints(int from_level, const FactionInfo& faction);
};

class ShippingHelper {
 public:
  // Check if faction can upgrade shipping
  static bool CanUpgradeShipping(const FactionInfo& faction);
  
  // Get maximum shipping level for faction
  static int GetMaxLevel(const FactionInfo& faction);
  
  // Get starting shipping level for faction
  static int GetStartLevel(const FactionInfo& faction);
  
  // Check if faction can advance to next shipping level
  static bool CanAdvanceShipping(const FactionInfo& faction, 
                               int current_level,
                               int available_priests, 
                               int available_coins);
  
  // Get points for advancing to next level
  static int GetAdvancementPoints(int current_level, const FactionInfo& faction);
};

}  // namespace terra_mystica
}  // namespace open_spiel

#endif  // OPEN_SPIEL_GAMES_TERRA_MYSTICA_SHIPPING_H_
