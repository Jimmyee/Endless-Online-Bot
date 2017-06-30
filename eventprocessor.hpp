#ifndef EVENTPROCESSOR_HPP_INCLUDED
#define EVENTPROCESSOR_HPP_INCLUDED

#include "config.hpp"
#include "const/character.hpp"

#include <SFML/System.hpp>
#include <vector>
#include <memory>

struct ChatBot
{
    Config config;
    sf::Clock clock;

    ChatBot();
    ~ChatBot();
    void Load();
    void Process();
    void ProcessMessage(std::string name, std::string message);
};

struct EORoulette
{
    bool run;
    short gameworld_id;
    sf::Clock clock;
    int gold_given;
    int spins;
    int max_spins;
    int spin_delay;
    bool play;
    short winner;

    EORoulette();
    void Run(short gameworld_id);
    void Process();
};

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

struct EventProcessor
{
    struct Trade
    {
        short victim_gameworld_id;
        std::vector<std::pair<short, int>> player_items;
        std::vector<std::pair<short, int>> victim_items;
        bool player_accepted;
        bool victim_accepted;

        Trade(short victim_gameworld_id_) { victim_gameworld_id = victim_gameworld_id_; player_accepted = false; victim_accepted = false; }
    };

    struct DelayMessage
    {
        std::string message;
        sf::Clock clock;
        int time_ms;

        DelayMessage(std::string message, int time_ms) { this->message = message; this->time_ms = time_ms; this->clock.restart(); }
    };

    std::shared_ptr<Trade> trade;
    sf::Clock help_message_clock;
    ChatBot chat_bot;
    std::vector<DelayMessage> d_messages;
    sf::Clock autosave_clock;
    EORoulette eo_roulette;
    ChaseBot chase_bot;
    sf::Clock uptime_clock;
    sf::Clock refresh_clock;

    EventProcessor();

    void Process();
    void DelayedMessage(std::string message);
};

#endif // EVENTPROCESSOR_HPP_INCLUDED
