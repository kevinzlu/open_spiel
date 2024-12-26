#ifndef OPEN_SPIEL_GAMES_TERRA_MYSTICA_POWER_H_
#define OPEN_SPIEL_GAMES_TERRA_MYSTICA_POWER_H_

#include <array>
#include <vector>
#include "open_spiel/games/terra_mystica/faction.h"

namespace open_spiel {
namespace terra_mystica {

// Power values for each building type
struct BuildingPowerValues {
  static int GetValue(Building building) {
    switch (building) {
      case Building::kStronghold:
      case Building::kSanctuary:
        return 3;
      case Building::kTemple:
      case Building::kTradingPost:
        return 2;
      case Building::kDwelling:
        return 1;
      default:
        return 0;
    }
  }
};

// Represents the three power bowls
struct PowerBowls {
  int bowl1 = 0;  // Power tokens in bowl 1
  int bowl2 = 0;  // Power tokens in bowl 2
  int bowl3 = 0;  // Power tokens in bowl 3
  
  // Get total power tokens across all bowls
  int GetTotalPower() const { return bowl1 + bowl2 + bowl3; }
};

// Result of calculating potential power gain
struct PowerGainOption {
  int power_amount = 0;     // Amount of power that can be gained
  int victory_point_cost = 0;  // Victory points that would be lost
  bool can_afford = false;   // Whether player has enough VP to pay the cost
  
  // Calculate VP cost for a power amount
  static int CalculateVPCost(int power_amount) {
    return power_amount > 0 ? power_amount - 1 : 0;
  }
};

class PowerHelper {
 public:
  // Gain power, moving tokens clockwise through bowls
  // Returns actual amount of power gained (may be less than requested if bowl3 fills)
  static int GainPower(PowerBowls& bowls, int amount);
  
  // Spend power from bowl3, moving tokens to bowl1
  // Returns true if enough power was available to spend
  static bool SpendPower(PowerBowls& bowls, int amount);
  
  // Get available power (amount in bowl3)
  static int GetAvailablePower(const PowerBowls& bowls) {
    return bowls.bowl3;
  }
  
  // Get total power in bowl2
  static int GetBowl2Power(const PowerBowls& bowls) {
    return bowls.bowl2;
  }
  
  // Burn power from bowl2 to bowl3
  // Returns true if power was successfully burned
  static bool BurnPower(PowerBowls& bowls, int amount);
  
  // Check if we can burn power (need at least 2 tokens in bowl2)
  static bool CanBurnPower(const PowerBowls& bowls) {
    return bowls.bowl2 >= 2;  // Need at least 2 tokens to burn 1
  }
  
  // Initialize power bowls with specific values
  static void InitializePowerBowls(PowerBowls& bowls, int bowl1, int bowl2, int bowl3) {
    bowls.bowl1 = bowl1;
    bowls.bowl2 = bowl2;
    bowls.bowl3 = bowl3;
  }
  
  // Initialize power bowls for a specific faction
  static void InitializePowerBowls(PowerBowls& bowls, const FactionInfo& faction) {
    bowls = faction.GetStartingPowerBowls();
  }
  
  // Calculate maximum power that could be gained and VP cost
  // Considers available tokens in bowls 1 and 2
  static PowerGainOption CalculatePowerGainOption(
      const PowerBowls& bowls,
      int total_power_value,  // Sum of adjacent building power values
      int available_vp);      // Current VP of the player
  
  // Constants for power costs
  static constexpr int kPowerToPriestCost = 5;
  static constexpr int kPowerToWorkerCost = 3;
  static constexpr int kPowerToCoinCost = 1;
};

}  // namespace terra_mystica
}  // namespace open_spiel

#endif  // OPEN_SPIEL_GAMES_TERRA_MYSTICA_POWER_H_
