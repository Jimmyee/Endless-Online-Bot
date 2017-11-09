#include "chatbot.hpp"
#include "singleton.hpp"

std::vector<std::string> GetWords(std::string message)
{
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

    return words;
}

ChatBot::ChatBot()
{
    this->config.Load("./chatbot.txt");
    this->clock.restart();
}

ChatBot::~ChatBot()
{
    this->Save();
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

            if(this->config.entries.size() > 33300)
            {
                this->config.entries.erase(this->config.entries.begin());
            }
        }
    }

    std::vector<std::string> words = GetWords(message);
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

std::string ChatBot::GenerateLine()
{
    S &s = S::GetInstance();
    unsigned int generate_words = s.rand_gen.RandInt(3, 12);
    std::string message;

    int word_count = 0;
    int word_count_max = s.rand_gen.RandInt(3, 12);
    for(unsigned int i = 0; i < generate_words; ++i)
    {
        if(this->config.entries.size() == 0) break;

        int rand_entry = s.rand_gen.RandInt(0, this->config.entries.size() - 1);
        std::vector<std::string> words = GetWords(this->config.entries[rand_entry].value);

        if(word_count >= word_count_max) word_count = 0;
        int start_ii = s.rand_gen.RandInt(0, words.size() - 1);
        for(unsigned int ii = start_ii; ii < words.size(); ++ii)
        {
            if(word_count < word_count_max)
            {
                message += message.empty()? words[ii] : " " + words[ii];
                word_count++;
            }
            else
            {
                break;
            }
        }
    }

    return message;
}
