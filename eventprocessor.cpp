#include "eventprocessor.hpp"
#include "singleton.hpp"
#include "util.hpp"

#include <fstream>
#include <time.h>
#include <sys/timeb.h>

ChatBot::ChatBot()
{
    this->Load();
    this->clock.restart();
}

void ChatBot::Load()
{
    std::ifstream file("./chatbot.txt", std::ios::in | std::ios::ate);
    if(!file.is_open())
    {
        printf("Chat bot: Could not load file");
        return;
    }

    std::streampos filesize;
    filesize = file.tellg();
    char *filedata = new char[filesize];
    file.seekg(0, std::ios::beg);
    file.read(filedata, filesize);
    file.close();

    std::string filedata_str(filedata);
    std::size_t pos = 0;

    do
    {
        pos = filedata_str.find_first_of('\n');
        if(pos == std::string::npos) continue;

        std::string message = filedata_str.substr(0, pos - 1);

        this->database.push_back(message);
        std::string newdata = filedata_str.substr(pos + 1);
        filedata_str = newdata;
    } while(pos != std::string::npos);
}

void ChatBot::Save()
{
    std::ofstream file("./chatbot.txt", std::ios::out | std::ios::trunc);
    if(!file.is_open())
    {
        printf("Chat bot: Could not open file");
        return;
    }

    std::string data = "";
    for(unsigned int i = 0; i < this->database.size(); ++i)
    {
        data += this->database[i] + '\n';

    }
    file.write(data.c_str(), data.size());
    file.close();
}

std::string ChatBot::GetMessage(std::string message)
{
    for(unsigned int i = 0; i < this->database.size(); ++i)
    {
        if(this->database[i] == message)
        {
            return this->database[i];
        }
    }

    return "";
}

void ChatBot::Process()
{
    S &s = S::GetInstance();

    if(this->clock.getElapsedTime().asSeconds() > 1200)
    {
        int i = s.rand_gen.RandInt(0, this->database.size() - 1);

        s.eprocessor.DelayedMessage(database[i]);
        this->clock.restart();
    }
}

void ChatBot::ProcessMessage(std::string message)
{
    S &s = S::GetInstance();

    int save_message = s.rand_gen.RandInt(0, 33);
    if(save_message == 0)
    {
        if(this->GetMessage(message) == "" && message.length() > 1)
        {
            this->database.push_back(message);
        }
    }

    std::vector<std::string> words;
    std::string word;
    for(unsigned int i = 0; i < message.length(); ++i)
    {
        if((message[i] == ' ' || i == message.length() - 1) && !word.empty())
        {
            words.push_back(word);
            word.clear();
        }
        else if(message[i] != ' ')
        {
            word += message[i];
        }
    }

    std::vector<std::string> message_proposals;

    for(unsigned int i = 0; i < this->database.size(); ++i)
    {
        for(unsigned int ii = 0; ii < words.size(); ++ii)
        {
            if(this->database[i].find(words[ii]) != std::string::npos)
            {
                message_proposals.push_back(this->database[i]);
            }
        }
    }

    if(message_proposals.size() > 0 && s.rand_gen.RandInt(0, 11) == 0)
    {
        int i = s.rand_gen.RandInt(0, message_proposals.size() - 1);
        s.eprocessor.DelayedMessage(message_proposals[i]);
    }
}

ChatBot::~ChatBot()
{
    this->Save();
}

EORoulette::EORoulette()
{
    this->run = false;
    this->gameworld_id = -1;
    this->clock.restart();
    this->jackpot_clock.restart();
    this->reminder_clock.restart();
    this->reminder_global.restart();
    this->gold_given = 0;
    this->spins = 0;
    this->max_spins = 1;
    this->spin_delay = 100;
    this->play = false;
    this->winner = -1;
    this->total_gold = 0;
    this->jackpot = false;
    this->jp_time = 14400;
    this->jpconfig.Load("./jackpots.txt");
    this->payments = 0;
}

void EORoulette::Run(short gameworld_id)
{
    this->gameworld_id = gameworld_id;
    this->gold_given = 0;
    this->spins = 0;
    this->max_spins = S::GetInstance().rand_gen.RandInt(12, 24);
    this->spin_delay = 100;
    this->play = false;
    this->winner = -1;
    this->clock.restart();
    this->run = true;
    this->jackpot = false;
    this->payments = 0;

    S &s = S::GetInstance();

    PacketBuilder packet(PacketFamily::Trade, PacketAction::Request);
    packet.AddChar(138);
    packet.AddShort(this->gameworld_id);
    s.eoclient.Send(packet);
}

