#pragma once

#include <string>

namespace bitwave::auth {

// Wraps libsodium's Argon2id so no other file includes <sodium.h>.
class PasswordHasher {
public:
    PasswordHasher(); // initializes libsodium; throws std::runtime_error on failure

    // Returns an encoded hash (salt + parameters included), or "" on failure.
    std::string hash(const std::string& password) const;

    // True only if `password` matches `storedHash`.
    bool verify(const std::string& storedHash, const std::string& password) const;
};

} // namespace bitwave::auth