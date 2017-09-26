#ifndef JOKE_HPP_INCLUDED
#define JOKE_HPP_INCLUDED

#include <SFML/System.hpp>

#include "config.hpp"
#include "util.hpp"

class Joke
{
private:
    std::string joke;
    std::string from;
    std::vector<std::string> votes;

public:
    sf::Clock clock;
    bool first_call;

    Joke(Config::Entry entry)
    {
        std::vector<std::string> args = Args(entry.key);

        this->joke = entry.value;
        this->from = args[0];

        args.erase(args.begin());
        this->votes = args;

        this->clock.restart();
        this->first_call = true;
    }

    Joke(std::string from, std::string joke)
    {
        this->from = from;
        this->joke = joke;

        this->clock.restart();
        this->first_call = true;
    }

    std::string Get() { return this->joke; }
    std::string From() { return this->from; }
    std::vector<std::string> Votes() { return this->votes; }
    Config::Entry ConfigEntry()
    {
        std::string key = from;
        std::string value = this->joke;
        for(unsigned int i = 0; i < this->votes.size(); ++i)
        {
            key += " " + this->votes[i];
        }

        return Config::Entry(key, value);
    }

    bool AddVote(std::string name)
    {
        for(unsigned int i = 0; i < this->votes.size(); ++i)
        {
            if(this->votes[i] == name)
            {
                return false;
            }
        }

        votes.push_back(name);
        return true;
    }
};

#endif // JOKE_HPP_INCLUDED
