#pragma once

#include "entities/Entity.h"

namespace bitwave::gameplay {

class Physics {
public:
    // Integrate velocity and gravity in world units per second.
    static bool step(entities::Entity& body, float dt, float gravity,
                     float worldWidth = 800, float worldHeight = 600);
};

} // namespace bitwave::gameplay
