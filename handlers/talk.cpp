// Endless Online Bot v0.0.1

#include "handlers.hpp"
#include "../singleton.hpp"
#include "../eventprocessor.hpp"

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

std::vector<std::string> ProcessCommand(std::string name, std::string message, short gameworld_id)
{
    S &s = S::GetInstance();
    std::vector<std::string> ret;

    std::vector<std::string> args = Args(message);

    std::string command = "";
    if(args.size() > 0)
    {
        command = args[0];
    }

    puts(message.c_str());
    printf("Command: '%s' %i arguments\n", command.c_str(), args.size());
    if(name == s.config.GetValue("Master"))
    {
        if(command == "exit")
        {
            s.call_exit = true;
        }
        else if(command == "relog")
        {
            s.eoclient.Disconnect();
        }
        else if(command == "rchase")
        {
            s.eprocessor.chase_bot.Reset();
        }
        else if(command == "dg" && args.size() == 2)
        {
            int amount = std::atoi(args[1].c_str());

            int amount_per_tile = amount / 4;
            int xoff[4] = { 0, 1, 0, -1 };
            int yoff[4] = { -1, 0, 1, 0 };
            for(int i = 0; i < 4; ++i)
            {
                if(i == 3 && amount % 4 != 0) amount_per_tile += amount % 4;

                PacketBuilder packet(PacketFamily::Item, PacketAction::Drop);
                packet.AddShort(1);
                packet.AddInt(amount_per_tile);
                packet.AddChar(s.character.x + xoff[i]);
                packet.AddChar(s.character.y + yoff[i]);
                s.eoclient.Send(packet);
            }
        }
        else if(command == "setjp" && args.size() == 2)
        {
            int amount = std::atoi(args[1].c_str());

            s.eprocessor.eo_roulette.total_gold = amount;

            ret.push_back(std::string() + "Jackpot set to " + std::to_string(amount) + " gold.");
        }
        else if(command == "jp")
        {
            s.eprocessor.eo_roulette.gameworld_id = -1;
            s.eprocessor.eo_roulette.gold_given = s.eprocessor.eo_roulette.total_gold;
            s.eprocessor.eo_roulette.spins = 0;
            s.eprocessor.eo_roulette.max_spins = S::GetInstance().rand_gen.RandInt(12, 24);
            s.eprocessor.eo_roulette.spin_delay = 100;
            s.eprocessor.eo_roulette.play = true;
            s.eprocessor.eo_roulette.winner = -1;
            s.eprocessor.eo_roulette.clock.restart();
            s.eprocessor.eo_roulette.run = true;
            s.eprocessor.eo_roulette.jackpot = true;

            s.eprocessor.eo_roulette.jackpot_clock.restart();
        }
        else if(command == "setjptime" && args.size() == 2)
        {
            int jp_time = std::atoi(args[1].c_str());

            s.eprocessor.eo_roulette.jp_time = jp_time;

            int hours = jp_time / 3600;
            int minutes = (jp_time % 3600) / 60;
            int seconds = (jp_time % 3600) % 60;

            std::string message = "Jackpot time set to ";
            message += std::to_string(hours) + "h " + std::to_string(minutes) + "m " + std::to_string(seconds) + "s.";
            ret.push_back(message);
        }
        else if(command == "finditem" && args.size() >= 2)
        {
            std::string item_name = "";
            for(unsigned int i = 1; i < args.size(); ++i)
            {
                std::string word = args[i];

                std::transform(word.begin(), word.end(), word.begin(), ::tolower);
                word[0] = std::toupper(args[i][0]);
                item_name += word;
                if(i != args.size() - 1)
                {
                    item_name += " ";
                }
            }

            EIF_Data eif_data = s.eif->GetByName(item_name);
            int item_id = eif_data.id;
            int item_amount = s.inventory.GetItemAmount(item_id);

            std::string message = item_name;
            message += " amount: " + std::to_string(item_amount);
            ret.push_back(message);
        }
        else if(command == "tradeitem" && args.size() >= 3 && !s.eprocessor.eo_roulette.run && !s.eprocessor.item_request.run)
        {
            std::string item_name = "";
            for(unsigned int i = 1; i < args.size() - 1; ++i)
            {
                std::string word = args[i];

                std::transform(word.begin(), word.end(), word.begin(), ::tolower);
                word[0] = std::toupper(args[i][0]);
                item_name += word;
                if(i < args.size() - 2)
                {
                    item_name += " ";
                }
            }

            EIF_Data eif_data = s.eif->GetByName(item_name);
            int item_id = eif_data.id;
            int item_amount = std::atoi(args[args.size() - 1].c_str());
            bool found = s.inventory.FindItem(item_id, item_amount);

            if(found)
            {
                ret.push_back(std::string() + "Item " + item_name + " x" + std::to_string(item_amount) + " found.");

                s.eprocessor.item_request.id = item_id;
                s.eprocessor.item_request.amount = item_amount;
                s.eprocessor.item_request.gameworld_id = gameworld_id;
                s.eprocessor.item_request.give = false;
                s.eprocessor.item_request.run = true;
                s.eprocessor.item_request.clock.restart();

                PacketBuilder packet(PacketFamily::Trade, PacketAction::Request);
                packet.AddChar(138);
                packet.AddShort(gameworld_id);
                s.eoclient.Send(packet);
            }
            else
            {
                ret.push_back(std::string() + "Item " + item_name + " not found.");
            }
        }
        else if(command == "inventory")
        {
            std::string message = "Random item from my inventory: ";

            if(!s.inventory.items.empty())
            {
                int item_id = s.inventory.items[s.rand_gen.RandInt(0, s.inventory.items.size() - 1)].first;
                std::string item_name = s.eif->Get(item_id).name;
                message += item_name;
            }
            else
            {
                message += "<no items>.";
            }

            ret.push_back(message);
        }
        else
        {
            puts("Wrong master command");
        }
    }

    if(command == "eor" && !s.eprocessor.eo_roulette.run && !s.eprocessor.item_request.run)
    {
        s.eprocessor.eo_roulette.Run(gameworld_id);

        std::string name_upper = name;
        name_upper[0] = std::toupper(name[0]);

        s.eoclient.TalkPublic("Welcome to EORoulette! Please trade me gold (at least 2g) to begin the game. You've got 12 seconds.");
    }
    else if(command == "uptime")
    {
        int elapsed = s.eprocessor.uptime_clock.getElapsedTime().asSeconds();

        int hours = elapsed / 3600;
        int minutes = (elapsed % 3600) / 60;
        int seconds = (elapsed % 3600) % 60;

        std::string message = std::to_string(hours) + "h " + std::to_string(minutes) + "m " + std::to_string(seconds) + "s.";

        ret.push_back(message);
    }
    else if(command == "trade")
    {
        if(!s.eprocessor.trade.get() && s.eprocessor.eo_roulette.run)
        {
            if(!s.eprocessor.eo_roulette.play && s.eprocessor.eo_roulette.winner == -1 && s.eprocessor.eo_roulette.payments < 16)
            {
                PacketBuilder packet(PacketFamily::Trade, PacketAction::Request);
                packet.AddChar(138);
                packet.AddShort(gameworld_id);
                s.eoclient.Send(packet);
            }
            else if(!s.eprocessor.eo_roulette.play && s.eprocessor.eo_roulette.winner == -1 && s.eprocessor.eo_roulette.payments >= 16)
            {
                s.eprocessor.DelayedMessage("Sorry, trade limit reached. The game will start shortly", 1000);
            }
        }
    }
    else if(command == "jackpot")
    {
        std::string message = "Jackpot: ";
        message += std::to_string(s.eprocessor.eo_roulette.total_gold) + " gold.";

        ret.push_back(message);
    }
    else if(command == "jackpotxxl")
    {
        std::string message = "Jackpot XXL: ";
        int amount = s.inventory.GetItemAmount(1);
        message += std::to_string(amount - s.eprocessor.eo_roulette.total_gold) + " gold.";

        ret.push_back(message);
    }
    else if(command == "jptime")
    {
        int elapsed = s.eprocessor.eo_roulette.jp_time - s.eprocessor.eo_roulette.jackpot_clock.getElapsedTime().asSeconds();

        int hours = elapsed / 3600;
        int minutes = (elapsed % 3600) / 60;
        int seconds = (elapsed % 3600) % 60;

        std::string message = "Time left for the jackpot game: ";
        message += std::to_string(hours) + "h " + std::to_string(minutes) + "m " + std::to_string(seconds) + "s.";
        ret.push_back(message);
        message = "Date for the jackpot XXL game (once a month): ";
        message += "4th day of particular month. Jackpot XXL amount: #jackpotxxl";
        ret.push_back(message);
    }
    else if(command == "wru")
    {
        std::string new_message = "I'm at map #";
        new_message += std::to_string(s.emf->id) + (s.emf->id == 5? " (Aeven)" : "");
        new_message += ", x: " + std::to_string(s.character.x) + ", y: " + std::to_string(s.character.y);

        ret.push_back(new_message);
    }
    else if(command == "help")
    {
        std::string message = "Commands: #help, #help_eor, #eor (EORoulette), #trade (eor), #uptime, #jackpot, #jptime (jackpot time).";
        ret.push_back(message);
        message = "#hscore (highest jackpot wins), #wru (where are you), #getitem (item name here) - get specified item from the bot";
        ret.push_back(message);
        message = "#giveitem - give items to the bot so other players can get them when they need to. #jackpotxxl.";
        ret.push_back(message);
        message = "#gitem - same like #getitem but doesn't force text to upper or lower case. #items (amount of inventory items).";
        ret.push_back(message);
        message = "You can cast commands through public and private chat";
        ret.push_back(message);
    }
    else if(command == "help_eor")
    {
        std::string message = "EORoulette instructions: type #eor to start the game. At least 2 players is needed to play this game effectively.";
        ret.push_back(message);
        message = "After you type #eor, trade desired amount of gold to the bot. It won't start the game if it has no gold.";
        ret.push_back(message);
        message = "Players that didn't give any gold don't play the game, excluding jackpot game - it's played for everyone.";
        ret.push_back(message);
        message = "The bot will rotate itself around and will stop at particular direction. The player the bot is faced to wins all the gold given.";
        ret.push_back(message);
        message = "Special jackpot game is played every 4 hours. Find the bot to have your chance to win big pize.";
        ret.push_back(message);
    }
    else if(command == "hscore")
    {
        std::vector<Config::Entry> entries = s.eprocessor.eo_roulette.jpconfig.entries;
        std::string winners;

        for(int place = 0; place < 3; ++place)
        {
            int highest = 0;
            int index = -1;
            for(unsigned int i = 0; i < entries.size(); ++i)
            {
                printf("'%s'\n", entries[i].value.c_str());
                std::vector<std::string> args = Args(entries[i].value);
                if(args.size() == 2)
                {
                    int amount = std::atoi(args[1].c_str());
                    if(amount > highest)
                    {
                        highest = amount;
                        index = i;
                    }
                }
            }

            if(highest > 0)
            {
                std::vector<std::string> args = Args(entries[index].value);

                std::string name = args[0];
                std::string name_upper = name;
                name_upper[0] = std::toupper(name[0]);

                winners += std::to_string(place + 1) + ". ";
                winners += name_upper + ": " + args[1] + " gold. ";

                entries.erase(entries.begin() + index);
            }
        }

        ret.push_back(winners);
    }
    else if(command == "gitem" && args.size() >= 2 && !s.eprocessor.trade.get() && !s.eprocessor.eo_roulette.run && !s.eprocessor.item_request.run)
    {
        std::string item_name = "";
        for(unsigned int i = 1; i < args.size(); ++i)
        {
            std::string word = args[i];
            item_name += word;
            if(i != args.size() - 1)
            {
                item_name += " ";
            }
        }

        int item_id = s.eif->GetByName(item_name).id;
        bool found = s.inventory.FindItem(item_id, s.eprocessor.item_request.amount);
        bool locked = false;

        if(item_id == 1)
        {
            locked = true;
        }

        if(!found)
        {
            item_id++;
            std::string found_name = s.eif->Get(item_id).name;
            if(found_name == item_name)
            {
                found = s.inventory.FindItem(item_id, s.eprocessor.item_request.amount);
            }
        }

        if(found && !locked)
        {
            s.eprocessor.item_request.id = item_id;
            s.eprocessor.item_request.amount = 1;
            s.eprocessor.item_request.gameworld_id = gameworld_id;
            s.eprocessor.item_request.give = false;
            s.eprocessor.item_request.run = true;
            s.eprocessor.item_request.clock.restart();

            PacketBuilder packet(PacketFamily::Trade, PacketAction::Request);
            packet.AddChar(138);
            packet.AddShort(gameworld_id);
            s.eoclient.Send(packet);
        }

        std::string message = "Item ";
        if(found && !locked)
        {
            message += "found. Please trade me to receive it (available for 12 seconds).";
        }
        else
        {
            if(locked && name != s.config.GetValue("Master"))
            {
                message += item_name + " is locked from giving away. It's used for EORoulette.";
            }
            else if(!locked)
            {
                message += "is not found in my inventory.";
            }
        }

        s.eprocessor.DelayedMessage(message, 1000);
    }
    else if(command == "getitem" && args.size() >= 2 && !s.eprocessor.trade.get() && !s.eprocessor.eo_roulette.run && !s.eprocessor.item_request.run)
    {
        std::string item_name = "";
        for(unsigned int i = 1; i < args.size(); ++i)
        {
            std::string word = args[i];

            std::transform(word.begin(), word.end(), word.begin(), ::tolower);
            word[0] = std::toupper(args[i][0]);
            item_name += word;
            if(i != args.size() - 1)
            {
                item_name += " ";
            }
        }

        int item_id = s.eif->GetByName(item_name).id;
        bool found = s.inventory.FindItem(item_id, s.eprocessor.item_request.amount);
        bool locked = false;

        if(item_name == "Gold" || item_id == 1)
        {
            locked = true;
        }

        if(!found)
        {
            item_id++;
            std::string found_name = s.eif->Get(item_id).name;
            if(found_name == item_name)
            {
                found = s.inventory.FindItem(item_id, s.eprocessor.item_request.amount);
            }
        }

        if(found && !locked)
        {
            s.eprocessor.item_request.id = item_id;
            s.eprocessor.item_request.amount = 1;
            s.eprocessor.item_request.gameworld_id = gameworld_id;
            s.eprocessor.item_request.give = false;
            s.eprocessor.item_request.run = true;
            s.eprocessor.item_request.clock.restart();

            PacketBuilder packet(PacketFamily::Trade, PacketAction::Request);
            packet.AddChar(138);
            packet.AddShort(gameworld_id);
            s.eoclient.Send(packet);
        }

        std::string message = "Item ";
        if(found && !locked)
        {
            message += "found. Please trade me to receive it (available for 12 seconds).";
        }
        else
        {
            if(locked && name != s.config.GetValue("Master"))
            {
                message += item_name + " is locked from giving away. It's used for EORoulette.";
            }
            else if(!locked)
            {
                message += "is not found in my inventory.";
            }
        }

        s.eprocessor.DelayedMessage(message, 1000);
    }
    else if(command == "giveitem" && !s.eprocessor.trade.get() && !s.eprocessor.eo_roulette.run && !s.eprocessor.item_request.run)
    {
        s.eprocessor.item_request.gameworld_id = gameworld_id;
        s.eprocessor.item_request.give = true;
        s.eprocessor.item_request.run = true;
        s.eprocessor.item_request.clock.restart();

        PacketBuilder packet(PacketFamily::Trade, PacketAction::Request);
        packet.AddChar(138);
        packet.AddShort(gameworld_id);
        s.eoclient.Send(packet);

        std::string message = "Please trade me the items you want to give (available 12 seconds).";
        s.eprocessor.DelayedMessage(message, 1000);
    }
    else if(command == "lvl" && args.size() >= 2)
    {
        std::string char_name = args[1];

        std::transform(char_name.begin(), char_name.end(), char_name.begin(), ::tolower);

        int index = s.map.GetCharacterIndex(char_name);

        std::string message = "";
        if(index != -1)
        {
            message += char_name + "'s level: ";
            message += std::to_string(s.map.characters[index].level);
        }
        else
        {
            message += "Player not found.";
        }

        ret.push_back(message);
    }
    else if(command == "items")
    {
        std::string message = "Inventory contains ";
        message += std::to_string(s.inventory.items.size()) + " items.";

        ret.push_back(message);
    }

    return ret;
}

