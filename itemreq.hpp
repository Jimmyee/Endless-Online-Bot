#ifndef ITEMREQ_HPP_INCLUDED
#define ITEMREQ_HPP_INCLUDED

#include <SFML/System.hpp>

struct ItemRequest
{
    bool run;
    short id;
    int amount;
    short gameworld_id;
    bool give;
    sf::Clock clock;

    ItemRequest() { this->run = false; id = 0; this->amount = 1; this->gameworld_id = 0; this->give = true; this->clock.restart(); }
};

#endif // ITEMREQ_HPP_INCLUDED
