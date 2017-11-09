// Endless Online Bot v0.0.1

#include "handlers.hpp"
#include "../singleton.hpp"

// bot drops an item
void Item_Drop(PacketReader reader)
{
    reader.GetShort(); // id
    reader.GetThree(); // amount
    reader.GetInt(); // remaining amount
    /*short uid = */reader.GetShort();
}

