#include "gameplay/Physics.h"

#include <algorithm>
#include <stdexcept>

namespace bitwave::gameplay {

bool Physics::step(entities::Entity& body, float dt, float gravity,
                   float worldWidth, float worldHeight) {
    if (dt < 0) throw std::invalid_argument("delta time cannot be negative");
    auto p = body.position();
    auto v = body.velocity();
    v.y += gravity * dt;
    p.x += v.x * dt;
    p.y += v.y * dt;
    const auto size = body.size();
    const float maxX = std::max(0.0f, worldWidth-size.x);
    const float maxY = std::max(0.0f, worldHeight-size.y);
    p.x = std::clamp(p.x, 0.0f, maxX);
    const bool hitFloor = p.y >= maxY;
    if (hitFloor) { p.y=maxY; v.y=0; }
    p.y = std::max(0.0f, p.y);
    body.setPosition(p);
    body.setVelocity(v);
    return hitFloor;
}

} // namespace bitwave::gameplay
