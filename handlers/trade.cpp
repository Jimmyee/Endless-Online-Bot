#include "handlers.hpp"
#include "../singleton.hpp"

void Trade_Request(PacketReader reader)
{
    S &s = S::GetInstance();

    reader.GetChar(); // ?
    short gameworld_id = reader.GetShort();
    std::string name = reader.GetEndString();

    if(!s.eprocessor.trade.get() && !s.eprocessor.sex_act.get())
    {
        PacketBuilder packet(PacketFamily::Trade, PacketAction::Accept);
        packet.AddChar(1);
        packet.AddShort(gameworld_id);
        s.eoclient.Send(packet);
    }
    else if(s.eprocessor.sex_act.get())
    {
        std::string name = name;
        name[0] = std::toupper(name[0]);

        s.eoclient.Talk(std::string() + name + ": sorry my darling I'm busy at the moment.");
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

    PacketBuilder packet(PacketFamily::Trade, PacketAction::Add);
    packet.AddShort(1);
    packet.AddInt(1);
    s.eoclient.Send(packet);
}

void Trade_Close(PacketReader reader) // other player closed trade
{
    short gameworld_id = reader.GetShort();

    S::GetInstance().eprocessor.trade.reset();
}

void Trade_Reply(PacketReader reader) // update of trade items
{
    S &s = S::GetInstance();

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

    bool player_put_gold = true;
    bool victim_put_gold = false;

    int gold_amount = 0;
    for(unsigned int i = 0; i < s.eprocessor.trade->victim_items.size(); ++i)
    {
        short id = std::get<0>(s.eprocessor.trade->victim_items[i]);
        int amount = std::get<1>(s.eprocessor.trade->victim_items[i]);

        if(id == 1 && amount > 1)
        {
            victim_put_gold = true;

            gold_amount = amount;
        }
    }

    for(unsigned int i = 0; i < s.eprocessor.trade->player_items.size(); ++i)
    {
        short id = std::get<0>(s.eprocessor.trade->player_items[i]);
        int amount = std::get<1>(s.eprocessor.trade->player_items[i]);

        if(id == 1 && amount > 1)
        {
            player_put_gold = true;
        }
    }

    if(player_put_gold && victim_put_gold)
    {
        EventProcessor::SexAct sex_act(s.eprocessor.trade->victim_gameworld_id);
        sex_act.max_sits = gold_amount * 5;
        s.eprocessor.sex_act = shared_ptr<EventProcessor::SexAct>(new EventProcessor::SexAct(sex_act));

        PacketBuilder packet(PacketFamily::Trade, PacketAction::Agree);
        packet.AddChar(true);
        s.eoclient.Send(packet);
    }
}

void Trade_Spec(PacketReader reader) // trade agree status update
{
    S &s = S::GetInstance();

    unsigned char agree = reader.GetChar();
    s.eprocessor.trade->player_accepted = agree;
}

void Trade_Agree(PacketReader reader) // trade agree status update
{
    S &s = S::GetInstance();

    short victim_gameworld_id = reader.GetShort();
    unsigned char agree = reader.GetChar();

    s.eprocessor.trade->victim_accepted = agree;
}

void Trade_Use(PacketReader reader) // trade finished
{
    S &s = S::GetInstance();

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

    for(unsigned int i = 0; i < s.eprocessor.trade->player_items.size(); ++i)
    {
        s.inventory.push_back(s.eprocessor.trade->player_items[i]);
    }

    s.eprocessor.trade.reset();

    s.eprocessor.sex_act->clock.restart();
    s.eprocessor.sex_act->timeout_clock.restart();

    int i = s.map.GetCharacterIndex(s.eprocessor.sex_act->victim_gameworld_id);
    std::string name = s.map.characters[i].name;
    name[0] = std::toupper(s.map.characters[i].name[0]);

    std::string message = std::string() + "Hey " + name + ", you have paid me gold so we can fuck now. Come and put your neat ass on me.";
    message += " You've got like " + std::to_string(s.eprocessor.sex_act->max_sits) + " seconds.";

    s.eoclient.Talk(message);

}
