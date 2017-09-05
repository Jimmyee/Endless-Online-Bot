#include "chatbot.hpp"
#include "singleton.hpp"

ChatBot::ChatBot()
{
    this->config.Load("./chatbot.txt");
    this->clock.restart();
}

void ChatBot::Save()
{
    this->config.Save("./chatbot.txt");
}

std::string ChatBot::GetMessage(std::string message)
{
    for(unsigned int i = 0; i < this->config.entries.size(); ++i)
    {
        if(this->config.entries[i].value == message)
        {
            return this->config.entries[i].value;
        }
    }

    return "";
}

void ChatBot::Process()
{
    //S &s = S::GetInstance();
}

void ChatBot::ProcessMessage(std::string message)
{
    S &s = S::GetInstance();

    int save_message = s.rand_gen.RandInt(0, 33);
    if(save_message == 0)
    {
        if(this->GetMessage(message) == "" && message.length() > 1)
        {
            Config::Entry entry("", "");
            entry.key = std::to_string(this->config.entries.size());
            entry.value = message;
            this->config.entries.push_back(entry);

            if(this->config.entries.size() > 900)
            {
                this->config.entries.erase(this->config.entries.begin());
            }
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

    for(unsigned int i = 0; i < this->config.entries.size(); ++i)
    {
        for(unsigned int ii = 0; ii < words.size(); ++ii)
        {
            if(this->config.entries[i].value.find(words[ii]) != std::string::npos)
            {
                message_proposals.push_back(this->config.entries[i].value);
            }
        }
    }

    if(message_proposals.size() > 0 && s.rand_gen.RandInt(0, 11) == 0)
    {
        //int i = s.rand_gen.RandInt(0, message_proposals.size() - 1);
    }
}

ChatBot::~ChatBot()
{
    this->Save();
}
