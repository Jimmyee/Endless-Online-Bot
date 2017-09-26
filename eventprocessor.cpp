#include "eventprocessor.hpp"
#include "singleton.hpp"
#include "util.hpp"

EventProcessor::EventProcessor()
{
    this->help_message_clock.restart();
    this->autosave_clock.restart();
    this->uptime_clock.restart();
    this->refresh_clock.restart();

    this->help_config.Load("help.txt");
    Config gitem_config("gitem.ini");

    for(unsigned int i = 0; i < gitem_config.entries.size(); ++i)
    {
        std::vector<std::string> args = Args(gitem_config.entries[i].value);

        this->free_inv.items.push_back(std::make_pair(std::atoi(args[0].c_str()), std::atoi(args[1].c_str())));
    }

    Config donated_config("donated.ini");

    for(unsigned int i = 0; i < donated_config.entries.size(); ++i)
    {
        std::vector<std::string> args = Args(donated_config.entries[i].value);

        this->donated.items.push_back(std::make_pair(std::atoi(args[0].c_str()), std::atoi(args[1].c_str())));
    }

    Config joke_config("jokes.ini");

    for(unsigned int i = 0; i < joke_config.entries.size(); ++i)
    {
        this->jokes.push_back(Joke(joke_config.entries[i]));
    }

    this->news.Load("news.ini");
}

void EventProcessor::Process()
{
    S &s = S::GetInstance();

    if(this->autosave_clock.getElapsedTime().asSeconds() >= 900)
    {
        this->chat_bot.Save();
        this->autosave_clock.restart();
    }

    this->eo_roulette.Process();
    this->item_request.Process();
    this->sitwin.Process();
    this->chat_bot.Process();
    if(s.config.GetValue("ChaseBot") == "yes") this->chase_bot.Process();
    this->quest_gen.Process();
    this->market.Process();

    if(this->help_message_clock.getElapsedTime().asSeconds() > 3600)
    {
        for(unsigned int i = 0; i < this->news.entries.size(); ++i)
        {
            this->DelayedMessage(this->news.entries[i].value);
        }

        this->help_message_clock.restart();
    }

    if(this->d_messages.size() > 0)
    {
        if(this->d_messages[0].clock.getElapsedTime().asMilliseconds() >= this->d_messages[0].time_ms)
        {
            std::vector<std::string> buffer;
            std::string message = this->d_messages[0].message;
            while(message.length() > 128)
            {
                buffer.push_back(message.substr(0, 128));
                message.erase(0, 128);
            }
            if(message.length() > 0)
            {
                buffer.push_back(message);
            }

            for(unsigned int i = 0; i < buffer.size(); ++i)
            {
                if(this->d_messages[0].channel == 0)
                {
                    s.eoclient.TalkPublic(buffer[i]);
                }
                if(this->d_messages[0].channel == 1)
                {
                    s.eoclient.TalkTell(this->d_messages[0].victim_name, buffer[i]);
                }
                if(this->d_messages[0].channel == 2)
                {
                    s.eoclient.TalkGlobal(buffer[i]);
                }
            }
            this->d_messages.erase(this->d_messages.begin());
            if(this->d_messages.size() > 0)
            {
                this->d_messages[0].clock.restart();
            }
        }
    }

    if(this->refresh_clock.getElapsedTime().asSeconds() >= 60)
    {
        s.eoclient.RefreshRequest();
        this->refresh_clock.restart();
    }
}

void EventProcessor::DelayedMessage(std::string message, int time_ms, int channel, std::string victim_name)
{
    DelayMessage d_message(message, time_ms);

    if(time_ms == 0)
    {
        d_message.time_ms = message.length() * 90 + S::GetInstance().rand_gen.RandInt(90, 900);
    }

    d_message.channel = channel;
    if(channel == 1 && victim_name != "") d_message.victim_name = victim_name;

    this->d_messages.push_back(d_message);
}

void EventProcessor::DelayedMessage(DelayMessage delay_message)
{
    if(this->d_messages.size() > 0)
    {
        bool found = false;
        for(unsigned int i = 0; i < this->d_messages.size(); ++i)
        {
            if(this->d_messages[i].message == delay_message.message && this->d_messages[i].victim_name == delay_message.victim_name)
            {
                found = true;
            }
        }

        if(found) return;
    }

    this->d_messages.push_back(delay_message);
}

