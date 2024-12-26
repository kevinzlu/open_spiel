#include "open_spiel/games/terra_mystica/terra_mystica.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <queue>
#include <vector>

namespace open_spiel {
namespace terra_mystica {

namespace {

// Register the game
const GameType kGameType{
    /*short_name=*/"terra_mystica",
    /*long_name=*/"Terra Mystica",
    GameType::Dynamics::kSequential,
    GameType::ChanceMode::kDeterministic,
    GameType::Information::kPerfectInformation,
    GameType::Utility::kZeroSum,
    GameType::RewardModel::kTerminal,
    /*max_num_players=*/5,
    /*min_num_players=*/2,
    /*provides_information_state_string=*/true,
    /*provides_information_state_tensor=*/false,
    /*provides_observation_string=*/true,
    /*provides_observation_tensor=*/true,
    /*parameter_specification=*/{}  // Parameters could be added here
};

std::shared_ptr<const Game> Factory(const GameParameters& params) {
  return std::shared_ptr<const Game>(new TerraMysticaGame(params));
}

REGISTER_SPIEL_GAME(kGameType, Factory);

}  // namespace

TerraMysticaState::TerraMysticaState(std::shared_ptr<const Game> game)
    : State(game) {
  // Initialize game state
  player_factions_.resize(kNumPlayers);
  player_resources_.resize(kNumPlayers);
  cult_tracks_.resize(4, std::vector<int>(kNumPlayers, 0));
  board_.resize(kBoardSize, Building::kNone);
  terrain_.resize(kBoardSize, Terrain::kPlains);  // Default terrain
  building_owners_.resize(kBoardSize, -1);  // -1 means no owner
  bridges_.clear();  // Initialize bridges
  
  // Initialize power actions (6 total actions)
  used_power_actions_.resize(6, false);  // All actions available at start
  
  // Initialize starting resources for each player
  for (auto& resources : player_resources_) {
    resources.workers = 3;
    resources.coins = 15;
    resources.priests = 1;
    resources.power = 5;
    resources.victoryPoints = 0;
    resources.shipping = 0;  // Initialize shipping level
    resources.terraforming = 0;  // Initialize terraforming level
  }
  
  // Initialize power gain state
  pending_power_hex_ = -1;
  next_power_player_ = -1;
  pending_spades_ = 0;
  can_build_after_spade_ = false;
}

Player TerraMysticaState::CurrentPlayer() const {
  return current_player_;
}

std::string TerraMysticaState::ActionToString(Player player,
                                            Action action_id) const {
  // TODO: Implement action string conversion
  return std::to_string(action_id);
}

std::string TerraMysticaState::ToString() const {
  // TODO: Implement game state string representation
  return "TerraMystica state string";
}

bool TerraMysticaState::IsTerminal() const {
  return current_round_ >= kMaxGameLength;
}

std::vector<double> TerraMysticaState::Returns() const {
  std::vector<double> returns(kNumPlayers, 0);
  if (IsTerminal()) {
    // Calculate final scores and normalize to [-1, 1]
    // TODO: Implement scoring
  }
  return returns;
}

std::string TerraMysticaState::InformationStateString(Player player) const {
  // Since this is a perfect information game, same as ToString()
  return ToString();
}

std::string TerraMysticaState::ObservationString(Player player) const {
  // Since this is a perfect information game, same as ToString()
  return ToString();
}

void TerraMysticaState::ObservationTensor(Player player,
                                         absl::Span<float> values) const {
  // TODO: Implement observation tensor
}

std::unique_ptr<State> TerraMysticaState::Clone() const {
  return std::unique_ptr<State>(new TerraMysticaState(*this));
}

bool TerraMysticaState::HasAdjacentBuilding(int hex_id, Player player) const {
  // Convert linear index to hex coordinates
  HexCoord coord = HexCoord::FromIndex(hex_id);
  
  // Get all adjacent hexes
  std::vector<HexCoord> adjacent = coord.GetAdjacent();
  
  // Check if player has a building in any adjacent hex
  for (const HexCoord& adj : adjacent) {
    int adj_index = HexCoord::ToIndex(adj.row, adj.col);
    if (adj_index != -1 && building_owners_[adj_index] == player) {
      return true;
    }
  }
  return false;
}

bool TerraMysticaState::HasIndirectConnection(int hex_id, Player player) const {
  // First check direct adjacency
  if (HasAdjacentBuilding(hex_id, player)) return true;
  
  // Get player's shipping level
  int shipping_level = player_resources_[player].shipping;
  if (shipping_level == 0) return false;  // No shipping capability
  
  // Get all hexes reachable through rivers within shipping distance
  std::vector<int> reachable_hexes = GetRiverConnectedHexes(hex_id, shipping_level);
  
  // Check if any reachable hex has a player's building
  for (int reachable_hex : reachable_hexes) {
    if (building_owners_[reachable_hex] == player) {
      return true;
    }
  }
  
  return false;
}

std::vector<int> TerraMysticaState::GetRiverConnectedHexes(int start_hex, int shipping_level) const {
  std::vector<int> connected_hexes;
  std::vector<bool> visited(kBoardSize, false);
  std::queue<std::pair<int, int>> to_visit;  // pair of (hex_id, distance)
  
  // Start BFS from the start hex
  to_visit.push({start_hex, 0});
  visited[start_hex] = true;
  
  while (!to_visit.empty()) {
    auto [current_hex, distance] = to_visit.front();
    to_visit.pop();
    
    // Don't exceed shipping level
    if (distance > shipping_level) continue;
    
    // Add this hex to connected hexes if it's not the start
    if (current_hex != start_hex) {
      connected_hexes.push_back(current_hex);
    }
    
    // Get adjacent hexes
    HexCoord current_coord = HexCoord::FromIndex(current_hex);
    std::vector<HexCoord> adjacent = current_coord.GetAdjacent();
    
    // Check each adjacent hex
    for (const HexCoord& adj : adjacent) {
      int adj_index = HexCoord::ToIndex(adj.row, adj.col);
      if (adj_index != -1 && !visited[adj_index]) {
        // Only follow river hexes
        if (terrain_[adj_index] == Terrain::kRiver) {
          to_visit.push({adj_index, distance + 1});
          visited[adj_index] = true;
        }
      }
    }
  }
  
  return connected_hexes;
}

PowerGainOption TerraMysticaState::GetPowerGainFromBuilding(Player player, int hex_id) const {
  // Calculate total power value from adjacent buildings
  int total_power_value = 0;
  
  // Check all adjacent hexes
  for (int adj_hex : GetAdjacentHexes(hex_id)) {
    if (adj_hex >= 0 && adj_hex < kBoardSize && 
        building_owners_[adj_hex] == player) {
      total_power_value += BuildingPowerValues::GetValue(board_[adj_hex]);
    }
  }
  
  // If no adjacent buildings, no power gain possible
  if (total_power_value == 0) {
    return PowerGainOption();
  }
  
  // Calculate power gain option based on bowls and VP
  return PowerHelper::CalculatePowerGainOption(
      player_resources_[player].power_bowls,
      total_power_value,
      player_resources_[player].victory_points);
}

std::vector<Action> TerraMysticaState::GetPowerGainActions(Player player, int hex_id) const {
  std::vector<Action> actions;
  
  // Get power gain option
  auto option = GetPowerGainFromBuilding(player, hex_id);
  
  // If no power available or can't afford, only option is to decline
  if (option.power_amount == 0) {
    Action decline;
    decline.type = ActionType::kDeclinePower;
    decline.hex_id = hex_id;
    decline.action_data = 0;
    actions.push_back(decline);
    return actions;
  }
  
  // Can either accept or decline
  Action accept;
  accept.type = ActionType::kAcceptPower;
  accept.hex_id = hex_id;
  accept.action_data = option.power_amount;  // Store power amount
  actions.push_back(accept);
  
  Action decline;
  decline.type = ActionType::kDeclinePower;
  decline.hex_id = hex_id;
  decline.action_data = 0;
  actions.push_back(decline);
  
  return actions;
}

void TerraMysticaState::HandlePowerFromBuilding(int hex_id) {
  // Find next player clockwise from current player
  next_power_player_ = (current_player_ + 1) % kNumPlayers;
  
  // Skip players until we find one with adjacent buildings
  while (next_power_player_ != current_player_) {
    auto option = GetPowerGainFromBuilding(next_power_player_, hex_id);
    if (option.power_amount > 0) {
      // Found a player who can gain power
      pending_power_hex_ = hex_id;
      return;
    }
    next_power_player_ = (next_power_player_ + 1) % kNumPlayers;
  }
  
  // No players can gain power
  pending_power_hex_ = -1;
  next_power_player_ = -1;
}

bool TerraMysticaState::CanBuildDwelling(int hex_id) const {
  const auto& resources = player_resources_[current_player_];
  const auto cost = BuildingCost::DwellingCost();
  
  return resources.workers >= cost.workers && 
         resources.coins >= cost.coins &&
         board_[hex_id] == Building::kNone;
}

void TerraMysticaState::BuildDwelling(int hex_id) {
  const auto cost = BuildingCost::DwellingCost();
  
  // Apply costs
  player_resources_[current_player_].workers -= cost.workers;
  player_resources_[current_player_].coins -= cost.coins;
  
  // Place the dwelling
  board_[hex_id] = Building::kDwelling;
  building_owners_[hex_id] = current_player_;
  
  // Handle power gains for other players
  HandlePowerFromBuilding(hex_id);
}

void TerraMysticaState::GainPower(Player player, int amount) {
  auto& resources = player_resources_[player];
  PowerHelper::GainPower(resources.power_bowls, amount);
}

bool TerraMysticaState::SpendPower(Player player, int amount) {
  auto& resources = player_resources_[player];
  return PowerHelper::SpendPower(resources.power_bowls, amount);
}

void TerraMysticaState::UpgradeBuilding(int hex_id, Building target_building) {
  Building current = board_[hex_id];
  const auto cost = BuildingHelper::GetUpgradeCost(current, target_building);
  
  // Apply costs
  player_resources_[current_player_].workers -= cost.workers;
  player_resources_[current_player_].coins -= cost.coins;
  
  // Upgrade the building
  board_[hex_id] = target_building;
  
  // Handle power gains for other players
  HandlePowerFromBuilding(hex_id);
}

std::vector<Action> TerraMysticaState::GetLegalTransformActions() const {
  std::vector<Action> actions;
  
  // Only allow terraforming during the action phase
  if (current_phase_ != Phase::kActions) return actions;
  
  // Check each hex for possible transformations
  for (int hex_id = 0; hex_id < kBoardSize; ++hex_id) {
    // Skip if there's a building or if it's a river
    if (board_[hex_id] != Building::kNone || terrain_[hex_id] == Terrain::kRiver) continue;
    
    // Check if we have a connection to this hex
    bool has_connection = HasIndirectConnection(hex_id, current_player_);
    if (!has_connection) continue;
    
    // Try each possible terrain type
    for (int t = 0; t < static_cast<int>(Terrain::kRiver); ++t) {
      Terrain target_terrain = static_cast<Terrain>(t);
      if (terrain_[hex_id] == target_terrain) continue;
      
      const auto& resources = player_resources_[current_player_];
      int transform_cost = TerraformHelper::GetTransformationCost(
          terrain_[hex_id],
          target_terrain,
          resources.terraforming);
          
      // Check if we can transform
      if (resources.workers >= transform_cost) {
        // Always add the transform-only action
        actions.push_back(Action::CreateTransformAction(hex_id, target_terrain));
        
        // If we have enough resources to also build, add that action too
        const auto dwelling_cost = BuildingCost::DwellingCost();
        if (resources.workers >= transform_cost + dwelling_cost.workers &&
            resources.coins >= dwelling_cost.coins) {
          actions.push_back(Action::CreateTransformBuildAction(hex_id, target_terrain));
        }
      }
    }
  }
  
  return actions;
}

std::vector<Action> TerraMysticaState::GetLegalUpgradeActions() const {
  std::vector<Action> actions;
  
  // Only allow upgrades during the action phase
  if (current_phase_ != Phase::kActions) return actions;
  
  // Check each hex for possible upgrades
  for (int hex_id = 0; hex_id < kBoardSize; ++hex_id) {
    // Skip if no building or not owned by current player
    if (board_[hex_id] == Building::kNone || 
        building_owners_[hex_id] != current_player_) continue;
    
    // Get possible upgrades for this building
    Building current = board_[hex_id];
    for (Building target : BuildingHelper::GetPossibleUpgrades(current)) {
      // Check if player has enough resources
      const auto cost = BuildingHelper::GetUpgradeCost(current, target);
      const auto& resources = player_resources_[current_player_];
      
      if (resources.workers >= cost.workers && 
          resources.coins >= cost.coins) {
        actions.push_back(Action::CreateUpgradeAction(hex_id, target));
      }
    }
  }
  
  return actions;
}

std::vector<Action> TerraMysticaState::GetLegalShippingActions() const {
  std::vector<Action> actions;
  const auto& resources = player_resources_[current_player_];
  const auto& faction = player_factions_[current_player_];
  
  if (ShippingHelper::CanAdvanceShipping(
      faction, 
      resources.shipping_level,
      resources.priests,
      resources.coins)) {
    Action action;
    action.type = ActionType::kUpgradeShipping;
    action.hex_id = 0;  // Not used for shipping
    action.action_data = 0;  // Not used for shipping
    actions.push_back(action);
  }
  
  return actions;
}

std::vector<Action> TerraMysticaState::GetLegalCultActions() const {
  std::vector<Action> actions;
  const auto& resources = player_resources_[current_player_];
  
  // Need at least 1 priest to advance on cult track
  if (resources.priests < 1) return actions;
  
  // Check each cult track
  for (int cult = 0; cult < 4; ++cult) {
    if (CultTrackHelper::CanAdvanceWithPriest(
        static_cast<CultType>(cult),
        cult_tracks_[cult][current_player_])) {
      Action action;
      action.type = ActionType::kSendPriest;
      action.hex_id = 0;  // Not used for cult
      action.action_data = cult;  // Which cult track
      actions.push_back(action);
    }
  }
  
  return actions;
}

std::vector<Action> TerraMysticaState::GetLegalExchangeActions() const {
  std::vector<Action> actions;
  const auto& resources = player_resources_[current_player_];
  const auto& faction = player_factions_[current_player_];
  
  if (ExchangeHelper::CanAdvanceExchange(
      faction,
      resources.terraforming,  // Current exchange level
      resources.workers,
      resources.coins,
      resources.priests)) {
    Action action;
    action.type = ActionType::kUpgradeExchange;
    action.hex_id = 0;  // Not used for exchange
    action.action_data = 0;  // Not used for exchange
    actions.push_back(action);
  }
  
  return actions;
}

std::vector<Action> TerraMysticaState::GetLegalPowerActions() const {
  std::vector<Action> actions;
  const auto& resources = player_resources_[current_player_];
  const auto& bowls = resources.power_bowls;
  
  // Check power-to-resource conversions
  if (PowerHelper::GetAvailablePower(bowls) >= PowerHelper::kPowerToPriestCost) {
    actions.push_back({ActionType::kPowerToPriest, -1, 0, false});
  }
  if (PowerHelper::GetAvailablePower(bowls) >= PowerHelper::kPowerToWorkerCost) {
    actions.push_back({ActionType::kPowerToWorker, -1, 0, false});
  }
  if (PowerHelper::GetAvailablePower(bowls) >= PowerHelper::kPowerToCoinCost) {
    actions.push_back({ActionType::kPowerToCoin, -1, 0, false});
  }
  
  // Check resource-to-resource conversions
  if (resources.priests > 0) {
    actions.push_back({ActionType::kPriestToWorker, -1, 0, false});
  }
  if (resources.workers > 0) {
    actions.push_back({ActionType::kWorkerToCoin, -1, 0, false});
  }
  
  // Check if we can burn power
  int bowl2_power = PowerHelper::GetBowl2Power(bowls);
  if (bowl2_power >= 2) {  // Need at least 2 tokens to burn power
    // Can burn up to bowl2_power/2 tokens (half go to bowl3, half removed)
    for (int i = 1; i <= bowl2_power/2; i++) {
      actions.push_back({ActionType::kBurnPower, -1, i, false});
    }
  }
  
  // Check power actions
  int available_power = PowerHelper::GetAvailablePower(bowls);
  
  // Bridge action
  if (!used_power_actions_[static_cast<int>(PowerActionType::kBridge)] &&
      available_power >= PowerActionCosts::kBridge) {
    // Find all possible bridge locations
    for (int hex1 = 0; hex1 < kBoardSize; hex1++) {
      for (int hex2 = hex1 + 1; hex2 < kBoardSize; hex2++) {
        if (CanBuildBridge(hex1, hex2, current_player_)) {
          // Pack both hex IDs into hex_id field (hex2 in upper 16 bits)
          int packed_hexes = hex1 | (hex2 << 16);
          actions.push_back({
            ActionType::kPowerAction,
            packed_hexes,
            static_cast<int>(PowerActionType::kBridge),
            false
          });
        }
      }
    }
  }
  
  // Priest action
  if (!used_power_actions_[static_cast<int>(PowerActionType::kPriest)] &&
      available_power >= PowerActionCosts::kPriest &&
      GetTotalPriests(current_player_) < kMaxPriests) {
    actions.push_back({
      ActionType::kPowerAction,
      -1,
      static_cast<int>(PowerActionType::kPriest),
      false
    });
  }
  
  // Workers action
  if (!used_power_actions_[static_cast<int>(PowerActionType::kWorkers)] &&
      available_power >= PowerActionCosts::kWorkers) {
    actions.push_back({
      ActionType::kPowerAction,
      -1,
      static_cast<int>(PowerActionType::kWorkers),
      false
    });
  }
  
  // Coins action
  if (!used_power_actions_[static_cast<int>(PowerActionType::kCoins)] &&
      available_power >= PowerActionCosts::kCoins) {
    actions.push_back({
      ActionType::kPowerAction,
      -1,
      static_cast<int>(PowerActionType::kCoins),
      false
    });
  }
  
  // Spade actions
  if (!used_power_actions_[static_cast<int>(PowerActionType::kSingleSpade)] &&
      available_power >= PowerActionCosts::kSingleSpade) {
    actions.push_back({
      ActionType::kPowerAction,
      -1,
      static_cast<int>(PowerActionType::kSingleSpade),
      false
    });
  }
  if (!used_power_actions_[static_cast<int>(PowerActionType::kDoubleSpade)] &&
      available_power >= PowerActionCosts::kDoubleSpade) {
    actions.push_back({
      ActionType::kPowerAction,
      -1,
      static_cast<int>(PowerActionType::kDoubleSpade),
      false
    });
  }
  
  return actions;
}

std::vector<Action> TerraMysticaState::GetLegalSpadeActions(int spades, bool can_build) const {
  std::vector<Action> actions;
  const auto& faction = GetFaction(current_player_);
  
  // For each hex on the board
  for (int hex = 0; hex < kBoardSize; hex++) {
    // Skip if we can't transform this hex
    if (!CanTransformTerrain(hex)) {
      continue;
    }
    
    // Get cost to transform to home terrain
    int transform_cost = GetTransformCost(terrain_[hex], faction.home_terrain);
    if (transform_cost <= spades) {
      // Can transform to home terrain
      if (can_build) {
        actions.push_back(Action::CreateTransformBuildAction(hex, faction.home_terrain));
      } else {
        actions.push_back(Action::CreateTransformAction(hex, faction.home_terrain));
      }
    } else if (faction.faction_id == FactionId::kDarklings) {
      // Darklings can pay priests for missing spades
      int missing_spades = transform_cost - spades;
      if (player_resources_[current_player_].priests >= missing_spades) {
        if (can_build) {
          actions.push_back(Action::CreateTransformBuildAction(hex, faction.home_terrain));
        } else {
          actions.push_back(Action::CreateTransformAction(hex, faction.home_terrain));
        }
      }
    } else {
      // Other factions can pay workers for missing spades
      int missing_spades = transform_cost - spades;
      int worker_cost = missing_spades * GetExchangeRate();
      if (player_resources_[current_player_].workers >= worker_cost) {
        if (can_build) {
          actions.push_back(Action::CreateTransformBuildAction(hex, faction.home_terrain));
        } else {
          actions.push_back(Action::CreateTransformAction(hex, faction.home_terrain));
        }
      }
    }
  }
  
  // For double spade action, allow transforming a second hex if first hex used exactly one spade
  if (spades == 2 && !actions.empty()) {
    for (int hex = 0; hex < kBoardSize; hex++) {
      if (!CanTransformTerrain(hex)) {
        continue;
      }
      
      // Can only transform one step with second spade
      for (Terrain target : GetAdjacentTerrains(terrain_[hex])) {
        if (target == faction.home_terrain) {
          // Can't build on second hex
          actions.push_back(Action::CreateTransformAction(hex, target));
        }
      }
    }
  }
  
  return actions;
}

void TerraMysticaState::DoApplyAction(Action action_id) {
  if (action_id.type == ActionType::kAcceptPower) {
    auto& resources = player_resources_[current_player_];
    auto option = GetPowerGainFromBuilding(current_player_, action_id.hex_id);
    
    // Pay VP cost
    resources.victory_points -= option.victory_point_cost;
    
    // Gain power
    GainPower(current_player_, option.power_amount);
    
    // Move to next player
    current_player_ = (current_player_ + 1) % kNumPlayers;
    
    // Clear power gain state if we're done
    if (current_player_ == next_power_player_) {
      pending_power_hex_ = -1;
      next_power_player_ = -1;
    }
  } else if (action_id.type == ActionType::kDeclinePower) {
    // Move to next player
    current_player_ = (current_player_ + 1) % kNumPlayers;
    
    // Clear power gain state if we're done
    if (current_player_ == next_power_player_) {
      pending_power_hex_ = -1;
      next_power_player_ = -1;
    }
  } else if (action_id.type == ActionType::kPowerToPriest) {
    auto& resources = player_resources_[current_player_];
    if (SpendPower(current_player_, PowerHelper::kPowerToPriestCost)) {
      resources.priests++;
    }
    EndTurn();  // Power to priest ends turn
  } else if (action_id.type == ActionType::kPowerToWorker) {
    auto& resources = player_resources_[current_player_];
    if (SpendPower(current_player_, PowerHelper::kPowerToWorkerCost)) {
      resources.workers++;
    }
    EndTurn();  // Power to worker ends turn
  } else if (action_id.type == ActionType::kPowerToCoin) {
    auto& resources = player_resources_[current_player_];
    if (SpendPower(current_player_, PowerHelper::kPowerToCoinCost)) {
      resources.coins++;
    }
    EndTurn();  // Power to coin ends turn
  } else if (action_id.type == ActionType::kPriestToWorker) {
    auto& resources = player_resources_[current_player_];
    if (resources.priests > 0) {
      resources.priests--;
      resources.workers++;
    }
    EndTurn();  // Priest to worker ends turn
  } else if (action_id.type == ActionType::kWorkerToCoin) {
    auto& resources = player_resources_[current_player_];
    if (resources.workers > 0) {
      resources.workers--;
      resources.coins++;
    }
    EndTurn();  // Worker to coin ends turn
  } else if (action_id.type == ActionType::kBurnPower) {
    auto& bowls = player_resources_[current_player_].power_bowls;
    PowerHelper::BurnPower(bowls, action_id.action_data);  // action_data contains amount to burn
    EndTurn();  // Burn power ends turn
  } else if (action_id.type == ActionType::kPowerAction) {
    auto power_action = static_cast<PowerActionType>(action_id.action_data);
    auto& resources = player_resources_[current_player_];
    
    // Check if action is available
    int action_index = static_cast<int>(power_action);
    if (used_power_actions_[action_index]) {
      return;  // Action already used this round
    }
    
    // Handle different power actions
    switch (power_action) {
      case PowerActionType::kBridge: {
        // Extract hex IDs from action
        int hex1 = action_id.hex_id;
        int hex2 = action_id.hex_id >> 16;  // Upper 16 bits
        
        if (SpendPower(current_player_, PowerActionCosts::kBridge)) {
          // Add bridge to game state
          bridges_.push_back({hex1, hex2, current_player_});
          used_power_actions_[action_index] = true;
          EndTurn();  // Bridge action ends turn
        }
        break;
      }
      case PowerActionType::kPriest: {
        if (GetTotalPriests(current_player_) < kMaxPriests &&
            SpendPower(current_player_, PowerActionCosts::kPriest)) {
          resources.priests++;
          used_power_actions_[action_index] = true;
          EndTurn();  // Priest action ends turn
        }
        break;
      }
      case PowerActionType::kWorkers: {
        if (SpendPower(current_player_, PowerActionCosts::kWorkers)) {
          resources.workers += 2;  // Gain 2 workers
          used_power_actions_[action_index] = true;
          EndTurn();  // Workers action ends turn
        }
        break;
      }
      case PowerActionType::kCoins: {
        if (SpendPower(current_player_, PowerActionCosts::kCoins)) {
          resources.coins += 7;  // Gain 7 coins
          used_power_actions_[action_index] = true;
          EndTurn();  // Coins action ends turn
        }
        break;
      }
      case PowerActionType::kSingleSpade: {
        if (SpendPower(current_player_, PowerActionCosts::kSingleSpade)) {
          pending_spades_ = 1;
          can_build_after_spade_ = true;
          used_power_actions_[action_index] = true;
          // Don't end turn - must use spade immediately
        }
        break;
      }
      case PowerActionType::kDoubleSpade: {
        if (SpendPower(current_player_, PowerActionCosts::kDoubleSpade)) {
          pending_spades_ = 2;
          can_build_after_spade_ = true;  // Can only build on first hex
          used_power_actions_[action_index] = true;
          // Don't end turn - must use spades immediately
        }
        break;
      }
    }
  } else if (action_id.type == ActionType::kSpadeAction) {
    // Handle spade action from power action
    if (pending_spades_ > 0) {
      if (action_id.transform_build) {
        // Transform and build (only allowed on first hex with double spades)
        TransformTerrain(action_id.hex_id, action_id.target_terrain);
        BuildDwelling(action_id.hex_id);
        pending_spades_--;
        can_build_after_spade_ = false;  // Can't build on second hex
      } else {
        // Just transform
        TransformTerrain(action_id.hex_id, action_id.target_terrain);
        pending_spades_--;
      }
      
      // End turn after all spades are used
      if (pending_spades_ == 0) {
        EndTurn();
      }
    }
  } else if (action_id.type == ActionType::kTransform || 
      action_id.type == ActionType::kTransformBuild) {
    int hex_id = action_id.hex_id;
    Terrain target_terrain = static_cast<Terrain>(action_id.action_data);
    
    // Calculate and apply transformation cost
    int transform_cost = TerraformHelper::GetTransformationCost(
        terrain_[hex_id],
        target_terrain,
        player_resources_[current_player_].terraforming);
        
    player_resources_[current_player_].workers -= transform_cost;
    
    // Transform the terrain
    terrain_[hex_id] = target_terrain;
    
    // If this is a transform-and-build action, also build the dwelling
    if (action_id.type == ActionType::kTransformBuild) {
      BuildDwelling(hex_id);
    }
  } else if (action_id.type == ActionType::kUpgrade) {
    int hex_id = action_id.hex_id;
    Building target_building = static_cast<Building>(action_id.action_data);
    UpgradeBuilding(hex_id, target_building);
  } else if (action_id.type == ActionType::kUpgradeShipping) {
    auto& resources = player_resources_[current_player_];
    const auto& faction = player_factions_[current_player_];
    
    // Pay costs
    resources.priests -= ShippingConstants::kBaseCostPriest;
    resources.coins -= ShippingConstants::kBaseCostCoins;
    
    // Get points
    int points = ShippingHelper::GetAdvancementPoints(
        resources.shipping_level, faction);
    resources.victory_points += points;
    
    // Increase shipping level
    resources.shipping_level++;
  } else if (action_id.type == ActionType::kSendPriest) {
    auto& resources = player_resources_[current_player_];
    int cult_track = action_id.action_data;
    
    // Pay priest
    resources.priests--;
    
    // Advance on cult track and get power
    int power_gained = CultTrackHelper::AdvanceWithPriest(
        static_cast<CultType>(cult_track),
        cult_tracks_[cult_track][current_player_]);
    GainPower(current_player_, power_gained);
    
    // Update cult track position
    cult_tracks_[cult_track][current_player_] += 
        CultTrackHelper::GetAdvancementSpaces(
            cult_tracks_[cult_track][current_player_]);
  } else if (action_id.type == ActionType::kUpgradeExchange) {
    auto& resources = player_resources_[current_player_];
    const auto& faction = player_factions_[current_player_];
    
    // Pay costs
    resources.workers -= ExchangeHelper::GetWorkerCost(faction);
    resources.coins -= ExchangeHelper::GetCoinCost(faction);
    resources.priests -= ExchangeHelper::GetPriestCost(faction);
    
    // Get victory points
    resources.victory_points += ExchangeHelper::GetUpgradePoints();
    
    // Increase terraforming level
    resources.terraforming++;
  }
}

std::vector<Action> TerraMysticaState::LegalActions() const {
  std::vector<Action> actions;
  
  if (IsTerminal()) return actions;
  
  // Always allow auxiliary power actions during your turn
  if (current_player_ != -1) {
    auto power_actions = GetLegalPowerActions();
    actions.insert(actions.end(), power_actions.begin(), power_actions.end());
  }
  
  // If there's a pending power gain decision, only those actions are legal
  if (pending_power_hex_ >= 0 && current_player_ == next_power_player_) {
    return GetPowerGainActions(current_player_, pending_power_hex_);
  }
  
  // Add transform actions
  auto transform_actions = GetLegalTransformActions();
  actions.insert(actions.end(), transform_actions.begin(), transform_actions.end());
  
  // Add upgrade actions
  auto upgrade_actions = GetLegalUpgradeActions();
  actions.insert(actions.end(), upgrade_actions.begin(), upgrade_actions.end());
  
  // Add shipping upgrade actions
  auto shipping_actions = GetLegalShippingActions();
  actions.insert(actions.end(), shipping_actions.begin(), shipping_actions.end());
  
  // Add cult advancement actions
  auto cult_actions = GetLegalCultActions();
  actions.insert(actions.end(), cult_actions.begin(), cult_actions.end());
  
  // Add exchange track upgrade actions
  auto exchange_actions = GetLegalExchangeActions();
  actions.insert(actions.end(), exchange_actions.begin(), exchange_actions.end());
  
  // Add spade actions
  if (pending_spades_ > 0) {
    auto spade_actions = GetLegalSpadeActions(pending_spades_, can_build_after_spade_);
    actions.insert(actions.end(), spade_actions.begin(), spade_actions.end());
  }
  
  return actions;
}

int TerraMysticaState::GetTotalPriests(Player player) const {
  int total = player_resources_[player].priests;
  
  // Add priests from cult tracks
  for (const auto& track : cult_tracks_) {
    total += track[player];  // Each step on cult track uses a priest
  }
  
  return total;
}

int TerraMysticaState::GetPlayerBridgeCount(Player player) const {
  int count = 0;
  for (const auto& bridge : bridges_) {
    if (bridge.owner == player) {
      count++;
    }
  }
  return count;
}

bool TerraMysticaState::CanBuildBridge(int hex1, int hex2, Player player) const {
  // Check if player has reached bridge limit
  if (GetPlayerBridgeCount(player) >= kMaxBridgesPerPlayer) {
    return false;
  }

  // Check if hexes are valid
  if (hex1 < 0 || hex1 >= kBoardSize || hex2 < 0 || hex2 >= kBoardSize) {
    return false;
  }
  
  // Check if there's already a bridge here
  if (HasBridge(hex1, hex2)) {
    return false;
  }
  
  // Check if hexes are adjacent with a river between them
  HexCoord coord1 = HexCoord::FromIndex(hex1);
  HexCoord coord2 = HexCoord::FromIndex(hex2);
  if (!coord1.IsAdjacentTo(coord2)) {
    return false;
  }
  
  // Find the river hex between them
  std::vector<HexCoord> between = coord1.GetHexesBetween(coord2);
  if (between.empty()) {
    return false;
  }
  int river_hex = HexCoord::ToIndex(between[0]);
  if (terrain_[river_hex] != Terrain::kRiver) {
    return false;
  }
  
  // Check if player has a building on at least one side
  if (building_owners_[hex1] != player && building_owners_[hex2] != player) {
    return false;
  }
  
  return true;
}

bool TerraMysticaState::HasBridge(int hex1, int hex2) const {
  for (const auto& bridge : bridges_) {
    if ((bridge.hex1 == hex1 && bridge.hex2 == hex2) ||
        (bridge.hex1 == hex2 && bridge.hex2 == hex1)) {
      return true;
    }
  }
  return false;
}

TerraMysticaGame::TerraMysticaGame(const GameParameters& params)
    : Game(kGameType, params) {}

int TerraMysticaGame::NumDistinctActions() const {
  // Calculate total number of possible actions
  // This includes terrain transformations, building, upgrades, 
  // cult track actions, power actions, and passing
  return 200;  // Approximate number, needs to be calculated precisely
}

std::unique_ptr<State> TerraMysticaGame::NewInitialState() const {
  return std::unique_ptr<State>(new TerraMysticaState(shared_from_this()));
}

std::vector<int> TerraMysticaGame::ObservationTensorShape() const {
  // Define the shape of the observation tensor
  // This should include:
  // - Board state (terrain and buildings)
  // - Player resources
  // - Cult track positions
  // - Available actions
  return {
      kBoardSize * (kNumTerrainTypes + 6),  // Board state (terrain + buildings)
      kNumPlayers * 6,  // Resources for each player
      4 * kNumPlayers,  // Cult track positions
      1  // Current phase
  };
}

}  // namespace terra_mystica
}  // namespace open_spiel
