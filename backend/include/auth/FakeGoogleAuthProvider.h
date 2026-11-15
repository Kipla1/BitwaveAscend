#pragma once

#include <string>

#include "auth/GoogleAuthProvider.h"

namespace bitwave::auth {

// Test double: no browser, no network. Set the public fields to script what
// the Google OAuth flow returns in unit tests.
class FakeGoogleAuthProvider : public GoogleAuthProvider {
public:
    bool        failAuth = false;
    std::string googleSub = "google-sub-12345";
    std::string email = "oscar@example.com";
    std::string error = "Google sign-in was cancelled.";

    GoogleIdentity authenticate() override {
        GoogleIdentity out;
        if (failAuth) {
            out.error = error;
            return out;
        }
        out.ok = true;
        out.googleSub = googleSub;
        out.email = email;
        return out;
    }
};

} // namespace bitwave::auth