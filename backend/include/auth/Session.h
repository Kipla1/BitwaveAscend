#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <utility>

namespace bitwave::auth {

// The logged-in player, as the rest of the backend needs to know them.
// Deliberately excludes the password hash: nothing outside auth ever needs it.
struct SessionUser {
    std::int64_t id = 0;
    std::string  username;
    std::string  alias;
};

// Holds "who is logged in right now". AuthServiceImpl writes to it;
// everything else (wallet, profile, store) should only read it.
class Session {
public:
    void start(SessionUser user) { user_ = std::move(user); }
    void end() { user_.reset(); }

    bool isActive() const { return user_.has_value(); }

    // Empty when nobody is logged in.
    const std::optional<SessionUser>& user() const { return user_; }

private:
    std::optional<SessionUser> user_;
};

} // namespace bitwave::auth