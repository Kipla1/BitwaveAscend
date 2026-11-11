#pragma once

#include "entities/Entity.h"

namespace bitwave::gameplay {

class Collision {
public:
    static bool overlaps(const entities::Entity& a, const entities::Entity& b) {
        return a.overlaps(b);
    }
};

} // namespace bitwave::gameplay
