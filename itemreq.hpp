#ifndef ITEMREQ_HPP_INCLUDED
#define ITEMREQ_HPP_INCLUDED

#include <SFML/System.hpp>
#include <vector>

struct ItemRequest
{
    bool run;
    short id;
    int amount;
    short gameworld_id;
    bool give;
    sf::Clock clock;
    std::pair<short, int> special_item;
    std::vector<std::pair<short, int>> requirements;

    ItemRequest();
    bool MeetsRequirements(std::vector<std::pair<short, int>> victim_items);
};

#endif // ITEMREQ_HPP_INCLUDED
