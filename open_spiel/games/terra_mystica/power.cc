#include "open_spiel/games/terra_mystica/power.h"
#include <algorithm>

namespace open_spiel {
namespace terra_mystica {

int PowerHelper::GainPower(PowerBowls& bowls, int amount) {
  int power_gained = 0;
  
  // First, move tokens from bowl1 to bowl2
  while (amount > 0 && bowls.bowl1 > 0) {
    bowls.bowl1--;
    bowls.bowl2++;
    amount--;
    power_gained++;
  }
  
  // Then, move tokens from bowl2 to bowl3
  while (amount > 0 && bowls.bowl2 > 0) {
    bowls.bowl2--;
    bowls.bowl3++;
    amount--;
    power_gained++;
  }
  
  // If all tokens are in bowl3, can't gain more power
  return power_gained;
}

bool PowerHelper::SpendPower(PowerBowls& bowls, int amount) {
  // Check if enough power is available in bowl3
  if (bowls.bowl3 < amount) {
    return false;
  }
  
  // Move tokens from bowl3 to bowl1
  bowls.bowl3 -= amount;
  bowls.bowl1 += amount;
  
  return true;
}

bool PowerHelper::BurnPower(PowerBowls& bowls, int amount) {
  // amount is how many tokens we want to move to bowl3
  // we need 2*amount tokens in bowl2 since half are removed from game
  if (bowls.bowl2 < 2 * amount) {
    return false;
  }
  
  // Remove 2*amount tokens from bowl2 (half moved to bowl3, half removed from game)
  bowls.bowl2 -= 2 * amount;
  // Add amount tokens to bowl3
  bowls.bowl3 += amount;
  
  return true;
}

PowerGainOption PowerHelper::CalculatePowerGainOption(
    const PowerBowls& bowls,
    int total_power_value,
    int available_vp) {
  PowerGainOption option;
  
  // Calculate maximum power that could be gained based on bowls
  int max_from_bowl1 = bowls.bowl1;  // All tokens can move from bowl1 to bowl2
  int max_from_bowl2 = bowls.bowl2;  // All tokens can move from bowl2 to bowl3
  int max_possible = max_from_bowl1 + max_from_bowl2;
  
  // Power gained is minimum of what's possible and what's available from buildings
  option.power_amount = std::min(max_possible, total_power_value);
  
  // Calculate VP cost
  option.victory_point_cost = PowerGainOption::CalculateVPCost(option.power_amount);
  
  // Check if player can afford the VP cost
  option.can_afford = (available_vp >= option.victory_point_cost);
  
  // If can't afford full amount, calculate maximum affordable amount
  if (!option.can_afford && available_vp > 0) {
    // Maximum power we can gain while keeping non-negative VP
    option.power_amount = available_vp + 1;  // Since cost is (power - 1)
    option.victory_point_cost = available_vp;
    option.can_afford = true;
  }
  
  return option;
}

}  // namespace terra_mystica
}  // namespace open_spiel
