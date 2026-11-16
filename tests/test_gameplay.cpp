#include <catch2/catch_test_macros.hpp>

#include "core/GameManager.h"
#include "entities/Enemy.h"
#include "entities/Player.h"
#include "gameplay/Combat.h"

using namespace bitwave;

TEST_CASE("Running mode moves and preserves jump physics", "[gameplay]") {
    core::GameManager games;
    games.startMode(shared::GameModeType::Running);
    games.handleAction(shared::PlayerAction::MoveForward, true);
    games.update(.1f);
    const float movedX=games.state().playerX;
    games.handleAction(shared::PlayerAction::Jump);
    games.update(.1f);
    REQUIRE(movedX>80);
    REQUIRE(games.state().playerY<568);
}

TEST_CASE("Flappy mode falls until the player flaps", "[gameplay]") {
    core::GameManager games;
    games.startMode(shared::GameModeType::Flappy);
    games.update(.1f);
    const float fallingY=games.state().playerY;
    games.handleAction(shared::PlayerAction::Flap);
    games.update(.1f);
    REQUIRE(games.state().playerY<fallingY);
}

TEST_CASE("Fighting characters can attack and defeat an enemy", "[gameplay]") {
    entities::Player player;
    entities::Enemy enemy("Training Bot", "enemy.training_bot", {100,280});
    std::vector<entities::Enemy> enemies{enemy};

    REQUIRE(gameplay::Combat::attack(player,enemies));
    REQUIRE(gameplay::Combat::attack(player,enemies));
    REQUIRE_FALSE(enemies.front().alive());
    REQUIRE(player.score()==1);
}

TEST_CASE("Difficulty changes movement speed", "[gameplay]") {
    core::GameManager easy, hard;
    easy.startMode(shared::GameModeType::Running,shared::Difficulty::Easy);
    hard.startMode(shared::GameModeType::Running,shared::Difficulty::Hard);
    easy.handleAction(shared::PlayerAction::MoveForward,true);
    hard.handleAction(shared::PlayerAction::MoveForward,true);
    easy.update(.1f); hard.update(.1f);
    REQUIRE(easy.state().playerX>hard.state().playerX);
}