void EORoulette::Process()
{
    S &s = S::GetInstance();

    if(this->run)
    {
        if(this->play)
        {
            if(this->spins < this->max_spins)
            {
                if(this->clock.getElapsedTime().asMilliseconds() >= this->spin_delay)
                {
                    int direction = (int)s.character.direction;
                    if(++direction >= 4) direction = 0;

                    s.eoclient.Face(static_cast<Direction>(direction));

                    this->clock.restart();
                    this->spins++;
                    this->spin_delay += 900 / this->max_spins + this->spins;
                }
            }
            else
            {
                this->play = false;

                int xoff = 0;
                int yoff = 0;
                switch(s.character.direction)
                {
                case Direction::Up:
                    yoff = -1;
                    break;

                case Direction::Right:
                    xoff = 1;
                    break;

                case Direction::Down:
                    yoff = 1;
                    break;

                case Direction::Left:
                    xoff = -1;
                    break;
                }

                int winner_x = s.character.x + xoff;
                int winner_y = s.character.y + yoff;

                std::vector<Character> winners;
                for(unsigned int i = 0; i < s.map.characters.size(); ++i)
                {
                    if(s.map.characters[i].x == winner_x && s.map.characters[i].y == winner_y)
                    {
                        if(s.map.characters[i].gameworld_id != s.character.gameworld_id)
                        winners.push_back(s.map.characters[i]);
                    }
                }

                if(winners.size() > 0)
                {
                    int index = s.rand_gen.RandInt(0, winners.size() - 1);
                    Character winner = winners[index];
                    this->winner = winner.gameworld_id;

                    int black_hole = s.rand_gen.RandInt(0, 10);
                    if(black_hole == 0)
                    {
                        this->run = false;
                        int thirdpart = this->gold_given / 3;
                        this->total_gold += this->jackpot? 0 : thirdpart * 2;
                        this->jackpot = false;

                        std::string message = "Sorry, the gold has been swallowed by jackpot monster :s";
                        s.eprocessor.DelayedMessage(message, 1000);
                        return;
                    }

                    if(this->jackpot)
                    {
                        Config::Entry entry("", "");
                        entry.key = std::to_string(this->jpconfig.entries.size());
                        entry.value = winner.name + " " + std::to_string(this->gold_given);
                        this->jpconfig.entries.push_back(entry);
                        this->jpconfig.Save("./jackpots.txt");
                    }

                    std::string name_upper = winner.name;
                    name_upper[0] = std::toupper(winner.name[0]);

                    std::string message = "Congratulations " + name_upper;
                    message += ", you won " + std::to_string(this->gold_given) + " gold!";
                    s.eprocessor.DelayedMessage(message, 1000);
                    message = "Please trade me to receive your award. (Available 15 seconds)";
                    s.eprocessor.DelayedMessage(message, 5000);

                    PacketBuilder packet(PacketFamily::Trade, PacketAction::Request);
                    packet.AddChar(138);
                    packet.AddShort(winner.gameworld_id);
                    s.eoclient.Send(packet);
                }
                else
                {
                    s.eoclient.TalkPublic("Sorry, no one won.");
                    this->run = false;
                    int thirdpart = this->gold_given / 3;
                    this->total_gold += this->jackpot? 0 : thirdpart * 2;
                    this->jackpot = false;
                }
            }
        }
        else
        {
            int time_delay = (s.eprocessor.trade.get() || this->winner != -1)? 15 : 7;
            if(this->clock.getElapsedTime().asSeconds() >= time_delay)
            {
                if(this->winner == -1)
                {
                    if(this->gold_given > 0)
                    {
                        this->play = true;
                        this->clock.restart();
                        s.eoclient.TalkPublic("Start!");
                    }
                    else
                    {
                        this->run = false;
                        if(s.eprocessor.trade.get())
                        {
                            PacketBuilder packet(PacketFamily::Trade, PacketAction::Close);
                            packet.AddChar(138);
                            s.eoclient.Send(packet);
                            s.eprocessor.trade.reset();
                        }
                        s.eoclient.TalkPublic("The game has been canceled: no gold in the bank.");
                    }
                }
                else
                {
                    this->run = false;
                    int thirdpart = this->gold_given / 3;
                    this->total_gold += this->jackpot? 0 : thirdpart;
                    this->jackpot = false;
                    s.eoclient.TalkPublic("The game has been finished.");
                }
            }
        }
    }
    else
    {
        int jackpot_time = this->jp_time;
        int elapsed = this->jackpot_clock.getElapsedTime().asSeconds();
        if(elapsed >= jackpot_time)
        {
            this->gameworld_id = -1;
            this->gold_given = this->total_gold;
            this->spins = 0;
            this->max_spins = S::GetInstance().rand_gen.RandInt(12, 24);
            this->spin_delay = 100;
            this->play = true;
            this->winner = -1;
            this->clock.restart();
            this->run = true;
            this->jackpot = true;
            this->payments = 0;

            this->jackpot_clock.restart();

            s.eoclient.TalkPublic("Jackpot game started!");
        }
        else if(elapsed >= jackpot_time - 180 && elapsed < jackpot_time - 10)
        {
            if(this->reminder_clock.getElapsedTime().asSeconds() >= 30)
            {
                std::string message = "Attention! EORoulette Jackpot game starts in ";
                message += std::to_string(jackpot_time - elapsed);
                message += " seconds!";

                s.eoclient.TalkPublic(message);
                this->reminder_clock.restart();
            }
            if(this->reminder_global.getElapsedTime().asSeconds() >= 120)
            {
                std::string message = "Attention! EORoulette Jackpot game starts in ";
                message += std::to_string(jackpot_time - elapsed);
                message += " seconds! You can get my location by sending me #wru through PM.";

                s.eoclient.TalkGlobal(message);
                this->reminder_global.restart();
            }
        }
        else if(elapsed >= jackpot_time - 10 && elapsed < jackpot_time)
        {
            if(this->reminder_clock.getElapsedTime().asSeconds() >= 1)
            {
                std::string message = std::to_string(jackpot_time - elapsed);

                s.eoclient.TalkPublic(message);
                this->reminder_clock.restart();
            }
        }
    }
}

