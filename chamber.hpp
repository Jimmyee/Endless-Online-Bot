#ifndef CHAMBER_HPP_INCLUDED
#define CHAMBER_HPP_INCLUDED

#include <string>
#include <vector>

class Chamber
{
public:
    void Command(std::string name, std::string command, std::vector<std::string> args, short gameworld_id, int channel);
};

#endif // CHAMBER_HPP_INCLUDED
