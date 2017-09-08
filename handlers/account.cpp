// Endless Online Awaken v0.0.1

#include "handlers.hpp"
#include "../singleton.hpp"
#include "../packet.hpp"
#include "../const/account.hpp"

void Account_Reply(PacketReader reader)
{
    //S &s = S::GetInstance();

    AccountReply reply = static_cast<AccountReply>(reader.GetShort());
    std::string reply_str = reader.GetEndString();

    if(reply_str == "OK")
    {
        if(reply == AccountReply::Continue)
        {
            puts("OK: Account name approved");

        }
        else if(reply == AccountReply::Created)
        {
            puts("OK: Account created");
        }
    }
    else if(reply_str == "NO")
    {
        std::string title =  "Could not create account";
        std::string message = "The reason is unknown.";

        if(reply == AccountReply::Exists)
        {
            puts("NO: Such account already exists");
            message = "Account with given name already exists.";
        }
        else if(reply == AccountReply::NotApproved)
        {
            puts("NO: account not approved");
            message = "Account has not been approved.";
        }
    }
}
