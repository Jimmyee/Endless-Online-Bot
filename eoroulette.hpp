#ifndef EOROULETTE_HPP_INCLUDED
#define EOROULETTE_HPP_INCLUDED

#include "config.hpp"
#include "character.hpp"

#include <SFML/System.hpp>

struct EORoulette
{
    bool run;
    short gameworld_id;
    sf::Clock clock;
    sf::Clock jackpot_clock;
    sf::Clock reminder_clock;
    sf::Clock reminder_global;
    int gold_given;
    int spins;
    int max_spins;
    int spin_delay;
    bool play;
    short winner;
    int total_gold;
    bool jackpot;
    int jp_time;
    Config jpconfig;
    std::vector<int> payments;
    std::vector<Character> players;

    EORoulette();
    void Run(short gameworld_id);
    void Process();
};

#endif // EOROULETTE_HPP_INCLUDED
