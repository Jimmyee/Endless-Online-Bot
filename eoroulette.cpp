#include "eoroulette.hpp"
#include "singleton.hpp"

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
                    unsigned int index = s.rand_gen.RandInt(0, winners.size() - 1);
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
                    if(this->jackpot || (this->payments == 1 && this->winner == this->gameworld_id)) jackpot_percentage = 0;
                    this->gold_given -= jackpot_percentage;
                    this->total_gold += jackpot_percentage;

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
            if(this->total_gold > 0)
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
            else
            {
                this->jackpot_clock.restart();

                s.eoclient.TalkPublic("The jackpot game was held: no gold in the bank.");
            }
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
