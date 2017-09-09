#include "questgen.hpp"
#include "util.hpp"
#include "singleton.hpp"

QuestGenerator::QuestGenerator()
{
    Config config("quests.ini");

    for(unsigned int i = 0; i < config.entries.size(); ++i)
    {
        std::vector<std::string> key_args = Args(config.entries[i].key);
        std::string name = key_args[0];
        std::vector<std::string> quest_data = Args(config.entries[i].value);

        Quest quest;
        quest.id = i + 1;
        quest.holder = name;
        quest.complete = std::atoi(key_args[1].c_str());

        short award_id = std::atoi(quest_data[0].c_str());
        int award_amount = std::atoi(quest_data[1].c_str());

        quest.award = std::make_pair(award_id, award_amount);

        int requirements = std::atoi(quest_data[2].c_str());

        for(int i = 0; i < requirements; ++i)
        {
            short item_id = std::atoi(quest_data[3 + i].c_str());
            int item_amount = std::atoi(quest_data[4 + i].c_str());

            quest.requirements.push_back(std::make_pair(item_id, item_amount));
        }

        this->quests.push_back(quest);
    }

    this->clock.restart();
}

QuestGenerator::~QuestGenerator()
{
    this->Save();
}

std::vector<Quest> QuestGenerator::GetPlayerQuests(std::string holder)
{
    std::vector<Quest> ret;

    for(unsigned int i = 0; i < this->quests.size(); ++i)
    {
        if(this->quests[i].holder == holder)
        {
            ret.push_back(this->quests[i]);
        }
    }

    return ret;
}

Quest QuestGenerator::GetQuest(int id)
{
    for(unsigned int i = 0; i < this->quests.size(); ++i)
    {
        if(this->quests[i].id == id)
        {
            return this->quests[i];
        }
    }

    return Quest();
}

void QuestGenerator::RemoveQuest(int id)
{
    for(unsigned int i = 0; i < this->quests.size(); ++i)
    {
        if(this->quests[i].id == id)
        {
            this->quests.erase(this->quests.begin() + i);
            break;
        }
    }
}

int QuestGenerator::GenerateID()
{
    int id = 1;

    while(this->GetQuest(id).id != 0)
    {
        id++;
    }

    return id;
}

void QuestGenerator::UpdateQuest(int id, Quest quest)
{
    for(unsigned int i = 0; i < this->quests.size(); ++i)
    {
        if(this->quests[i].id == id)
        {
            this->quests[i] = quest;
            break;
        }
    }
}

void QuestGenerator::Process()
{
    S &s = S::GetInstance();

    if(this->item_request.run)
    {
        int time_delay = (s.eprocessor.trade.get())? 30 : 12;
        if(this->item_request.clock.getElapsedTime().asSeconds() >= time_delay)
        {
            this->item_request.run = false;

            if(s.eprocessor.trade.get())
            {
                s.eprocessor.trade.reset();

                s.eoclient.TradeClose();
            }

            if(this->active_quest.get())
            {
                this->active_quest.reset();
            }

            s.eprocessor.DelayedMessage("Trade canceled due to player inactivity.", 1000);
        }
    }
    else
    {
        if(this->new_quest.get() && this->clock.getElapsedTime().asSeconds() >= 30)
        {
            this->new_quest.reset();
            s.eprocessor.DelayedMessage("Quest creation canceled due to player inactivity.", 1000);
        }
    }
}

void QuestGenerator::Save()
{
    Config config;

    for(unsigned int i = 0; i < this->quests.size(); ++i)
    {
        Config::Entry entry("", "");
        entry.key = this->quests[i].holder + " " + std::to_string(this->quests[i].complete);
        entry.value = std::to_string(this->quests[i].award.first);
        entry.value += " " + std::to_string(this->quests[i].award.second);

        entry.value += " " + std::to_string(this->quests[i].requirements.size());

        for(unsigned int ii = 0; ii < this->quests[i].requirements.size(); ++ii)
        {
            entry.value += " " + std::to_string(this->quests[i].requirements[ii].first);
            entry.value += " " + std::to_string(this->quests[i].requirements[ii].second);
        }

        config.entries.push_back(entry);
    }

    config.Save("quests.ini");
}
