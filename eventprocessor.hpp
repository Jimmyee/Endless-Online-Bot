#ifndef EVENTPROCESSOR_HPP_INCLUDED
#define EVENTPROCESSOR_HPP_INCLUDED

#include "config.hpp"
#include "character.hpp"

#include "chatbot.hpp"
#include "eoroulette.hpp"
#include "chasebot.hpp"
#include "itemreq.hpp"
#include "sitwin.hpp"
#include "lottery.hpp"

#include <SFML/System.hpp>
#include <vector>
#include <memory>

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

struct EventProcessor
{
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
    SitWin sitwin;
    SitWinJackpot sitwin_jackpot;
    Lottery lottery;
    std::vector<std::string> whitelist;
    Config help_config;

    EventProcessor();

    void Process();
    void DelayedMessage(std::string message, int time_ms = 0);
    void DelayedMessage(DelayMessage delay_message);
    bool BlockingEvent();
    bool Whitelist(std::string name);
};

#endif // EVENTPROCESSOR_HPP_INCLUDED
