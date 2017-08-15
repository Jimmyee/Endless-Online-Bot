// Endless Online Bot v0.0.1

#include "handlers.hpp"
#include "../singleton.hpp"
#include "../util.hpp"

void Walk_Player(PacketReader reader)
{
    S &s = S::GetInstance();

    short gameworld_id = reader.GetShort();

    int i = s.map.GetCharacterIndex(gameworld_id);
    if(i == -1) return;

    s.map.characters[i].direction = static_cast<Direction>(reader.GetChar());
    s.map.characters[i].x = reader.GetChar();
    s.map.characters[i].y = reader.GetChar();

    //int distance = path_length(s.character.x, s.character.y, s.map.characters[i].x, s.map.characters[i].y);
    int distance_center = path_length(s.eprocessor.chase_bot.center_x, s.eprocessor.chase_bot.center_y, s.character.x, s.character.y);

    if(!s.eprocessor.chase_bot.go_center &&
       (s.eprocessor.chase_bot.follow_clock.getElapsedTime().asSeconds() >= 500 || s.eprocessor.chase_bot.victim_gameworld_id == -1)
       && distance_center < 6)
    {
        s.eprocessor.chase_bot.victim_gameworld_id = gameworld_id;
        s.eprocessor.chase_bot.follow_clock.restart();
    }
}
