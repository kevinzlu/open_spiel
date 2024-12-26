#include "open_spiel/games/terra_mystica/shipping.h"

namespace open_spiel {
namespace terra_mystica {

int ShippingPoints::GetPoints(int from_level, const FactionInfo& faction) {
  // Points for advancing from current level to next level
  if (faction.GetId() == FactionId::kMermaids) {
    switch (from_level) {
      case 1: return 2;  // 1->2
      case 2: return 3;  // 2->3
      case 3: return 4;  // 3->4
      case 4: return 5;  // 4->5
      default: return 0;
    }
  } else {
    switch (from_level) {
      case 0: return 2;  // 0->1
      case 1: return 3;  // 1->2
      case 2: return 4;  // 2->3
      default: return 0;
    }
  }
}

bool ShippingHelper::CanUpgradeShipping(const FactionInfo& faction) {
  FactionId id = faction.GetId();
  return id != FactionId::kDwarves && id != FactionId::kFakirs;
}

int ShippingHelper::GetMaxLevel(const FactionInfo& faction) {
  return faction.GetId() == FactionId::kMermaids ? 
         ShippingConstants::kMermaidMaxLevel : 
         ShippingConstants::kDefaultMaxLevel;
}

int ShippingHelper::GetStartLevel(const FactionInfo& faction) {
  return faction.GetId() == FactionId::kMermaids ? 
         ShippingConstants::kMermaidStartLevel : 
         ShippingConstants::kDefaultStartLevel;
}

bool ShippingHelper::CanAdvanceShipping(
    const FactionInfo& faction, 
    int current_level,
    int available_priests, 
    int available_coins) {
  // Check if faction is allowed to upgrade shipping
  if (!CanUpgradeShipping(faction)) return false;
  
  // Check if at max level
  if (current_level >= GetMaxLevel(faction)) return false;
  
  // Check resources
  return available_priests >= ShippingConstants::kBaseCostPriest &&
         available_coins >= ShippingConstants::kBaseCostCoins;
}

int ShippingHelper::GetAdvancementPoints(int current_level, const FactionInfo& faction) {
  return ShippingPoints::GetPoints(current_level, faction);
}

}  // namespace terra_mystica
}  // namespace open_spiel
