#include "sitwin.hpp"
#include "singleton.hpp"

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
            //int elapsed_reminder = this->reminder_clock.getElapsedTime().asSeconds();
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
