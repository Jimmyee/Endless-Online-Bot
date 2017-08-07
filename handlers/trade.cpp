#include "handlers.hpp"
#include "../singleton.hpp"

void Trade_Request(PacketReader reader)
{
    S &s = S::GetInstance();

    reader.GetChar(); // ?
    short gameworld_id = reader.GetShort();
    std::string name = reader.GetEndString();

    if(s.eprocessor.trade.get()) return;

    if(s.eprocessor.eo_roulette.run)
    {
        if(!s.eprocessor.eo_roulette.play && s.eprocessor.eo_roulette.payments < 10)
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
        else if(!s.eprocessor.eo_roulette.play && s.eprocessor.eo_roulette.payments >= 16)
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
}

void Trade_Open(PacketReader reader)
{
    S &s = S::GetInstance();

    short gameworld_id = reader.GetShort();
    std::string name = reader.GetBreakString();
    short my_gameworld_id = reader.GetShort();
    std::string my_name = reader.GetBreakString();

    s.eprocessor.trade = std::shared_ptr<EventProcessor::Trade>(new EventProcessor::Trade(gameworld_id));

    if(!s.eprocessor.eo_roulette.run && !s.eprocessor.item_request.run && !s.eprocessor.sitwin.run)
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

    s.eoclient.TalkPublic("Trading...");
}

void Trade_Close(PacketReader reader) // other player closed trade
{
    S &s = S::GetInstance();
    short gameworld_id = reader.GetShort();

    s.eprocessor.trade.reset();

    if(s.eprocessor.eo_roulette.run && !s.eprocessor.eo_roulette.play)
    {
        s.eprocessor.eo_roulette.payments++;
    }
}

void Trade_Reply(PacketReader reader) // update of trade items
{
    S &s = S::GetInstance();

    if(!s.eprocessor.trade.get()) return;

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
            s.eprocessor.trade->victim_items.push_back(std::make_pair(item_id, item_amount));
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
            s.eprocessor.trade->victim_items.push_back(std::make_pair(item_id, item_amount));
    }
    reader.GetByte(); // 255

    if(!s.eprocessor.eo_roulette.run && !s.eprocessor.item_request.run && !s.eprocessor.sitwin.run)
    {
        s.eoclient.TradeClose();
        return;
    }

    if(s.eprocessor.eo_roulette.run || s.eprocessor.sitwin.run)
    {
        bool player_put_item = true;
        bool victim_put_item = false;

        int gold_amount = 0;
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
        }

        for(unsigned int i = 0; i < s.eprocessor.trade->player_items.size(); ++i)
        {
            short id = std::get<0>(s.eprocessor.trade->player_items[i]);
            int amount = std::get<1>(s.eprocessor.trade->player_items[i]);

            if(s.eprocessor.eo_roulette.run)
            {
                if(id == 1)
                {
                    player_put_item = true;
                }
            }
            else if(s.eprocessor.sitwin.run)
            {
                player_put_item = true;
            }
        }

        if(player_put_item && victim_put_item)
        {
            s.eoclient.TradeAgree();
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

    short victim_gameworld_id = reader.GetShort();
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
            s.eprocessor.trade->victim_items.push_back(std::make_pair(item_id, item_amount));
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
            s.eprocessor.trade->victim_items.push_back(std::make_pair(item_id, item_amount));
    }
    reader.GetByte(); // 255

    for(unsigned int i = 0; i < s.eprocessor.trade->victim_items.size(); ++i)
    {
        short id = std::get<0>(s.eprocessor.trade->victim_items[i]);
        int amount = std::get<1>(s.eprocessor.trade->victim_items[i]);

        s.inventory.AddItem(id, amount);

        if(id == 1 && s.eprocessor.eo_roulette.run)
        {
            s.eprocessor.eo_roulette.gold_given += amount;
            s.eprocessor.eo_roulette.payments++;
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

    s.eprocessor.trade.reset();

    if(s.eprocessor.eo_roulette.run)
    {
        std::string message;
        if(s.eprocessor.eo_roulette.winner == -1)
        {
            s.eprocessor.eo_roulette.clock.restart();

            message = std::string() + std::to_string(s.eprocessor.eo_roulette.gold_given);
            message += " gold in the bank. You can still add more gold. Starting in 12 seconds...";
        }
        else
        {
            s.eprocessor.eo_roulette.run = false;
            int thirdpart = s.eprocessor.eo_roulette.gold_given / 3;
            s.eprocessor.eo_roulette.gold_given = 0;
            if(s.eprocessor.eo_roulette.jackpot)
            {
                s.eprocessor.eo_roulette.total_gold = 0;
            }
            else
            {
                s.eprocessor.eo_roulette.total_gold += thirdpart;
            }
            s.eprocessor.eo_roulette.jackpot = false;

            message = "The game has been finished.";
        }

        s.eoclient.TalkPublic(message);
    }
    else if(s.eprocessor.item_request.run)
    {
        s.eprocessor.item_request.run = false;

        if(s.eprocessor.item_request.give)
        {
            std::string message = "Thank you ";
            int i = s.map.GetCharacterIndex(victim_gameworld_id);

            std::string name = s.map.characters[i].name;
            name[0] = std::toupper(s.map.characters[i].name[0]);
            if(i != -1) message += name + ".";
            message += " Now someone can take particular items if he/she needs to.";
            EventProcessor::DelayMessage delay_message(message, 1000);
            delay_message.channel = 1;
            delay_message.victim_name = s.map.characters[i].name;
            s.eprocessor.DelayedMessage(delay_message);
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
            message += " Please sit next to me to have a chance to win! Starting in 20 seconds...";
        }
        else
        {
            s.eprocessor.sitwin.run = false;
            message = "The game has been finished.";
        }

        s.eoclient.TalkPublic(message);
    }
}
