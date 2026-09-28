#pragma once

// Player
// TODO(Day 2+): Define the Player entity.
// Placeholder only — no gameplay logic during repository initialization.



class Player{
    private:
        int health;
        int score;
    
    public:
        void takeDamage();
        void addScore();
        int getHealth() const;
        int getScore() const;
};


