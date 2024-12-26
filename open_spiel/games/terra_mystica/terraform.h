#ifndef OPEN_SPIEL_GAMES_TERRA_MYSTICA_TERRAFORM_H_
#define OPEN_SPIEL_GAMES_TERRA_MYSTICA_TERRAFORM_H_

#include <vector>
#include "open_spiel/spiel.h"

namespace open_spiel {
namespace terra_mystica {

// Forward declarations
class FactionInfo;

// Terrain types
enum class Terrain {
  kPlains = 0,
  kSwamp,
  kLakes,
  kForest,
  kMountains,
  kWasteland,
  kDesert,
  kRiver  // Special terrain type for shipping
};

// Terraforming costs between terrain types
struct TerrainDistance {
  static int GetDistance(Terrain from, Terrain to);
  static int GetWorkerCost(int distance, int terraforming_level,
                          const FactionInfo& faction,
                          Terrain from_terrain);
};

// Terraforming helper class
class TerraformHelper {
 public:
  // Check if a hex can be transformed
  static bool CanTransformTerrain(
      int hex_id,
      Terrain current_terrain,
      Terrain target_terrain,
      int terraforming_level,
      int available_workers,
      const FactionInfo& faction,
      bool has_connection);

  // Get the worker cost for a transformation
  static int GetTransformationCost(
      Terrain from,
      Terrain to,
      int terraforming_level,
      const FactionInfo& faction);

  // Check if terrain type is transformable
  static bool IsTransformable(Terrain terrain) {
    return terrain != Terrain::kRiver;
  }
};

}  // namespace terra_mystica
}  // namespace open_spiel

#endif  // OPEN_SPIEL_GAMES_TERRA_MYSTICA_TERRAFORM_H_
