#ifndef CHATBOT_HPP_INCLUDED
#define CHATBOT_HPP_INCLUDED

#include "config.hpp"

#include <SFML/System.hpp>
#include <vector>
#include <string>

struct ChatBot
{
    sf::Clock clock;
    Config config;

    ChatBot();
    ~ChatBot();
    void Save();
    std::string GetMessage(std::string message);
    void Process();
    void ProcessMessage(std::string message);
};

#endif // CHATBOT_HPP_INCLUDED
