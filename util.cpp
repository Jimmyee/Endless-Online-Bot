// Endless Online Bot v0.0.1

#include "util.hpp"

#include <algorithm>

int path_length(int x1, int y1, int x2, int y2)
{
	int dx = std::abs(x1 - x2);
	int dy = std::abs(y1 - y2);

	return dx + dy;
}

std::vector<std::string> Args(std::string str)
{
    std::vector<std::string> args;
    std::string word;
    for(unsigned int i = 0; i < str.length(); ++i)
    {
        if((str[i] == ' ' || i == str.length() - 1))
        {
            if(str[i] != ' ') word += str[i];
            if(!word.empty()) args.push_back(word);
            word.clear();
        }
        else if(str[i] != ' ')
        {
            word += str[i];
        }
    }

    return args;
}

std::string Lowercase(std::string str)
{
    std::string ret = str;

    std::transform(ret.begin(), ret.end(), ret.begin(), ::tolower);

    return ret;
}
