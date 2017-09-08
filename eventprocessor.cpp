#include "eventprocessor.hpp"
#include "singleton.hpp"

EventProcessor::EventProcessor()
{
    this->help_message_clock.restart();
    this->autosave_clock.restart();
    this->uptime_clock.restart();
    this->refresh_clock.restart();

    this->help_config.Load("help.txt");
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

                s.eoclient.TradeClose();
            }

            this->DelayedMessage("Trade canceled due to player inactivity.", 1000);
        }
    }

    this->sitwin.Process();

    this->chat_bot.Process();
    if(s.config.GetValue("ChaseBot") == "yes") this->chase_bot.Process();
    this->lottery.Process();
    this->quest_gen.Process();

    if(this->help_message_clock.getElapsedTime().asSeconds() > 14400)
    {
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
        message_delay = message.length() * 90 + S::GetInstance().rand_gen.RandInt(90, 900);
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
    if(this->trade.get() || this->eo_roulette.run || this->item_request.run || this->sitwin.run || this->lottery.run
       || this->quest_gen.new_quest.get() || this->quest_gen.item_request.run)
    {
        return true;
    }

    return false;
}
