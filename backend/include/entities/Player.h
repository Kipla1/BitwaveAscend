#pragma once

// Player
// TODO(Day 2+): Define the Player entity.
// Placeholder only — no gameplay logic during repository initialization.



class Player{
    private:
        int health = 100;
        int score = 0;
    
    public:
        Player();
        void takeDamage();
        void addScore();
        int getHealth() const;
        int getScore() const;
};


