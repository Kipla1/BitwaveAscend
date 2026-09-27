#pragma once

#include <cstdint>

// Basic backend-wide type aliases.
// Kept minimal and dependency-free during repository initialization.

namespace bitwave::common {

using EntityId  = std::int64_t;
using UserId     = std::int64_t;
using ItemId     = std::int64_t;
using Currency   = std::int64_t; // smallest in-game currency unit (avoid floating point money)
using Score      = std::int64_t;

} // namespace bitwave::common
