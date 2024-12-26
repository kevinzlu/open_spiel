#include "open_spiel/games/terra_mystica/exchange.h"

namespace open_spiel {
namespace terra_mystica {

bool ExchangeHelper::CanUpgradeExchange(const FactionInfo& faction) {
  // Darklings cannot upgrade exchange track
  if (faction.GetId() == FactionId::kDarklings) return false;
  return true;
}

int ExchangeHelper::GetMaxLevel(const FactionInfo& faction) {
  if (faction.GetId() == FactionId::kFakirs) {
    return ExchangeConstants::kFakirMaxLevel;
  }
  return ExchangeConstants::kDefaultMaxLevel;
}

int ExchangeHelper::GetWorkerCost(const FactionInfo& faction) {
  if (faction.GetId() == FactionId::kHalflings) {
    return ExchangeConstants::kHalflingCostWorkers;
  }
  return ExchangeConstants::kBaseCostWorkers;
}

int ExchangeHelper::GetCoinCost(const FactionInfo& faction) {
  if (faction.GetId() == FactionId::kHalflings) {
    return ExchangeConstants::kHalflingCostCoins;
  }
  return ExchangeConstants::kBaseCostCoins;
}

int ExchangeHelper::GetPriestCost(const FactionInfo& faction) {
  if (faction.GetId() == FactionId::kHalflings) {
    return ExchangeConstants::kHalflingCostPriest;
  }
  return ExchangeConstants::kBaseCostPriest;
}

int ExchangeHelper::GetUpgradePoints() {
  return ExchangeConstants::kUpgradePoints;
}

bool ExchangeHelper::CanAdvanceExchange(
    const FactionInfo& faction, 
    int current_level,
    int available_workers,
    int available_coins,
    int available_priests) {
  // Check if faction can use exchange track
  if (!CanUpgradeExchange(faction)) return false;
  
  // Check if at max level
  if (current_level >= GetMaxLevel(faction)) return false;
  
  // Check resources
  return available_workers >= GetWorkerCost(faction) &&
         available_coins >= GetCoinCost(faction) &&
         available_priests >= GetPriestCost(faction);
}

}  // namespace terra_mystica
}  // namespace open_spiel
