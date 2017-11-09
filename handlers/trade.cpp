#include "handlers.hpp"
#include "../singleton.hpp"

void Trade_Request(PacketReader reader)
{
    S &s = S::GetInstance();

    reader.GetChar(); // ?
    short gameworld_id = reader.GetShort();
    std::string name = reader.GetEndString();

    if(s.eprocessor.trade.get()) return;

    int i = s.map.GetCharacterIndex(gameworld_id);

    if(s.eprocessor.eo_roulette.run && i != -1)
    {
        if(!s.eprocessor.eo_roulette.play && s.map.characters[i].eor_payments < 16)
        {
            if(s.eprocessor.eo_roulette.winner == -1)
            {
                s.eoclient.TradeAccept(gameworld_id);
            }
            else
            {
                if(gameworld_id == s.eprocessor.eo_roulette.winner)
                {
                    s.eoclient.TradeAccept(gameworld_id);
                }
            }
        }
        else if(!s.eprocessor.eo_roulette.play && s.map.characters[i].eor_payments >= 16)
        {
            s.eprocessor.DelayedMessage("Sorry, you can only add gold up to 16 times.", 1000);
        }
    }
    else if(s.eprocessor.item_request.run)
    {
        if(gameworld_id == s.eprocessor.item_request.gameworld_id)
        {
            s.eoclient.TradeAccept(gameworld_id);
        }
    }
    else if(s.eprocessor.sitwin.run)
    {
        if(!s.eprocessor.sitwin.play)
        {
            if(s.eprocessor.sitwin.winner == -1)
            {
                if(gameworld_id == s.eprocessor.sitwin.gameworld_id && s.eprocessor.sitwin.item_id == 0)
                {
                    s.eoclient.TradeAccept(gameworld_id);
                }
            }
            else
            {
                if(gameworld_id == s.eprocessor.sitwin.winner)
                {
                    s.eoclient.TradeAccept(gameworld_id);
                }
            }
        }
    }
    else if(s.eprocessor.quest_gen.item_request.run)
    {
        if(gameworld_id == s.eprocessor.quest_gen.item_request.gameworld_id)
        {
            s.eoclient.TradeAccept(gameworld_id);
        }
    }
    else if(s.eprocessor.market.item_request.run)
    {
        if(gameworld_id == s.eprocessor.market.item_request.gameworld_id)
        {
            s.eoclient.TradeAccept(gameworld_id);
        }
    }
}

