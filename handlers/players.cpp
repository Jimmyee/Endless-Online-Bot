// Endless Online Bot v0.0.1

#include "handlers.hpp"
#include "../singleton.hpp"

void Players_Agree(PacketReader reader)
{
    S &s = S::GetInstance();

    reader.GetByte(); // 255
    Character character;
    character.name = reader.GetBreakString();
    character.gameworld_id = reader.GetShort();
    reader.GetShort(); // map id
    character.x = reader.GetShort();
    character.y = reader.GetShort();
    character.direction = static_cast<Direction>(reader.GetChar());
    reader.GetChar(); // 6 (?)
    character.guild_tag = reader.GetFixedString(3);
    character.level = reader.GetChar();
    character.gender = static_cast<Gender>(reader.GetChar());
    character.hair_style = reader.GetChar();
    character.hair_color = reader.GetChar();
    character.race = static_cast<Skin>(reader.GetChar());
    character.max_hp = reader.GetShort();
    character.hp = reader.GetShort();
    character.max_tp = reader.GetShort();
    character.tp = reader.GetShort();

    // equipment
    for(int i = 0; i < 9; ++i)
    {
        reader.GetShort();
    }

    character.sitting = static_cast<SitState>(reader.GetChar());
    character.visibility = reader.GetChar();
    reader.GetByte(); // 255
    reader.GetByte(); // 0 = NPC, 1 = player

    s.map.characters.push_back(character);

    for(unsigned int i = 0; i < s.eprocessor.players_known.size(); ++i)
    {
        if(s.eprocessor.players_known[i] == character.gameworld_id)
        {
            return;
        }
    }

    std::string name = character.name;
    name[0] = std::toupper(character.name[0]);
    /*std::string message = std::string() + "Hi " + name + " [" + character.guild_tag + "]. L" + std::to_string(character.level);
    message += ", HP " + std::to_string(character.hp) + ", TP " + std::to_string(character.tp) + ".";
    if(character.hp <= 60) message += " Easy to kill.";*/

    std::string message = std::string() + "Hi " + name + ".";

    s.eprocessor.DelayedMessage(message);

    s.eprocessor.players_known.push_back(character.gameworld_id);
}
