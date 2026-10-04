#include "auth/AliasServiceImpl.h"

#include <algorithm>
#include <cctype>

namespace bitwave::auth {

namespace {

constexpr std::size_t kMinAliasLength = 3;
constexpr std::size_t kMaxAliasLength = 20;

bool isValidAlias(const std::string& alias) {
    if (alias.size() < kMinAliasLength || alias.size() > kMaxAliasLength) {
        return false;
    }
    return std::all_of(alias.begin(), alias.end(), [](unsigned char c) {
        return std::isalnum(c) || c == '_';
    });
}

} // namespace

AliasServiceImpl::AliasServiceImpl(persistence::UserRepository& users, Session& session)
    : users_(users), session_(session) {}

bool AliasServiceImpl::isAliasAvailable(const std::string& alias) const {
    return isValidAlias(alias) && users_.isAliasAvailable(alias);
}

bool AliasServiceImpl::setAlias(const std::string& alias) {
    if (!session_.isActive() || !isValidAlias(alias)) {
        return false;
    }
    const bool changed = users_.updateAlias(session_.user()->id, alias);
    if (changed) {
        // Keep the session's cached alias in sync with what was just saved.
        SessionUser updated = *session_.user();
        updated.alias = alias;
        session_.start(std::move(updated));
    }
    return changed;
}

} // namespace bitwave::auth