bool EventProcessor::BlockingEvent()
{
    if(this->trade.get() || this->eo_roulette.run || this->item_request.run || this->sitwin.run
       || this->quest_gen.new_quest.get() || this->quest_gen.item_request.run || this->market.new_offer.get()
       || this->market.item_request.run)
    {
        return true;
    }

    return false;
}

void EventProcessor::SaveFreeItems()
{
    Config gitem_config;

    for(unsigned int i = 0; i < this->free_inv.items.size(); ++i)
    {
        Config::Entry entry("", "");
        entry.key = std::to_string(i + 1);
        entry.value = std::to_string(this->free_inv.items[i].first) + " " + std::to_string(this->free_inv.items[i].second);

        gitem_config.entries.push_back(entry);
    }

    gitem_config.Save("gitem.ini");
}

void EventProcessor::SaveDonated()
{
    Config donated_config;

    for(unsigned int i = 0; i < this->donated.items.size(); ++i)
    {
        Config::Entry entry("", "");
        entry.key = std::to_string(i + 1);
        entry.value = std::to_string(this->donated.items[i].first) + " " + std::to_string(this->donated.items[i].second);

        donated_config.entries.push_back(entry);
    }

    donated_config.Save("donated.ini");
}

int EventProcessor::GetJokeAmount(std::string name)
{
    int joke_count = 0;
    for(unsigned int i = 0; i < this->jokes.size(); ++i)
    {
        if(this->jokes[i].From() == name)
            joke_count++;
    }

    return joke_count;
}

void EventProcessor::RemoveJokeOf(std::string name)
{
    for(unsigned int i = 0; i < this->jokes.size(); ++i)
    {
        if(this->jokes[i].From() == name)
        {
            this->jokes.erase(this->jokes.begin() + i);
            break;
        }
    }
}

Joke EventProcessor::GetJokeByID(unsigned int id)
{
    for(unsigned int i = 0; i < this->jokes.size(); ++i)
    {
        if(i == id - 1)
        {
            return this->jokes[i];
        }
    }

    return Joke("", "");
}

int EventProcessor::GetJokeID(std::string joke)
{
    for(unsigned int i = 0; i < this->jokes.size(); ++i)
    {
        if(this->jokes[i].Get() == joke)
        {
            return i + 1;
        }
    }

    return -1;
}

void EventProcessor::UpdateJoke(unsigned int id, Joke joke)
{
    for(unsigned int i = 0; i < this->jokes.size(); ++i)
    {
        if(i == id - 1)
        {
            this->jokes[i] = joke;
        }
    }
}

Joke EventProcessor::GetBestJoke()
{
    int best_joke_index = -1;
    unsigned int most_votes = 0;
    for(unsigned int i = 0; i < this->jokes.size(); ++i)
    {
        if(this->jokes[i].Votes().size() > most_votes)
        {
            most_votes = this->jokes[i].Votes().size();
            best_joke_index = i;
        }
    }

    if(best_joke_index != -1)
    {
        return this->jokes[best_joke_index];
    }

    return Joke("", "");
}

Joke EventProcessor::GetFreeJoke()
{
    std::vector<Joke> free_jokes;

    for(unsigned int i = 0; i < this->jokes.size(); ++i)
    {
        if(this->jokes[i].first_call || this->jokes[i].clock.getElapsedTime().asSeconds() >= 300)
        {
            free_jokes.push_back(this->jokes[i]);
        }
    }

    if(free_jokes.size() > 0)
    {
        Joke ret = free_jokes[S::GetInstance().rand_gen.RandInt(0, free_jokes.size() - 1)];
        int id = this->GetJokeID(ret.Get());
        this->jokes[id - 1].clock.restart();
        this->jokes[id - 1].first_call = false;
        return ret;
    }

    return Joke("", "");
}

void EventProcessor::SaveJokes()
{
    Config joke_config;

    for(unsigned int i = 0; i < this->jokes.size(); ++i)
    {
        joke_config.entries.push_back(this->jokes[i].ConfigEntry());
    }

    joke_config.Save("jokes.ini");
}
