#pragma once

#include <string>

namespace bitwave::auth {

// Wraps libsodium's Argon2id password hashing so no other file
// ever includes <sodium.h> directly.
class PasswordHasher {
public:
    PasswordHasher();                       // calls sodium_init()

    // Returns an encoded hash string (salt + params included), or "" on failure.
    std::string hash(const std::string& password) const;

    // True only if `password` matches `storedHash`.
    bool verify(const std::string& storedHash, const std::string& password) const;
};

} // namespace bitwave::auth