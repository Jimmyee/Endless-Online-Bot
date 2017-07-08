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
        else if(!s.eprocessor.eo_roulette.play && s.eprocessor.eo_roulette.payments >= 16)
        {
            s.eprocessor.DelayedMessage("Sorry, you can only add gold up to 16 times.", 1000);
        }
    }
    else if(s.eprocessor.item_request.run)
    {
        if(gameworld_id == s.eprocessor.item_request.gameworld_id)
        {
            PacketBuilder packet(PacketFamily::Trade, PacketAction::Accept);
            packet.AddChar(138);
            packet.AddShort(gameworld_id);
            s.eoclient.Send(packet);
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

    if(!s.eprocessor.eo_roulette.run && !s.eprocessor.item_request.run)
    {
        s.eprocessor.trade.reset();

        PacketBuilder packet(PacketFamily::Trade, PacketAction::Close);
        packet.AddChar(138);
        s.eoclient.Send(packet);
        return;
    }

    if(s.eprocessor.eo_roulette.run)
    {
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
    }
    else if(s.eprocessor.item_request.run)
    {
        s.eprocessor.item_request.clock.restart();

        if(s.eprocessor.item_request.give)
        {
            PacketBuilder packet(PacketFamily::Trade, PacketAction::Add);
            packet.AddShort(1);
            packet.AddInt(1);
            s.eoclient.Send(packet);
        }
        else
        {
            PacketBuilder packet(PacketFamily::Trade, PacketAction::Add);
            packet.AddShort(s.eprocessor.item_request.id);
            packet.AddInt(s.eprocessor.item_request.amount);
            s.eoclient.Send(packet);
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

    if(!s.eprocessor.eo_roulette.run && !s.eprocessor.item_request.run)
    {
        s.eprocessor.trade.reset();

        PacketBuilder packet(PacketFamily::Trade, PacketAction::Close);
        packet.AddChar(138);
        s.eoclient.Send(packet);
        return;
    }

    if(s.eprocessor.eo_roulette.run)
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
        }

        if(player_put_item && victim_put_item)
        {
            PacketBuilder packet(PacketFamily::Trade, PacketAction::Agree);
            packet.AddChar(true);
            s.eoclient.Send(packet);
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
        PacketBuilder packet(PacketFamily::Trade, PacketAction::Agree);
        packet.AddChar(true);
        s.eoclient.Send(packet);
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
            s.eprocessor.eo_roulette.gold_given = 0;
            if(s.eprocessor.eo_roulette.jackpot)
            {
                s.eprocessor.eo_roulette.total_gold = 0;
                s.eprocessor.eo_roulette.jackpot = false;
            }
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
            s.eprocessor.DelayedMessage(message, 1000);
        }
    }
}
