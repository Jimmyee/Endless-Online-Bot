// Endless Online Bot v0.0.1

#include <string>
#include <random>
#include <chrono>

#include "singleton.hpp"

// TODO: create an alternative for singleton

void initialize_data_handlers()
{
    S &s = S::GetInstance();

    s.config.Load("./config.ini");
    s.eif = shared_ptr<EIF>(new EIF("./pub/dat001.eif"));
    s.enf = shared_ptr<ENF>(new ENF("./pub/dtn001.enf"));
    s.esf = shared_ptr<ESF>(new ESF("./pub/dsl001.esf"));
    s.ecf = shared_ptr<ECF>(new ECF("./pub/dat001.ecf"));

    puts("data handlers initialized");
}

int main(int, char**)
{
    initialize_data_handlers();

    S &s = S::GetInstance();

    sf::Clock reconnect_clock;
    sf::Clock init_clock;
    bool reconnect = false;

    while (!s.call_exit)
    {
        s.eoclient.Tick();

        if(!s.eoclient.Connected())
        {
            bool connect = false;
            if(reconnect)
            {
                if(reconnect_clock.getElapsedTime().asSeconds() >= 5)
                {
                    connect = true;
                    reconnect_clock.restart();
                }
            }
            else
            {
                connect = true;
            }

            if(connect)
            {
                if(s.eoclient.Connect())
                {
                    s.eoclient.RequestInit();
                    init_clock.restart();
                }
            }
            reconnect = true;
        }
        else
        {
            if(s.eoclient.GetState() == EOClient::State::Uninitialized && init_clock.getElapsedTime().asSeconds() >= 10)
            {
                puts("Initialization time out.");
                s.eoclient.Disconnect();
            }
        }

        if(s.eoclient.GetState() == EOClient::State::Playing)
        {
            s.eprocessor.Process();
        }

        if(s.call_exit)
        {
            if(s.eoclient.Connected())
            {
                s.eoclient.Disconnect();
            }
        }

        sf::sleep(sf::milliseconds(1));
    }

    return 0;
}
