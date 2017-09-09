#ifndef MARKET_HPP_INCLUDED
#define MARKET_HPP_INCLUDED

#include "itemreq.hpp"

#include <string>
#include <vector>
#include <memory>

struct Market
{
    struct Offer
    {
        int id;
        std::string player_name;
        bool transaction_done;
        unsigned char transaction_type;
        std::pair<short, int> item;
        int price;

        Offer() : id(0), transaction_done(false), transaction_type(0), item(std::make_pair(0, 0)), price(0) { }
    };

    std::vector<Offer> offers;
    ItemRequest item_request;
    sf::Clock clock;
    std::shared_ptr<Offer> new_offer;
    std::shared_ptr<Offer> active_offer;

    Market();
    ~Market();
    Offer GetOffer(int id);
    std::vector<Offer> GetPlayerOffers(std::string name);
    void RemoveOffer(int id);
    int GenerateID();
    void UpdateOffer(int id, Market::Offer offer);
    void Process();
    void Save();
};

#endif // MARKET_HPP_INCLUDED
