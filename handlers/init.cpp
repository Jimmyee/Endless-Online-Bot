// Endless Online Bot v0.0.1

#include "handlers.hpp"
#include "../packet.hpp"
#include "../singleton.hpp"
#include "../const/init.hpp"

#include <cstdio>

void INIT_INIT(PacketReader reader)
{
    S &s = S::GetInstance();

    InitReply result(static_cast<InitReply>(reader.GetByte()));

    if(result == InitReply::OK)
    {
        s.eoclient.Initialize(reader);

        s.eoclient.RegisterHandler(PacketFamily::Connection, PacketAction::Player, Connection_Player);
        s.eoclient.RegisterHandler(PacketFamily::Login, PacketAction::Reply, Login_Reply);
        s.eoclient.RegisterHandler(PacketFamily::Welcome, PacketAction::Reply, Welcome_Reply);

        PacketBuilder packet(PacketFamily::Connection, PacketAction::Accept);
        s.eoclient.Send(packet);

        s.eoclient.LoginRequest(s.config.GetValue("Username"), s.config.GetValue("Password"));
    }
    else
    {
        std::string reason = "Unknown";

        switch(result)
        {
        case InitReply::OutOfDate:
            reason = "OutOfDate";
            break;

        case InitReply::Banned:
            reason = "Banned";
            break;

        case InitReply::Map:
            reason = "Map";
            break;

        case InitReply::EIF:
            reason = "EIF";
            break;

        case InitReply::ENF:
            reason = "ENF";
            break;

        case InitReply::ESF:
            reason = "ESF";
            break;

        case InitReply::Players:
            reason = "Players";
            break;

        case InitReply::MapMutation:
            reason = "MapMutation";
            break;

        case InitReply::FriendListPlayers:
            reason = "FriendListPlayers";
            break;

        case InitReply::ECF:
            reason = "ECF";
            break;

        default:
            break;
        }

        printf("EOClient: init failed, reason: %s\n", reason.c_str());
        s.eoclient.Disconnect();
    }
}
