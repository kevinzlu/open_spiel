#ifndef OPEN_SPIEL_GAMES_TERRA_MYSTICA_EXCHANGE_H_
#define OPEN_SPIEL_GAMES_TERRA_MYSTICA_EXCHANGE_H_

#include "open_spiel/games/terra_mystica/faction.h"

namespace open_spiel {
namespace terra_mystica {

// Constants for exchange track upgrades
struct ExchangeConstants {
  // Default costs
  static constexpr int kBaseCostWorkers = 2;
  static constexpr int kBaseCostCoins = 5;
  static constexpr int kBaseCostPriest = 1;
  
  // Halfling costs
  static constexpr int kHalflingCostWorkers = 2;
  static constexpr int kHalflingCostCoins = 1;
  static constexpr int kHalflingCostPriest = 1;
  
  // Victory points awarded
  static constexpr int kUpgradePoints = 6;
  
  // Maximum level
  static constexpr int kDefaultMaxLevel = 2;
  static constexpr int kFakirMaxLevel = 1;
};

class ExchangeHelper {
 public:
  // Check if faction can upgrade exchange track
  static bool CanUpgradeExchange(const FactionInfo& faction);
  
  // Get maximum exchange level for faction
  static int GetMaxLevel(const FactionInfo& faction);
  
  // Get worker cost for upgrade
  static int GetWorkerCost(const FactionInfo& faction);
  
  // Get coin cost for upgrade
  static int GetCoinCost(const FactionInfo& faction);
  
  // Get priest cost for upgrade
  static int GetPriestCost(const FactionInfo& faction);
  
  // Get points for upgrade
  static int GetUpgradePoints();
  
  // Check if faction can advance exchange track
  static bool CanAdvanceExchange(const FactionInfo& faction, 
                               int current_level,
                               int available_workers,
                               int available_coins,
                               int available_priests);
};

}  // namespace terra_mystica
}  // namespace open_spiel

#endif  // OPEN_SPIEL_GAMES_TERRA_MYSTICA_EXCHANGE_H_
