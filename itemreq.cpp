#include "itemreq.hpp"

ItemRequest::ItemRequest()
{
    this->run = false; id = 0;
    this->amount = 1;
    this->gameworld_id = 0;
    this->give = true;
    this->clock.restart();
    this->special_item.first = 0;
}

bool ItemRequest::MeetsRequirements(std::vector<std::pair<short, int>> victim_items)
{
    unsigned int items_met = 0;

    for(unsigned int i = 0; i < this->requirements.size(); ++i)
    {
        for(unsigned int ii = 0; ii < victim_items.size(); ++ii)
        {
            if(this->requirements[i].first == victim_items[ii].first
               && this->requirements[i].second == victim_items[ii].second)
            {
                items_met++;
                break;
            }
        }
    }

    if(items_met >= requirements.size())
    {
        return true;
    }

    return false;
}
