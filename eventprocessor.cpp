#include "eventprocessor.hpp"
#include "singleton.hpp"

#include <fstream>

ChatBot::ChatBot()
{
    this->Load();
    this->clock.restart();
}

void ChatBot::Load()
{
    std::ifstream file("./chatbot.txt", std::ios::in);
    if(!file.is_open()) return;
    file.close();
    this->config.Load("./chatbot.txt");
}

void ChatBot::Process()
{
    S &s = S::GetInstance();

    if(this->clock.getElapsedTime().asSeconds() > 1200)
    {
        int i = s.rand_gen.RandInt(0, this->config.values.size() - 1);
        int c = 0;

        std::unordered_map<std::string, std::string>::iterator it;
        for(it = this->config.values.begin(); it != this->config.values.end(); ++it)
        {
            if(c == i)
            {
                s.eprocessor.DelayedMessage(it->first);
            }

            c++;
        }
    }
}

void ChatBot::ProcessMessage(std::string name, std::string message)
{
    S &s = S::GetInstance();

    int save_message = s.rand_gen.RandInt(0, 5);
    std::unordered_map<std::string, std::string>::iterator it;
    if(save_message == 0)
    {
        it = this->config.values.find(message);
        if(it == this->config.values.end() && name != "panddda")
        {
            this->config.values[message] = name;
        }
    }

    std::vector<std::string> words;
    std::string word;
    for(unsigned int i = 0; i < message.length(); ++i)
    {
        if((message[i] == ' ' || i == message.length() - 1) && !word.empty())
        {
            words.push_back(word);
            word.clear();
        }
        else if(message[i] != ' ')
        {
            word += message[i];
        }
    }

    std::vector<std::string> message_proposals;
    for(it = this->config.values.begin(); it != this->config.values.end(); ++it)
    {
        std::string itname = it->second;
        std::string itmessage = it->first;

        for(unsigned int i = 0; i < words.size(); ++i)
        {
            if(itmessage.find(words[i]) != std::string::npos && itname != name)
            {
                message_proposals.push_back(itmessage);
            }
        }
    }

    if(message_proposals.size() > 0 && s.rand_gen.RandInt(0, 2) == 0)
    {
        int i = s.rand_gen.RandInt(0, message_proposals.size() - 1);
        s.eprocessor.DelayedMessage(message_proposals[i]);
    }
}

ChatBot::~ChatBot()
{
    this->config.Save("./chatbot.txt");
}

EventProcessor::EventProcessor()
{
    this->welcome_clock.restart();
    this->sex_message_clock.restart();
}

void EventProcessor::Process()
{
    S &s = S::GetInstance();

    if(this->welcome_clock.getElapsedTime().asSeconds() >= 1800 && this->players_known.size() > 0)
    {
        this->players_known.erase(this->players_known.begin());
        this->welcome_clock.restart();
    }

    this->chat_bot.Process();

    if(this->trade.get())
    {
        if(this->trade->player_accepted && this->trade->victim_accepted)
        {
            this->trade.reset();
        }
    }

    if(this->sex_act.get())
    {
        if(this->sex_act->fuck)
        {
            if(s.character.sitting == SitState::Stand && !this->sex_act->sit_request && this->sex_act->clock.getElapsedTime().asMilliseconds() > 500)
            {
                if(this->sex_act->sits < this->sex_act->max_sits)
                {
                    PacketBuilder packet(PacketFamily::Sit, PacketAction::Request);
                    SitAction action = SitAction::Sit;
                    packet.AddChar((unsigned char)action);
                    s.eoclient.Send(packet);

                    this->sex_act->sit_request = true;
                }
                else
                {
                    this->DelayedMessage("OK it's done. I hope you enjoyed :)");
                    this->sex_act.reset();
                }
            }
            else if(s.character.sitting == SitState::Floor && !this->sex_act->sit_request && this->sex_act->clock.getElapsedTime().asMilliseconds() > 500)
            {
                PacketBuilder packet(PacketFamily::Sit, PacketAction::Request);
                SitAction action = SitAction::Stand;
                packet.AddChar((unsigned char)action);
                s.eoclient.Send(packet);

                this->sex_act->sit_request = true;
            }
        }
        else if(this->sex_act->timeout_clock.getElapsedTime().asSeconds() > 30)
        {
            this->DelayedMessage("Time is out. Next!");
            this->sex_act.reset();
        }
    }

    if(this->sex_message_clock.getElapsedTime().asSeconds() > 1800)
    {
        //s.eoclient.Talk("I will fuck anybody for at least 2 gold. Please trade me and I'm yours. More gold = more time!");
        this->sex_message_clock.restart();
    }

    if(this->d_messages.size() > 0)
    {
        if(this->d_messages[0].clock.getElapsedTime().asMilliseconds() >= this->d_messages[0].time_ms)
        {
            s.eoclient.Talk(this->d_messages[0].message);
            this->d_messages.erase(this->d_messages.begin());
            if(this->d_messages.size() > 0)
            {
                this->d_messages[0].clock.restart();
            }
        }
    }
}

void EventProcessor::DelayedMessage(std::string message)
{
    int message_delay = message.length() * 120 + S::GetInstance().rand_gen.RandInt(0, 1200);
    this->d_messages.push_back(DelayMessage(message, message_delay));
}
