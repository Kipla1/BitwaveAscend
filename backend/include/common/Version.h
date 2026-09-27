#pragma once

namespace bitwave::common {

// Simple compiled translation unit used to verify that the backend library
// actually configures, compiles and links during repository initialization.
const char* backendVersion();

} // namespace bitwave::common
