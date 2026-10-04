#pragma once

#include <string>

namespace bitwave::auth {

class AliasService {
public:
    virtual ~AliasService() = default;

    // UX convenience only — not the safety check. See AliasServiceImpl.cpp.
    virtual bool isAliasAvailable(const std::string& alias) const = 0;

    // Returns true on success. False if invalid, taken, or no one is logged in.
    virtual bool setAlias(const std::string& alias) = 0;
};

} // namespace bitwave::auth