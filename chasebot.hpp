#ifndef CHASEBOT_HPP_INCLUDED
#define CHASEBOT_HPP_INCLUDED

#include "const/character.hpp"
#include "util.hpp"

#include <SFML/System.hpp>

struct ChaseBot
{
    short victim_gameworld_id;
    sf::Clock walk_clock;
    sf::Clock follow_clock;
    unsigned char center_x;
    unsigned char center_y;
    bool go_center;

    ChaseBot();
    void Reset();
    void Process();
    bool Walk(Direction direction);
    void WalkTo(unsigned char x, unsigned char y);
    void Act();
};

#endif // CHASEBOT_HPP_INCLUDED
