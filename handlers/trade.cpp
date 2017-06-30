#include "handlers.hpp"
#include "../singleton.hpp"

void Trade_Request(PacketReader reader)
{
    S &s = S::GetInstance();

    reader.GetChar(); // ?
    short gameworld_id = reader.GetShort();
    std::string name = reader.GetEndString();

    if(!s.eprocessor.trade.get() && s.eprocessor.eo_roulette.run)
    {
        if(!s.eprocessor.eo_roulette.play)
        {
            if(s.eprocessor.eo_roulette.winner == -1)
            {
                PacketBuilder packet(PacketFamily::Trade, PacketAction::Accept);
                packet.AddChar(138);
                packet.AddShort(gameworld_id);
                s.eoclient.Send(packet);
            }
            else
            {
                if(gameworld_id == s.eprocessor.eo_roulette.winner)
                {
                    PacketBuilder packet(PacketFamily::Trade, PacketAction::Accept);
                    packet.AddChar(138);
                    packet.AddShort(gameworld_id);
                    s.eoclient.Send(packet);
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

    if(!s.eprocessor.eo_roulette.run)
    {
        PacketBuilder packet(PacketFamily::Trade, PacketAction::Close);
        packet.AddChar(138);
        s.eoclient.Send(packet);
        return;
    }

    s.eprocessor.eo_roulette.clock.restart();

    if(s.eprocessor.eo_roulette.winner == -1)
    {
        PacketBuilder packet(PacketFamily::Trade, PacketAction::Add);
        packet.AddShort(1);
        packet.AddInt(1);
        s.eoclient.Send(packet);
    }
    else
    {
        PacketBuilder packet(PacketFamily::Trade, PacketAction::Add);
        packet.AddShort(1);
        packet.AddInt(s.eprocessor.eo_roulette.gold_given);
        s.eoclient.Send(packet);
    }

    s.eoclient.Talk("Trading...");
}

void Trade_Close(PacketReader reader) // other player closed trade
{
    short gameworld_id = reader.GetShort();

    S::GetInstance().eprocessor.trade.reset();
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

    if(!s.eprocessor.eo_roulette.run)
    {
        PacketBuilder packet(PacketFamily::Trade, PacketAction::Close);
        packet.AddChar(138);
        s.eoclient.Send(packet);
        return;
    }

    bool player_put_item = true;
    bool victim_put_item = false;

    int gold_amount = 0;
    for(unsigned int i = 0; i < s.eprocessor.trade->victim_items.size(); ++i)
    {
        short id = std::get<0>(s.eprocessor.trade->victim_items[i]);
        int amount = std::get<1>(s.eprocessor.trade->victim_items[i]);

        if(s.eprocessor.eo_roulette.winner == -1 && id == 1 && amount > 1)
        {
            victim_put_item = true;
        }
        else if(s.eprocessor.eo_roulette.winner != -1)
        {
            victim_put_item = true;
        }
    }

    for(unsigned int i = 0; i < s.eprocessor.trade->player_items.size(); ++i)
    {
        short id = std::get<0>(s.eprocessor.trade->player_items[i]);
        int amount = std::get<1>(s.eprocessor.trade->player_items[i]);

        if(id == 1)
        {
            player_put_item = true;
        }
    }

    if(player_put_item && victim_put_item)
    {
        PacketBuilder packet(PacketFamily::Trade, PacketAction::Agree);
        packet.AddChar(true);
        s.eoclient.Send(packet);
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
        s.inventory.push_back(s.eprocessor.trade->victim_items[i]);

        short id = std::get<0>(s.eprocessor.trade->victim_items[i]);
        int amount = std::get<1>(s.eprocessor.trade->victim_items[i]);

        if(id == 1 && s.eprocessor.eo_roulette.run)
        {
            s.eprocessor.eo_roulette.gold_given += amount;
        }
    }

    s.eprocessor.trade.reset();

    if(s.eprocessor.eo_roulette.run)

    {
        std::string message;
        if(s.eprocessor.eo_roulette.winner == -1)
        {
            s.eprocessor.eo_roulette.clock.restart();

            message = std::string() + std::to_string(s.eprocessor.eo_roulette.gold_given);
            message += " gold in the bank. You can still add more gold. Starting in 5 seconds...";
        }
        else
        {
            s.eprocessor.eo_roulette.run = false;
            message = "The game has been finished.";
        }

        s.eoclient.Talk(message);
    }
}
