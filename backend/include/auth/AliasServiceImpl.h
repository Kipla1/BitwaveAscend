#pragma once

#include "auth/AliasService.h"
#include "auth/Session.h"
#include "persistence/UserRepository.h"

namespace bitwave::auth {

class AliasServiceImpl : public AliasService {
public:
    // Both references must outlive this object.
    AliasServiceImpl(persistence::UserRepository& users, Session& session);

    bool isAliasAvailable(const std::string& alias) const override;
    bool setAlias(const std::string& alias) override;

private:
    persistence::UserRepository& users_;
    Session& session_;
};

} // namespace bitwave::auth