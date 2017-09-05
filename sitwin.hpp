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
    void RunJackpot(short item_id, int item_amount);
    void Process();
    void Play();
};

struct SitWinJackpot
{
    short item_id;
    int item_amount;
    sf::Clock clock;
    sf::Clock reminder_clock;
    sf::Clock reminder_global;
    int jp_time;

    SitWinJackpot();
    bool GenerateItem();
    void Process();
    void Reset();
};

#endif // SITWIN_HPP_INCLUDED