void Talk_Player(PacketReader reader)
{
    S &s = S::GetInstance();

    short gameworld_id = reader.GetShort();
    std::string message = reader.GetEndString();

    std::string name = "[Unknown]";
    int i = s.map.GetCharacterIndex(gameworld_id);
    if(i != -1)
    {
        name = s.map.characters[i].name;
    }

    if(message[0] == '#')
    {
        message.erase(0, 1);

        std::vector<std::string> ret = ProcessCommand(name, message, gameworld_id);
        std::vector<std::string> args = Args(message);

        for(unsigned int i = 0; i < ret.size(); ++i)
        {
            EventProcessor::DelayMessage delay_message(ret[i], 1000);
            if(ret[0].length() > 32)
            {
                delay_message.channel = 1;
                std::string name_lower = name;
                name_lower[0] = std::tolower(name[0]);
                delay_message.victim_name = name_lower;
            }

            s.eprocessor.DelayedMessage(delay_message);
        }
    }
    else
    {
        if(name != "panddda" && name != "sordie")
        {
            s.eprocessor.chat_bot.ProcessMessage(message);
        }
    }
}

void Talk_Tell(PacketReader reader)
{
    S &s = S::GetInstance();

    std::string name = reader.GetBreakString();
	std::string message = reader.GetBreakString();

	if(message[0] == '#')
    {
        message.erase(0, 1);

        int index = s.map.GetCharacterIndex(name);

        std::vector<std::string> ret = ProcessCommand(name, message, s.map.characters[index].gameworld_id);
        for(unsigned int i = 0; i < ret.size(); ++i)
        {
            EventProcessor::DelayMessage delay_message(ret[i], 1000);
            delay_message.channel = 1;
            delay_message.victim_name = name;
            s.eprocessor.DelayedMessage(delay_message);
        }
    }
}

void Talk_Message(PacketReader reader)
{
    std::string name = reader.GetBreakString();
    std::string message = reader.GetBreakString();

    S &s = S::GetInstance();

	if(message[0] == '#' && name == s.config.GetValue("Master"))
    {
        message.erase(0, 1);

        int index = s.map.GetCharacterIndex(name);

        std::vector<std::string> ret = ProcessCommand(name, message, s.map.characters[index].gameworld_id);
        for(unsigned int i = 0; i < ret.size(); ++i)
        {
            EventProcessor::DelayMessage delay_message(ret[i], 1000);
            delay_message.channel = 2;
            delay_message.victim_name = name;
            s.eprocessor.DelayedMessage(delay_message);
        }
    }
}
