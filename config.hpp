// Endless Online Bot v0.0.1

#ifndef CONFIG_HPP_INCLUDED
#define CONFIG_HPP_INCLUDED

#include <string>
#include <unordered_map>

class Config
{
public:
    std::unordered_map<std::string, std::string> values;

    Config();
    Config(std::string filename);
    bool Load(std::string filename);
    void Save(std::string filename);
};

#endif // CONFIG_HPP_INCLUDED