ChaseBot::ChaseBot()
{
    this->Reset();
}

void ChaseBot::Reset()
{
    this->victim_gameworld_id = -1;
    this->walk_clock.restart();
    this->follow_clock.restart();
    this->center_x = 34;
    this->center_y = 42;
    this->go_center = false;
}

void ChaseBot::Process()
{
    this->Act();
}

int GetTimestamp()
{
    time_t rawtime;
    struct tm *realtime;
    struct _timeb timebuffer;
    int hour, minn, sec, msec;

    time ( &rawtime );
    realtime=localtime( &rawtime );
    _ftime( &timebuffer );
    hour = realtime->tm_hour;
    minn = realtime->tm_min;
    sec = realtime->tm_sec;
    msec = timebuffer.millitm;

    return hour*360000 + minn*6000 + sec*100 + msec/10;
}

bool Walkable(unsigned char x, unsigned char y)
{
    S &s = S::GetInstance();

    bool warp_near = false;

    if(s.emf->GetWarp(x, y - 1).map != 0 || s.emf->GetWarp(x + 1, y).map != 0
       || s.emf->GetWarp(x, y + 1).map != 0 || s.emf->GetWarp(x - 1, y).map != 0)
    {
        warp_near = true;
    }

    if(s.emf->Walkable(x, y)/* && !s.map.Occupied(x, y)*/ && !warp_near)
    {
        return true;
    }

    return false;
}

bool ChaseBot::Walk(Direction direction)
{
    S &s = S::GetInstance();

    int xoff = 0;
    int yoff = 0;

    switch(direction)
    {
    case Direction::Up:
        yoff--;
        break;

    case Direction::Right:
        xoff++;
        break;

    case Direction::Down:
        yoff++;
        break;

    case Direction::Left:
        xoff--;
        break;
    }

    if(xoff != 0 || yoff != 0)
    {
        if(Walkable(s.character.x + xoff, s.character.y + yoff) && this->walk_clock.getElapsedTime().asMilliseconds() >= 460)
        {
            s.character.direction = direction;

            PacketBuilder packet(PacketFamily::Walk, PacketAction::Player);
            packet.AddChar((unsigned char)s.character.direction);
            packet.AddThree(GetTimestamp());
            packet.AddChar(s.character.x + xoff);
            packet.AddChar(s.character.y + yoff);
            s.eoclient.Send(packet);

            s.character.x += xoff;
            s.character.y += yoff;

            this->walk_clock.restart();

            return true;
        }
    }

    return false;
}

void ChaseBot::WalkTo(unsigned char x, unsigned char y)
{
    S &s = S::GetInstance();

    int xdiff = s.character.x - x;
    int ydiff = s.character.y - y;
    int absxdiff = std::abs(xdiff);
    int absydiff = std::abs(ydiff);
    Direction direction;

    if ((absxdiff == 1 && absydiff == 0) || (absxdiff == 0 && absydiff == 1) || (absxdiff == 0 && absydiff == 0))
    {
        return;
    }
    else if (absxdiff > absydiff)
    {
        if (xdiff < 0)
        {
            direction = Direction::Right;
        }
        else
        {
            direction = Direction::Left;
        }
    }
    else
    {
        if (ydiff < 0)
        {
            direction = Direction::Down;
        }
        else
        {
            direction = Direction::Up;
        }
    }

    if(!this->Walk(direction))
    {
        if (direction == Direction::Up || direction == Direction::Down)
        {
            if (xdiff < 0)
            {
                direction = Direction::Right;
            }
            else
            {
                direction = Direction::Left;
            }
        }

        if(!this->Walk(direction))
        {
            this->Walk(static_cast<Direction>(s.rand_gen.RandInt(0,3)));
        }
    }
}