void Trade_Open(PacketReader reader)
{
    S &s = S::GetInstance();

    short gameworld_id = reader.GetShort();
    std::string name = reader.GetBreakString();
    /*short my_gameworld_id = */reader.GetShort();
    std::string my_name = reader.GetBreakString();

    s.eprocessor.trade = std::shared_ptr<Trade>(new Trade(gameworld_id));

    if(!s.eprocessor.eo_roulette.run && !s.eprocessor.item_request.run && !s.eprocessor.sitwin.run
       && !s.eprocessor.quest_gen.item_request.run && !s.eprocessor.market.item_request.run)
    {
        s.eoclient.TradeClose();
        return;
    }

    if(s.eprocessor.eo_roulette.run)
    {
        s.eprocessor.eo_roulette.clock.restart();

        if(s.eprocessor.eo_roulette.winner == -1)
        {
            s.eoclient.TradeAdd(1, 1);
        }
        else
        {
            s.eoclient.TradeAdd(1, s.eprocessor.eo_roulette.gold_given);
        }
    }
    else if(s.eprocessor.item_request.run)
    {
        s.eprocessor.item_request.clock.restart();

        if(s.eprocessor.item_request.give)
        {
            s.eoclient.TradeAdd(1, 1);
        }
        else
        {
            s.eoclient.TradeAdd(s.eprocessor.item_request.id, s.eprocessor.item_request.amount);
        }
    }
    else if(s.eprocessor.sitwin.run)
    {
        s.eprocessor.sitwin.clock.restart();

        if(s.eprocessor.sitwin.winner == -1)
        {
            s.eoclient.TradeAdd(1, 1);
        }
        else
        {
            s.eoclient.TradeAdd(s.eprocessor.sitwin.item_id, s.eprocessor.sitwin.item_amount);
        }
    }
    else if(s.eprocessor.quest_gen.item_request.run)
    {
        s.eprocessor.quest_gen.item_request.clock.restart();

        if(s.eprocessor.quest_gen.new_quest.get())
        {
            s.eoclient.TradeAdd(1, 1);
        }
        else
        {
            if(s.eprocessor.quest_gen.active_quest->complete)
            {
                for(unsigned int i = 0; i < s.eprocessor.quest_gen.active_quest->requirements.size(); ++i)
                {
                    short item_id = s.eprocessor.quest_gen.active_quest->requirements[i].first;
                    int item_amount = s.eprocessor.quest_gen.active_quest->requirements[i].second;

                    s.eoclient.TradeAdd(item_id, item_amount);
                }
            }
            else
            {
                short id = s.eprocessor.quest_gen.item_request.special_item.first;
                int amount = s.eprocessor.quest_gen.item_request.special_item.second;
                s.eoclient.TradeAdd(id, amount);
            }
        }
    }
    else if(s.eprocessor.market.item_request.run)
    {
        if(s.eprocessor.market.new_offer.get())
        {
            s.eoclient.TradeAdd(1, 1);
        }
        else
        {
            if(s.eprocessor.market.item_request.give)
            {
                if(s.eprocessor.market.active_offer->transaction_type == 0)
                {
                    if(s.eprocessor.market.active_offer->transaction_done)
                    {
                        short id = s.eprocessor.market.item_request.special_item.first;
                        int amount = s.eprocessor.market.item_request.special_item.second;
                        s.eoclient.TradeAdd(id, amount);
                    }
                    else
                    {
                        s.eoclient.TradeAdd(1, s.eprocessor.market.active_offer->price);
                    }
                }
                else
                {
                    if(s.eprocessor.market.active_offer->transaction_done)
                    {
                        s.eoclient.TradeAdd(1, s.eprocessor.market.active_offer->price);
                    }
                    else
                    {
                        short id = s.eprocessor.market.item_request.special_item.first;
                        int amount = s.eprocessor.market.item_request.special_item.second;
                        s.eoclient.TradeAdd(id, amount);
                    }
                }
            }
            else
            {
                if(s.eprocessor.market.active_offer->transaction_type == 0)
                {
                    s.eoclient.TradeAdd(1, s.eprocessor.market.active_offer->price);
                }
                else
                {
                    short id = s.eprocessor.market.item_request.special_item.first;
                    int amount = s.eprocessor.market.item_request.special_item.second;
                    s.eoclient.TradeAdd(id, amount);
                }
            }
        }
    }

    s.eoclient.TalkPublic("Trading...");
}

void Trade_Close(PacketReader reader) // other player closed trade
{
    S &s = S::GetInstance();
    short gameworld_id = reader.GetShort();

    s.eprocessor.trade.reset();

    if(s.eprocessor.eo_roulette.run && !s.eprocessor.eo_roulette.play)
    {
        int i = s.map.GetCharacterIndex(gameworld_id);
        if(i != -1)
        {
            s.map.characters[i].eor_payments++;
            s.eprocessor.eo_roulette.clock.restart();
            s.eprocessor.DelayedMessage("Trade canceled. The game will start in 30 seconds...", 1000);
        }
    }
}

