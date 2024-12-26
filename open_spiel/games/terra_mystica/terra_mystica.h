#ifndef OPEN_SPIEL_GAMES_TERRA_MYSTICA_H_
#define OPEN_SPIEL_GAMES_TERRA_MYSTICA_H_

#include <array>
#include <memory>
#include <string>
#include <vector>

#include "open_spiel/spiel.h"
#include "open_spiel/games/terra_mystica/terraform.h"
#include "open_spiel/games/terra_mystica/upgrade.h"
#include "open_spiel/games/terra_mystica/power.h"

namespace open_spiel {
namespace terra_mystica {

// Constants
inline constexpr int kNumPlayers = 4;  // 2-5 players supported, default 4
inline constexpr int kNumFactions = 14;  // All factions in the base game
inline constexpr int kNumRows = 9;  // Number of rows on the board
inline constexpr int kEvenRowHexesPerRow = 13;  // Maximum hexes in a row (odd rows)
inline constexpr int kOddRowHexesPerRow = 12;  // Maximum hexes in a column (even rows)
inline constexpr int kBoardSize = ((kNumRows + 1) / 2) * kEvenRowHexesPerRow + (kNumRows / 2) * kOddRowHexesPerRow;  // Total hexes
inline constexpr int kMaxGameLength = 6;  // 6 rounds

// Game phases
enum class Phase {
  kSetup,
  kIncome,
  kActions,
  kCultTrackBonus,
  kEndOfRound,
  kGameEnd
};

// Building types
enum class Building {
  kNone,
  kDwelling,
  kTradingHouse,
  kTemple,
  kStronghold,
  kSanctuary
};

// Player resources
struct PlayerResources {
  int workers = 0;
  int coins = 0;
  int priests = 0;
  PowerBowls power_bowls;  // Power tokens in bowls 1, 2, 3
  int shipping_level = 0;
  int terraforming = 0;    // Exchange track level
  int victory_points = 0;
};

// Resources
struct Resources {
  PlayerResources player_resources_[kNumPlayers];
  int shipping = 0;      // Shipping level (0-3)
  int terraforming = 0;  // Terraforming level (0-2), 0 is base level
};

// Action types
enum class ActionType {
  kTransform,          // Transform terrain
  kTransformBuild,     // Transform terrain and build
  kBuild,              // Build on existing terrain
  kUpgrade,            // Upgrade existing building
  kSendPriest,         // Send priest to cult track
  kPowerAction,        // Use power action
  kAcceptPower,        // Accept power from adjacent build
  kPowerToPriest,      // Convert power to priest
  kPowerToWorker,      // Convert power to worker
  kPowerToCoin,        // Convert power to coin
  kPriestToWorker,     // Convert priest to worker
  kWorkerToCoin,       // Convert worker to coin
  kBurnPower,          // Burn power from bowl 2 to bowl 3
  kSpadeAction         // Use spades from power action
};

// Power action types
enum class PowerActionType {
  kBridge,         // Build bridge between hexes (3 power)
  kPriest,         // Gain priest (4 power)
  kWorkers,        // Gain 2 workers (4 power)
  kCoins,          // Gain 7 coins (6 power)
  kSingleSpade,    // Gain 1 spade (4 power)
  kDoubleSpade,    // Gain 2 spades (6 power)
  kCultAdvance     // Advance on cult track (4 power)
};

// Power action costs
struct PowerActionCosts {
  static constexpr int kBridge = 3;
  static constexpr int kPriest = 3;
  static constexpr int kWorkers = 4;
  static constexpr int kCoins = 4;
  static constexpr int kSingleSpade = 4;
  static constexpr int kDoubleSpade = 6;
  static constexpr int kCultAdvance = 4;
};

// Bridge between two hexes
struct Bridge {
  int hex1;
  int hex2;
  Player owner;
};

// Action representation
struct Action {
  static Action CreateTransformAction(int hex_id, Terrain target_terrain) {
    return {ActionType::kTransform, hex_id, static_cast<int>(target_terrain), false};
  }
  
  static Action CreateTransformBuildAction(int hex_id, Terrain target_terrain) {
    return {ActionType::kTransformBuild, hex_id, static_cast<int>(target_terrain), false};
  }
  
