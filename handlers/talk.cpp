// Endless Online Bot v0.0.1

#include "handlers.hpp"
#include "../singleton.hpp"
#include "../eventprocessor.hpp"
#include "../util.hpp"

std::vector<std::string> ProcessCommand(std::string name, std::string message, short gameworld_id)
{
    S &s = S::GetInstance();

    std::vector<std::string> ret;

    int char_index = s.map.GetCharacterIndex(gameworld_id);

    if(char_index != -1)
    {
        if(s.map.characters[char_index].command_clock.getElapsedTime().asMilliseconds() < 1000)
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
        /*if(command == "exit")
        {
            s.call_exit = true;
        }
        else if(command == "relog")
        {
            s.eoclient.Disconnect();
        }*/
        /*else if(command == "rchase")
        {
            s.eprocessor.chase_bot.Reset();
        }*/
        /*else if(command == "dg" && args.size() == 2)
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
        }*/
        /*if(command == "setjp" && args.size() == 2)
        {
            int amount = std::atoi(args[1].c_str());

            s.eprocessor.eo_roulette.total_gold = amount;

            ret.push_back(std::string() + "Jackpot set to " + std::to_string(amount) + " gold.");
        }*/
        /*else if(command == "jp")
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
        }*/
        if(command == "peekitem" && args.size() >= 2)
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
        /*else if(command == "tradeitem" && args.size() >= 3 && !s.eprocessor.BlockingEvent())
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
        }*/
        /*else if(command == "inventory")
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
        }*/
        /*else if(command == "stgen")
        {
            s.eprocessor.sitwin_jackpot.GenerateItem();

            if(s.eprocessor.sitwin_jackpot.item_id == 0)
            {
                ret.push_back("SitAndWin jackpot is loading...");
            }
            else
            {
                std::string message = "SitAndWin jackpot: ";
                std::string name = s.eif->Get(s.eprocessor.sitwin_jackpot.item_id).name;
                std::string item_name = name;
                item_name[0] = std::toupper(name[0]);
                std::string item_amount = std::to_string(s.eprocessor.sitwin_jackpot.item_amount);
                message += item_name + " x" + item_amount;

                ret.push_back(message);
            }
        }*/
        /*else if(command == "setstitem" && args.size() >= 2)
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
            if(found)
            {
                s.eprocessor.sitwin_jackpot.item_id = item_id;
                s.eprocessor.sitwin_jackpot.item_amount = 1;

                s.eoclient.TalkPublic(std::string() + "ST item set to " + item_name);
            }
        }*/
        /*else if(command == "atk")
        {
            s.eoclient.Attack(s.character.direction);
        }*/
        else if(command == "face")
        {
            s.eoclient.Face((Direction)s.rand_gen.RandInt(0, 3));
        }
        /*else if(command == "wear" && args.size() >= 2)
        {
            std::string item_name = "";
            for(unsigned int i = 1; i < args.size(); ++i)
            {
                std::string word = args[i];

                //std::transform(word.begin(), word.end(), word.begin(), ::tolower);
                //word[0] = std::toupper(args[i][0]);
                item_name += word;
                if(i != args.size() - 1)
                {
                    item_name += " ";
                }
            }

            int item_id = s.eif->GetByName(item_name).id;
            bool found = true;//s.inventory.FindItem(item_id, s.eprocessor.item_request.amount);

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
                std::string word = args[i];

                //std::transform(word.begin(), word.end(), word.begin(), ::tolower);
                //word[0] = std::toupper(args[i][0]);
                item_name += word;
                if(i != args.size() - 1)
                {
                    item_name += " ";
                }
            }

            int item_id = s.eif->GetByName(item_name).id;
            bool found = true;//s.inventory.FindItem(item_id, s.eprocessor.item_request.amount);

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
        }*/
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
                s.eoclient.TradeRequest(gameworld_id);
            }
            else if(!s.eprocessor.eo_roulette.play && s.eprocessor.eo_roulette.winner == -1 && s.eprocessor.eo_roulette.payments >= 16)
            {
                s.eprocessor.DelayedMessage("Sorry, trade limit reached. The game will start shortly", 1000);
            }
        }
    }
    else if(command == "jackpoteor" || command == "jackpot")
    {
        std::string message = "Jackpot: ";
        message += std::to_string(s.eprocessor.eo_roulette.total_gold) + " gold.";

        ret.push_back(message);
    }
    else if(command == "jackpotxxleor" || command == "jackpotxxl")
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
    /*else if(command == "getitem" && args.size() >= 2 && !s.eprocessor.BlockingEvent())
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
        bool found = s.inventory.FindItem(item_id, s.eprocessor.item_request.amount);
        bool locked = false;

        if(item_id == 1 || item_id == s.eprocessor.sitwin_jackpot.item_id)
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

            s.eoclient.TradeRequest(gameworld_id);
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
                message += item_name + " is locked from giving away.";
            }
            else if(!locked)
            {
                message += "is not found in my inventory.";
            }
            else if(locked)
            {
                message += item_name + " is locked from giving away.";
            }
        }

        s.eprocessor.DelayedMessage(message, 1000);
    }
    else if(command == "giveitem" && !s.eprocessor.BlockingEvent())
    {
        s.eprocessor.item_request.gameworld_id = gameworld_id;
        s.eprocessor.item_request.give = true;
        s.eprocessor.item_request.run = true;
        s.eprocessor.item_request.clock.restart();

        s.eoclient.TradeRequest(gameworld_id);

        std::string message = "Please trade me the items you want to give (available 12 seconds).";
        s.eprocessor.DelayedMessage(message, 1000);
    }*/
    else if(command == "items")
    {
        std::string message = "Inventory contains ";
        message += std::to_string(s.inventory.items.size()) + " items.";

        ret.push_back(message);
    }
    else if(command == "sitwin" && !s.eprocessor.BlockingEvent())
    {
        std::string message = "Welcome to SitAndWin. Please trade me the item you want to put in the game. You've got 15 seconds.";

        s.eprocessor.sitwin.Run(gameworld_id);

        s.eoclient.TalkPublic(message);
    }
    else if(command == "rand_winner")
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
    else if(command == "lottery" && args.size() >= 2 && !s.eprocessor.BlockingEvent())
    {
        int ticket_price = std::atoi(args[1].c_str());

        s.eprocessor.lottery.Run(ticket_price);
    }
    else if(command == "number" && args.size() >= 2)
    {
        if(s.eprocessor.lottery.run && !s.eprocessor.lottery.play && !s.eprocessor.trade.get())
        {
            int number = std::atoi(args[1].c_str());

            if(number < 1 || number > 7)
            {
                ret.push_back("Number out of range, has to be 1-7.");
                return ret;
            }

            bool registered = false;

            for(unsigned int i = 0; i < s.eprocessor.lottery.tickets.size(); ++i)
            {
                int index = s.map.GetCharacterIndex(s.eprocessor.lottery.tickets[i].gameworld_id);
                if(index != -1)
                {
                    if(s.map.characters[index].gameworld_id == gameworld_id)
                    {
                        registered = true;
                    }
                }
            }

            if(!registered)
            {
                s.eprocessor.lottery.requests.push_back(Lottery::Ticket(gameworld_id, number));
                s.eprocessor.lottery.clock.restart();

                std::string upper_name = name;
                upper_name[0] = std::toupper(name[0]);

                std::string message = upper_name + ", please pay ";
                message += std::to_string(s.eprocessor.lottery.ticket_price) + "g for the ticket (available 15 seconds).";
                s.eoclient.TalkPublic(message);
                s.eoclient.TradeRequest(gameworld_id);
            }
            else
            {
                // already registered
            }
        }
    }
    else if(command == "line")
    {
        if(s.eprocessor.chat_bot.config.entries.empty())
        {
            return ret;
        }

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

        for(unsigned int i = 0; i < s.eprocessor.quest_gen.quests.size(); ++i)
        {
            //if(s.eprocessor.quest_gen.quests[i].complete) continue;

            std::string quest_id = std::to_string(s.eprocessor.quest_gen.quests[i].id);
            std::string holder_name = s.eprocessor.quest_gen.quests[i].holder;
            holder_name[0] = std::toupper(s.eprocessor.quest_gen.quests[i].holder[0]);
            message = holder_name + "'s quest #" + quest_id + " for ";
            std::string item_name = s.eif->Get(s.eprocessor.quest_gen.quests[i].award.first).name;
            message += item_name + " x" + std::to_string(s.eprocessor.quest_gen.quests[i].award.second) + ".";

            delay_message.message = message;

            s.eprocessor.DelayedMessage(delay_message);
        }
    }
    else if(command == "questinfo" && args.size() >= 2)
    {
        int quest_id = std::atoi(args[1].c_str());

        Quest quest = s.eprocessor.quest_gen.GetQuestByID(quest_id);

        if(quest.id == 0)
        {
            ret.push_back("Sorry, quest not found.");
            return ret;
        }

        std::string message = "Quest requirements:";
        for(unsigned int i = 0; i < quest.requirements.size(); ++i)
        {
            std::string item_name = s.eif->Get(quest.requirements[i].first).name;
            std::string item_amount = std::to_string(quest.requirements[i].second);

            message += (i > 0? ", " : " ") + item_name + " x" + item_amount;
        }

        ret.push_back(message);
    }
    else if(command == "newquest" && !s.eprocessor.BlockingEvent() && !s.eprocessor.quest_gen.new_quest.get())
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
            ret.push_back("The given item has been not found in database.");
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
        s.eprocessor.quest_gen.item_request.clock.restart();

        ret.push_back("Alright. Please trade me the award item for this quest.");

        s.eoclient.TradeRequest(gameworld_id);
    }
    else if(command == "makequest" && args.size() >= 2 && !s.eprocessor.BlockingEvent())
    {
        int quest_id = std::atoi(args[1].c_str());

        Quest quest = s.eprocessor.quest_gen.GetQuestByID(quest_id);

        if(quest.id == 0)
        {
            ret.push_back("Quest not found. Sorry.");
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

        Quest quest = s.eprocessor.quest_gen.GetQuestByID(quest_id);

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

        ret.push_back("Quest is complete. Please trade me to collect your award.");
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

	if(message[0] == '#')
    {
        message.erase(0, 1);

        int index = s.map.GetCharacterIndex(name);

        std::vector<std::string> ret = ProcessCommand(name, message, s.map.characters[index].gameworld_id);
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

        std::vector<std::string> ret = ProcessCommand(name, message, s.map.characters[index].gameworld_id);
        for(unsigned int i = 0; i < ret.size(); ++i)
        {
            EventProcessor::DelayMessage delay_message(ret[i], 1000);
            delay_message.channel = 2;
            delay_message.victim_name = name;
            s.eprocessor.DelayedMessage(delay_message);
        }
    }*/
}
