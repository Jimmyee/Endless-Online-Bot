// Endless Online Bot v0.0.1

#include "handlers.hpp"
#include "../singleton.hpp"

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

        std::vector<std::string> words;
        std::string word;
        for(unsigned int i = 0; i < message.length(); ++i)
        {
            if((message[i] == ' ' || i == message.length() - 1) && !word.empty())
            {
                if(message[i] != ' ') word += message[i];
                words.push_back(word);
                word.clear();
            }
            else if(message[i] != ' ')
            {
                word += message[i];
            }
        }

        std::string command = "";
        if(words.size() > 0)
        {
            command = words[0];
        }

        printf("Command: '%s'\n", command.c_str());
        if(name == s.config.GetEntry("Master").value)
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
            else if(command == "dg" && words.size() == 2)
            {
                int amount = std::atoi(words[1].c_str());

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
        }

        if(command == "eor" && !s.eprocessor.eo_roulette.run)
        {
            s.eprocessor.eo_roulette.Run(gameworld_id);

            std::string name_upper = name;
            name_upper[0] = std::toupper(name[0]);

            s.eoclient.Talk("Welcome to EORoulette! Please trade me gold (at least 2g) to begin the game. You've got 5 seconds.");
        }
        else if(command == "help")
        {
            std::string message = "Commands: #help, #eor, #trade (eor), #uptime, #wru (PM only)";
            s.eoclient.Talk(message);
        }
        else if(command == "uptime")
        {
            int sec = s.eprocessor.uptime_clock.getElapsedTime().asSeconds();

            s.eoclient.Talk(std::string() + "I've been up for " + std::to_string(sec) + " seconds for far.");
        }
        else if(command == "trade")
        {
            if(!s.eprocessor.trade.get() && s.eprocessor.eo_roulette.run)
            {
                if(!s.eprocessor.eo_roulette.play && s.eprocessor.eo_roulette.winner == -1)
                {
                    PacketBuilder packet(PacketFamily::Trade, PacketAction::Request);
                    packet.AddChar(138);
                    packet.AddShort(gameworld_id);
                    s.eoclient.Send(packet);

                    s.eprocessor.eo_roulette.clock.restart();
                }
            }
        }
    }
    else
    {
        s.eprocessor.chat_bot.ProcessMessage(name, message);
    }
}

void Talk_Tell(PacketReader reader)
{
    S &s = S::GetInstance();

    std::string name = reader.GetBreakString();
	std::string message = reader.GetBreakString();

	if(name == s.config.GetEntry("Master").value)
    {
    }

    if(message[0] == '#')
    {
        message.erase(0, 1);

        std::vector<std::string> words;
        std::string word;
        for(unsigned int i = 0; i < message.length(); ++i)
        {
            if((message[i] == ' ' || i == message.length() - 1) && !word.empty())
            {
                if(message[i] != ' ') word += message[i];
                words.push_back(word);
                word.clear();
            }
            else if(message[i] != ' ')
            {
                word += message[i];
            }
        }

        std::string command = "";
        if(words.size() > 0)
        {
            command = words[0];
        }

        printf("Command: '%s'\n", message.c_str());
        if(command == "wru")
        {
            std::string new_message = "I'm at ";
            new_message += std::to_string(s.emf->id) + " x " + std::to_string(s.character.x) + " x " + std::to_string(s.character.y);

            PacketBuilder packet(PacketFamily::Talk, PacketAction::Tell);
            packet.AddBreakString(name);
            packet.AddString(new_message);
            s.eoclient.Send(packet);
        }
    }
}