  static Action CreateUpgradeAction(int hex_id, Building target_building) {
    return {ActionType::kUpgrade, hex_id, static_cast<int>(target_building), false};
  }

  ActionType type;
  int hex_id;       // Hex ID for terrain/building actions, unused for others
  int action_data;  // Target terrain/building for transforms/upgrades, cult track for cult actions
  bool executed;    // Internal state to track partial execution
};

// Game state
class TerraMysticaState : public State {
 public:
  TerraMysticaState(std::shared_ptr<const Game> game);
  TerraMysticaState(const TerraMysticaState&) = default;

  Player CurrentPlayer() const override;
  std::string ActionToString(Player player, Action action_id) const override;
  std::string ToString() const override;
  bool IsTerminal() const override;
  std::vector<double> Returns() const override;
  std::string InformationStateString(Player player) const override;
  std::string ObservationString(Player player) const override;
  void ObservationTensor(Player player,
                        absl::Span<float> values) const override;
  std::unique_ptr<State> Clone() const override;
  std::vector<Action> LegalActions() const override;

 protected:
  void DoApplyAction(Action action_id) override;

 private:
  // Helper methods for terraforming and adjacency
  bool HasAdjacentBuilding(int hex_id, Player player) const;
  bool HasIndirectConnection(int hex_id, Player player) const;
  std::vector<int> GetRiverConnectedHexes(int start_hex, int shipping_level) const;
  std::vector<Action> GetLegalTransformActions() const;
  std::vector<Action> GetLegalUpgradeActions() const;
  std::vector<Action> GetLegalShippingActions() const;
  std::vector<Action> GetLegalCultActions() const;
  std::vector<Action> GetLegalExchangeActions() const;
  std::vector<Action> GetLegalPowerActions() const;
  std::vector<Action> GetLegalSpadeActions(int spades, bool can_build) const;
  std::vector<Action> GetPowerGainActions(Player player, int hex_id) const;
  
  // Bridge helper methods
  bool CanBuildBridge(int hex1, int hex2, Player player) const;
  bool HasBridge(int hex1, int hex2) const;
  int GetPlayerBridgeCount(Player player) const;
  static constexpr int kMaxBridgesPerPlayer = 2;
  
  // Building helper methods
  bool CanBuildDwelling(int hex_id) const;
  void BuildDwelling(int hex_id);
  void UpgradeBuilding(int hex_id, Building target_building);
  
  // Power helper methods
  void GainPower(Player player, int amount);
  bool SpendPower(Player player, int amount);
  void HandlePowerFromBuilding(int hex_id);
  PowerGainOption GetPowerGainFromBuilding(Player player, int hex_id) const;
  int GetTotalPriests(Player player) const;  // Count priests in resources + cult tracks
  static constexpr int kMaxPriests = 7;
  
  Phase current_phase_ = Phase::kSetup;
  int current_round_ = 0;
  Player current_player_ = -1;
  
  // Game state
  std::vector<FactionId> player_factions_;
  std::vector<PlayerResources> player_resources_;
  std::vector<std::vector<int>> cult_tracks_;  // [cult_type][player]
  std::vector<Building> board_;
  std::vector<Terrain> terrain_;
  std::vector<int> building_owners_;  // -1 means no owner
  
  // Power gain state
  int pending_power_hex_ = -1;
  int next_power_player_ = -1;
  
  // Power action state
  std::vector<bool> used_power_actions_;  // Which power actions have been used this round
  std::vector<Bridge> bridges_;  // All bridges built
  
  // State for spade power actions
  int pending_spades_ = 0;  // Number of spades from power action waiting to be used
  bool can_build_after_spade_ = false;  // Whether we can build after using spades
};

// Game object
class TerraMysticaGame : public Game {
 public:
  explicit TerraMysticaGame(const GameParameters& params);
  int NumDistinctActions() const override;
  std::unique_ptr<State> NewInitialState() const override;
  int NumPlayers() const override { return kNumPlayers; }
  double MinUtility() const override { return -1; }
  double MaxUtility() const override { return 1; }
  int MaxGameLength() const override { return kMaxGameLength; }
  std::vector<int> ObservationTensorShape() const override;
};

}  // namespace terra_mystica
}  // namespace open_spiel

#endif  // OPEN_SPIEL_GAMES_TERRA_MYSTICA_H_
