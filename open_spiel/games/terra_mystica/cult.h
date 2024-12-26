#ifndef OPEN_SPIEL_GAMES_TERRA_MYSTICA_CULT_H_
#define OPEN_SPIEL_GAMES_TERRA_MYSTICA_CULT_H_

#include <array>
#include <vector>
#include "open_spiel/spiel.h"

namespace open_spiel {
namespace terra_mystica {

// Types of cult tracks
enum class CultType {
  kFire,
  kWater,
  kEarth,
  kAir
};

// Represents a single priest spot on a cult track
struct CultSpot {
  int advancement;  // How many steps this spot advances (2 or 3)
  int player_id;    // ID of player occupying this spot (-1 if empty)
};

// Represents a player's progress on a cult track
struct CultProgress {
  int level;        // Current level (0-10)
  bool has_key;     // Whether player has key to reach level 10
};

// Class to manage a single cult track
class CultTrack {
 public:
  static constexpr int kMaxLevel = 10;
  static constexpr int kNumPriestSpots = 4;
  
  CultTrack();
  
  // Place a priest and advance on track
  // Returns power gained from advancement
  int PlacePriest(int player_id, bool sacrifice_priest);
  
  // Try to advance on track without placing priest
  // Returns false if advancement not possible (e.g., trying to reach 10 without key)
  bool TryAdvance(int player_id, int steps);
  
  // Get available priest spots
  std::vector<int> GetAvailableSpots() const;
  
  // Get player's current progress
  const CultProgress& GetProgress(int player_id) const;
  
  // Check if player can advance to next level
  bool CanAdvance(int player_id, int steps) const;
  
  // Get power gained from advancing to a specific level
  static int GetPowerGain(int from_level, int to_level);
  
 private:
  std::array<CultSpot, kNumPriestSpots> priest_spots_;
  std::vector<CultProgress> player_progress_;
  
  // Initialize priest spots with their advancement values
  void InitializePriestSpots();
  
  // Calculate power gained from advancement
  int CalculatePowerGain(int player_id, int old_level, int new_level) const;
};

// Manager class for all cult tracks
class CultManager {
 public:
  CultManager(int num_players);
  
  // Place a priest on a cult track
  // Returns power gained from the action
  int PlacePriest(int player_id, CultType cult_type, int spot_index, bool sacrifice_priest);
  
  // Get available priest spots for a cult track
  std::vector<int> GetAvailableSpots(CultType cult_type) const;
  
  // Get player's progress on a specific cult track
  const CultProgress& GetProgress(int player_id, CultType cult_type) const;
  
  // Give a key to a player for a specific cult track
  void GiveKey(int player_id, CultType cult_type);
  
  // Get all cult tracks
  const std::array<CultTrack, 4>& GetTracks() const { return tracks_; }
  
 private:
  std::array<CultTrack, 4> tracks_;  // One track for each cult type
};

}  // namespace terra_mystica
}  // namespace open_spiel

#endif  // OPEN_SPIEL_GAMES_TERRA_MYSTICA_CULT_H_