void Trade_Reply(PacketReader reader) // update of trade items
{
    S &s = S::GetInstance();

    if(!s.eprocessor.trade.get()) return;

    short victim_id = 0;
    short gameworld_id = reader.GetShort();

    s.eprocessor.trade->player_items.clear();
    s.eprocessor.trade->victim_items.clear();
    while(reader.PeekByte() != 255)
    {
        short item_id = reader.GetShort();
        int item_amount = reader.GetInt();

        if(gameworld_id == s.character.gameworld_id)
            s.eprocessor.trade->player_items.push_back(std::make_pair(item_id, item_amount));
        else
        {
            s.eprocessor.trade->victim_items.push_back(std::make_pair(item_id, item_amount));
            victim_id = gameworld_id;
        }
    }
    reader.GetByte(); // 255

    gameworld_id = reader.GetShort();
    while(reader.PeekByte() != 255)
    {
        short item_id = reader.GetShort();
        int item_amount = reader.GetInt();

        if(gameworld_id == s.character.gameworld_id)
            s.eprocessor.trade->player_items.push_back(std::make_pair(item_id, item_amount));
        else
        {
            s.eprocessor.trade->victim_items.push_back(std::make_pair(item_id, item_amount));
            victim_id = gameworld_id;
        }
    }
    reader.GetByte(); // 255

    if(!s.eprocessor.eo_roulette.run && !s.eprocessor.item_request.run
       && !s.eprocessor.sitwin.run && !s.eprocessor.quest_gen.item_request.run
       && !s.eprocessor.market.item_request.run)
    {
        s.eoclient.TradeClose();
        return;
    }

    bool player_put_item = false;
    bool victim_put_item = false;

    if(s.eprocessor.eo_roulette.run || s.eprocessor.item_request.run || s.eprocessor.sitwin.run
       || s.eprocessor.quest_gen.item_request.run || s.eprocessor.market.item_request.run)
    {
        for(unsigned int i = 0; i < s.eprocessor.trade->victim_items.size(); ++i)
        {
            short id = std::get<0>(s.eprocessor.trade->victim_items[i]);
            int amount = std::get<1>(s.eprocessor.trade->victim_items[i]);

            if(s.eprocessor.eo_roulette.run)
            {
                if(s.eprocessor.eo_roulette.winner == -1 && id == 1 && amount > 1)
                {
                    victim_put_item = true;
                }
                else if(s.eprocessor.eo_roulette.winner != -1)
                {
                    victim_put_item = true;
                }
            }
            else if(s.eprocessor.item_request.run)
            {
                victim_put_item = true;
            }
            else if(s.eprocessor.sitwin.run)
            {
                if(s.eprocessor.sitwin.winner == -1)
                {
                    s.eprocessor.sitwin.item_id = id;
                    s.eprocessor.sitwin.item_amount = amount;
                    victim_put_item = true;
                }
                else
                {
                    victim_put_item = true;
                }
            }
            else if(s.eprocessor.quest_gen.item_request.run)
            {
                victim_put_item = true;
            }
            else if(s.eprocessor.market.item_request.run)
            {
                if(s.eprocessor.market.new_offer.get())
                {
                    if(s.eprocessor.market.new_offer->transaction_type == 0)
                    {
                        if(id == 1 && amount >= 1)
                        {
                            s.eprocessor.market.new_offer->price = amount;
                            victim_put_item = true;
                        }
                    }
                    else if(s.eprocessor.market.new_offer->transaction_type == 1)
                    {
                        s.eprocessor.market.new_offer->item = std::make_pair(id, amount);
                        victim_put_item = true;
                    }
                }
                else
                {
                    if(s.eprocessor.market.item_request.give)
                    {
                        victim_put_item = true;
                    }
                }
            }
        }

        for(unsigned int i = 0; i < s.eprocessor.trade->player_items.size(); ++i)
        {
            short id = std::get<0>(s.eprocessor.trade->player_items[i]);
            /*int amount = */std::get<1>(s.eprocessor.trade->player_items[i]);

            if(s.eprocessor.eo_roulette.run)
            {
                if(id == 1)
                {
                    player_put_item = true;
                }
            }
            else if(s.eprocessor.item_request.run)
            {
                player_put_item = true;
            }
            else if(s.eprocessor.sitwin.run)
            {
                player_put_item = true;
            }
            else if(s.eprocessor.quest_gen.item_request.run)
            {
                player_put_item = true;
            }
            else if(s.eprocessor.market.item_request.run)
            {
                player_put_item = true;
            }
        }
    }
    if(s.eprocessor.eo_roulette.run)
    {
        if(player_put_item && victim_put_item)
        {
            bool can_play = false;

            if(s.eprocessor.eo_roulette.payments.size() > 0)
            {
                int bet_price = s.eprocessor.eo_roulette.payments[s.eprocessor.eo_roulette.payments.size() - 1] / 3;

                if(s.eprocessor.trade->victim_items[0].second >= bet_price)
                {
                    can_play = true;
                }
                else if(s.eprocessor.eo_roulette.winner == -1)
                {
                    int i = s.map.GetCharacterIndex(victim_id);
                    std::string name = "";
                    if(i != -1) name = s.map.characters[i].name;
                    std::string price = std::to_string(bet_price);
                    s.eprocessor.DelayedMessage("You need to bet at least " + price + " gold (33% of the previous bet)."
                                                ,1, 1, name);
                }
            }
            else
            {
                can_play = true;
            }

            if(can_play || s.eprocessor.eo_roulette.winner != -1) s.eoclient.TradeAgree();
        }
    }
    if(s.eprocessor.item_request.run || s.eprocessor.sitwin.run)
    {
        if(player_put_item && victim_put_item)
        {
            s.eoclient.TradeAgree();
        }
    }
    if(s.eprocessor.quest_gen.item_request.run)
    {
        if(s.eprocessor.quest_gen.new_quest.get())
        {
            if(player_put_item && victim_put_item)
            {
                s.eoclient.TradeAgree();
            }
        }
        else
        {
            if(!s.eprocessor.quest_gen.item_request.requirements.empty())
            {
                if(s.eprocessor.quest_gen.active_quest->complete)
                {
                    if(player_put_item && victim_put_item)
                    {
                        s.eoclient.TradeAgree();
                    }
                }
                else
                {
                    if(s.eprocessor.quest_gen.item_request.MeetsRequirements(s.eprocessor.trade->victim_items))
                    {
                        s.eoclient.TradeAgree();
                    }
                }
            }
        }
    }
    if(s.eprocessor.market.item_request.run)
    {
        if(s.eprocessor.market.new_offer.get())
        {
            if(player_put_item && victim_put_item)
            {
                s.eoclient.TradeAgree();
            }
        }
        else
        {
            if(s.eprocessor.market.item_request.give)
            {
                if(player_put_item && victim_put_item)
                {
                    s.eoclient.TradeAgree();
                }
            }
            else
            {
                if(s.eprocessor.market.item_request.MeetsRequirements(s.eprocessor.trade->victim_items))
                {
                    s.eoclient.TradeAgree();
                }
            }
        }
    }
}

