#include "market.hpp"

#include "config.hpp"
#include "util.hpp"
#include "singleton.hpp"

Market::Market()
{
    Config config("market.ini");

    for(unsigned int i = 0; i < config.entries.size(); ++i)
    {
        std::vector<std::string> key_args = Args(config.entries[i].key);
        std::vector<std::string> value_args = Args(config.entries[i].value);

        Offer offer;
        offer.id = i + 1;
        offer.player_name = key_args[0];
        offer.transaction_done = std::atoi(key_args[1].c_str());
        offer.transaction_type = std::atoi(value_args[0].c_str());
        short item_id = std::atoi(value_args[1].c_str());
        int item_amount = std::atoi(value_args[2].c_str());
        offer.item = std::make_pair(item_id, item_amount);
        offer.price = std::atoi(value_args[3].c_str());

        this->offers.push_back(offer);
    }

    this->clock.restart();
}

Market::~Market()
{

}

Market::Offer Market::GetOffer(int id)
{
    for(unsigned int i = 0; i < this->offers.size(); ++i)
    {
        if(this->offers[i].id == id)
        {
            return this->offers[i];
        }
    }

    return Offer();
}

std::vector<Market::Offer> Market::GetPlayerOffers(std::string name)
{
    std::vector<Offer> ret;

    for(unsigned int i = 0; i < this->offers.size(); ++i)
    {
        if(this->offers[i].player_name == name)
        {
            ret.push_back(this->offers[i]);
        }
    }

    return ret;
}

void Market::RemoveOffer(int id)
{
    for(unsigned int i = 0; i < this->offers.size(); ++i)
    {
        if(this->offers[i].id == id)
        {
            this->offers.erase(this->offers.begin() + i);
            break;
        }
    }
}

int Market::GenerateID()
{
    int id = 1;

    while(this->GetOffer(id).id != 0)
    {
        id++;
    }

    return id;
}

void Market::UpdateOffer(int id, Market::Offer offer)
{
    for(unsigned int i = 0; i < this->offers.size(); ++i)
    {
        if(this->offers[i].id == id)
        {
            this->offers[i] = offer;
            break;
        }
    }
}

void Market::Process()
{
    S &s = S::GetInstance();

    if(this->item_request.run)
    {
        int time_delay = (s.eprocessor.trade.get())? 30 : 12;
        if(this->item_request.clock.getElapsedTime().asSeconds() >= time_delay)
        {
            this->item_request.run = false;

            if(s.eprocessor.trade.get())
            {
                s.eprocessor.trade.reset();

                s.eoclient.TradeClose();
            }

            if(this->active_offer.get())
            {
                this->active_offer.reset();
            }

            //s.eprocessor.DelayedMessage("Trade canceled due to player inactivity.", 1000);
        }
    }
    else
    {
        if(this->new_offer.get() && this->clock.getElapsedTime().asSeconds() >= 30)
        {
            this->new_offer.reset();
            s.eprocessor.DelayedMessage("Transaction canceled due to player inactivity.", 1000);
        }
    }
}

void Market::Save()
{
    Config config;

    for(unsigned int i = 0; i < this->offers.size(); ++i)
    {
        Config::Entry entry("", "");
        entry.key = this->offers[i].player_name + " " + std::to_string(this->offers[i].transaction_done);
        entry.value = std::to_string(this->offers[i].transaction_type) + " ";
        entry.value += std::to_string(this->offers[i].item.first) + " " + std::to_string(this->offers[i].item.second);
        entry.value += " " + std::to_string(this->offers[i].price);

        config.entries.push_back(entry);
    }

    config.Save("market.ini");
}
