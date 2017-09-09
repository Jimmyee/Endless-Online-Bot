#ifndef QUESTGEN_HPP_INCLUDED
#define QUESTGEN_HPP_INCLUDED

#include "config.hpp"
#include "itemreq.hpp"

#include <vector>
#include <memory>

struct Quest
{
    int id;
    std::string holder;
    std::pair<short, int> award;
    std::vector<std::pair<short, int>> requirements;
    bool complete;

    Quest() : id(0), complete(false) { }
};

struct QuestGenerator
{
    std::vector<Quest> quests;
    std::shared_ptr<Quest> new_quest;
    std::shared_ptr<Quest> active_quest;
    ItemRequest item_request;
    sf::Clock clock;

    QuestGenerator();
    ~QuestGenerator();
    std::vector<Quest> GetPlayerQuests(std::string holder);
    Quest GetQuest(int id);
    void RemoveQuest(int id);
    int GenerateID();
    void UpdateQuest(int id, Quest quest);
    void Process();
    void Save();
};

#endif // QUESTGEN_HPP_INCLUDED
