#pragma once

namespace bitwave::entities {

struct Vec2 { float x{0}, y{0}; };

class Entity {
public:
    Entity(Vec2 position = {}, Vec2 size = {32, 32});
    virtual ~Entity() = default;
    Vec2 position() const { return position_; }
    Vec2 velocity() const { return velocity_; }
    Vec2 size() const { return size_; }
    void setPosition(Vec2 value) { position_ = value; }
    void setVelocity(Vec2 value) { velocity_ = value; }
    bool overlaps(const Entity& other) const;

private:
    Vec2 position_, velocity_, size_;
};

} // namespace bitwave::entities
