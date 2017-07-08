#ifndef EVENTPROCESSOR_HPP_INCLUDED
#define EVENTPROCESSOR_HPP_INCLUDED

#include "config.hpp"
#include "const/character.hpp"
#include "character.hpp"

#include <SFML/System.hpp>
#include <vector>
#include <memory>

struct ChatBot
{
    sf::Clock clock;
    std::vector<std::string> database;

    ChatBot();
    ~ChatBot();
    void Load();
    void Save();
    std::string GetMessage(std::string message);
    void Process();
    void ProcessMessage(std::string message);
};

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
    int payments;

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
        int channel;
        std::string victim_name;

        DelayMessage(std::string message, int time_ms) { this->message = message; this->time_ms = time_ms; this->clock.restart(); this->channel = 0; }
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
    ItemRequest item_request;

    EventProcessor();

    void Process();
    void DelayedMessage(std::string message, int time_ms = 0);
    void DelayedMessage(DelayMessage delay_message);
};

#endif // EVENTPROCESSOR_HPP_INCLUDED
