#include "open_spiel/games/terra_mystica/cult.h"

namespace open_spiel {
namespace terra_mystica {

CultTrack::CultTrack() {
  InitializePriestSpots();
}

void CultTrack::InitializePriestSpots() {
  // First spot advances 3 spaces, all others advance 2 spaces
  priest_spots_[0] = {3, -1};  // -1 means unoccupied
  priest_spots_[1] = {2, -1};
  priest_spots_[2] = {2, -1};
  priest_spots_[3] = {2, -1};
}

std::vector<int> CultTrack::GetAvailableSpots() const {
  std::vector<int> available;
  for (int i = 0; i < kNumPriestSpots; ++i) {
    if (priest_spots_[i].player_id == -1) {
      available.push_back(i);
    }
  }
  return available;
}

const CultProgress& CultTrack::GetProgress(int player_id) const {
  // Ensure player_progress_ has an entry for this player
  while (static_cast<int>(player_progress_.size()) <= player_id) {
    player_progress_.push_back({0, false});  // Start at level 0 with no key
  }
  return player_progress_[player_id];
}

int CultTrack::GetPowerGain(int from_level, int to_level) {
  int power = 0;
  for (int level = from_level + 1; level <= to_level; ++level) {
    switch (level) {
      case 3: power += 1; break;  // 2->3 gives 1 power
      case 5: power += 2; break;  // 4->5 gives 2 power
      case 7: power += 2; break;  // 6->7 gives 2 power
      case 10: power += 3; break; // 9->10 gives 3 power
    }
  }
  return power;
}

bool CultTrack::CanAdvance(int player_id, int steps) const {
  const CultProgress& progress = GetProgress(player_id);
  int new_level = progress.level + steps;
  
  // Without a key, can only advance up to level 9
  if (!progress.has_key && new_level > 9) {
    return false;
  }
  
  // Even with a key, can't go beyond max level
  if (new_level > kMaxLevel) {
    return false;
  }
  
  return true;
}

int CultTrack::CalculatePowerGain(int player_id, int old_level, int new_level) const {
  return GetPowerGain(old_level, new_level);
}

bool CultTrack::TryAdvance(int player_id, int steps) {
  const CultProgress& progress = GetProgress(player_id);
  
  // If we can't make the full advancement, try partial advancement
  if (!CanAdvance(player_id, steps)) {
    // Without key, can advance up to level 9
    if (!progress.has_key && progress.level < 9) {
      steps = std::min(steps, 9 - progress.level);
    } else {
      return false;
    }
  }
  
  // Ensure we have space for this player
  while (static_cast<int>(player_progress_.size()) <= player_id) {
    player_progress_.push_back({0, false});
  }
  
  int old_level = player_progress_[player_id].level;
  player_progress_[player_id].level += steps;
  
  return true;
}

int CultTrack::PlacePriest(int player_id, bool sacrifice_priest) {
  // For sacrificing priest (returning to supply)
  if (sacrifice_priest) {
    if (!CanAdvance(player_id, 1)) return 0;
    
    int old_level = GetProgress(player_id).level;
    TryAdvance(player_id, 1);
    return CalculatePowerGain(player_id, old_level, old_level + 1);
  }
  
  // Check if any spots are available
  auto available = GetAvailableSpots();
  if (available.empty()) return 0;
  
  // Find best available spot
  int best_spot = available[0];
  int max_advancement = priest_spots_[best_spot].advancement;
  
  for (int spot : available) {
    if (priest_spots_[spot].advancement > max_advancement) {
      best_spot = spot;
      max_advancement = priest_spots_[spot].advancement;
    }
  }
  
  // Check if we can advance
  if (!CanAdvance(player_id, max_advancement)) return 0;
  
  // Place priest and advance
  priest_spots_[best_spot].player_id = player_id;
  int old_level = GetProgress(player_id).level;
  TryAdvance(player_id, max_advancement);
  
  return CalculatePowerGain(player_id, old_level, old_level + max_advancement);
}

CultManager::CultManager(int num_players) {
  // Initialize all tracks
  for (auto& track : tracks_) {
    track = CultTrack();
  }
}

int CultManager::PlacePriest(int player_id, CultType cult_type, int spot_index,
                           bool sacrifice_priest) {
  return tracks_[static_cast<int>(cult_type)].PlacePriest(player_id, sacrifice_priest);
}

std::vector<int> CultManager::GetAvailableSpots(CultType cult_type) const {
  return tracks_[static_cast<int>(cult_type)].GetAvailableSpots();
}

const CultProgress& CultManager::GetProgress(int player_id, CultType cult_type) const {
  return tracks_[static_cast<int>(cult_type)].GetProgress(player_id);
}

void CultManager::GiveKey(int player_id, CultType cult_type) {
  auto& track = tracks_[static_cast<int>(cult_type)];
  while (static_cast<int>(track.GetProgress(player_id).level) <= player_id) {
    track.player_progress_.push_back({0, false});
  }
  track.player_progress_[player_id].has_key = true;
}

}  // namespace terra_mystica
}  // namespace open_spiel
