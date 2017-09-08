#ifndef SITWIN_HPP_INCLUDED
#define SITWIN_HPP_INCLUDED

#include <SFML/System.hpp>

struct SitWin
{
    bool run;
    bool play;
    short item_id;
    int item_amount;
    short gameworld_id;
    short winner;
    sf::Clock clock;
    sf::Clock reminder_clock;

    SitWin();
    SitWin(short item_id, short item_amount, short gameworld_id);
    void Run(short gameworld_id);
    void Process();
    void Play();
};

#endif // SITWIN_HPP_INCLUDED
