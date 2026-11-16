#pragma once

#include "persistence/PlayerProfileRepository.h"

namespace bitwave::persistence {

// Compatibility name for code that treats a player's progress as a record.
using PlayerRepository = PlayerProfileRepository;

} // namespace bitwave::persistence
