#include "open_spiel/games/terra_mystica/power.h"
#include "open_spiel/spiel_utils.h"
#include "open_spiel/tests/basic_tests.h"

namespace open_spiel {
namespace terra_mystica {
namespace {

void TestPowerBowlMovement() {
  PowerBowls bowls;
  PowerHelper::InitializePowerBowls(bowls, 7, 5, 0);  // Custom initialization
  
  SPIEL_CHECK_EQ(bowls.bowl1, 7);
  SPIEL_CHECK_EQ(bowls.bowl2, 5);
  SPIEL_CHECK_EQ(bowls.bowl3, 0);
  
  // Test gaining power
  int gained = PowerHelper::GainPower(bowls, 5);
  SPIEL_CHECK_EQ(gained, 5);
  SPIEL_CHECK_EQ(bowls.bowl1, 2);
  SPIEL_CHECK_EQ(bowls.bowl2, 10);
  SPIEL_CHECK_EQ(bowls.bowl3, 0);
  
  // Test gaining more power
  gained = PowerHelper::GainPower(bowls, 3);
  SPIEL_CHECK_EQ(gained, 3);
  SPIEL_CHECK_EQ(bowls.bowl1, 2);
  SPIEL_CHECK_EQ(bowls.bowl2, 7);
  SPIEL_CHECK_EQ(bowls.bowl3, 3);
  
  // Test spending power
  bool spent = PowerHelper::SpendPower(bowls, 2);
  SPIEL_CHECK_TRUE(spent);
  SPIEL_CHECK_EQ(bowls.bowl1, 4);
  SPIEL_CHECK_EQ(bowls.bowl2, 7);
  SPIEL_CHECK_EQ(bowls.bowl3, 1);
}

void TestFactionStartingPower() {
  // Test Engineers (and other 3/9/0 factions)
  {
    FactionInfo engineers(FactionType::kEngineers);
    PowerBowls bowls = engineers.GetStartingPowerBowls();
    SPIEL_CHECK_EQ(bowls.bowl1, 3);
    SPIEL_CHECK_EQ(bowls.bowl2, 9);
    SPIEL_CHECK_EQ(bowls.bowl3, 0);
    
    // Test other factions with same configuration
    FactionInfo mermaids(FactionType::kMermaids);
    bowls = mermaids.GetStartingPowerBowls();
    SPIEL_CHECK_EQ(bowls.bowl1, 3);
    SPIEL_CHECK_EQ(bowls.bowl2, 9);
    SPIEL_CHECK_EQ(bowls.bowl3, 0);
    
    FactionInfo swarmlings(FactionType::kSwarmlings);
    bowls = swarmlings.GetStartingPowerBowls();
    SPIEL_CHECK_EQ(bowls.bowl1, 3);
    SPIEL_CHECK_EQ(bowls.bowl2, 9);
    SPIEL_CHECK_EQ(bowls.bowl3, 0);
    
    FactionInfo halflings(FactionType::kHalflings);
    bowls = halflings.GetStartingPowerBowls();
    SPIEL_CHECK_EQ(bowls.bowl1, 3);
    SPIEL_CHECK_EQ(bowls.bowl2, 9);
    SPIEL_CHECK_EQ(bowls.bowl3, 0);
  }
  
  // Test Fakirs (7/5/0)
  {
    FactionInfo fakirs(FactionType::kFakirs);
    PowerBowls bowls = fakirs.GetStartingPowerBowls();
    SPIEL_CHECK_EQ(bowls.bowl1, 7);
    SPIEL_CHECK_EQ(bowls.bowl2, 5);
    SPIEL_CHECK_EQ(bowls.bowl3, 0);
  }
  
  // Test default faction (5/7/0)
  {
    FactionInfo chaos(FactionType::kChaosmagicians);
    PowerBowls bowls = chaos.GetStartingPowerBowls();
    SPIEL_CHECK_EQ(bowls.bowl1, 5);
    SPIEL_CHECK_EQ(bowls.bowl2, 7);
    SPIEL_CHECK_EQ(bowls.bowl3, 0);
  }
}

void TestPowerGainOptions() {
  // Test case 1: Full power gain possible
  {
    FactionInfo default_faction(FactionType::kChaosmagicians);
    PowerBowls bowls = default_faction.GetStartingPowerBowls();
    
    auto option = PowerHelper::CalculatePowerGainOption(bowls, 4, 10);
    SPIEL_CHECK_EQ(option.power_amount, 4);
    SPIEL_CHECK_EQ(option.victory_point_cost, 3);
    SPIEL_CHECK_TRUE(option.can_afford);
  }
  
  // Test case 2: Limited by available VP
  {
    FactionInfo default_faction(FactionType::kChaosmagicians);
    PowerBowls bowls = default_faction.GetStartingPowerBowls();
    
    auto option = PowerHelper::CalculatePowerGainOption(bowls, 4, 1);
    SPIEL_CHECK_EQ(option.power_amount, 2);  // Can only gain 2 power
    SPIEL_CHECK_EQ(option.victory_point_cost, 1);  // Costs 1 VP
    SPIEL_CHECK_TRUE(option.can_afford);
  }
  
  // Test case 3: No VP available
  {
    FactionInfo default_faction(FactionType::kChaosmagicians);
    PowerBowls bowls = default_faction.GetStartingPowerBowls();
    
    auto option = PowerHelper::CalculatePowerGainOption(bowls, 4, 0);
    SPIEL_CHECK_EQ(option.power_amount, 0);
    SPIEL_CHECK_EQ(option.victory_point_cost, 0);
    SPIEL_CHECK_FALSE(option.can_afford);
  }
  
  // Test case 4: Limited by available tokens in bowls
  {
    FactionInfo engineers(FactionType::kEngineers);
    PowerBowls bowls = engineers.GetStartingPowerBowls();
    
    auto option = PowerHelper::CalculatePowerGainOption(bowls, 4, 10);
    SPIEL_CHECK_EQ(option.power_amount, 3);  // Can only gain 3 power
    SPIEL_CHECK_EQ(option.victory_point_cost, 2);
    SPIEL_CHECK_TRUE(option.can_afford);
  }
  
  // Test case 5: Some tokens already in bowl3
  {
    FactionInfo default_faction(FactionType::kChaosmagicians);
    PowerBowls bowls = default_faction.GetStartingPowerBowls();
    PowerHelper::GainPower(bowls, 4);  // Move some tokens to bowl2/3
    
    auto option = PowerHelper::CalculatePowerGainOption(bowls, 6, 10);
    SPIEL_CHECK_EQ(option.power_amount, 8);  // Can gain remaining tokens
    SPIEL_CHECK_EQ(option.victory_point_cost, 7);
    SPIEL_CHECK_TRUE(option.can_afford);
  }
}

TEST(PowerTest, TestBurnPower) {
  PowerBowls bowls;
  PowerHelper::InitializePowerBowls(bowls, 0, 8, 4);  // 0/8/4
  
  // Try to burn 3 power (need 6 in bowl2)
  EXPECT_TRUE(PowerHelper::BurnPower(bowls, 3));
  EXPECT_EQ(bowls.bowl1, 0);
  EXPECT_EQ(bowls.bowl2, 2);  // 8 - 6 = 2
  EXPECT_EQ(bowls.bowl3, 7);  // 4 + 3 = 7
  
  // Try to burn 2 power (need 4 in bowl2) - should fail
  EXPECT_FALSE(PowerHelper::BurnPower(bowls, 2));
  EXPECT_EQ(bowls.bowl1, 0);
  EXPECT_EQ(bowls.bowl2, 2);  // Unchanged
  EXPECT_EQ(bowls.bowl3, 7);  // Unchanged
  
  // Try to burn 1 power (need 2 in bowl2)
  EXPECT_TRUE(PowerHelper::BurnPower(bowls, 1));
  EXPECT_EQ(bowls.bowl1, 0);
  EXPECT_EQ(bowls.bowl2, 0);  // 2 - 2 = 0
  EXPECT_EQ(bowls.bowl3, 8);  // 7 + 1 = 8
}

void TestBuildingPowerValues() {
  SPIEL_CHECK_EQ(BuildingPowerValues::GetValue(Building::kStronghold), 3);
  SPIEL_CHECK_EQ(BuildingPowerValues::GetValue(Building::kSanctuary), 3);
  SPIEL_CHECK_EQ(BuildingPowerValues::GetValue(Building::kTemple), 2);
  SPIEL_CHECK_EQ(BuildingPowerValues::GetValue(Building::kTradingPost), 2);
  SPIEL_CHECK_EQ(BuildingPowerValues::GetValue(Building::kDwelling), 1);
}

}  // namespace
}  // namespace terra_mystica
}  // namespace open_spiel

int main(int argc, char** argv) {
  open_spiel::terra_mystica::TestPowerBowlMovement();
  open_spiel::terra_mystica::TestFactionStartingPower();
  open_spiel::terra_mystica::TestPowerGainOptions();
  open_spiel::terra_mystica::TestBuildingPowerValues();
  return testing::InitGoogleTest(&argc, argv).RUN_ALL_TESTS();
}
