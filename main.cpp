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

int main()
{
    initialize_data_handlers();

    S &s = S::GetInstance();

    while (!s.call_exit)
    {
        bool was_connected = s.eoclient.Connected();

        s.eoclient.Tick();

        if(!s.eoclient.Connected() && was_connected)
        {
            s.eoclient.Connect();
        }

        s.eprocessor.Process();

        if(s.call_exit)
        {
            if(s.eoclient.Connected())
            {
                s.eoclient.Disconnect();
            }
        }
    }

    return 0;
}
