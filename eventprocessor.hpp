#ifndef EVENTPROCESSOR_HPP_INCLUDED
#define EVENTPROCESSOR_HPP_INCLUDED

#include "config.hpp"
#include "character.hpp"

#include "chatbot.hpp"
#include "eoroulette.hpp"
#include "itemreq.hpp"
#include "sitwin.hpp"
#include "questgen.hpp"
#include "market.hpp"
#include "inventory.hpp"
#include "joke.hpp"
#include "chamber.hpp"

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
    sf::Clock uptime_clock;
    sf::Clock refresh_clock;
    ItemRequest item_request;
    SitWin sitwin;
    Config help_config;
    QuestGenerator quest_gen;
    Market market;
    Inventory free_inv;
    Inventory donated;
    std::vector<Joke> jokes;
    Config news;
    Chamber chamber;
    sf::Clock line_clock;

    EventProcessor();

    void Process();
    void DelayedMessage(std::string message, int time_ms = 0, int channel = 0, std::string victim_name = "");
    void DelayedMessage(DelayMessage delay_message);
    bool BlockingEvent();

    void SaveFreeItems();
    void SaveDonated();
    int GetJokeAmount(std::string name);
    void RemoveJokeOf(std::string name);
    Joke GetJokeByID(unsigned int id);
    int GetJokeID(std::string joke);
    void UpdateJoke(unsigned int id, Joke joke);
    Joke GetBestJoke();
    Joke GetFreeJoke();
    void SaveJokes();
};

#endif // EVENTPROCESSOR_HPP_INCLUDED
