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
    this->payments.clear();
    this->players.clear();

    S &s = S::GetInstance();

    for(unsigned int i = 0; i < s.map.characters.size(); ++i)
    {
        s.map.characters[i].eor_payments = 0;
    }

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
                if(this->jackpot)
                {
                    this->players = s.map.characters;
                }
                for(unsigned int i = 0; i < this->players.size(); ++i)
                {
                    if(this->players[i].x == winner_x && this->players[i].y == winner_y)
                    {
                        if(this->players[i].gameworld_id != s.character.gameworld_id && !this->jackpot)
                        {
                            winners.push_back(this->players[i]);
                        }
                        else if(this->players[i].gameworld_id != s.character.gameworld_id && this->jackpot)
                        {
                            winners.push_back(this->players[i]);
                        }
                    }
                }

                bool alternative_winner = false;
                if(winners.empty())
                {
                    for(unsigned int i = 0; i < this->players.size(); ++i)
                    {
                        if(this->players[i].gameworld_id != s.character.gameworld_id)
                        {
                            winners.push_back(this->players[i]);
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
                    if(this->jackpot || (this->payments.size() == 1 && this->winner == this->gameworld_id)) jackpot_percentage = 0;
                    this->gold_given -= jackpot_percentage;
                    this->total_gold += jackpot_percentage;

                    if(alternative_winner)
                    {
                        bool not_player = false;
                        for(unsigned int i = 0; i < s.map.characters.size(); ++i)
                        {
                            if(s.map.characters[i].x == winner_x && s.map.characters[i].y == winner_y)
                            {
                                if(s.map.characters[i].gameworld_id != s.character.gameworld_id && !this->jackpot)
                                {
                                    not_player = true;
                                    break;
                                }
                            }
                        }

                        std::string message = "No one won, giving gold to random player...";
                        if(not_player) message += " You have to bet to play.";

                        s.eprocessor.DelayedMessage(message, 1000);
                    }

                    std::string name_upper = winner.name;
                    name_upper[0] = std::toupper(winner.name[0]);

                    std::string message = "Congratulations " + name_upper;
                    message += ", you won " + std::to_string(this->gold_given) + " gold!";
                    message += " (" + std::to_string(jackpot_percentage) + "g taken to the Jackpot)";
                    s.eprocessor.DelayedMessage(message, alternative_winner? 3000 : 1000);
                    message = "Please trade me to receive your award. (Available 30 seconds)";
                    s.eprocessor.DelayedMessage(message, 3000);

                    s.eoclient.TradeRequest(winner.gameworld_id);
                }
                else
                {
                    this->run = false;
                    int jackpot_percentage = (this->gold_given / 3) * 2;
                    this->total_gold += this->jackpot? 0 : jackpot_percentage;
                    this->jackpot = false;

                    for(unsigned int i = 0; i < s.map.characters.size(); ++i)
                    {
                        if(s.map.characters[i].x == winner_x && s.map.characters[i].y == winner_y)
                        {
                            if(s.map.characters[i].gameworld_id != s.character.gameworld_id && !this->jackpot)
                            {
                                winners.push_back(s.map.characters[i]);
                            }
                        }
                    }

                    std::string message = "Sorry, no one won.";
                    if(winners.size() > 0) message += " You have to bet to join the game.";
                    s.eprocessor.DelayedMessage(message, 1000);
                }
            }
        }
        else
        {
            if(this->clock.getElapsedTime().asSeconds() >= 30)
            {
                if(s.eprocessor.trade.get())
                {
                    int i = s.map.GetCharacterIndex(s.eprocessor.trade->victim_gameworld_id);
                    if(i != -1)
                    {
                        s.eoclient.TradeClose();
                        s.map.characters[i].eor_payments++;
                        this->clock.restart();
                        s.eprocessor.DelayedMessage("Trade canceled. The game will start in 30 seconds...", 1000);
                    }
                    return;
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
                    this->total_gold = this->jackpot? 0 : this->gold_given;
                    this->gold_given = 0;
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
            this->jp_time = 14400;
            this->jackpot_clock.restart();

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
                this->payments.clear();
                this->players.clear();

                for(unsigned int i = 0; i < s.map.characters.size(); ++i)
                {
                    s.map.characters[i].eor_payments = 0;
                }

                s.eoclient.TalkPublic("Jackpot game started!");
            }
            else
            {
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
    }
}