void Trade_Spec(PacketReader reader) // trade agree status update
{
    S &s = S::GetInstance();

    if(!s.eprocessor.trade.get()) return;

    unsigned char agree = reader.GetChar();
    s.eprocessor.trade->player_accepted = agree;
}

void Trade_Agree(PacketReader reader) // trade agree status update
{
    S &s = S::GetInstance();

    if(!s.eprocessor.trade.get()) return;

    /*short victim_gameworld_id = */reader.GetShort();
    unsigned char agree = reader.GetChar();

    s.eprocessor.trade->victim_accepted = agree;

    if(s.eprocessor.item_request.run && s.eprocessor.trade->victim_items.size() > 0)
    {
        s.eoclient.TradeAgree();
    }
}

void Trade_Use(PacketReader reader) // trade finished
{
    S &s = S::GetInstance();

    if(!s.eprocessor.trade.get()) return;

    short victim_id = 0;
    short gameworld_id = reader.GetShort();

    s.eprocessor.trade->player_items.clear();
    s.eprocessor.trade->victim_items.clear();
    while(reader.PeekByte() != 255)
    {
        short item_id = reader.GetShort();
        int item_amount = reader.GetInt();

        if(gameworld_id == s.character.gameworld_id)
            s.eprocessor.trade->player_items.push_back(std::make_pair(item_id, item_amount));
        else
        {
            s.eprocessor.trade->victim_items.push_back(std::make_pair(item_id, item_amount));
            victim_id = gameworld_id;
        }
    }
    reader.GetByte(); // 255

    gameworld_id = reader.GetShort();
    while(reader.PeekByte() != 255)
    {
        short item_id = reader.GetShort();
        int item_amount = reader.GetInt();

        if(gameworld_id == s.character.gameworld_id)
            s.eprocessor.trade->player_items.push_back(std::make_pair(item_id, item_amount));
        else
        {
            s.eprocessor.trade->victim_items.push_back(std::make_pair(item_id, item_amount));
            victim_id = gameworld_id;
        }
    }
    reader.GetByte(); // 255

    for(unsigned int i = 0; i < s.eprocessor.trade->victim_items.size(); ++i)
    {
        short id = std::get<0>(s.eprocessor.trade->victim_items[i]);
        int amount = std::get<1>(s.eprocessor.trade->victim_items[i]);

        s.inventory.AddItem(id, amount);

        if(id == 1 && s.eprocessor.eo_roulette.run)
        {
            int i = s.map.GetCharacterIndex(victim_id);
            s.eprocessor.eo_roulette.gold_given += amount;
            s.eprocessor.eo_roulette.payments.push_back(amount);
            if(i != -1)
            {
                s.map.characters[i].eor_payments++;

                bool playing = false;
                for(unsigned int ii = 0; ii < s.eprocessor.eo_roulette.players.size(); ++ii)
                {
                    if(s.eprocessor.eo_roulette.players[ii].gameworld_id == victim_id)
                        playing = true;
                }

                if(!playing) s.eprocessor.eo_roulette.players.push_back(s.map.characters[i]);
            }
        }
        if(s.eprocessor.sitwin.run)
        {
            if(s.eprocessor.sitwin.winner == -1)
            {
                s.eprocessor.sitwin.item_id = id;
                s.eprocessor.sitwin.item_amount = amount;
            }
        }
    }

    for(unsigned int i = 0; i < s.eprocessor.trade->player_items.size(); ++i)
    {
        short id = std::get<0>(s.eprocessor.trade->player_items[i]);
        int amount = std::get<1>(s.eprocessor.trade->player_items[i]);

        s.inventory.DelItem(id, amount);
    }

    short victim_gameworld_id = s.eprocessor.trade->victim_gameworld_id;

    if(s.eprocessor.eo_roulette.run)
    {
        std::string message;
        if(s.eprocessor.eo_roulette.winner == -1)
        {
            s.eprocessor.eo_roulette.clock.restart();

            message = std::string() + std::to_string(s.eprocessor.eo_roulette.gold_given);
            message += " gold in the bank. You can still add more gold. Starting in 30 seconds...";
        }
        else
        {
            s.eprocessor.eo_roulette.run = false;
            s.eprocessor.eo_roulette.gold_given = 0;
            if(s.eprocessor.eo_roulette.jackpot)
            {
                s.eprocessor.eo_roulette.total_gold = 0;
            }
            s.eprocessor.eo_roulette.jackpot = false;

            message = "The game has been finished.";
        }

        s.eoclient.TalkPublic(message);
    }
    else if(s.eprocessor.item_request.run)
    {
        s.eprocessor.item_request.run = false;

        if(s.eprocessor.item_request.special_item.first == 1)
        {
            if(s.eprocessor.item_request.give)
            {
                for(unsigned int i = 0; i < s.eprocessor.trade->victim_items.size(); ++i)
                {
                    s.eprocessor.donated.AddItem(s.eprocessor.trade->victim_items[i].first, s.eprocessor.trade->victim_items[i].second);
                }
                s.eprocessor.SaveDonated();

                std::string message = "Thank you ";
                int i = s.map.GetCharacterIndex(victim_gameworld_id);

                std::string name = s.map.characters[i].name;
                name[0] = std::toupper(s.map.characters[i].name[0]);
                if(i != -1) message += name + ".";
                message += " I love to be the service to you. ~Jimmyee.";
                DelayMessage delay_message(message, 1000);
                delay_message.channel = 1;
                delay_message.victim_name = s.map.characters[i].name;
                s.eprocessor.DelayedMessage(delay_message);
            }
            else
            {
                s.eprocessor.donated.DelItem(s.eprocessor.trade->player_items[0].first, s.eprocessor.trade->player_items[0].second);
                s.eprocessor.SaveDonated();
            }
        }
        else
        {
            if(s.eprocessor.item_request.give)
            {
                for(unsigned int i = 0; i < s.eprocessor.trade->victim_items.size(); ++i)
                {
                    s.eprocessor.free_inv.AddItem(s.eprocessor.trade->victim_items[i].first, s.eprocessor.trade->victim_items[i].second);
                }
                s.eprocessor.SaveFreeItems();

                std::string message = "Thank you ";
                int i = s.map.GetCharacterIndex(victim_gameworld_id);

                std::string name = s.map.characters[i].name;
                name[0] = std::toupper(s.map.characters[i].name[0]);
                if(i != -1) message += name + ".";
                message += " Now someone can take particular items if he/she needs to.";
                DelayMessage delay_message(message, 1000);
                delay_message.channel = 1;
                delay_message.victim_name = s.map.characters[i].name;
                s.eprocessor.DelayedMessage(delay_message);
            }
            else
            {
                s.eprocessor.free_inv.DelItem(s.eprocessor.trade->player_items[0].first, s.eprocessor.trade->player_items[0].second);
                s.eprocessor.SaveFreeItems();
            }
        }
    }
    else if(s.eprocessor.sitwin.run)
    {
        std::string message;
        if(s.eprocessor.sitwin.winner == -1)
        {
            s.eprocessor.sitwin.clock.restart();

            std::string item_name = s.eif->Get(s.eprocessor.sitwin.item_id).name;

            message = std::string() + "->" + item_name + " x" + std::to_string(s.eprocessor.sitwin.item_amount) + "<-";
            message += " in the bank.";
            message += " Please sit next to me to have a chance to win! Starting in 30 seconds...";
        }
        else
        {
            s.eprocessor.sitwin.run = false;
            message = "The game has been finished.";
        }

        s.eoclient.TalkPublic(message);
    }
    else if(s.eprocessor.quest_gen.item_request.run)
    {
        s.eprocessor.quest_gen.item_request.run = false;

        if(s.eprocessor.quest_gen.new_quest.get())
        {
            s.eprocessor.quest_gen.new_quest->id = s.eprocessor.quest_gen.GenerateID();
            s.eprocessor.quest_gen.new_quest->award = s.eprocessor.trade->victim_items[0];

            s.eprocessor.quest_gen.quests.push_back(*s.eprocessor.quest_gen.new_quest.get());
            s.eprocessor.quest_gen.Save();

            std::string item_name = s.eif->Get(s.eprocessor.quest_gen.new_quest->award.first).name;
            std::string item_amount = std::to_string(s.eprocessor.quest_gen.new_quest->award.second);

            std::string message = "Quest for " + item_name + " x" + item_amount + " created.";
            s.eprocessor.DelayedMessage(message, 1000);

            s.eprocessor.quest_gen.new_quest.reset();
        }
        else
        {
            if(!s.eprocessor.quest_gen.active_quest->complete)
            {
                s.eprocessor.quest_gen.active_quest->complete = true;
                s.eprocessor.quest_gen.UpdateQuest(s.eprocessor.quest_gen.active_quest->id, *s.eprocessor.quest_gen.active_quest.get());
                s.eprocessor.quest_gen.active_quest.reset();
                s.eprocessor.quest_gen.Save();

                int i = s.map.GetCharacterIndex(victim_gameworld_id);
                std::string name = s.map.characters[i].name;
                name[0] = std::toupper(s.map.characters[i].name[0]);
                std::string message = "Congratulations " + name + "! Quest complete!";
                s.eprocessor.DelayedMessage(message, 1000);
                std::string item_name =  s.eif->Get(s.eprocessor.quest_gen.item_request.special_item.first).name;
                std::string item_amount = std::to_string(s.eprocessor.quest_gen.item_request.special_item.second);
                message = item_name + " x" + item_amount + " is yours.";
                s.eprocessor.DelayedMessage(message, 1000);
            }
            else
            {
                s.eprocessor.quest_gen.RemoveQuest(s.eprocessor.quest_gen.active_quest->id);
                s.eprocessor.quest_gen.Save();
                s.eprocessor.quest_gen.active_quest.reset();

                int i = s.map.GetCharacterIndex(victim_gameworld_id);
                std::string name = s.map.characters[i].name;
                name[0] = std::toupper(s.map.characters[i].name[0]);
                std::string message = "Congratulations " + name + "!";
                s.eprocessor.DelayedMessage(message, 1000);
            }
        }
    }
    else if(s.eprocessor.market.item_request.run)
    {
        s.eprocessor.market.item_request.run = false;

        if(s.eprocessor.market.new_offer.get())
        {
            int i = s.map.GetCharacterIndex(victim_gameworld_id);
            std::string name = s.map.characters[i].name;

            s.eprocessor.market.new_offer->id = s.eprocessor.market.GenerateID();
            s.eprocessor.market.new_offer->player_name = name;
            s.eprocessor.market.offers.push_back(*s.eprocessor.market.new_offer.get());
            s.eprocessor.market.Save();

            std::string transaction_type = s.eprocessor.market.new_offer->transaction_type == 0? "BUY" : "SELL";
            std::string message = "Offer saved (" + transaction_type + " -> ";
            std::string item_name =  s.eif->Get(s.eprocessor.market.new_offer->item.first).name;
            std::string item_amount = std::to_string(s.eprocessor.market.new_offer->item.second);
            message += item_name + " x" + item_amount;
            message += " for " + std::to_string(s.eprocessor.market.new_offer->price) + " gold).";
            s.eprocessor.market.new_offer.reset();

            s.eprocessor.DelayedMessage(message, 1000);
        }
        else
        {
            if(s.eprocessor.market.item_request.give)
            {
                s.eprocessor.market.RemoveOffer(s.eprocessor.market.active_offer->id);
                s.eprocessor.market.active_offer.reset();
                s.eprocessor.market.Save();

                s.eprocessor.DelayedMessage("Offer successfully removed.", 1000);
            }
            else
            {
                s.eprocessor.market.active_offer->transaction_done = true;
                s.eprocessor.market.UpdateOffer(s.eprocessor.market.active_offer->id, *s.eprocessor.market.active_offer.get());
                s.eprocessor.market.Save();

                std::string transaction_type = s.eprocessor.market.active_offer->transaction_type == 0? "bought" : "sold";
                std::string item_name =  s.eif->Get(s.eprocessor.market.active_offer->item.first).name;
                std::string item_amount = std::to_string(s.eprocessor.market.active_offer->item.second);
                std::string message = item_name + " x" + item_amount + " sold for ";
                message += std::to_string(s.eprocessor.market.active_offer->price) + " gold.";
                s.eprocessor.market.active_offer.reset();

                s.eprocessor.DelayedMessage(message, 1000);
            }
        }
    }

    s.eprocessor.trade.reset();
}
