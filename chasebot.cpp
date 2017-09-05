#include "chasebot.hpp"
#include "singleton.hpp"

ChaseBot::ChaseBot()
{
    this->Reset();
}

void ChaseBot::Reset()
{
    this->victim_gameworld_id = -1;
    this->walk_clock.restart();
    this->follow_clock.restart();
    this->center_x = 38;
    this->center_y = 40;
    this->go_center = false;
}

void ChaseBot::Process()
{
    this->Act();
}

bool Walkable(unsigned char x, unsigned char y)
{
    S &s = S::GetInstance();

    bool warp_near = false;

    if(s.emf->GetWarp(x, y - 1).map != 0 || s.emf->GetWarp(x + 1, y).map != 0
       || s.emf->GetWarp(x, y + 1).map != 0 || s.emf->GetWarp(x - 1, y).map != 0
       || s.emf->GetWarp(x, y).map != 0)
    {
        warp_near = true;
    }

    if(s.emf->Walkable(x, y) && !s.map.Occupied(x, y) && !warp_near)
    {
        return true;
    }

    return false;
}

bool ChaseBot::Walk(Direction direction)
{
    S &s = S::GetInstance();

    if(s.eprocessor.BlockingEvent()) return false;

    int xoff = 0;
    int yoff = 0;

    switch(direction)
    {
    case Direction::Up:
        yoff--;
        break;

    case Direction::Right:
        xoff++;
        break;

    case Direction::Down:
        yoff++;
        break;

    case Direction::Left:
        xoff--;
        break;
    }

    if(xoff != 0 || yoff != 0)
    {
        if(Walkable(s.character.x + xoff, s.character.y + yoff) && this->walk_clock.getElapsedTime().asMilliseconds() >= 460)
        {
            s.eoclient.Walk(direction);

            s.character.x += xoff;
            s.character.y += yoff;

            this->walk_clock.restart();

            return true;
        }
    }

    return false;
}

void ChaseBot::WalkTo(unsigned char x, unsigned char y)
{
    S &s = S::GetInstance();

    int xdiff = s.character.x - x;
    int ydiff = s.character.y - y;
    int absxdiff = std::abs(xdiff);
    int absydiff = std::abs(ydiff);
    Direction direction;

    if ((absxdiff == 1 && absydiff == 0) || (absxdiff == 0 && absydiff == 1) || (absxdiff == 0 && absydiff == 0))
    {
        return;
    }
    else if (absxdiff > absydiff)
    {
        if (xdiff < 0)
        {
            direction = Direction::Right;
        }
        else
        {
            direction = Direction::Left;
        }
    }
    else
    {
        if (ydiff < 0)
        {
            direction = Direction::Down;
        }
        else
        {
            direction = Direction::Up;
        }
    }

    if(!this->Walk(direction))
    {
        if (direction == Direction::Up || direction == Direction::Down)
        {
            if (xdiff < 0)
            {
                direction = Direction::Right;
            }
            else
            {
                direction = Direction::Left;
            }
        }

        if(!this->Walk(direction))
        {
            this->Walk(static_cast<Direction>(s.rand_gen.RandInt(0,3)));
        }
    }
}

void ChaseBot::Act()
{
    S &s = S::GetInstance();

    int distance_center = path_length(this->center_x, this->center_y, s.character.x, s.character.y);

    if(this->victim_gameworld_id == -1 && !this->go_center)
	{
	    short closest = -1;
		unsigned char closest_distance = 1;

		for(unsigned int i = 0; i < s.map.characters.size(); ++i)
		{
			int distance = path_length(s.map.characters[i].x, s.map.characters[i].y, s.character.x, s.character.y);

			if (distance == 0)
				distance = 1;

			if (distance < closest_distance && s.map.characters[i].gameworld_id != s.character.gameworld_id)
			{
				closest = s.map.characters[i].gameworld_id;
				closest_distance = distance;
			}
		}

		if(closest != -1)
		{
		    this->victim_gameworld_id = closest;
            this->follow_clock.restart();
		}
	}

	if(this->victim_gameworld_id != -1)
    {
        if(!this->go_center && this->follow_clock.getElapsedTime().asSeconds() < 600)
        {
            int victim = s.map.GetCharacterIndex(this->victim_gameworld_id);
            if(victim == -1)
            {
                this->victim_gameworld_id = -1;
                return;
            }

            int distance = path_length(s.map.characters[victim].x, s.map.characters[victim].y, s.character.x, s.character.y);
            //int char_distance_center = path_length(this->center_x, this->center_y, s.map.characters[victim].x, s.map.characters[victim].y);

            if(distance > 2)
            {
                this->WalkTo(s.map.characters[victim].x, s.map.characters[victim].y);
            }
        }
        else if(!this->go_center && this->follow_clock.getElapsedTime().asSeconds() >= 600)
        {
            this->go_center = true;
            this->victim_gameworld_id = -1;
        }
    }
    else
    {
        if(distance_center >= 6 && !this->go_center)
        {
            this->go_center = true;
        }
    }

    if(this->go_center)
    {
        if(distance_center >= 6)
        {
            this->WalkTo(this->center_x, this->center_y);
        }
        else
        {
            this->go_center = false;
        }
    }
}
