#ifndef LOTTERY_HPP_INCLUDED
#define LOTTERY_HPP_INCLUDED

#include <SFML/System.hpp>
#include <vector>

struct Lottery
{
    struct Ticket
    {
        short gameworld_id;
        int number;

        Ticket(short gameworld_id, int number) { this->gameworld_id = gameworld_id; this->number = number; }
    };

    bool run;
    bool play;
    std::vector<Ticket> tickets;
    std::vector<Ticket> requests;
    sf::Clock clock;
    short winner;
    int ticket_price;
    int award;

    Lottery();
    void Run();
    void Run(int ticket_price);
    void Process();
};

#endif // LOTTERY_HPP_INCLUDED