void ChaseBot::Act()
{
    S &s = S::GetInstance();

    int distance_center = path_length(this->center_x, this->center_y, s.character.x, s.character.y);

    if(this->victim_gameworld_id == -1 && !this->go_center)
	{
	    short farthest = -1;
		unsigned char farthest_distance = 1;

		for(unsigned int i = 0; i < s.map.characters.size(); ++i)
		{
			int distance = path_length(s.map.characters[i].x, s.map.characters[i].y, s.character.x, s.character.y);

			if (distance == 0)
				distance = 1;

			if (distance > farthest_distance && s.map.characters[i].gameworld_id != s.character.gameworld_id)
			{
				farthest = s.map.characters[i].gameworld_id;
				farthest_distance = distance;
			}
		}

		if(farthest != -1)
		{
		    this->victim_gameworld_id = farthest;
            this->follow_clock.restart();
		}
	}

	if(this->victim_gameworld_id != -1)
    {
        if(!this->go_center && this->follow_clock.getElapsedTime().asSeconds() < 180)
        {
            int victim = s.map.GetCharacterIndex(this->victim_gameworld_id);
            if(victim == -1)
            {
                this->victim_gameworld_id = -1;
                return;
            }

            int distance = path_length(s.map.characters[victim].x, s.map.characters[victim].y, s.character.x, s.character.y);

            if(distance > 2)
            {
                this->WalkTo(s.map.characters[victim].x, s.map.characters[victim].y);
            }
        }
        else if(!this->go_center && this->follow_clock.getElapsedTime().asSeconds() >= 180)
        {
            this->go_center = true;
            this->victim_gameworld_id = -1;
        }
    }
    else
    {
        if(distance_center > 3 && !this->go_center)
        {
            this->go_center = true;
        }
    }

    if(this->go_center)
    {
        distance_center = path_length(this->center_x, this->center_y, s.character.x, s.character.y);

        if(distance_center > 3)
        {
            this->WalkTo(this->center_x, this->center_y);
        }
        else
        {
            this->go_center = false;
        }
    }
}

EventProcessor::EventProcessor()
{
    this->help_message_clock.restart();
    this->autosave_clock.restart();
    this->uptime_clock.restart();
    this->refresh_clock.restart();
}

void EventProcessor::Process()
{
    S &s = S::GetInstance();

    if(this->autosave_clock.getElapsedTime().asSeconds() >= 900)
    {
        this->chat_bot.Save();
        this->autosave_clock.restart();
    }

    this->eo_roulette.Process();

    if(this->item_request.run)
    {
        if(this->item_request.clock.getElapsedTime().asSeconds() >= 15)
        {
            this->item_request.run = false;

            if(this->trade.get())
            {
                this->trade.reset();

                PacketBuilder packet(PacketFamily::Trade, PacketAction::Close);
                packet.AddChar(138);
                s.eoclient.Send(packet);
            }

            this->DelayedMessage("Trade canceled due to player inactivity.", 1000);
        }
    }

    if(!this->eo_roulette.run)
    {
        //this->chase_bot.Process();
    }

    this->chat_bot.Process();

    if(this->help_message_clock.getElapsedTime().asSeconds() > 1200)
    {
        this->DelayedMessage("Commands: type #help for command list.");
        this->help_message_clock.restart();
    }

    if(this->d_messages.size() > 0)
    {
        if(this->d_messages[0].clock.getElapsedTime().asMilliseconds() >= this->d_messages[0].time_ms)
        {
            s.eoclient.TalkPublic(this->d_messages[0].message);
            this->d_messages.erase(this->d_messages.begin());
            if(this->d_messages.size() > 0)
            {
                this->d_messages[0].clock.restart();
            }
        }
    }

    if(this->refresh_clock.getElapsedTime().asSeconds() >= 60)
    {
        s.eoclient.RefreshRequest();
        this->refresh_clock.restart();
    }
}

void EventProcessor::DelayedMessage(std::string message, int time_ms)
{
    int message_delay = time_ms;
    if(message_delay == 0)
    {
        message_delay = message.length() * 120 + S::GetInstance().rand_gen.RandInt(120, 1200);
    }

    this->d_messages.push_back(DelayMessage(message, message_delay));
}
