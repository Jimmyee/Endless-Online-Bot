// Endless Online Bot v0.0.1

#include "handlers.hpp"
#include "../singleton.hpp"
#include "../eventprocessor.hpp"
#include "../util.hpp"

std::vector<std::string> ProcessCommand(std::string name, std::string message, short gameworld_id, int channel)
{
    S &s = S::GetInstance();

    std::vector<std::string> ret;

    int char_index = s.map.GetCharacterIndex(gameworld_id);

    if(char_index != -1)
    {
        if(s.map.characters[char_index].command_clock.getElapsedTime().asMilliseconds() <= 600)
        {
            return ret;
        }
        else
        {
            s.map.characters[char_index].command_clock.restart();
        }
    }

    std::vector<std::string> args = Args(message);

    std::string command = "";
    if(args.size() > 0)
    {
        command = args[0];
    }

    puts(message.c_str());
    printf("Command: '%s' %i arguments\n", command.c_str(), (int)args.size());
    if(name == s.config.GetValue("Master"))
    {
        if(command == "reload")
        {
            s.config.Reload("config.ini");
        }
        else if(command == "peekitem" && args.size() >= 2)
        {
            std::string item_name = "";
            for(unsigned int i = 1; i < args.size(); ++i)
            {
                item_name += args[i];
                if(i != args.size() - 1)
                {
                    item_name += " ";
                }
            }

            EIF_Data eif_data = s.eif->GetByNameLowercase(item_name);
            int item_id = eif_data.id;
            int item_amount = s.inventory.GetItemAmount(item_id);

            std::string message = item_name;
            message += " amount: " + std::to_string(item_amount);
            ret.push_back(message);
        }
        else if(command == "sentence")
        {
            std::string message = s.eprocessor.chat_bot.GenerateLine();

            if(!message.empty()) s.eprocessor.DelayedMessage(message, 1000);
        }
    }
    if(name == s.config.GetValue("Master") && s.config.GetValue("MasterCommands") == "yes")
    {
        if(command == "relog")
        {
            s.eoclient.Disconnect();
        }
        else if(command == "face")
        {
            s.eoclient.Face((Direction)s.rand_gen.RandInt(0, 3));
        }
        else if(command == "tradeitem" && args.size() >= 3 && !s.eprocessor.BlockingEvent())
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

                s.eoclient.TradeRequest(gameworld_id);
            }
            else
            {
                ret.push_back(std::string() + "Item " + item_name + " not found.");
            }
        }
        else if(command == "ri" && args.size() >= 3)
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

            EIF_Data eif_data = s.eif->GetByNameLowercase(item_name);
            int item_id = eif_data.id;
            int item_amount = std::atoi(args[args.size() - 1].c_str());

            if(item_id != 0)
            {
                s.eprocessor.free_inv.AddItem(item_id, item_amount);
                s.eprocessor.SaveFreeItems();
                ret.push_back("Added to free items.");
            }
            else
            {
                ret.push_back("Item not found in database.");
            }
        }
        else if(command == "ei" && args.size() >= 3)
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

            EIF_Data eif_data = s.eif->GetByNameLowercase(item_name);
            int item_id = eif_data.id;
            int item_amount = std::atoi(args[args.size() - 1].c_str());

            if(item_id != 0)
            {
                s.eprocessor.free_inv.DelItem(item_id, item_amount);
                s.eprocessor.SaveFreeItems();
                ret.push_back("Deleted from free items.");
            }
            else
            {
                ret.push_back("Item not found in database.");
            }
        }
        else if(command == "offerz")
        {
            std::string message = "Active offers:";

            DelayMessage delay_message(message, 1);
            delay_message.channel = 1;
            delay_message.victim_name = name;
            s.eprocessor.DelayedMessage(delay_message);

            for(unsigned int i = 0; i < s.eprocessor.market.offers.size(); ++i)
            {
                std::string offer_id = std::to_string(s.eprocessor.market.offers[i].id);
                std::string transaction_type = s.eprocessor.market.offers[i].transaction_type == 0? "BUY" : "SELL";
                EIF_Data eif_data = s.eif->Get(s.eprocessor.market.offers[i].item.first);
                std::string item_name = eif_data.name;
                std::string item_amount = std::to_string(s.eprocessor.market.offers[i].item.second);
                std::string armor_type = "";
                if(eif_data.type == EIF_Data::Type::Armor)
                {
                    armor_type = eif_data.gender == 0? " (female) " : " (male) ";
                }

                delay_message.message = s.eprocessor.market.offers[i].transaction_done? "COMPLETE " : "";
                delay_message.message += "#" + offer_id + ": " + transaction_type + " -> " + item_name + armor_type;
                delay_message.message += " x" + item_amount;
                delay_message.message += " - " + std::to_string(s.eprocessor.market.offers[i].price) + " gold.";
                s.eprocessor.DelayedMessage(delay_message);
            }
        }
        else if(command == "wear" && args.size() >= 2)
        {
            std::string item_name = "";
            for(unsigned int i = 1; i < args.size(); ++i)
            {
                item_name += args[i];
                if(i != args.size() - 1)
                {
                    item_name += " ";
                }
            }

            int item_id = s.eif->GetByNameLowercase(item_name).id;
            bool found = true;//s.inventory.FindItem(item_id, 1);

            if(found)
            {
                PacketBuilder packet(PacketFamily::Paperdoll, PacketAction::Add);
                packet.AddShort(item_id);
                packet.AddChar(0);
                s.eoclient.Send(packet);
            }
            else
            {
                s.eoclient.TalkPublic("Item not found.");
            }
        }
        else if(command == "takeoff" && args.size() >= 2)
        {
            std::string item_name = "";
            for(unsigned int i = 1; i < args.size(); ++i)
            {
                item_name += args[i];
                if(i != args.size() - 1)
                {
                    item_name += " ";
                }
            }

            int item_id = s.eif->GetByNameLowercase(item_name).id;
            bool found = true;

            if(found)
            {
                PacketBuilder packet(PacketFamily::Paperdoll, PacketAction::Remove);
                packet.AddShort(item_id);
                packet.AddChar(0);
                s.eoclient.Send(packet);
            }
            else
            {
                s.eoclient.TalkPublic("Item not found.");
            }
        }
        else if(command == "itemlist")
        {
            std::string message = "Items:";

            DelayMessage delay_message(message, 1);
            delay_message.channel = 1;
            delay_message.victim_name = name;
            s.eprocessor.DelayedMessage(delay_message);

            int message_count = 0;

            for(unsigned int i = 0; i < s.inventory.items.size(); ++i)
            {
                std::string item_name = s.eif->Get(s.inventory.items[i].first).name;
                std::string item_amount = std::to_string(s.inventory.items[i].second);

                delay_message.message = item_name + " x" + item_amount;

                if(message_count >= 20)
                {
                    message_count = 0;
                    delay_message.time_ms = 1000;
                }

                s.eprocessor.DelayedMessage(delay_message);

                if(message_count == 0)
                {
                    delay_message.time_ms = 1;
                }

                message_count++;
            }
        }
        else if(command == "itemlistfree")
        {
            std::string message = "Items:";

            DelayMessage delay_message(message, 1);
            delay_message.channel = 1;
            delay_message.victim_name = name;
            s.eprocessor.DelayedMessage(delay_message);

            int message_count = 0;

            for(unsigned int i = 0; i < s.eprocessor.free_inv.items.size(); ++i)
            {
                std::string item_name = s.eif->Get(s.eprocessor.free_inv.items[i].first).name;
                std::string item_amount = std::to_string(s.eprocessor.free_inv.items[i].second);

                delay_message.message = item_name + " x" + item_amount;

                if(message_count >= 20)
                {
                    message_count = 0;
                    delay_message.time_ms = 1000;
                }

                s.eprocessor.DelayedMessage(delay_message);

                if(message_count == 0)
                {
                    delay_message.time_ms = 1;
                }

                message_count++;
            }
        }
        else if(command == "donated")
        {
            std::string message = "Donated items:";

            DelayMessage delay_message(message, 1);
            delay_message.channel = 1;
            delay_message.victim_name = name;
            s.eprocessor.DelayedMessage(delay_message);

            int message_count = 0;

            for(unsigned int i = 0; i < s.eprocessor.donated.items.size(); ++i)
            {
                std::string item_name = s.eif->Get(s.eprocessor.donated.items[i].first).name;
                std::string item_amount = std::to_string(s.eprocessor.donated.items[i].second);

                delay_message.message = item_name + " x" + item_amount;

                if(message_count >= 20)
                {
                    message_count = 0;
                    delay_message.time_ms = 1000;
                }

                s.eprocessor.DelayedMessage(delay_message);

                if(message_count == 0)
                {
                    delay_message.time_ms = 1;
                }

                message_count++;
            }
        }
        else if(command == "getd" && args.size() >= 3)
        {
            std::string item_name = "";
            for(unsigned int i = 1; i < args.size() - 1; ++i)
            {
                item_name += args[i];
                if(i < args.size() - 2)
                {
                    item_name += " ";
                }
            }

            int item_id = s.eif->GetByNameLowercase(item_name).id;
            int item_amount = std::atoi(args[args.size() - 1].c_str());

            bool found = s.eprocessor.donated.FindItem(item_id, item_amount);
            if(!found)
            {
                item_id++;
                std::string found_name = s.eif->Get(item_id).name;
                if(found_name == item_name)
                {
                    found = s.eprocessor.donated.FindItem(item_id, item_amount);
                }
            }

            if(found)
            {
                if(item_id == 1)
                {
                    item_amount = s.eprocessor.donated.GetItemAmount(1);
                }

                s.eprocessor.item_request.id = item_id;
                s.eprocessor.item_request.amount = item_amount;
                s.eprocessor.item_request.gameworld_id = gameworld_id;
                s.eprocessor.item_request.give = false;
                s.eprocessor.item_request.run = true;
                s.eprocessor.item_request.clock.restart();
                s.eprocessor.item_request.special_item = std::make_pair(0, 0);

                s.eoclient.TradeRequest(gameworld_id);
            }

            std::string message = "Item ";
            if(found)
            {
                message += "found. Please trade me to receive it (available for 30 seconds).";
            }
            else
            {
                message += "is not found in my inventory.";
            }

            s.eprocessor.DelayedMessage(message, 1000);
        }
        else if(command == "eid" && args.size() >= 3)
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

            EIF_Data eif_data = s.eif->GetByNameLowercase(item_name);
            int item_id = eif_data.id;
            int item_amount = std::atoi(args[args.size() - 1].c_str());

            if(item_id != 0)
            {
                s.eprocessor.donated.DelItem(item_id, item_amount);
                s.eprocessor.SaveDonated();
                ret.push_back("Deleted.");
            }
            else
            {
                ret.push_back("Item not found in database.");
            }
        }
        else if(command == "setjptime" && args.size() >= 2)
        {
            int seconds = std::atoi(args[1].c_str());

            s.eprocessor.eo_roulette.jp_time = seconds;

            ret.push_back("Jackpot time updated.");
        }
        else if(command == "setjp"  && args.size() >= 2)\
        {
            int amount = std::atoi(args[1].c_str());

            s.eprocessor.eo_roulette.total_gold = amount;

            ret.push_back("Jackpot updated.");
        }
        else
        {
            puts("Wrong master command");
        }
    }

    if(command == "eor" && !s.eprocessor.BlockingEvent())
    {
        s.eprocessor.eo_roulette.Run(gameworld_id);

        std::string name_upper = name;
        name_upper[0] = std::toupper(name[0]);

        s.eoclient.TalkPublic("Welcome to EORoulette! Please trade me gold (at least 2g) to join the game. You've got 30 seconds.");
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
        int i = s.map.GetCharacterIndex(gameworld_id);
        if(!s.eprocessor.trade.get() && s.eprocessor.eo_roulette.run && i != -1)
        {
            if(!s.eprocessor.eo_roulette.play && s.eprocessor.eo_roulette.winner == -1 && s.map.characters[i].eor_payments < 16)
            {
                s.eoclient.TradeRequest(gameworld_id);
            }
            else if(!s.eprocessor.eo_roulette.play && s.eprocessor.eo_roulette.winner == -1 && s.map.characters[i].eor_payments >= 16)
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
    else if(command == "jackpoteortime" || command == "jptime")
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
        new_message += std::to_string(s.emf->id) + " '";
        if(s.emf->id == 5) new_message += "Aeven";
        if(s.emf->id == 84) new_message += "1st bank";
        new_message += "'";
        new_message += ", x: " + std::to_string(s.character.x) + ", y: " + std::to_string(s.character.y);

        ret.push_back(new_message);
    }
    else if(command == "help")
    {
        std::string message = "Commands:";

        for(unsigned int i = 0; i < s.eprocessor.help_config.entries.size(); ++i)
        {
            message += " #help_" + s.eprocessor.help_config.entries[i].key;
        }

        ret.push_back(message);
    }
    else if(command.substr(0, 5) == "help_" && command.length() > 5)
    {
        std::string help_section = command.substr(5);

        std::string text = s.eprocessor.help_config.GetValue(help_section);

        if(!text.empty())
        {
            ret.push_back(text);
        }
    }
    else if(command == "news")
    {
        for(unsigned int i = 0; i < s.eprocessor.news.entries.size(); ++i)
        {
            s.eprocessor.DelayedMessage(s.eprocessor.news.entries[i].value, 1, 1, name);
        }
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
    else if(command == "lvl" && args.size() >= 2)
    {
        std::string char_name = args[1];

        std::transform(char_name.begin(), char_name.end(), char_name.begin(), ::tolower);

        int index = s.map.GetCharacterIndex(char_name);

        std::string message = "";
        if(index != -1)
        {
            std::string name_upper = char_name;
            name_upper[0] = std::toupper(char_name[0]);
            message += name_upper + "'s level: ";
            message += std::to_string(s.map.characters[index].level);
        }
        else
        {
            message += "Player not found.";
        }

        ret.push_back(message);
    }
    else if(command == "getitem" && args.size() >= 2 && !s.eprocessor.BlockingEvent())
    {
        std::string item_name = "";
        for(unsigned int i = 1; i < args.size(); ++i)
        {
            item_name += args[i];
            if(i != args.size() - 1)
            {
                item_name += " ";
            }
        }

        int item_id = s.eif->GetByNameLowercase(item_name).id;
        int item_amount = 1;

        bool found = s.eprocessor.free_inv.FindItem(item_id, item_amount);
        if(!found)
        {
            item_id++;
            std::string found_name = s.eif->Get(item_id).name;
            if(found_name == item_name)
            {
                found = s.eprocessor.free_inv.FindItem(item_id, item_amount);
            }
        }

        if(found)
        {
            if(item_id == 1)
            {
                item_amount = s.eprocessor.free_inv.GetItemAmount(1);
            }

            s.eprocessor.item_request.id = item_id;
            s.eprocessor.item_request.amount = item_amount;
            s.eprocessor.item_request.gameworld_id = gameworld_id;
            s.eprocessor.item_request.give = false;
            s.eprocessor.item_request.run = true;
            s.eprocessor.item_request.clock.restart();
            s.eprocessor.item_request.special_item = std::make_pair(0, 0);

            s.eoclient.TradeRequest(gameworld_id);
        }

        std::string message = "Item ";
        if(found)
        {
            message += "found. Please trade me to receive it (available for 30 seconds).";
        }
        else
        {
            message += "is not found in my inventory.";
        }

        s.eprocessor.DelayedMessage(message, 1000);
    }
    else if(command == "giveitem" && !s.eprocessor.BlockingEvent())
    {
        s.eprocessor.item_request.gameworld_id = gameworld_id;
        s.eprocessor.item_request.give = true;
        s.eprocessor.item_request.run = true;
        s.eprocessor.item_request.clock.restart();
        s.eprocessor.item_request.special_item = std::make_pair(0, 0);

        s.eoclient.TradeRequest(gameworld_id);

        std::string message = "Please trade me the items you want to give (available 30 seconds).";
        s.eprocessor.DelayedMessage(message, 1000);
    }
    else if(command == "donateitem" && !s.eprocessor.BlockingEvent())
    {
        s.eprocessor.item_request.gameworld_id = gameworld_id;
        s.eprocessor.item_request.give = true;
        s.eprocessor.item_request.run = true;
        s.eprocessor.item_request.clock.restart();
        s.eprocessor.item_request.special_item = std::make_pair(1, 1);

        s.eoclient.TradeRequest(gameworld_id);

        std::string message = "Please trade me the items you want to give (available 30 seconds).";
        s.eprocessor.DelayedMessage(message, 1000);
    }
    else if(command == "items")
    {
        std::string message = "Inventory contains ";
        message += std::to_string(s.eprocessor.free_inv.items.size()) + " free items.";

        ret.push_back(message);
    }
    else if(command == "sitwin" && !s.eprocessor.BlockingEvent())
    {
        std::string message = "Welcome to SitAndWin. Please trade me the item you want to put in the game. You've got 30 seconds.";

        s.eprocessor.sitwin.Run(gameworld_id);

        s.eoclient.TalkPublic(message);
    }
    else if(command == "randwin")
    {
        if(!s.eprocessor.eo_roulette.jpconfig.entries.empty())
        {
            int index = s.rand_gen.RandInt(0, s.eprocessor.eo_roulette.jpconfig.entries.size() - 1);
            std::vector<std::string> config_args = Args(s.eprocessor.eo_roulette.jpconfig.entries[index].value);
            std::string char_name = config_args[0];
            std::string upper_name = char_name;
            upper_name[0] = std::toupper(char_name[0]);

            std::string amount = config_args[1];

            ret.push_back(std::string() + upper_name + ": " + amount + " gold.");
        }
        else
        {
            ret.push_back("Sorry. no data found.");
        }
    }
    else if(command == "line")
    {
        if(s.eprocessor.chat_bot.config.entries.empty())
        {
            return ret;
        }

        if(s.eprocessor.line_clock.getElapsedTime().asSeconds() < 32)
        {
            int elapsed = 32 - s.eprocessor.line_clock.getElapsedTime().asSeconds();
            s.eprocessor.DelayedMessage("Sorry, you need to wait " + std::to_string(elapsed) +
                                        " seconds more due to spam protection.", 1000, 1, name);
            return ret;
        }

        s.eprocessor.line_clock.restart();

        std::string message = s.eprocessor.chat_bot.config.entries[s.rand_gen.RandInt(0, s.eprocessor.chat_bot.config.entries.size() - 1)].value;
        s.eprocessor.DelayedMessage(message, 1000);
    }
    else if(command == "quests")
    {
        std::string message = "List of available quests:";

        DelayMessage delay_message(message, 1);
        delay_message.channel = 1;
        delay_message.victim_name = name;
        s.eprocessor.DelayedMessage(delay_message);

        int message_count = 0;

        for(unsigned int i = 0; i < s.eprocessor.quest_gen.quests.size(); ++i)
        {
            if(s.eprocessor.quest_gen.quests[i].complete) continue;

            std::string quest_id = std::to_string(s.eprocessor.quest_gen.quests[i].id);
            std::string holder_name = s.eprocessor.quest_gen.quests[i].holder;
            holder_name[0] = std::toupper(s.eprocessor.quest_gen.quests[i].holder[0]);
            message = "#" + quest_id + " " + holder_name + "'s quest for ";
            std::string item_name = s.eif->Get(s.eprocessor.quest_gen.quests[i].award.first).name;
            message += item_name + " x" + std::to_string(s.eprocessor.quest_gen.quests[i].award.second) + ".";

            delay_message.message = message;

            if(message_count >= 20)
            {
                message_count = 0;
                delay_message.time_ms = 1000;
            }

            s.eprocessor.DelayedMessage(delay_message);

            if(message_count == 0)
            {
                delay_message.time_ms = 1;
            }

            message_count++;
        }
    }
    else if(command == "questinfo" && args.size() >= 2)
    {
        int quest_id = std::atoi(args[1].c_str());

        Quest quest = s.eprocessor.quest_gen.GetQuest(quest_id);

        if(quest.id == 0)
        {
            ret.push_back("Sorry, quest not found.");
            return ret;
        }

        if(quest.complete && quest.holder != name)
        {
            ret.push_back("This quest is already complete.");
            return ret;
        }

        std::string message = "";
        DelayMessage delay_message(message, 1);
        delay_message.channel = 1;
        delay_message.victim_name = name;
        s.eprocessor.DelayedMessage(delay_message);
        if(quest.complete && quest.holder == name)
        {
            message += "(COMPLETE) ";
        }

        std::string quest_id_str = std::to_string(quest.id);
        std::string holder_name = quest.holder;
        holder_name[0] = std::toupper(quest.holder[0]);
        message = "#" + quest_id_str + " " + holder_name + "'s quest for ";
        std::string item_name = s.eif->Get(quest.award.first).name;
        message += item_name + " x" + std::to_string(quest.award.second) + ".";
        delay_message.message = message;
        s.eprocessor.DelayedMessage(delay_message);

        message = "Quest requirements:";
        for(unsigned int i = 0; i < quest.requirements.size(); ++i)
        {
            std::string item_name = s.eif->Get(quest.requirements[i].first).name;
            std::string item_amount = std::to_string(quest.requirements[i].second);

            message += (i > 0? ", " : " ") + item_name + " x" + item_amount;
        }
        delay_message.message = message;
        s.eprocessor.DelayedMessage(delay_message);
    }
    else if(command == "myquests")
    {
        std::vector<Quest> quests = s.eprocessor.quest_gen.GetPlayerQuests(name);

        std::string message = "Quests created by you:";

        DelayMessage delay_message(message, 1);
        delay_message.channel = 1;
        delay_message.victim_name = name;
        s.eprocessor.DelayedMessage(delay_message);

        for(unsigned int i = 0; i < quests.size(); ++i)
        {
            message = "";
            if(quests[i].complete)
            {
                message = "(COMPLETE) ";
            }
            std::string quest_id = std::to_string(quests[i].id);
            message += "#" + quest_id + " for ";
            std::string item_name = s.eif->Get(quests[i].award.first).name;
            message += item_name + " x" + std::to_string(quests[i].award.second) + ".";

            delay_message.message = message;

            s.eprocessor.DelayedMessage(delay_message);
        }
    }
    else if(command == "newquest" && !s.eprocessor.BlockingEvent())
    {
        if(s.eprocessor.quest_gen.GetPlayerQuests(name).size() >= 3)
        {
            ret.push_back("Sorry, you can only create up to 3 quests.");
            return ret;
        }

        if(s.eprocessor.quest_gen.quests.size() >= 300)
        {
            ret.push_back("Maximum amount of quests has been reached (300). Please wait until there will be space for you.");
            return ret;
        }

        s.eprocessor.quest_gen.new_quest = std::shared_ptr<Quest>(new Quest());
        s.eprocessor.quest_gen.new_quest->holder = name;
        s.eprocessor.quest_gen.clock.restart();

        ret.push_back("Quest creation started. Please set item requirements (available 30 seconds for each command).");
    }
    else if(command == "questreq" && args.size() >= 3 && s.eprocessor.quest_gen.new_quest.get())
    {
        std::string item_name = "";
        for(unsigned int i = 1; i < args.size() - 1; ++i)
        {
            item_name += args[i];
            if(i < args.size() - 2)
            {
                item_name += " ";
            }
        }

        EIF_Data eif_data = s.eif->GetByNameLowercase(item_name);
        int item_id = eif_data.id;
        int item_amount = std::atoi(args[args.size() - 1].c_str());

        if(item_id == 0)
        {
            ret.push_back("The given item has not been found in database.");
            return ret;
        }

        s.eprocessor.quest_gen.new_quest->requirements.push_back(std::make_pair(item_id, item_amount));
        s.eprocessor.quest_gen.clock.restart();

        ret.push_back("Item added.");
    }
    else if(command == "savequest" && s.eprocessor.quest_gen.new_quest.get())
    {
        if(s.eprocessor.quest_gen.new_quest->requirements.empty())
        {
            ret.push_back("Cannot create quest: no requirements for have been set.");
            return ret;
        }

        s.eprocessor.quest_gen.item_request.run = true;
        s.eprocessor.item_request.gameworld_id = gameworld_id;
        s.eprocessor.quest_gen.item_request.give = false;
        s.eprocessor.quest_gen.item_request.requirements.clear();
        s.eprocessor.quest_gen.item_request.clock.restart();

        ret.push_back("Alright. Please trade me the award item for this quest.");

        s.eoclient.TradeRequest(gameworld_id);
    }
    else if(command == "makequest" && args.size() >= 2 && !s.eprocessor.BlockingEvent())
    {
        int quest_id = std::atoi(args[1].c_str());

        Quest quest = s.eprocessor.quest_gen.GetQuest(quest_id);

        if(quest.id == 0)
        {
            ret.push_back("Quest not found. Sorry.");
            return ret;
        }

        if(quest.complete)
        {
            DelayMessage dmessage("This quest is already complete.", 1000);
            dmessage.channel = 1;
            dmessage.victim_name = name;
            s.eprocessor.DelayedMessage(dmessage);
            return ret;
        }

        s.eprocessor.quest_gen.item_request.run = true;
        s.eprocessor.quest_gen.item_request.gameworld_id = gameworld_id;
        s.eprocessor.quest_gen.item_request.give = true;
        s.eprocessor.quest_gen.item_request.special_item = quest.award;
        s.eprocessor.quest_gen.item_request.requirements = quest.requirements;
        s.eprocessor.quest_gen.item_request.clock.restart();

        s.eprocessor.quest_gen.active_quest = std::shared_ptr<Quest>(new Quest(quest));

        s.eoclient.TradeRequest(gameworld_id);

        std::string holder_name = quest.holder;
        holder_name[0] = std::toupper(quest.holder[0]);
        std::string award_name = s.eif->Get(quest.award.first).name;
        std::string award_amount = std::to_string(quest.award.second);

        std::string message = holder_name + "'s quest for " + award_name + " x" + award_amount + ".";
        ret.push_back(message);
        message = "Please trade me all requirements to receive your award.";
        ret.push_back(message);
    }
    else if(command == "getaward" && args.size() >= 2 && !s.eprocessor.BlockingEvent())
    {
        int quest_id = std::atoi(args[1].c_str());

        Quest quest = s.eprocessor.quest_gen.GetQuest(quest_id);

        if(quest.id == 0)
        {
            ret.push_back("Quest not found. Sorry.");
            return ret;
        }

        if(!quest.complete)
        {
            ret.push_back("Sorry, quest hasn't been made by anyone.");
            return ret;
        }

        if(Lowercase(name) != Lowercase(quest.holder))
        {
            ret.push_back("Access denied. You're not the quest holder.");
            return ret;
        }

        s.eprocessor.quest_gen.item_request.run = true;
        s.eprocessor.quest_gen.item_request.gameworld_id = gameworld_id;
        s.eprocessor.quest_gen.item_request.give = true;
        s.eprocessor.quest_gen.item_request.special_item = quest.award;
        s.eprocessor.quest_gen.item_request.requirements = quest.requirements;
        s.eprocessor.quest_gen.item_request.clock.restart();

        s.eprocessor.quest_gen.active_quest = std::shared_ptr<Quest>(new Quest(quest));

        s.eoclient.TradeRequest(gameworld_id);

        ret.push_back("Quest is complete. Please trade me to collect your award.");
    }
    else if(command == "offers")
    {
        std::string message = "Active offers:";

        DelayMessage delay_message(message, 1);
        delay_message.channel = 1;
        delay_message.victim_name = name;
        s.eprocessor.DelayedMessage(delay_message);

        int message_count = 0;

        for(unsigned int i = 0; i < s.eprocessor.market.offers.size(); ++i)
        {
            if(s.eprocessor.market.offers[i].transaction_done) continue;

            std::string offer_id = std::to_string(s.eprocessor.market.offers[i].id);
            std::string transaction_type = s.eprocessor.market.offers[i].transaction_type == 0? "BUY" : "SELL";
            EIF_Data eif_data = s.eif->Get(s.eprocessor.market.offers[i].item.first);
            std::string item_name = eif_data.name;
            std::string item_amount = std::to_string(s.eprocessor.market.offers[i].item.second);
            std::string armor_type = "";
            if(eif_data.type == EIF_Data::Type::Armor)
            {
                armor_type = eif_data.gender == 0? " (female) " : " (male) ";
            }

            delay_message.message = "#" + offer_id + ": " + transaction_type + " -> " + item_name + armor_type;
            delay_message.message += " x" + item_amount;
            delay_message.message += " - " + std::to_string(s.eprocessor.market.offers[i].price) + " gold.";

            if(message_count >= 20)
            {
                message_count = 0;
                delay_message.time_ms = 1000;
            }

            s.eprocessor.DelayedMessage(delay_message);

            if(message_count == 0)
            {
                delay_message.time_ms = 1;
            }

            message_count++;
        }
    }
    else if(command == "offerinfo" && args.size() >= 2)
    {
        int offer_id = std::atoi(args[1].c_str());

        Market::Offer offer = s.eprocessor.market.GetOffer(offer_id);

        if(offer.id == 0)
        {
            ret.push_back("Offer not found.");
            return ret;
        }

        std::string offerid = std::to_string(offer.id);
        std::string transaction_type = offer.transaction_type == 0? "BUY" : "SELL";
        std::string item_name = s.eif->Get(offer.item.first).name;
        std::string item_amount = std::to_string(offer.item.second);

        DelayMessage delay_message("", 1);
        delay_message.channel = 1;
        delay_message.victim_name = name;
        if(offer.transaction_done && offer.player_name == name)
        {
            delay_message.message = "(COMPLETE) ";
        }
        delay_message.message += "#" + offerid + ": " + transaction_type + " -> " + item_name + " x" + item_amount;
        delay_message.message += " for " + std::to_string(offer.price) + " gold.";
        s.eprocessor.DelayedMessage(delay_message);
    }
    else if(command == "myoffers")
    {
        std::vector<Market::Offer> offers = s.eprocessor.market.GetPlayerOffers(name);

        std::string message = "Your offers:";

        DelayMessage delay_message(message, 1);
        delay_message.channel = 1;
        delay_message.victim_name = name;
        s.eprocessor.DelayedMessage(delay_message);

        for(std::size_t i = 0; i < offers.size(); ++i)
        {
            std::string offer_id = std::to_string(offers[i].id);
            std::string transaction_type = offers[i].transaction_type == 0? "BUY" : "SELL";
            std::string item_name = s.eif->Get(offers[i].item.first).name;
            std::string item_amount = std::to_string(offers[i].item.second);

            delay_message.message = "";
            if(offers[i].transaction_done && offers[i].player_name == name)
            {
                delay_message.message = "(COMPLETE) ";
            }

            delay_message.message += "#" + offer_id + ": " + transaction_type + " -> " + item_name + " x" + item_amount;
            delay_message.message += " for " + std::to_string(offers[i].price) + " gold.";
            s.eprocessor.DelayedMessage(delay_message);
        }
    }
    if(command == "buyoffer" && args.size() >= 2 && !s.eprocessor.BlockingEvent())
    {
        if(s.eprocessor.market.GetPlayerOffers(name).size() >= 3)
        {
            ret.push_back("Nope. You can create up to 3 offers at the same time.");
            return ret;
        }

        if(s.eprocessor.market.offers.size() >= 300)
        {
            ret.push_back("Maximum offers amount has been reached (300). Please wait for free space.");
            return ret;
        }

        std::string item_name = "";
        for(unsigned int i = 1; i < args.size() - 1; ++i)
        {
            item_name += args[i];
            if(i < args.size() - 2)
            {
                item_name += " ";
            }
        }

        EIF_Data eif_data = s.eif->GetByNameLowercase(item_name);
        int item_id = eif_data.id;
        int item_amount = std::atoi(args[args.size() - 1].c_str());

        if(item_id == 0)
        {
            ret.push_back("The given item has not been found in database.");
            return ret;
        }

        if(item_amount < 1)
        {
            ret.push_back("Invalid item amount has been given.");
            return ret;
        }

        s.eprocessor.market.item_request.run = true;
        s.eprocessor.item_request.gameworld_id = gameworld_id;
        s.eprocessor.market.item_request.give = false;
        s.eprocessor.market.item_request.requirements.clear();
        s.eprocessor.market.item_request.clock.restart();

        s.eprocessor.market.new_offer = std::shared_ptr<Market::Offer>(new Market::Offer());
        s.eprocessor.market.new_offer->transaction_type = 0;
        s.eprocessor.market.new_offer->item = std::make_pair(item_id, item_amount);

        s.eoclient.TradeRequest(gameworld_id);

        ret.push_back("Please trade me the gold you offer for that item.");
    }
    else if(command == "selloffer" && args.size() >= 2 && !s.eprocessor.BlockingEvent())
    {
        if(s.eprocessor.market.GetPlayerOffers(name).size() >= 3)
        {
            ret.push_back("Nope. You can create up to 3 offers at the same time.");
            return ret;
        }

        if(s.eprocessor.market.offers.size() >= 300)
        {
            ret.push_back("Maximum offers amount has been reached (300). Please wait for free space.");
            return ret;
        }

        int gold_amount = std::atoi(args[1].c_str());

        if(gold_amount < 1)
        {
            ret.push_back("You have entered invalid gold amount.");
            return ret;
        }

        s.eprocessor.market.item_request.run = true;
        s.eprocessor.item_request.gameworld_id = gameworld_id;
        s.eprocessor.market.item_request.give = false;
        s.eprocessor.market.item_request.requirements.clear();
        s.eprocessor.market.item_request.clock.restart();

        s.eprocessor.market.new_offer = std::shared_ptr<Market::Offer>(new Market::Offer());
        s.eprocessor.market.new_offer->transaction_type = 1;
        s.eprocessor.market.new_offer->price = gold_amount;

        s.eoclient.TradeRequest(gameworld_id);

        ret.push_back("Please trade me the item you offer to sell.");
    }
    else if(command == "maketrade" && args.size() >= 2)
    {
        int offer_id = std::atoi(args[1].c_str());

        Market::Offer offer = s.eprocessor.market.GetOffer(offer_id);

        if(offer.id == 0)
        {
            ret.push_back("Offer not found.");
            return ret;
        }

        if(offer.transaction_done)
        {
            DelayMessage dmessage("This transaction is inactive.", 1000);
            dmessage.channel = 1;
            dmessage.victim_name = name;
            s.eprocessor.DelayedMessage(dmessage);
            return ret;
        }

        s.eprocessor.market.active_offer = std::shared_ptr<Market::Offer>(new Market::Offer(offer));

        s.eprocessor.market.item_request.run = true;
        s.eprocessor.item_request.gameworld_id = gameworld_id;
        s.eprocessor.market.item_request.give = false;
        s.eprocessor.market.item_request.requirements.clear();
        if(offer.transaction_type == 0)
        {
            s.eprocessor.market.item_request.requirements.push_back(offer.item);
        }
        else
        {
            s.eprocessor.market.item_request.special_item = offer.item;
            s.eprocessor.market.item_request.requirements.push_back(std::make_pair(1, offer.price));
        }
        s.eprocessor.market.item_request.clock.restart();

        s.eoclient.TradeRequest(gameworld_id);

        std::string item_name = s.eif->Get(s.eprocessor.market.item_request.requirements[0].first).name;
        std::string item_amount = std::to_string(s.eprocessor.market.item_request.requirements[0].second);

        ret.push_back(std::string() + "Please trade me " + item_name + " x" + item_amount + " to complete transaction.");
    }
    else if(command == "collect" && args.size() >= 2 && !s.eprocessor.BlockingEvent())
    {
        int offer_id = std::atoi(args[1].c_str());

        Market::Offer offer = s.eprocessor.market.GetOffer(offer_id);

        if(offer.id == 0)
        {
            s.eprocessor.DelayedMessage("Offer not found.", 1000, 1, name);
            return ret;
        }

        if(offer.player_name != name)
        {
            s.eprocessor.DelayedMessage("This offer is not yours.", 1000, 1, name);
            return ret;
        }

        s.eprocessor.market.active_offer = std::shared_ptr<Market::Offer>(new Market::Offer(offer));

        s.eprocessor.market.item_request.run = true;
        s.eprocessor.item_request.gameworld_id = gameworld_id;
        s.eprocessor.market.item_request.give = true;
        s.eprocessor.market.item_request.requirements.clear();
        if(offer.transaction_type == 0)
        {
            if(offer.transaction_done)
            {
                s.eprocessor.market.item_request.special_item = offer.item;
            }
            else
            {
                s.eprocessor.market.item_request.special_item = std::make_pair(1, offer.price);
            }
        }
        else
        {
            if(offer.transaction_done)
            {
                s.eprocessor.market.item_request.special_item = std::make_pair(1, offer.price);
            }
            else
            {
                s.eprocessor.market.item_request.special_item = offer.item;
            }
        }
        s.eprocessor.market.item_request.clock.restart();

        s.eoclient.TradeRequest(gameworld_id);

        s.eprocessor.DelayedMessage("Please trade me to collect your items.", 1000, 1, name);
    }
    else if(command == "randoffer")
    {
        std::vector<Market::Offer> active_offers;
        for(unsigned int i = 0; i < s.eprocessor.market.offers.size(); ++i)
        {
            if(!s.eprocessor.market.offers[i].transaction_done)
            {
                active_offers.push_back(s.eprocessor.market.offers[i]);
            }
        }

        if(active_offers.size() == 0)
        {
            s.eprocessor.DelayedMessage("Sorry, no offers available", 1000, 1, name);
            return ret;
        }

        Market::Offer offer = active_offers[s.rand_gen.RandInt(0, active_offers.size() - 1)];

        EIF_Data item = s.eif->Get(offer.item.first);

        std::string message = offer.transaction_type == 0? "BUY" : "SELL";
        message += " " + item.name + " x" + std::to_string(offer.item.second);
        message += " --- " + std::to_string(offer.price) + " gold.";

        s.eprocessor.DelayedMessage(message, 1000);
    }
    else if(command == "addjoke" && args.size() >= 2)
    {
        std::string joke;
        for(std::size_t i = 1; i < args.size(); ++i)
        {
            joke += (i > 1? " " : "") + args[i];
        }

        if(s.eprocessor.GetJokeID(joke) != -1)
        {
            return ret;
        }

        if(s.eprocessor.GetJokeAmount(name) >= 333)
        {
            s.eprocessor.RemoveJokeOf(name);
        }

        if(s.eprocessor.jokes.size() >= 33300)
        {
            s.eprocessor.jokes.erase(s.eprocessor.jokes.begin());
        }

        s.eprocessor.jokes.push_back(Joke(name, joke));
        s.eprocessor.SaveJokes();

        s.eprocessor.DelayedMessage("Joke successfully added!", 1000, 1, name);
    }
    else if(command == "joke" && args.size() == 1)
    {
        if(s.eprocessor.jokes.empty())
        {
            ret.push_back("No jokes in the database");
            return ret;
        }

        if(channel == 1)
        {
            int joke_index = s.rand_gen.RandInt(0, s.eprocessor.jokes.size() - 1);
            Joke joke = s.eprocessor.jokes[joke_index];

            std::string msg = "#" + std::to_string(joke_index + 1) + ": " + joke.Get();

            s.eprocessor.DelayedMessage(msg, 1000, channel, name);

            return ret;
        }

        Joke joke = s.eprocessor.GetFreeJoke();

        if(joke.Get() == "")
        {
            s.eprocessor.DelayedMessage("Sorry, no new jokes available. Please try again later.", 1000, 1, name);
            return ret;
        }

        int id = s.eprocessor.GetJokeID(joke.Get());

        s.eprocessor.DelayedMessage(std::string() + "#" + std::to_string(id) + ": " + joke.Get(), 1000, 1, name);
    }
    else if(command == "joke" && args.size() >= 2)
    {
        int id = std::atoi(args[1].c_str());

        Joke joke = s.eprocessor.GetJokeByID(id);

        if(joke.Get() != "")
        {
            s.eprocessor.DelayedMessage(std::string() + "#" + std::to_string(id) + ": " + joke.Get(), 1000, 1, name);
        }
    }
    else if(command == "votejoke" && args.size() >= 2)
    {
        int id = std::atoi(args[1].c_str());

        Joke joke = s.eprocessor.GetJokeByID(id);

        if(joke.Get() == "")
        {
            s.eprocessor.DelayedMessage("Joke not found", 1000, 1, name);
            return ret;
        }

        if(joke.AddVote(name))
        {
            s.eprocessor.UpdateJoke(id, joke);
            s.eprocessor.SaveJokes();

            s.eprocessor.DelayedMessage("Your vote has been saved.", 1000, 1, name);
        }
        else
        {
            s.eprocessor.DelayedMessage("Sorry, you have already voted for this joke.", 1000, 1, name);
        }
    }
    else if(command == "bestjoke")
    {
        Joke joke = s.eprocessor.GetBestJoke();

        if(joke.Get() != "")
        {
            int id = s.eprocessor.GetJokeID(joke.Get());
            s.eprocessor.DelayedMessage(std::string() + "#" + std::to_string(id) + ": " + joke.Get(), 1000, 1, name);
        }
    }

    if(char_index == -1) return ret;

    Character character = s.map.characters[char_index];

    if(character.guild_tag == "FV")
    {
        s.eprocessor.chamber.Command(name, command, args, gameworld_id, channel);
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
    if(i != -1) name = s.map.characters[i].name;

    if(message[0] == '#')
    {
        message.erase(0, 1);

        std::vector<std::string> ret = ProcessCommand(name, message, gameworld_id, 0);
        std::vector<std::string> args = Args(message);

        for(unsigned int i = 0; i < ret.size(); ++i)
        {
            DelayMessage delay_message(ret[i], 1000);
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
        if(name != "panddda")
        {
            //s.eprocessor.chat_bot.ProcessMessage(message);
        }
    }
}

void Talk_Tell(PacketReader reader)
{
    S &s = S::GetInstance();

    std::string name = reader.GetBreakString();
	std::string message = reader.GetBreakString();

	if(message[0] == '#' && message.length() > 1)
    {
        message.erase(0, 1);

        int index = s.map.GetCharacterIndex(name);
        short gameworld_id = 0;
        if(index != -1) gameworld_id = s.map.characters[index].gameworld_id;

        std::vector<std::string> ret = ProcessCommand(name, message, gameworld_id, 1);
        for(unsigned int i = 0; i < ret.size(); ++i)
        {
            DelayMessage delay_message(ret[i], 1000);
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

    //S &s = S::GetInstance();

	/*if(message[0] == '#' && name == s.config.GetValue("Master"))
    {
        message.erase(0, 1);

        int index = s.map.GetCharacterIndex(name);
        short gameworld_id = 0;
        if(index != -1) gameworld_id = s.map.characters[index].gameworld_id;

        std::vector<std::string> ret = ProcessCommand(name, message, gameworld_id);
        for(unsigned int i = 0; i < ret.size(); ++i)
        {
            EventProcessor::DelayMessage delay_message(ret[i], 1000);
            delay_message.channel = 2;
            delay_message.victim_name = name;
            s.eprocessor.DelayedMessage(delay_message);
        }
    }*/
}
