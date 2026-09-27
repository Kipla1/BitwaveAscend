#pragma once

// Backend-internal enums that are not part of the frontend/backend shared
// contract (see shared/include/shared/Contracts.h for the shared ones).
// Placeholder only — extend as real systems are implemented.

namespace bitwave::common {

enum class LogLevel {
    Trace,
    Debug,
    Info,
    Warn,
    Error
};

} // namespace bitwave::common
