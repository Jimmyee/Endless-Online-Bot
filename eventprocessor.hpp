#ifndef EVENTPROCESSOR_HPP_INCLUDED
#define EVENTPROCESSOR_HPP_INCLUDED

#include "config.hpp"

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

    struct SexAct
    {
        short victim_gameworld_id;
        int sits;
        int max_sits;
        sf::Clock clock;
        bool fuck;
        bool sit_request;
        sf::Clock timeout_clock;

        SexAct(short victim_gameworld_id_) { victim_gameworld_id = victim_gameworld_id_; sits = 0; max_sits = 0; clock.restart(); fuck = false; sit_request = false; timeout_clock.restart(); }
    };

    struct DelayMessage
    {
        std::string message;
        sf::Clock clock;
        int time_ms;

        DelayMessage(std::string message, int time_ms) { this->message = message; this->time_ms = time_ms; this->clock.restart(); }
    };

    sf::Clock welcome_clock;
    std::vector<short> players_known;
    std::shared_ptr<Trade> trade;
    std::shared_ptr<SexAct> sex_act;
    sf::Clock sex_message_clock;
    ChatBot chat_bot;
    std::vector<DelayMessage> d_messages;

    EventProcessor();

    void Process();
    void DelayedMessage(std::string message);
};

#endif // EVENTPROCESSOR_HPP_INCLUDED
