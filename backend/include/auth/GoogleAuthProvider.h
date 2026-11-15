#pragma once

#include <string>

namespace bitwave::auth {

struct GoogleIdentity {
    bool        ok = false;
    std::string googleSub; // Google's stable unique account ID ("sub" claim)
    std::string email;
    std::string error;
};

// Boundary for Google OAuth sign-in. The real implementation opens the
// system browser, listens on a loopback port for the redirect, and exchanges
// the authorization code over HTTP; tests use FakeGoogleAuthProvider.
class GoogleAuthProvider {
public:
    virtual ~GoogleAuthProvider() = default;

    virtual GoogleIdentity authenticate() = 0;
};

} // namespace bitwave::auth