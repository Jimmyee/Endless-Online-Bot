#include "lottery.hpp"
#include "singleton.hpp"

Lottery::Lottery()
{
    this->run = false;
    this->play = false;
    this->clock.restart();
    this->winner = -1;
    this->ticket_price = 2;
    this->award = 0;
}

void Lottery::Run()
{
    this->run = true;
    this->clock.restart();
    this->tickets.clear();
    this->requests.clear();
    this->winner = -1;
    this->ticket_price = 2;
    this->award = 0;

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
                int index = s.map.GetCharacterIndex(this->tickets[0].gameworld_id);
                winners.push_back(s.map.characters[index]);
            }

            int gold = this->tickets.size() * this->ticket_price;
            int jackpot_percentage = gold / 20;
            if(this->tickets.size() == 1) jackpot_percentage = 0;
            this->award = gold;
            this->award -= jackpot_percentage;
            s.eprocessor.eo_roulette.total_gold += jackpot_percentage / 2;

            std::string message = "The winning number is: " + std::to_string(winning_number) + ".";
            s.eprocessor.DelayedMessage(DelayMessage(message, 2000));

            if(winners.empty())
            {
                this->run = false;
                message = "Sorry, no one won.";
                s.eprocessor.DelayedMessage(DelayMessage(message, 2000));
                return;
            }

            unsigned int winner_index = s.rand_gen.RandInt(0, winners.size() - 1);
            this->winner = winners[winner_index].gameworld_id;

            std::string name = winners[winner_index].name;
            std::string upper_name = name;
            upper_name[0] = std::toupper(name[0]);

            if(this->tickets.size() == 1)
            {
                message = upper_name + ", no one played with you. Here's your gold back.";
            }
            else
            {
                message = "Congratulations " + upper_name + ", you won " + std::to_string(this->award) + " gold.";
            }
            s.eprocessor.DelayedMessage(DelayMessage(message, 2000));
            message = "Please trade me to receive your reward (available 15 seconds).";
            s.eprocessor.DelayedMessage(DelayMessage(message, 2000));

            this->tickets.clear();

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
                    std::string message = "Time out. The game has been finished.";

                    s.eoclient.TalkPublic(message);

                    this->run = false;
                }
            }
        }
    }
}
