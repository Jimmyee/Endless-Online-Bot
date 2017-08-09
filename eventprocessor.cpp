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
    std::ifstream file("./chatbot.txt", std::ios::in);
    if(!file.is_open())
    {
        printf("Chat bot: Could not load file");
        return;
    }

    std::streampos filesize;
    file.seekg(0, std::ios::end);
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

        std::string message = filedata_str.substr(0, pos);

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
    this->max_spins += S::GetInstance().rand_gen.RandInt(0, 3);
    this->spin_delay = 100;
    this->play = false;
    this->winner = -1;
    this->clock.restart();
    this->run = true;
    this->jackpot = false;
    this->payments = 0;

    S &s = S::GetInstance();

    s.eoclient.TradeRequest(this->gameworld_id);
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
                        if(s.map.characters[i].gameworld_id != s.character.gameworld_id && !this->jackpot)
                        {
                            winners.push_back(s.map.characters[i]);
                        }
                        else if(s.map.characters[i].gameworld_id != s.character.gameworld_id && this->jackpot)
                        {
                            winners.push_back(s.map.characters[i]);
                        }
                    }
                }

                bool alternative_winner = false;
                if(winners.empty())
                {
                    for(unsigned int i = 0; i < s.map.characters.size(); ++i)
                    {
                        int path_len = path_length(s.character.x, s.character.y, s.map.characters[i].x, s.map.characters[i].y);

                        if(path_len == 1)
                        {
                            winners.push_back(s.map.characters[i]);
                            alternative_winner = true;
                        }
                    }
                }

                if(winners.size() > 0)
                {
                    int index = s.rand_gen.RandInt(0, winners.size() - 1);
                    index += s.rand_gen.RandInt(0, 3);
                    if(index >= winners.size()) index = s.rand_gen.RandInt(0, winners.size() - 1);

                    Character winner = winners[index];
                    this->winner = winner.gameworld_id;

                    if((this->jackpot && this->gold_given > 0) || this->gold_given >= 999)
                    {
                        Config::Entry entry("", "");
                        entry.key = std::to_string(this->jpconfig.entries.size());
                        entry.value = winner.name + " " + std::to_string(this->gold_given);
                        this->jpconfig.entries.push_back(entry);
                        this->jpconfig.Save("./jackpots.txt");
                    }

                    int jackpot_percentage = this->gold_given / 20;
                    this->gold_given -= jackpot_percentage;

                    if(alternative_winner)
                    {
                        s.eprocessor.DelayedMessage("No one won, giving gold to random player...", 1000);
                    }

                    std::string name_upper = winner.name;
                    name_upper[0] = std::toupper(winner.name[0]);

                    std::string message = "Congratulations " + name_upper;
                    message += ", you won " + std::to_string(this->gold_given) + " gold!";
                    s.eprocessor.DelayedMessage(message, alternative_winner? 3000 : 1000);
                    message = "Please trade me to receive your award. (Available 15 seconds)";
                    s.eprocessor.DelayedMessage(message, 3000);

                    s.eoclient.TradeRequest(winner.gameworld_id);
                }
                else
                {
                    std::string message = "Sorry, no one won.";
                    s.eprocessor.DelayedMessage(message, 1000);

                    this->run = false;
                    int jackpot_percentage = (this->gold_given / 3) * 2;
                    this->total_gold += this->jackpot? 0 : jackpot_percentage;
                    this->jackpot = false;
                }
            }
        }
        else
        {
            int time_delay = (s.eprocessor.trade.get() || this->winner != -1)? 15 : 12;
            if(this->clock.getElapsedTime().asSeconds() >= time_delay)
            {
                if(s.eprocessor.trade.get())
                {
                    s.eoclient.TradeClose();
                }

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
                        s.eoclient.TalkPublic("The game has been canceled: no gold in the bank.");
                    }
                }
                else
                {
                    this->run = false;
                    int jackpot_percentage = (this->gold_given / 3) * 2;
                    this->gold_given = 0;
                    this->total_gold += this->jackpot? 0 : jackpot_percentage;
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
        if(elapsed >= jackpot_time && !s.eprocessor.BlockingEvent())
        {
            this->gameworld_id = -1;
            this->gold_given = this->total_gold;
            this->spins = 0;
            this->max_spins = S::GetInstance().rand_gen.RandInt(12, 24);
            this->max_spins += S::GetInstance().rand_gen.RandInt(0, 3);
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
            if(this->reminder_clock.getElapsedTime().asSeconds() >= 120)
            {
                std::string message = "Attention! EORoulette Jackpot game starts in ";
                message += std::to_string(jackpot_time - elapsed);
                message += " seconds!";

                s.eoclient.TalkPublic(message);
                this->reminder_clock.restart();
            }
            else if(this->reminder_global.getElapsedTime().asSeconds() >= 180)
            {
                std::string message = "Attention! EORoulette Jackpot game starts in ";
                message += std::to_string(jackpot_time - elapsed);
                message += " seconds! You can get my location by sending me #wru through PM.";

                //s.eoclient.TalkGlobal(message);
                this->reminder_global.restart();
            }
        }
        /*else if(elapsed >= jackpot_time - 10 && elapsed < jackpot_time)
        {
            if(this->reminder_clock.getElapsedTime().asSeconds() >= 1)
            {
                std::string message = std::to_string(jackpot_time - elapsed);

                s.eoclient.TalkPublic(message);
                this->reminder_clock.restart();
            }
        }*/
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

bool Walkable(unsigned char x, unsigned char y)
{
    S &s = S::GetInstance();

    bool warp_near = false;

    if(s.emf->GetWarp(x, y - 1).map != 0 || s.emf->GetWarp(x + 1, y).map != 0
       || s.emf->GetWarp(x, y + 1).map != 0 || s.emf->GetWarp(x - 1, y).map != 0)
    {
        warp_near = true;
    }

    if(s.emf->Walkable(x, y) && !s.map.Occupied(x, y) && !warp_near)
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
            s.eoclient.Walk(direction);

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
	    short closest = -1;
		unsigned char closest_distance = 1;

		for(unsigned int i = 0; i < s.map.characters.size(); ++i)
		{
			int distance = path_length(s.map.characters[i].x, s.map.characters[i].y, s.character.x, s.character.y);

			if (distance == 0)
				distance = 1;

			if (distance < closest_distance && s.map.characters[i].gameworld_id != s.character.gameworld_id)
			{
				closest = s.map.characters[i].gameworld_id;
				closest_distance = distance;
			}
		}

		if(closest != -1)
		{
		    this->victim_gameworld_id = closest;
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
            int char_distance_center = path_length(this->center_x, this->center_y, s.map.characters[victim].x, s.map.characters[victim].y);

            if(distance > 2 && char_distance_center < 6)
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
        if(distance_center >= 6 && !this->go_center)
        {
            this->go_center = true;
        }
    }

    if(this->go_center)
    {
        if(distance_center >= 6)
        {
            this->WalkTo(this->center_x, this->center_y);
        }
        else
        {
            this->go_center = false;
        }
    }
}

SitWin::SitWin(short item_id, short gameworld_id, short item_amount)
{
    this->run = false;
    this->play = false;
    this->item_id = item_id;
    this->item_amount = item_amount;
    this->gameworld_id = gameworld_id;
    this->winner = -1;
    this->clock.restart();
}

SitWin::SitWin()
{
    this->run = false;
    this->play = false;
    this->item_id = 0;
    this->item_amount = 0;
    this->gameworld_id = -1;
    this->winner = -1;
    this->clock.restart();
    this->reminder_clock.restart();
}

void SitWin::Run(short gameworld_id)
{
    this->run = true;
    this->play = false;
    this->item_id = 0;
    this->item_amount = 0;
    this->gameworld_id = gameworld_id;
    this->winner = -1;
    this->clock.restart();
    this->reminder_clock.restart();

    S::GetInstance().eoclient.TradeRequest(this->gameworld_id);
}

void SitWin::RunJackpot(short item_id, int item_amount)
{
    this->run = true;
    this->play = false;
    this->item_id = item_id;
    this->item_amount = item_amount;
    this->gameworld_id = -1;
    this->winner = -1;
    this->clock.restart();
    this->reminder_clock.restart();

    S::GetInstance().eoclient.TalkPublic("20 seconds to jackpot!");
}

void SitWin::Process()
{
    S &s = S::GetInstance();

    if(this->run)
    {
        if(this->play)
        {
            this->play = false;

            std::vector<Character> winners;
            for(unsigned int i = 0; i < s.map.characters.size(); ++i)
            {
                int distance = path_length(s.map.characters[i].x, s.map.characters[i].y, s.character.x, s.character.y);
                if(distance <= 1)
                {
                    if(s.map.characters[i].gameworld_id != s.character.gameworld_id && s.map.characters[i].sitting == SitState::Floor)
                    {
                        winners.push_back(s.map.characters[i]);
                    }
                }
            }

            if(winners.size() > 0)
            {
                unsigned int index = s.rand_gen.RandInt(0, winners.size() - 1);
                index += s.rand_gen.RandInt(0, 3);
                if(index >= winners.size()) index = s.rand_gen.RandInt(0, winners.size() - 1);

                Character winner = winners[index];
                this->winner = winner.gameworld_id;

                std::string name_upper = winner.name;
                name_upper[0] = std::toupper(winner.name[0]);

                std::string item_name = s.eif->Get(this->item_id).name;
                std::string item_amount_str = std::to_string(this->item_amount);

                std::string message = "Congratulations " + name_upper;
                message += ", you won " + item_name + " x" + item_amount_str + ".";
                s.eprocessor.DelayedMessage(message, 1000);
                message = "Please trade me to receive your award. (Available 15 seconds)";
                s.eprocessor.DelayedMessage(message, 5000);

                s.eoclient.TradeRequest(this->winner);
                s.eprocessor.sitwin.clock.restart();
            }
            else
            {
                s.eoclient.TalkPublic("Sorry, no one won.");
                this->run = false;

            }
        }
        else
        {
            int elapsed = this->clock.getElapsedTime().asSeconds();
            int elapsed_reminder = this->reminder_clock.getElapsedTime().asSeconds();
            int time_delay = (s.eprocessor.trade.get() || this->winner != -1)? 15 : 20;
            if(elapsed >= time_delay)
            {
                if(this->winner == -1)
                {
                    if(s.eprocessor.trade.get())
                    {
                        s.eoclient.TradeClose();
                    }

                    if(this->item_id != 0 && this->item_amount != 0)
                    {
                        this->Play();
                    }
                    else
                    {
                        this->gameworld_id = -1;
                        this->run = false;
                        s.eoclient.TalkPublic("Game canceled: no item selected.");
                    }
                }
                else
                {
                    if(s.eprocessor.trade.get())
                    {
                        s.eoclient.TradeClose();
                    }

                    s.eoclient.TalkPublic("Time's up. The game has been finished.");
                    this->gameworld_id = -1;
                    this->run = false;
                }
            }
        }
    }
}

void SitWin::Play()
{
    this->play = true;
    this->winner = -1;
    this->clock.restart();
}

SitWinJackpot::SitWinJackpot()
{
    this->item_id = 0;
    this->item_amount = 0;
    this->jp_time = 12600;
    //this->Reset();
}

bool SitWinJackpot::GenerateItem()
{
    S &s = S::GetInstance();

    short new_item_id = 0;
    int new_item_amount = 0;

    if(s.inventory.items.size() == 0)
    {
        this->item_id = new_item_id;
        this->item_amount = new_item_amount;

        return false;
    }

    unsigned int index = s.rand_gen.RandInt(0, s.inventory.items.size() - 1);
    while(s.inventory.items[index].first == 1 && s.inventory.items.size() > 1)
    {
        index = s.rand_gen.RandInt(0, s.inventory.items.size() - 1);
    }
    if(s.inventory.items[index].first == 1) return false;

    new_item_id = (short)s.inventory.items[index].first;
    new_item_amount = s.rand_gen.RandInt(1, (int)s.inventory.items[index].second);

    this->item_id = new_item_id;
    this->item_amount = new_item_amount;

    return true;
}

void SitWinJackpot::Process()
{
    S &s = S::GetInstance();

    int elapsed = this->clock.getElapsedTime().asSeconds();
    if(elapsed >= this->jp_time && !s.eprocessor.eo_roulette.run && !s.eprocessor.sitwin.run && !s.eprocessor.item_request.run && !s.eprocessor.trade.get())
    {
        int item_id = this->item_id;
        int item_amount = this->item_amount;

        this->Reset();

        s.eprocessor.sitwin.RunJackpot(item_id, item_amount);
    }
    else if(elapsed >= this->jp_time - 180 && elapsed < this->jp_time - 10)
    {
        if(this->reminder_clock.getElapsedTime().asSeconds() >= 120)
        {
            std::string message = "Attention! SitAndWin Jackpot game starts in ";
            message += std::to_string(this->jp_time - elapsed);
            message += " seconds!";

            s.eoclient.TalkPublic(message);
            this->reminder_clock.restart();
        }
        else if(this->reminder_global.getElapsedTime().asSeconds() >= 180)
        {
            std::string message = "Attention! SitAndWin Jackpot game starts in ";
            message += std::to_string(this->jp_time - elapsed);
            message += " seconds! You can get my location by sending me #wru through PM.";

            //s.eoclient.TalkGlobal(message);
            this->reminder_global.restart();
        }
    }
    /*else if(elapsed >= this->jp_time - 10 && elapsed < this->jp_time)
    {
        if(this->reminder_clock.getElapsedTime().asSeconds() >= 1)
        {
            std::string message = std::to_string(this->jp_time - elapsed);

            s.eoclient.TalkPublic(message);
            this->reminder_clock.restart();
        }
    }*/
}

void SitWinJackpot::Reset()
{
    this->clock.restart();
    this->reminder_clock.restart();
    this->reminder_global.restart();

    this->GenerateItem();
}

Lottery::Lottery()
{
    this->run = false;
    this->play = false;
    this->clock.restart();
    this->winner = -1;
    this->ticket_price = 2;
}

void Lottery::Run()
{
    this->run = true;
    this->clock.restart();
    this->tickets.clear();
    this->requests.clear();
    this->winner = -1;
    this->ticket_price = 2;

    S::GetInstance().eoclient.TalkPublic("Lottery game started! Please choose 1 number with #number <number> (in range of 1-7).");
}

void Lottery::Run(int ticket_price)
{
    this->Run();
    this->ticket_price = ticket_price;
}

void Lottery::Process()
{
    S &s = S::GetInstance();

    if(this->run)
    {
        if(this->play)
        {
            this->play = false;

            if(this->tickets.empty())
            {
                this->run = false;
                s.eoclient.TalkPublic("The game has been canceled - no tickets registered.");
                return;
            }

            int winning_number = s.rand_gen.RandInt(1, 7);
            std::vector<Character> winners;

            for(unsigned int i = 0; i < this->tickets.size(); ++i)
            {
                if(this->tickets[i].number == winning_number)
                {
                    int index = s.map.GetCharacterIndex(this->tickets[i].gameworld_id);
                    if(index != -1)
                    {
                        winners.push_back(s.map.characters[index]);
                    }
                }
            }

            if(this->tickets.size() == 1 && winners.empty())
            {
                int index = s.map.GetCharacterIndex(this->tickets[1].gameworld_id);
                winners.push_back(s.map.characters[index]);
            }

            std::string message = "The winning number is: " + std::to_string(winning_number) + ".";
            s.eprocessor.DelayedMessage(EventProcessor::DelayMessage(message, 2000));

            if(winners.empty())
            {
                this->run = false;
                message = "Sorry, no one won.";
                s.eprocessor.DelayedMessage(EventProcessor::DelayMessage(message, 2000));
                return;
            }

            unsigned int winner_index = s.rand_gen.RandInt(0, winners.size() - 1);
            this->winner = winners[winner_index].gameworld_id;
            int gold = this->tickets.size() * this->ticket_price;
            int award = gold;
            award -= gold / 20;

            std::string name = winners[winner_index].name;
            std::string upper_name = name;
            upper_name[0] = std::toupper(name[0]);

            message = "Congratulations " + upper_name + ", you won " + std::to_string(award) + " gold.";
            s.eprocessor.DelayedMessage(EventProcessor::DelayMessage(message, 2000));
            message = "Please trade me to receive your reward (available 15 seconds).";
            s.eprocessor.DelayedMessage(EventProcessor::DelayMessage(message, 2000));

            s.eoclient.TradeRequest(this->winner);
        }
        else
        {
            int time_delay = 15;
            if(this->clock.getElapsedTime().asSeconds() >= time_delay)
            {
                if(s.eprocessor.trade.get())
                {
                    s.eoclient.TradeClose();
                }

                if(!this->tickets.empty())
                {
                    s.eoclient.TalkPublic("Start!");
                    this->play = true;
                    this->clock.restart();
                }
                else
                {
                    std::string message = "Time out. No tickets registered.";

                    this->run = false;
                }
            }
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
        int time_delay = (s.eprocessor.trade.get())? 30 : 12;
        if(this->item_request.clock.getElapsedTime().asSeconds() >= time_delay)
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

    this->sitwin.Process();
    this->sitwin_jackpot.Process();

    this->chat_bot.Process();
    //this->chase_bot.Process();
    this->lottery.Process();

    if(this->help_message_clock.getElapsedTime().asSeconds() > 1200)
    {
        this->DelayedMessage("Commands: type #help for command list.");
        //this->DelayedMessage("Hey, on 4th august Jimmyeebot is gonna launch a game in which you can win more than 400 000 gold.");
        //this->DelayedMessage("The time the game will be played at is: 00:22, UTC+02:00, near Aeven church. See you there and good luck!");
        this->help_message_clock.restart();
    }

    if(this->d_messages.size() > 0)
    {
        if(this->d_messages[0].clock.getElapsedTime().asMilliseconds() >= this->d_messages[0].time_ms)
        {
            std::vector<std::string> buffer;
            std::string message = this->d_messages[0].message;
            while(message.length() > 128)
            {
                buffer.push_back(message.substr(0, 128));
                message.erase(0, 128);
            }
            if(message.length() > 0)
            {
                buffer.push_back(message);
            }

            for(unsigned int i = 0; i < buffer.size(); ++i)
            {
                if(this->d_messages[0].channel == 0)
                {
                    s.eoclient.TalkPublic(buffer[i]);
                }
                if(this->d_messages[0].channel == 1)
                {
                    s.eoclient.TalkTell(this->d_messages[0].victim_name, buffer[i]);
                }
                if(this->d_messages[0].channel == 2)
                {
                    s.eoclient.TalkGlobal(buffer[i]);
                }
            }
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

void EventProcessor::DelayedMessage(DelayMessage delay_message)
{
    if(this->d_messages.size() > 0)
    {
        bool found = false;
        for(unsigned int i = 0; i < this->d_messages.size(); ++i)
        {
            if(this->d_messages[i].message == delay_message.message && this->d_messages[i].victim_name == delay_message.victim_name)
            {
                found = true;
            }
        }

        if(found) return;
    }

    this->d_messages.push_back(delay_message);
}

bool EventProcessor::BlockingEvent()
{
    if(this->trade.get() || this->eo_roulette.run || this->item_request.run || this->sitwin.run || this->lottery.run)
    {
        return true;
    }

    return false;
}

bool EventProcessor::Whitelist(std::string name)
{
    for(unsigned int i = 0; i < this->whitelist.size(); ++i)
    {
        if(this->whitelist[i] == name)
        {
            return true;
        }
    }

    return false;
}
